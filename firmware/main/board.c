/* Adapted display/touch initialization from Waveshare ESP32-S3-Touch-AMOLED-1.75
 * BSP at e4344e70c2fa78a13e8a06566507f1ba8af6672a (Apache-2.0).
 * PocketD-D changes: direct LVGL 9 partial DMA buffers, one owner task, no audio,
 * no SD formatting, no PMIC charge/voltage writes. See THIRD_PARTY.md. */
#include "board.h"
#include "esp_lcd_co5300.h"
#include "esp_lcd_touch_cst9217.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"
#include "driver/i2c_master.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "bootloader_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define RES 466
#define LINES 40
static esp_lcd_panel_handle_t panel;
static esp_lcd_touch_handle_t touch;
static SemaphoreHandle_t flushed;
static int64_t last_tick;
static i2c_master_dev_handle_t expander;
static bool pwr_ready;
static uint8_t pwr_polarity;
static int pwr_level=-1;
static int64_t buttons_read_at=-100;
static esp_err_t expander_read(uint8_t reg,uint8_t *value) {return i2c_master_transmit_receive(expander,&reg,1,value,1,5);}

static pd_screen *screen_control;
static bool transfer_pending;
static const co5300_lcd_init_cmd_t commands[]={
    {0xFE,(uint8_t[]){0x20},1,0}, {0x19,(uint8_t[]){0x10},1,0}, {0x1C,(uint8_t[]){0xA0},1,0},
    {0xFE,(uint8_t[]){0x00},1,0}, {0xC4,(uint8_t[]){0x80},1,0}, {0x3A,(uint8_t[]){0x55},1,0},
    {0x35,(uint8_t[]){0x00},1,0}, {0x53,(uint8_t[]){0x20},1,0}, {0x51,(uint8_t[]){0x80},1,0},
    {0x63,(uint8_t[]){0xFF},1,0}, {0x2A,(uint8_t[]){0x00,0x06,0x01,0xD7},4,0},
    {0x2B,(uint8_t[]){0x00,0x00,0x01,0xD1},4,600}, {0x11,NULL,0,600}, {0x29,NULL,0,0}
};
static bool color_done(esp_lcd_panel_io_handle_t io,esp_lcd_panel_io_event_data_t *event,void *ctx) {
    BaseType_t wake=pdFALSE; xSemaphoreGiveFromISR(flushed,&wake); return wake==pdTRUE;
}
static void flush(lv_display_t *d,const lv_area_t *a,uint8_t *pixels) {
    lv_draw_sw_rgb565_swap(pixels,lv_area_get_size(a));
    transfer_pending=true;
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel,a->x1,a->y1,a->x2+1,a->y2+1,pixels));
}
static void flush_wait(lv_display_t *d) {
    if (transfer_pending) { xSemaphoreTake(flushed,portMAX_DELAY); transfer_pending=false; }
}
static void round_area(lv_event_t *e) {
    lv_area_t *a=lv_event_get_param(e);
    a->x1 &= ~1; a->y1 &= ~1; a->x2 |= 1; a->y2 |= 1;
}
static void read_touch(lv_indev_t *indev,lv_indev_data_t *data) {
    esp_lcd_touch_point_data_t point={0}; uint8_t count=0;
    data->state=LV_INDEV_STATE_RELEASED;
    if (esp_lcd_touch_read_data(touch)!=ESP_OK){if(screen_control)pd_screen_touch(screen_control,false,false,lv_tick_get());return;}
    bool valid=esp_lcd_touch_get_data(touch,&point,&count,1)==ESP_OK;
    bool pressed=valid&&count;
    bool allow=screen_control?pd_screen_touch(screen_control,valid,pressed,lv_tick_get()):pressed;
    if(allow&&point.x<RES&&point.y<RES){data->state=LV_INDEV_STATE_PRESSED;data->point.x=point.x;data->point.y=point.y;}

}
void pd_board_init(void) {
    /* RF stays off. SAR-ADC entropy source remains enabled for esp_random().
       Disable it before any future ADC/PMIC sensing feature that conflicts. */
    bootloader_random_enable();
    flushed=xSemaphoreCreateBinary(); assert(flushed);
    spi_bus_config_t bus=CO5300_PANEL_BUS_QSPI_CONFIG(38,4,5,6,7,RES*LINES*2);
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST,&bus,SPI_DMA_CH_AUTO));
    esp_lcd_panel_io_spi_config_t io_config=CO5300_PANEL_IO_QSPI_CONFIG(12,color_done,NULL);
    io_config.trans_queue_depth=1;
    esp_lcd_panel_io_handle_t io;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST,&io_config,&io));
    co5300_vendor_config_t vendor={.init_cmds=commands,.init_cmds_size=sizeof commands/sizeof commands[0],.flags.use_qspi_interface=1};
    esp_lcd_panel_dev_config_t cfg={.reset_gpio_num=39,.rgb_ele_order=LCD_RGB_ELEMENT_ORDER_RGB,.bits_per_pixel=16,.vendor_config=&vendor};
    ESP_ERROR_CHECK(esp_lcd_new_panel_co5300(io,&cfg,&panel));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel,6,0));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel)); ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel,true));
    i2c_master_bus_handle_t i2c;
    i2c_master_bus_config_t i2c_cfg={.i2c_port=1,.sda_io_num=15,.scl_io_num=14,.clk_source=I2C_CLK_SRC_DEFAULT,.flags.enable_internal_pullup=1};
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_cfg,&i2c));
    esp_lcd_panel_io_i2c_config_t touch_io=ESP_LCD_TOUCH_IO_I2C_CST9217_CONFIG();
    touch_io.scl_speed_hz=400000;
    esp_lcd_panel_io_handle_t tp_io;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c,&touch_io,&tp_io));
    esp_lcd_touch_config_t tp={.x_max=RES,.y_max=RES,.rst_gpio_num=40,.int_gpio_num=11,
        .levels={.reset=0,.interrupt=0},.flags={.mirror_x=1,.mirror_y=1}};
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst9217(tp_io,&tp,&touch));
    /* Only P4 direction may change. Preserve every other expander bit and output.
       PWR SYS_OUT is active high; do not touch AXP2101 charging/power registers. */
    i2c_device_config_t ex={.dev_addr_length=I2C_ADDR_BIT_LEN_7,.device_address=0x20,.scl_speed_hz=400000};
    if(i2c_master_bus_add_device(i2c,&ex,&expander)==ESP_OK) {
        uint8_t direction=0;
        if(expander_read(3,&direction)==ESP_OK&&expander_read(2,&pwr_polarity)==ESP_OK) {
            uint8_t config[]={3,direction|0x10};
            pwr_ready=(direction&0x10)||i2c_master_transmit(expander,config,sizeof config,5)==ESP_OK;
        }
    }
    gpio_config_t key={.pin_bit_mask=UINT64_C(1)<<0,.mode=GPIO_MODE_INPUT,.pull_up_en=GPIO_PULLUP_DISABLE,.pull_down_en=GPIO_PULLDOWN_DISABLE,.intr_type=GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&key)); /* GPIO0 already has the board pull-up. */
    lv_init();
    lv_display_t *display=lv_display_create(RES,RES);
    uint8_t *buffer=heap_caps_malloc(RES*LINES*2,MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL);
    assert(display && buffer);
    lv_display_set_color_format(display,LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display,buffer,NULL,RES*LINES*2,LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display,flush); lv_display_set_flush_wait_cb(display,flush_wait);
    lv_display_add_event_cb(display,round_area,LV_EVENT_INVALIDATE_AREA,NULL);
    lv_indev_t *input=lv_indev_create(); assert(input);
    lv_indev_set_type(input,LV_INDEV_TYPE_POINTER); lv_indev_set_read_cb(input,read_touch);
    last_tick=esp_timer_get_time()/1000;
}
void pd_board_tick(void) {
    int64_t now=esp_timer_get_time()/1000;
    lv_tick_inc((uint32_t)(now-last_tick)); last_tick=now;
    lv_timer_handler();
}
uint32_t pd_board_random(void *unused) { return esp_random(); }

void pd_board_buttons(int levels[2]) {
    int64_t now=esp_timer_get_time()/1000;
    if(now-buttons_read_at>=20) {
        buttons_read_at=now;uint8_t raw;
        pwr_level=pwr_ready&&expander_read(0,&raw)==ESP_OK?((raw^pwr_polarity)&0x10?1:0):-1;
    }
    levels[0]=pwr_level;levels[1]=gpio_get_level(GPIO_NUM_0)==0;
}

void pd_board_screen(pd_screen *controller){screen_control=controller;}
bool pd_board_brightness(unsigned percent,void *unused){
    (void)unused;if(percent<10||percent>100)return false;
    /* Drain the outstanding DMA transfer before sending a control command. */
    flush_wait(NULL);
    return esp_lcd_panel_co5300_set_brightness(panel,percent)==ESP_OK;
}
