#include "pocketdd.h"
#include "board.h"
#include "storage.h"
#include "dice_animation.h"
#include "visual.h"
#include "avatar.h"
#include "checks.h"
#include "../assets/device_art.h"
#include "src/misc/cache/instance/lv_image_cache.h"
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

typedef enum { HERO, SHEET, DICE, RESOURCES, APPEARANCE, CHECK_DETAIL } page_t;
static pd_state state,previous,pending;
static bool undo_available,import_pending,has_roll;
static pd_roll result;
static pd_dice_animation dice_animation;
static lv_obj_t *dice_canvas;
static uint16_t *dice_pixels;
static lv_timer_t *dice_timer;
static lv_obj_t *avatar_sprite;
static lv_timer_t *avatar_timer;
static uint32_t avatar_started;
static unsigned avatar_phase;
static bool avatar_large;
_Alignas(4) static uint8_t avatar_data[PD_AVATAR_LARGE_BYTES];
static lv_image_dsc_t avatar_image={
    .header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_I4,
             .w=PD_AVATAR_WIDTH,.h=PD_AVATAR_HEIGHT,.stride=PD_AVATAR_WIDTH/2},
    .data_size=PD_AVATAR_IMAGE_BYTES,.data=avatar_data
};
static void avatar_frame(lv_timer_t *timer);
static void dice_frame(lv_timer_t *timer);
static page_t page;
static int ability=-1,mode,resource_index;
static pd_check_kind check_kind=PD_CHECK;
static bool ignore_effects;
static int sheet_tab,skill_page;
static bool pool_view,dice_setup,die_picker,pool_damage;
static unsigned pool_sides=6,pool_count=1;
static int pool_bonus;
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
static void label_font(lv_obj_t *l,const lv_font_t *font) {
    lv_obj_set_style_text_font(l,font,0);
    /* DOT needs a fixed height: a long imported name must not wrap over the stats. */
    lv_obj_set_height(l,lv_font_get_line_height(font));
}
static lv_obj_t *label(const char *text,int x,int y,int width,bool large) {
    lv_obj_t *l=lv_label_create(lv_screen_active());
    lv_obj_set_pos(l,x,y); lv_obj_set_width(l,width);
    lv_label_set_long_mode(l,LV_LABEL_LONG_DOT); lv_label_set_text(l,text);
    lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_style_text_color(l,lv_color_hex(PD_TEXT),0);
    label_font(l,large?&lv_font_montserrat_26:&lv_font_montserrat_20);
    return l;
}
static lv_obj_t *small_label(const char *text,int x,int y,int w,unsigned color) {
    lv_obj_t *l=label(text,x,y,w,false);
    label_font(l,&lv_font_montserrat_16);
    lv_obj_set_style_text_color(l,lv_color_hex(color),0);return l;
}
/* Decoration never participates in hit testing; only explicit buttons/canvas do. */
static lv_obj_t *decoration(int x,int y,int w,int h,unsigned color,int radius) {
    lv_obj_t *o=lv_obj_create(lv_screen_active());lv_obj_remove_style_all(o);
    lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
    lv_obj_set_style_bg_color(o,lv_color_hex(color),0);lv_obj_set_style_bg_opa(o,LV_OPA_COVER,0);
    lv_obj_set_style_radius(o,radius,0);lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    return o;
}
static void art(const lv_image_dsc_t *source,int x,int y,lv_obj_t *parent) {
    lv_obj_t *o=lv_image_create(parent);lv_image_set_src(o,source);lv_obj_set_pos(o,x,y);
    lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
}
static void avatar_show(const pd_avatar *appearance,int x,int y,lv_obj_t *parent,bool large,bool animate) {
    avatar_large=large;
    if(large)pd_avatar_large(appearance,0,avatar_data);else pd_avatar_image(appearance,0,avatar_data);
    avatar_image.header.w=PD_AVATAR_WIDTH*(large?2:1);avatar_image.header.h=PD_AVATAR_HEIGHT*(large?2:1);
    avatar_image.header.stride=avatar_image.header.w/2;
    avatar_image.data_size=large?PD_AVATAR_LARGE_BYTES:PD_AVATAR_IMAGE_BYTES;
    avatar_phase=0;avatar_started=lv_tick_get();
    avatar_sprite=lv_image_create(parent);lv_image_set_src(avatar_sprite,&avatar_image);
    lv_obj_set_pos(avatar_sprite,x,y);lv_obj_remove_flag(avatar_sprite,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    if(animate&&state.idle_animation)avatar_timer=lv_timer_create(avatar_frame,200,NULL);
}
static void meter(unsigned current,unsigned maximum,int y,unsigned color) {
    decoration(89,y,288,4,PD_EDGE,0);
    if(current&&maximum)decoration(89,y,(288*current+maximum-1)/maximum,4,color,0);
}
static void frame(void) {
    lv_obj_t *rim=decoration(5,5,456,456,PD_INK,LV_RADIUS_CIRCLE);
    lv_obj_set_style_bg_opa(rim,LV_OPA_TRANSP,0);lv_obj_set_style_border_width(rim,2,0);
    lv_obj_set_style_border_color(rim,lv_color_hex(0x1b2b40),0);
    /* Small pixel marks give the circular housing an identity without a busy HUD. */
    decoration(229,17,8,4,PD_GOLD,0);decoration(231,15,4,8,PD_GOLD,0);
    decoration(27,226,3,14,PD_EDGE,0);decoration(436,226,3,14,PD_EDGE,0);
    lv_obj_t *brand=small_label("POCKET D-D",113,37,240,PD_MINT);
    lv_obj_set_style_text_letter_space(brand,3,0);
    const char *status=pd_storage_status();
    unsigned c=strstr(status,"Errore")?PD_DANGER:strstr(status,"Recuperata")?PD_GOLD:PD_MUTED;
    small_label(status,78,72,310,c);
    decoration(203,422,60,2,PD_EDGE,0);
    decoration(227,422,12,2,PD_GOLD,0);
}
static void event(lv_event_t *e) {
    intptr_t action=(intptr_t)lv_event_get_user_data(e);
    /* Disabled controls also reject queued/synthetic events while dice are in flight. */
    if (dice_animation.active && action!=13 && action!=16) return;
    pd_state next=state;
    if (action>=100 && action<104) { page=action-100;dice_setup=false;die_picker=false; }
    else if ((action>=200&&action<206)||(action>=220&&action<238)||(action>=240&&action<246)) {
        pd_check_kind selected_kind=action>=240?PD_SAVE:action>=220?PD_SKILL:PD_CHECK;
        int index=action-(selected_kind==PD_SAVE?240:selected_kind==PD_SKILL?220:200);
        pd_check_info info;if(!pd_describe_check(&state,selected_kind,index,&info))return;
        check_kind=selected_kind;ability=index;
        page=DICE;has_roll=false;mode=0;ignore_effects=false;pool_view=false;dice_setup=false;die_picker=false;
        pd_dice_idle(&dice_animation,pd_check_mode(&state,check_kind,ability,mode,false)?2:1);
    }
    else if (action==1) { pd_hp(&next,-1);apply(&next); }
    else if (action==2) { pd_hp(&next,1);apply(&next); }
    else if (action==3 && undo_available) {
        if (pd_storage_save(&previous)) { state=previous; undo_available=false; }
    }
    else if (action>=10 && action<=12) { mode=action-11;has_roll=false;pd_dice_idle(&dice_animation,mode?2:1); }
    else if (action==15) { mode=mode==0?1:mode==1?-1:0;has_roll=false;pd_dice_idle(&dice_animation,mode?2:1); }
    else if (action==13||action==72) {
        if (dice_animation.active) { pd_dice_finish(&dice_animation);has_roll=true; }
        else {
            if(pool_view) {
                pd_pool next_roll;
                pd_pool *last=&dice_animation.pool;
                if(!pd_roll_pool(last->sides,last->count,last->bonus,last->damage,pd_board_random,NULL,&next_roll))return;
                pd_dice_pool_start(&dice_animation,&next_roll,lv_tick_get());
            } else {
                pd_check_info info={0};
                if(ability>=0&&(!pd_describe_check(&state,check_kind,ability,&info)||info.blocked))return;
                int effective=ability<0?mode:pd_check_mode(&state,check_kind,ability,mode,ignore_effects);
                result=pd_d20(info.total,effective,pd_board_random,NULL);
                pd_dice_start(&dice_animation,&result,lv_tick_get());
            }
            page=DICE;
            has_roll=false;
        }
    }
    else if (action==16) {
        if(!dice_animation.active)return;
        pd_dice_finish(&dice_animation);has_roll=true;
    }
    else if (action==14) { ability=-1;page=DICE;has_roll=false;mode=0;ignore_effects=false;pool_view=false;dice_setup=false;die_picker=false;pd_dice_idle(&dice_animation,1); }
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
    else if (action==50)page=APPEARANCE;
    else if (action==51) {
        next.idle_animation=!state.idle_animation;
        if(pd_storage_save(&next)) {
            state=next;previous.idle_animation=next.idle_animation;
        }
    }
    else if (action==52)page=HERO;
    else if(action>=60&&action<=62) { sheet_tab=action-60;skill_page=0; }
    else if(action==63&&skill_page>0)skill_page--;
    else if(action==64&&skill_page<5)skill_page++;
    else if(action==70&&ability>=0&&!pool_view)page=CHECK_DETAIL;
    else if(action==71)page=DICE;
    else if(action==73) { ignore_effects=!ignore_effects;has_roll=false;pd_dice_idle(&dice_animation,1); }
    render();
}
static lv_obj_t *button(const char *text,int x,int y,int w,int h,intptr_t action,bool enabled) {
    lv_obj_t *b=lv_button_create(lv_screen_active());lv_obj_set_pos(b,x,y);lv_obj_set_size(b,w,h);
    bool primary=action==13||action==46||action==30;
    lv_obj_set_style_bg_color(b,lv_color_hex(primary?PD_MINT:PD_PANEL),0);lv_obj_set_style_radius(b,6,0);
    lv_obj_set_style_border_width(b,1,0);lv_obj_set_style_border_color(b,lv_color_hex(primary?PD_MINT:PD_EDGE),0);
    lv_obj_set_style_text_color(b,lv_color_hex(primary?PD_INK:PD_TEXT),0);
    lv_obj_set_style_text_align(b,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x273d4f),LV_STATE_PRESSED);
    lv_obj_set_style_text_color(b,lv_color_hex(PD_TEXT),LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x101a28),LV_STATE_DISABLED);
    lv_obj_set_style_border_color(b,lv_color_hex(0x243448),LV_STATE_DISABLED);
    lv_obj_set_style_text_color(b,lv_color_hex(0x7b8a9c),LV_STATE_DISABLED);
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
            if(pool_sides==sides[i]) {
                lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);
                lv_obj_set_style_text_color(b,lv_color_hex(PD_GOLD),0);
            }
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
static void check_name(char *out,size_t size,bool bonus) {
    pd_check_info info={0};bool known=ability>=0&&pd_describe_check(&state,check_kind,ability,&info);
    const char *name=ability<0?"d20 libero":check_kind==PD_SKILL?pd_skill_names[ability]:pd_abilities[ability];
    if(bonus&&known&&!info.blocked)snprintf(out,size,"%s%s %+d >",check_kind==PD_SAVE?"Salv. ":check_kind==PD_CHECK?"d20 ":"",name,info.total);
    else snprintf(out,size,"%s%s%s",check_kind==PD_SAVE?"Salvezza ":"",name,known&&info.blocked?" ?":"");
}
static void render_check_detail(void) {
    char text[160];pd_check_info info={0};bool known=pd_describe_check(&state,check_kind,ability,&info);
    check_name(text,sizeof text,false);lv_obj_t *title=label(text,83,111,300,true);
    if(check_kind==PD_SKILL)label_font(title,&lv_font_montserrat_20);
    if(!known||info.blocked)snprintf(text,sizeof text,"Da verificare");else snprintf(text,sizeof text,"Bonus %+d",info.total);
    lv_obj_t *total=label(text,83,150,300,true);lv_obj_set_style_text_color(total,lv_color_hex(info.blocked?PD_GOLD:PD_MINT),0);
    snprintf(text,sizeof text,"%s %+d",pd_abilities[info.ability],info.base);label(text,83,194,300,false);
    snprintf(text,sizeof text,"%s %+d",info.half?"Mezza comp.":info.rank==2?"Maestria":info.rank==1?"Competenza":"Non competente",info.trained);
    label(text,83,224,300,false);snprintf(text,sizeof text,"Extra %+d",info.extra);label(text,83,254,300,false);
    int effective=pd_check_mode(&state,check_kind,ability,mode,ignore_effects);
    bool poison=state.training.poisoned&&check_kind!=PD_SAVE;
    bool strength=state.training.strength_advantage&&info.ability==0;
    const char *reason=ignore_effects?"Effetti PC esclusi":poison&&strength?"PC: avvelenato + Ira":poison?"PC: avvelenato":strength?"PC: Ira FOR":"Manuale";
    snprintf(text,sizeof text,"%s | %s",effective>0?"Vantaggio":effective<0?"Svantaggio":"Normale",reason);
    if(!state.training.available)snprintf(text,sizeof text,"Vecchia copia: solo caratteristica");
    small_label(text,83,282,300,PD_MUTED);
    button(mode<0?"Svant":mode>0?"Vant":"Base",83,307,144,44,15,true);
    lv_obj_t *effects=button(ignore_effects?"Effetti PC: no":"Effetti PC: si",239,307,144,44,73,state.training.available);
    lv_obj_set_style_text_font(effects,&lv_font_montserrat_16,0);
    button("Indietro",83,359,144,48,71,true);
    button(info.blocked||!known?"Libero":"Lancia",239,359,144,48,info.blocked||!known?14:72,true);
}
static void render_sheet(void) {
    char text[120];
    if(sheet_tab==1) {
        snprintf(text,sizeof text,"Abilita %d/6",skill_page+1);label(text,139,111,188,false);
        button("<",83,99,48,44,63,skill_page>0);button(">",335,99,48,44,64,skill_page<5);
    } else label(sheet_tab==2?"Tiri salvezza":"Caratteristiche",83,111,300,true);
    const char *tabs[]={"Caratt.","Abilita","Salvezze"};
    for(int i=0;i<3;i++) {
        lv_obj_t *b=button(tabs[i],83+i*104,149,92,44,60+i,true);
        lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        if(sheet_tab==i)lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);
    }
    if(sheet_tab&&!state.training.available) {
        small_label("Competenze non disponibili",83,221,300,PD_GOLD);
        small_label("Importa una copia aggiornata",83,251,300,PD_TEXT);
        small_label("La scheda attuale e conservata",83,281,300,PD_MUTED);return;
    }
    if(sheet_tab==1)for(int i=0;i<3;i++) {
        unsigned index=skill_page*3+i;pd_check_info info;
        pd_describe_check(&state,PD_SKILL,index,&info);
        if(info.blocked)snprintf(text,sizeof text,"%s  ?",pd_skill_names[index]);
        else snprintf(text,sizeof text,"%s  %+d",pd_skill_names[index],info.total);
        button(text,83,199+i*48,300,44,220+index,true);
    } else for(int i=0;i<6;i++) {
        pd_check_info info;pd_describe_check(&state,sheet_tab==2?PD_SAVE:PD_CHECK,i,&info);
        if(sheet_tab==2&&info.blocked)snprintf(text,sizeof text,"%s\n?",pd_abilities[i]);
        else if(sheet_tab==2)snprintf(text,sizeof text,"%s\n%+d",pd_abilities[i],info.total);
        else if(info.blocked)snprintf(text,sizeof text,"%s %u\n?",pd_abilities[i],state.abilities[i]);
        else snprintf(text,sizeof text,"%s %u\n%+d",pd_abilities[i],state.abilities[i],info.total);
        lv_obj_t *b=button(text,83+(i%3)*104,199+(i/3)*73,92,64,(sheet_tab==2?240:200)+i,true);
        lv_obj_set_style_text_color(b,lv_color_hex(info.blocked?PD_GOLD:PD_MINT),0);
    }
}
static void render(void) {
    /* Destroy the LVGL image/cache before freeing its pixel buffer. A root redraw
       cancels the old timer; the roll itself remains in the pure presentation state. */
    if(dice_timer) { lv_timer_delete(dice_timer);dice_timer=NULL; }
    if(avatar_timer) { lv_timer_delete(avatar_timer);avatar_timer=NULL; }
    lv_obj_t *root=lv_screen_active();lv_obj_clean(root);
    avatar_sprite=NULL;lv_image_cache_drop(&avatar_image);
    lv_free(dice_pixels);dice_pixels=NULL;dice_canvas=NULL;
    lv_obj_set_style_bg_color(root,lv_color_hex(PD_INK),0);
    lv_obj_remove_flag(root,LV_OBJ_FLAG_SCROLLABLE);
    char text[160];
    frame();
    if (import_pending) {
        label("COPIA DA USB",83,112,300,true);
        avatar_show(&pending.avatar,83,151,root,false,false);
        label(pending.name,139,151,244,true);
        snprintf(text,sizeof text,"Liv %u  PF %u/%u  CA %u",pending.level,pending.hp,pending.max_hp,pending.armor);
        small_label(text,139,190,244,PD_GOLD);
        label("Sostituisce la copia locale.",83,229,300,false);
        label("PC: raccolta conservata.",83,257,300,false);
        button("Annulla",83,303,144,54,31,true); button("Conferma",239,303,144,54,30,true);
        return;
    }
    if(page==DICE && dice_setup) { render_dice_setup();return; }
    if(page==CHECK_DETAIL) { render_check_detail();return; }
    if(page==APPEARANCE) {
        label("Aspetto",83,111,300,true);
        avatar_show(&state.avatar,185,143,root,true,true);
        snprintf(text,sizeof text,"Mantello %s / %s",(const char *const[]){"bosco","vino","notte"}[state.avatar.cloak],
                 (const char *const[]){"corti","lunghi","rasati"}[state.avatar.style]);
        small_label(text,83,268,300,PD_GOLD);
        button(state.idle_animation?"Animazione: si":"Animazione: no",83,299,300,48,51,true);
        button("Indietro",83,359,300,48,52,true);return;
    }
    if (page==HERO) {
        label(state.name,83,101,300,true);
        snprintf(text,sizeof text,"LIVELLO %02u   /   CA %02u",state.level,state.armor);
        lv_obj_t *meta=small_label(text,83,133,300,PD_GOLD);lv_obj_set_style_text_letter_space(meta,1,0);
        lv_obj_t *stage=button("Aspetto",83,153,300,72,50,true);
        lv_obj_set_style_pad_all(stage,0,0);lv_obj_set_style_border_width(stage,0,0);
        lv_obj_set_style_bg_opa(stage,LV_OPA_TRANSP,0);
        art(&pd_art_scene,0,0,stage);avatar_show(&state.avatar,126,0,stage,false,true);
        lv_obj_t *hint=lv_obj_get_child(stage,0);lv_obj_set_style_text_font(hint,&lv_font_montserrat_16,0);
        lv_obj_set_style_text_color(hint,lv_color_hex(PD_GOLD),0);
        lv_obj_set_style_bg_color(hint,lv_color_hex(PD_INK),0);lv_obj_set_style_bg_opa(hint,LV_OPA_COVER,0);
        lv_obj_align(hint,LV_ALIGN_BOTTOM_RIGHT,-2,0);lv_obj_move_foreground(hint);
        snprintf(text,sizeof text,"%u / %u",state.hp,state.max_hp);
        lv_obj_t *hp=label(text,133,225,200,true);label_font(hp,&lv_font_montserrat_32);
        lv_obj_set_style_text_color(hp,lv_color_hex(state.hp?PD_MINT:PD_DANGER),0);
        small_label("PF",83,236,46,PD_MUTED);
        snprintf(text,sizeof text,"+%u PF temporanei",state.temp_hp);small_label(text,83,263,300,PD_MUTED);
        meter(state.hp,state.max_hp,283,state.hp?PD_MINT:PD_DANGER);
        button("-1 PF",83,292,92,48,1,state.hp||state.temp_hp);
        lv_obj_t *undo=button("Annulla",187,292,92,48,3,undo_available);
        lv_obj_set_style_text_font(undo,&lv_font_montserrat_16,0);
        button("+1 PF",291,292,92,48,2,state.hp<state.max_hp);
    } else if (page==SHEET) {
        render_sheet();
    } else if (page==DICE) {
        if(pool_view)snprintf(text,sizeof text,"%s %ud%u %+d",dice_animation.pool.damage?"Danni":"Somma",dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus);
        else if(ability<0)snprintf(text,sizeof text,"d20 libero +0");else check_name(text,sizeof text,true);
        if(!pool_view&&ability>=0) {
            lv_obj_t *detail=button(text,83,94,300,44,70,!dice_animation.active);
            if(check_kind==PD_SKILL)lv_obj_set_style_text_font(detail,&lv_font_montserrat_20,0);
        } else label(text,83,105,300,true);
        pd_check_info selected={0};bool allowed=pool_view||ability<0||(pd_describe_check(&state,check_kind,ability,&selected)&&!selected.blocked);
        int effective=pool_view?0:ability<0?mode:pd_check_mode(&state,check_kind,ability,mode,ignore_effects);
        if(!pool_view&&!has_roll&&!dice_animation.active)pd_dice_idle(&dice_animation,effective?2:1);
        if(!pd_dice_count(&dice_animation))pd_dice_idle(&dice_animation,mode?2:1);
        dice_pixels=lv_malloc(PD_DICE_PIXELS*sizeof *dice_pixels);
        if(dice_pixels) {
            dice_canvas=lv_canvas_create(root);lv_obj_set_pos(dice_canvas,73,138);
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
            else if(result.count==2)snprintf(text,sizeof text,"%u / %u %+d = %d [%s]",result.dice[0],result.dice[1],result.bonus,result.total,effective>0?"Vant":"Svant");
            else snprintf(text,sizeof text,"%u %+d = %d%s",result.dice[0],result.bonus,result.total,result.natural==20?"  -  20 naturale":"");
        } else snprintf(text,sizeof text,"%s",!allowed?"Da verificare: apri i dettagli":dice_animation.active?"In volo...":effective>0?"Pronto: vantaggio":effective<0?"Pronto: svantaggio":"Pronto al lancio");
        label(text,73,269,320,false);
        if(!dice_pixels)label("Animazione non disponibile",73,183,320,false);
        button(pool_view?"Prova":mode==-1?"Svant":mode==1?"Vant":"Base",83,296,76,44,pool_view?14:15,!dice_animation.active);
        button(dice_animation.active?"Accelera":has_roll?"Rilancia":"Lancia",167,296,132,44,13,allowed);
        button("Altri",307,296,76,44,40,!dice_animation.active);
    } else {
        label("Contatori manuali",83,111,300,true);
        if(state.resource_count) {
            pd_resource *r=&state.resources[resource_index];label(r->name,83,158,300,false);
            snprintf(text,sizeof text,"%u / %u",r->current,r->maximum);
            lv_obj_t *value=label(text,83,194,300,true);label_font(value,&lv_font_montserrat_32);
            lv_obj_set_style_text_color(value,lv_color_hex(PD_VIOLET),0);meter(r->current,r->maximum,235,PD_VIOLET);
            button("-1",83,247,144,44,20,r->current>0);button("+1",239,247,144,44,21,r->current<r->maximum);
            snprintf(text,sizeof text,"Prossima  %d/%u",resource_index+1,state.resource_count);
            button(text,83,299,300,44,22,state.resource_count>1);
        } else label("Nessun contatore importato",83,194,300,false);
    }
    const char *nav[]={"Eroe","Scheda","Dadi","Usi"};
    const lv_image_dsc_t *icons[]={&pd_art_hero,&pd_art_sheet,&pd_art_dice,&pd_art_uses};
    const int nav_x[]={79,153,251,325},nav_w[]={68,92,68,64};
    for (int i=0;i<4;i++) {
        lv_obj_t *b=button(nav[i],nav_x[i],350,nav_w[i],56,100+i,!dice_animation.active);
        lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);lv_obj_set_style_pad_all(b,2,0);
        lv_obj_align(lv_obj_get_child(b,0),LV_ALIGN_BOTTOM_MID,0,-2);
        art(icons[i],(nav_w[i]-4-24)/2,2,b);
        if (page==i) {
            lv_obj_set_style_bg_color(b,lv_color_hex(0x302d29),0);
            lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);
            lv_obj_set_style_text_color(b,lv_color_hex(PD_GOLD),0);
        }
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
static void avatar_frame(lv_timer_t *timer) {
    (void)timer;
    unsigned pose=pd_avatar_pose(lv_tick_get()-avatar_started);
    if(pose==avatar_phase)return;
    avatar_phase=pose;
    lv_image_cache_drop(&avatar_image);
    if(avatar_large)pd_avatar_large(&state.avatar,pose,avatar_data);else pd_avatar_image(&state.avatar,pose,avatar_data);
    lv_obj_invalidate(avatar_sprite);
}
static bool stage_import(const pd_state *candidate) {
    if(import_pending) { reply("PDD ERROR busy\n");return false; }
    if(dice_animation.active) { pd_dice_finish(&dice_animation);has_roll=true; }
    pending=*candidate;pending.idle_animation=state.idle_animation;import_pending=true;
#ifndef PD_UI_TEST
    import_started=xTaskGetTickCount();
#endif
    reply("PDD WAIT_CONFIRM\n");render();return true;
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
                    if(!strcmp(line,"PDD HELLO")) reply("PDD READY 3\n");
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
            stage_import(&candidate);
        }
        if(import_pending && (xTaskGetTickCount()-import_started)>pdMS_TO_TICKS(120000)) {
            import_pending=false;reply("PDD CANCELLED timeout\n");render();
        }
        pd_board_tick();vTaskDelay(pdMS_TO_TICKS(5));
    }
}

#endif
