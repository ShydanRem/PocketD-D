#include "pocketdd.h"
#include "board.h"
#include "storage.h"
#include "dice_animation.h"
#ifndef PD_UI_TEST
#include "driver/usb_serial_jtag.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

typedef enum { HERO, SHEET, DICE, RESOURCES } page_t;
static pd_state state,previous,pending;
static bool undo_available,import_pending,has_roll;
static pd_roll result;
static pd_dice_animation dice_animation;
static lv_obj_t *dice_canvas;
static uint16_t *dice_pixels;
static lv_timer_t *dice_timer;
static void dice_frame(lv_timer_t *timer);
static page_t page;
static int ability=-1,mode,resource_index;
static bool pool_view,dice_setup,die_picker,pool_damage;
static unsigned pool_sides=6,pool_count=1;
static int pool_bonus;
static const char *const abilities[]={"FOR","DES","COS","INT","SAG","CAR"};
#ifndef PD_UI_TEST
static QueueHandle_t packets;
static TickType_t import_started;
static char line[PD_PACKET_MAX+1];
#endif
static void render(void);
#ifndef PD_UI_TEST
static void reply(const char *text) { usb_serial_jtag_write_bytes(text,strlen(text),pdMS_TO_TICKS(100)); }
#else
static void reply(const char *text) { (void)text; }
#endif
static void apply(pd_state *next) {
    if (pd_storage_save(next)) { previous=state; undo_available=true; state=*next; }
}
static lv_obj_t *label(const char *text,int x,int y,int width,bool large) {
    lv_obj_t *l=lv_label_create(lv_screen_active());
    lv_obj_set_pos(l,x,y); lv_obj_set_width(l,width);
    lv_label_set_long_mode(l,LV_LABEL_LONG_DOT); lv_label_set_text(l,text);
    lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_style_text_color(l,lv_color_hex(0xebe7d4),0);
    if (large) lv_obj_set_style_text_font(l,&lv_font_montserrat_26,0);
    return l;
}
static void event(lv_event_t *e) {
    intptr_t action=(intptr_t)lv_event_get_user_data(e);
    /* Disabled controls also reject queued/synthetic events while dice are in flight. */
    if (dice_animation.active && action!=13 && action!=16) return;
    pd_state next=state;
    if (action>=100 && action<104) { page=action-100;dice_setup=false;die_picker=false; }
    else if (action>=200 && action<206) { ability=action-200;page=DICE;has_roll=false;mode=0;pool_view=false;dice_setup=false;die_picker=false;pd_dice_idle(&dice_animation,1); }
    else if (action==1) { pd_hp(&next,-1);apply(&next); }
    else if (action==2) { pd_hp(&next,1);apply(&next); }
    else if (action==3 && undo_available) {
        if (pd_storage_save(&previous)) { state=previous; undo_available=false; }
    }
    else if (action>=10 && action<=12) { mode=action-11;has_roll=false;pd_dice_idle(&dice_animation,mode?2:1); }
    else if (action==15) { mode=mode==0?1:mode==1?-1:0;has_roll=false;pd_dice_idle(&dice_animation,mode?2:1); }
    else if (action==13) {
        if (dice_animation.active) { pd_dice_finish(&dice_animation);has_roll=true; }
        else {
            if(pool_view) {
                pd_pool next_roll;
                pd_pool *last=&dice_animation.pool;
                if(!pd_roll_pool(last->sides,last->count,last->bonus,last->damage,pd_board_random,NULL,&next_roll))return;
                pd_dice_pool_start(&dice_animation,&next_roll,lv_tick_get());
            } else {
                result=pd_d20(ability<0?0:pd_modifier(state.abilities[ability]),mode,pd_board_random,NULL);
                pd_dice_start(&dice_animation,&result,lv_tick_get());
            }
            has_roll=false;
        }
    }
    else if (action==16) {
        if(!dice_animation.active)return;
        pd_dice_finish(&dice_animation);has_roll=true;
    }
    else if (action==14) { ability=-1;has_roll=false;mode=0;pool_view=false;dice_setup=false;die_picker=false;pd_dice_idle(&dice_animation,1); }
    else if (action==40) {
        dice_setup=true;die_picker=false;
        if(pool_view) { pool_sides=dice_animation.pool.sides;pool_count=dice_animation.pool.count;pool_bonus=dice_animation.pool.bonus;pool_damage=dice_animation.pool.damage; }
    }
    else if (action==41) { if(die_picker)die_picker=false;else dice_setup=false; }
    else if (action==42)die_picker=true;
    else if (action==43)pool_damage=!pool_damage;
    else if (action==44 && pool_count>1)pool_count--;
    else if (action==45 && pool_count<PD_POOL_MAX)pool_count++;
    else if (action==47 && pool_bonus>PD_POOL_MIN_BONUS)pool_bonus--;
    else if (action==48 && pool_bonus<PD_POOL_MAX_BONUS)pool_bonus++;
    else if (action==46) {
        pd_pool next_roll;
        if(!pd_roll_pool(pool_sides,pool_count,pool_bonus,pool_damage,pd_board_random,NULL,&next_roll))return;
        pool_view=true;dice_setup=false;die_picker=false;has_roll=false;
        pd_dice_pool_start(&dice_animation,&next_roll,lv_tick_get());
    }
    else if(action>=404 && action<=420 && pd_pool_sides(action-400) && die_picker) { pool_sides=action-400;die_picker=false; }
    else if (action==20 || action==21) { if(pd_resource_change(&next,resource_index,action==20?-1:1))apply(&next); }
    else if (action==22 && state.resource_count) resource_index=(resource_index+1)%state.resource_count;
    else if (action==30) {
        if (pd_storage_save(&pending)) {
            state=pending;undo_available=false;resource_index=0;has_roll=false;ability=-1;mode=0;page=HERO;pool_view=false;dice_setup=false;die_picker=false;pd_dice_idle(&dice_animation,1);
            reply("PDD IMPORTED\n"); import_pending=false;
        } else { reply("PDD ERROR storage\n");import_pending=false; }
    }
    else if (action==31) { import_pending=false;reply("PDD CANCELLED\n"); }
    render();
}
static lv_obj_t *button(const char *text,int x,int y,int w,int h,intptr_t action,bool enabled) {
    lv_obj_t *b=lv_button_create(lv_screen_active());lv_obj_set_pos(b,x,y);lv_obj_set_size(b,w,h);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x29463d),0);lv_obj_set_style_radius(b,12,0);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x1b2b25),LV_STATE_DISABLED);
    lv_obj_set_style_text_color(b,lv_color_hex(0x6e8179),LV_STATE_DISABLED);
    lv_obj_set_style_shadow_width(b,0,0);lv_obj_set_style_pad_all(b,4,0);
    lv_obj_t *l=lv_label_create(b);lv_label_set_text(l,text);lv_obj_center(l);
    if (!enabled) lv_obj_add_state(b,LV_STATE_DISABLED);
    lv_obj_add_event_cb(b,event,LV_EVENT_CLICKED,(void*)action);return b;
}
static void render_dice_setup(void) {
    char text[80];
    if(die_picker) {
        label("Scegli il dado",83,105,300,true);
        const unsigned sides[]={4,6,8,10,12,20};
        for(int i=0;i<6;i++) {
            snprintf(text,sizeof text,"d%u",sides[i]);
            lv_obj_t *b=button(text,83+(i%3)*104,158+(i/3)*72,92,56,400+sides[i],true);
            if(pool_sides==sides[i])lv_obj_set_style_bg_color(b,lv_color_hex(0x75562b),0);
        }
        button("Indietro",83,308,300,48,41,true);return;
    }
    label("Prepara il tiro",83,105,300,true);
    snprintf(text,sizeof text,"Tipo: d%u",pool_sides);button(text,83,148,144,48,42,true);
    button(pool_damage?"Danni":"Somma",239,148,144,48,43,true);
    button("-",83,210,48,48,44,pool_count>1);
    snprintf(text,sizeof text,"Quantita %u",pool_count);label(text,139,223,188,false);
    button("+",335,210,48,48,45,pool_count<PD_POOL_MAX);
    button("-",83,272,48,48,47,pool_bonus>PD_POOL_MIN_BONUS);
    snprintf(text,sizeof text,"Bonus %+d",pool_bonus);label(text,139,285,188,false);
    button("+",335,272,48,48,48,pool_bonus<PD_POOL_MAX_BONUS);
    button("Esci",83,332,76,48,41,true);button("Lancia",167,332,132,48,46,true);button("Prova",307,332,76,48,14,true);
}
static void render(void) {
    /* Destroy the LVGL image/cache before freeing its pixel buffer. A root redraw
       cancels the old timer; the roll itself remains in the pure presentation state. */
    if(dice_timer) { lv_timer_delete(dice_timer);dice_timer=NULL; }
    lv_obj_t *root=lv_screen_active();lv_obj_clean(root);
    lv_free(dice_pixels);dice_pixels=NULL;dice_canvas=NULL;
    lv_obj_set_style_bg_color(root,lv_color_hex(0x0d1916),0);
    lv_obj_remove_flag(root,LV_OBJ_FLAG_SCROLLABLE);
    char text[160];
    label(pd_storage_status(),78,73,310,false);
    if (import_pending) {
        label("COPIA DA USB",83,112,300,true);
        label(pending.name,83,151,300,true);
        snprintf(text,sizeof text,"Liv %u  PF %u/%u  CA %u",pending.level,pending.hp,pending.max_hp,pending.armor);
        label(text,83,190,300,false);
        label("Sostituisce la copia locale.",83,229,300,false);
        label("PC: raccolta conservata.",83,257,300,false);
        button("Annulla",83,303,144,54,31,true); button("Conferma",239,303,144,54,30,true);
        return;
    }
    if(page==DICE && dice_setup) { render_dice_setup();return; }
    if (page==HERO) {
        label(state.name,83,111,300,true);
        snprintf(text,sizeof text,"Livello %u     CA %u",state.level,state.armor);label(text,83,151,300,false);
        snprintf(text,sizeof text,"PF %u / %u   +%u temp",state.hp,state.max_hp,state.temp_hp);label(text,83,190,300,true);
        button("-1 PF",83,230,144,50,1,state.hp||state.temp_hp);button("+1 PF",239,230,144,50,2,state.hp<state.max_hp);
        button("Annulla modifica",83,290,300,48,3,undo_available);
    } else if (page==SHEET) {
        label("Caratteristiche",83,111,300,true);
        for(int i=0;i<6;i++) {
            snprintf(text,sizeof text,"%s %u\n%+d",abilities[i],state.abilities[i],pd_modifier(state.abilities[i]));
            button(text,83+(i%3)*104,154+(i/3)*73,92,64,200+i,true);
        }
        label("Prove base: nessun effetto",83,306,300,false);
    } else if (page==DICE) {
        if(pool_view)snprintf(text,sizeof text,"%s %ud%u %+d",dice_animation.pool.damage?"Danni":"Somma",dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus);
        else snprintf(text,sizeof text,"d20 %s %+d",ability<0?"libero":abilities[ability],ability<0?0:pd_modifier(state.abilities[ability]));
        label(text,83,105,300,true);
        if(!pd_dice_count(&dice_animation))pd_dice_idle(&dice_animation,mode?2:1);
        dice_pixels=lv_malloc(PD_DICE_PIXELS*sizeof *dice_pixels);
        if(dice_pixels) {
            dice_canvas=lv_canvas_create(root);lv_obj_set_pos(dice_canvas,73,134);
            lv_canvas_set_buffer(dice_canvas,dice_pixels,PD_DICE_WIDTH,PD_DICE_HEIGHT,LV_COLOR_FORMAT_RGB565);
            lv_obj_add_flag(dice_canvas,LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(dice_canvas,event,LV_EVENT_CLICKED,(void*)(intptr_t)16);
            pd_dice_paint(&dice_animation,dice_pixels);
            if(dice_animation.active)dice_timer=lv_timer_create(dice_frame,33,NULL);
        } else if(dice_animation.active) {
            /* Low memory must never hide or re-roll an outcome that already exists. */
            pd_dice_finish(&dice_animation);has_roll=true;
        }
        if(has_roll) {
            if(pool_view) {
                pd_pool *r=&dice_animation.pool;
                if(r->damage && r->sum+r->bonus<0)snprintf(text,sizeof text,"Somma %d -> Danni 0",r->sum+r->bonus);
                else snprintf(text,sizeof text,"%d %+d = %d",r->sum,r->bonus,r->total);
            }
            else if(result.count==2)snprintf(text,sizeof text,"%u / %u %+d = %d",result.dice[0],result.dice[1],result.bonus,result.total);
            else snprintf(text,sizeof text,"%u %+d = %d%s",result.dice[0],result.bonus,result.total,result.natural==20?"  -  20 naturale":"");
        } else snprintf(text,sizeof text,"%s",dice_animation.active?"In volo...":"Pronto al lancio");
        label(text,73,265,320,false);
        if(!dice_pixels)label("Animazione non disponibile",73,183,320,false);
        button(pool_view?"Prova":mode==-1?"Svant":mode==1?"Vant":"Base",83,296,76,44,pool_view?14:15,!dice_animation.active);
        button(dice_animation.active?"Accelera":has_roll?"Rilancia":"Lancia",167,296,132,44,13,true);
        button("Altri",307,296,76,44,40,!dice_animation.active);
    } else {
        label("Contatori manuali",83,111,300,true);
        if(state.resource_count) {
            pd_resource *r=&state.resources[resource_index];label(r->name,83,158,300,false);
            snprintf(text,sizeof text,"%u / %u",r->current,r->maximum);label(text,83,194,300,true);
            button("-1",83,235,144,50,20,r->current>0);button("+1",239,235,144,50,21,r->current<r->maximum);
            snprintf(text,sizeof text,"Prossima  %d/%u",resource_index+1,state.resource_count);
            button(text,83,294,300,44,22,state.resource_count>1);
        } else label("Nessun contatore importato",83,194,300,false);
    }
    const char *nav[]={"Eroe","Scheda","Dadi","Usi"};
    const int nav_x[]={79,153,251,325},nav_w[]={68,92,68,64};
    for (int i=0;i<4;i++) {
        lv_obj_t *b=button(nav[i],nav_x[i],350,nav_w[i],48,100+i,!dice_animation.active);
        if (page==i)lv_obj_set_style_bg_color(b,lv_color_hex(0x75562b),0);
    }
}
static void dice_frame(lv_timer_t *timer) {
    (void)timer;
    if(pd_dice_tick(&dice_animation,lv_tick_get())) {
        has_roll=true;render();return;
    }
    pd_dice_paint(&dice_animation,dice_pixels);
    lv_obj_invalidate(dice_canvas);
}
#ifndef PD_UI_TEST
static void usb_task(void *unused) {
    size_t used=0;bool overflow=false;uint8_t data[64];
    for (;;) {
        int count=usb_serial_jtag_read_bytes(data,sizeof data,pdMS_TO_TICKS(100));
        for(int i=0;i<count;i++) {
            char c=data[i];
            if(c=='\r')continue;
            if(c=='\n') {
                if(!overflow && used) {
                    line[used]=0;
                    if(!strcmp(line,"PDD HELLO")) reply("PDD READY 1\n");
                    else {
                        pd_state candidate;
                        if(pd_parse_packet(line,used,&candidate)) {
                            if(xQueueSend(packets,&candidate,0)!=pdPASS)reply("PDD ERROR busy\n");
                        } else reply("PDD ERROR packet\n");
                    }
                } else if(overflow) reply("PDD ERROR size\n");
                used=0;overflow=false;
            } else if (used<PD_PACKET_MAX && !overflow) line[used++]=c;
            else overflow=true;
        }
    }
}
void app_main(void) {
    pd_demo(&state);pd_storage_load(&state);
    pd_board_init();
    usb_serial_jtag_driver_config_t usb=USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    usb.rx_buffer_size=PD_PACKET_MAX+128;usb.tx_buffer_size=1024;
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb));
    packets=xQueueCreate(1,sizeof(pd_state));assert(packets);
    assert(xTaskCreate(usb_task,"usb_import",8192,NULL,3,NULL)==pdPASS);
    render();
    for(;;) {
        pd_state candidate;
        if(xQueueReceive(packets,&candidate,0)==pdPASS) {
            if(import_pending)reply("PDD ERROR busy\n");
            else { if(dice_animation.active) { pd_dice_finish(&dice_animation);has_roll=true; } pending=candidate;import_pending=true;import_started=xTaskGetTickCount();reply("PDD WAIT_CONFIRM\n");render(); }
        }
        if(import_pending && (xTaskGetTickCount()-import_started)>pdMS_TO_TICKS(120000)) {
            import_pending=false;reply("PDD CANCELLED timeout\n");render();
        }
        pd_board_tick();vTaskDelay(pdMS_TO_TICKS(5));
    }
}

#endif
