idf_component_register(SRCS "main.c" "board.c" "storage.c" "../core/pocketdd.c" "../core/packet.c" "../core/dice_animation.c" "../core/dice_pool.c" "../core/avatar.c" "../core/checks.c" "../core/combat.c" "../core/magic.c" "../core/buttons.c" "../core/rest.c" "../core/status.c" "../core/screen.c"
    INCLUDE_DIRS "." "../core"
    REQUIRES lvgl esp_lcd_co5300 esp_lcd_touch_cst9217 esp_lcd_touch esp_driver_gpio
    esp_driver_spi esp_driver_i2c esp_driver_usb_serial_jtag nvs_flash json esp_timer bootloader_support)
