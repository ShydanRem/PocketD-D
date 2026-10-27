#include "companion.h"
#include "companion_assets.h"
#include <stdio.h>
#include <string.h>

#define INK 0x081c25
#define IVORY 0xffefd2
#define MINT 0x8ce6c2
#define GOLD 0xf2c882
#define RED 0xf7a592

static lv_obj_t *surface, *hero, *feedback_label, *motes[3], *targets[10];
static lv_timer_t *timer;
static companion_view current;
static companion_handler receiver;
static void *receiver_context;
static unsigned target_count, focused;
static uint32_t started;
static int hero_x,hero_y;

static lv_obj_t *block(lv_obj_t *parent,int x,int y,int w,int h,unsigned color,lv_opa_t opacity,int radius) {
    lv_obj_t *o=lv_obj_create(parent);lv_obj_remove_style_all(o);
    lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
    lv_obj_set_style_bg_color(o,lv_color_hex(color),0);lv_obj_set_style_bg_opa(o,opacity,0);
    lv_obj_set_style_radius(o,radius,0);
    lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);return o;
}
static lv_obj_t *text(lv_obj_t *parent,const char *value,int x,int y,int w,const lv_font_t *font,unsigned color) {
    lv_obj_t *o=lv_label_create(parent);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,font->line_height);
    lv_label_set_long_mode(o,LV_LABEL_LONG_DOT);lv_label_set_text(o,value);
    lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_style_text_color(o,lv_color_hex(color),0);
    lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE);return o;
}
static lv_obj_t *sprite(lv_obj_t *parent,const lv_image_dsc_t *image,int x,int y) {
    lv_obj_t *o=lv_image_create(parent);lv_image_set_src(o,image);lv_obj_set_pos(o,x,y);
    lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);return o;
}
static void dispatch(lv_event_t *event) {
    lv_obj_t *o=lv_event_get_target_obj(event);
    if(current.paused||lv_obj_has_state(o,LV_STATE_DISABLED))return;
    companion_action action=(companion_action)(uintptr_t)lv_event_get_user_data(event);
    /* The callback may synchronously replace this whole view. Do not access it after. */
    if(receiver)receiver(action,receiver_context);
}
static lv_obj_t *control(const char *name,int x,int y,int w,int h,companion_action action,unsigned color) {
    lv_obj_t *b=lv_button_create(surface);lv_obj_remove_style_all(b);
    lv_obj_set_pos(b,x,y);lv_obj_set_size(b,w,h);lv_obj_remove_flag(b,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(b,lv_color_hex(color),0);lv_obj_set_style_bg_opa(b,235,0);
    lv_obj_set_style_border_width(b,2,0);lv_obj_set_style_border_color(b,lv_color_hex(0x4f7274),0);
    lv_obj_set_style_radius(b,6,0);lv_obj_set_style_bg_opa(b,LV_OPA_COVER,LV_STATE_PRESSED);
    lv_obj_set_style_border_color(b,lv_color_hex(IVORY),LV_STATE_PRESSED);
    lv_obj_set_style_opa(b,LV_OPA_40,LV_STATE_DISABLED);
    lv_obj_add_event_cb(b,dispatch,LV_EVENT_CLICKED,(void*)(uintptr_t)action);
    if(name&&*name)text(b,name,3,h-24,w-6,&lv_font_montserrat_16,IVORY);
    if(target_count<sizeof targets/sizeof targets[0])targets[target_count++]=b;
    return b;
}
static void focus(void) {
    for(unsigned i=0;i<target_count;i++) {
        bool active=i==focused;
        lv_obj_set_style_outline_width(targets[i],active?2:0,0);
        lv_obj_set_style_outline_pad(targets[i],2,0);
        lv_obj_set_style_outline_color(targets[i],lv_color_hex(GOLD),0);
    }
}
static void heart(lv_obj_t *parent,int x,int y) {
    const char *rows[]={".##.##.","#######","#######",".#####.","..###..","...#..."};
    for(int r=0;r<6;r++)for(int c=0;c<7;c++)if(rows[r][c]=='#')
        block(parent,x+c*3,y+r*3,3,3,r<2?0xffb292:0xe97868,255,0);
}
static void status(void) {
    char value[80];
    text(surface,current.name,116,34,234,&lv_font_montserrat_20,IVORY);
    snprintf(value,sizeof value,"LIVELLO %u",current.level);
    text(surface,value,148,59,170,&lv_font_montserrat_16,GOLD);
    if(current.power_text){
        unsigned color=current.power_warning==2?RED:current.power_warning?GOLD:current.power_charging?MINT:IVORY;
        block(surface,320,66,14,8,color,255,0);block(surface,334,68,2,4,color,255,0);
        block(surface,322,68,10,4,INK,255,0);
        if(current.power_percent_valid&&current.power_percent){unsigned filled=current.power_percent>=100?10:current.power_percent/10;if(!filled)filled=1;block(surface,322,68,filled,4,color,255,0);}
        else if(current.power_charging)block(surface,325,67,3,6,MINT,255,0);
        text(surface,current.power_text,338,59,48,&lv_font_montserrat_16,color);
    }
    lv_obj_t *hp=control("",90,81,218,46,COMPANION_OPEN_VITALS,INK);
    lv_obj_set_style_bg_opa(hp,160,0);lv_obj_set_style_border_width(hp,0,0);
    heart(hp,7,13);
    snprintf(value,sizeof value,"%u / %u",current.hp,current.maximum);
    text(hp,value,34,4,180,&lv_font_montserrat_32,current.hp?IVORY:RED);
    block(surface,309,84,80,42,INK,190,6);
    snprintf(value,sizeof value,"CA %u",current.armor);
    text(surface,value,310,94,78,&lv_font_montserrat_20,0xc1e4e7);
    block(surface,132,131,202,7,0x061218,255,3);
    unsigned filled=current.maximum?200*(current.hp<current.maximum?current.hp:current.maximum)/current.maximum:0;
    if(filled)block(surface,133,132,(int)filled,5,current.hp?MINT:RED,255,2);
    if(current.temporary){snprintf(value,sizeof value,"Temp +%u",current.temporary);text(surface,value,153,143,160,&lv_font_montserrat_16,MINT);}
}
static void home(void) {
    lv_obj_t *shade=block(surface,0,0,466,185,INK,255,0);
    lv_obj_set_style_bg_grad_color(shade,lv_color_hex(INK),0);
    lv_obj_set_style_bg_grad_dir(shade,LV_GRAD_DIR_VER,0);
    lv_obj_set_style_bg_main_opa(shade,200,0);lv_obj_set_style_bg_grad_opa(shade,0,0);
    status();
    block(surface,197,324,72,13,0x06161c,180,LV_RADIUS_CIRCLE);
    lv_obj_t *appearance=control("",177,200,112,132,COMPANION_APPEARANCE,INK);
    lv_obj_set_style_bg_opa(appearance,LV_OPA_TRANSP,0);lv_obj_set_style_border_width(appearance,0,0);
    const lv_image_dsc_t *image=current.avatar?current.avatar:&companion_hero;
    hero_x=(112-(int)image->header.w)/2;hero_y=128-(int)image->header.h;
    hero=sprite(appearance,image,hero_x,hero_y);
    const lv_image_dsc_t *images[]={&companion_dock_dice,NULL,&companion_dock_pack};
    const char *names[]={"Dadi","Menu","Zaino"};
    companion_action actions[]={COMPANION_DICE,COMPANION_OPEN_MENU,COMPANION_PACK};
    for(unsigned i=0;i<3;i++) {
        lv_obj_t *b=control(names[i],92+105*i,334,72,76,actions[i],i==1?0x333b32:0x12333e);
        if(images[i])sprite(b,images[i],12,2);
        else for(int row=0;row<2;row++)for(int col=0;col<2;col++)
            block(b,20+19*col,12+19*row,14,14,IVORY,255,2);
    }
    if(!current.storage_error&&!current.paused&&current.feedback_age<2000&&(current.applied_hp||current.applied_temporary)) {
        char value[80];
        if(current.applied_hp&&current.applied_temporary)snprintf(value,sizeof value,"PF %+d\nTemp %+d",current.applied_hp,current.applied_temporary);
        else snprintf(value,sizeof value,current.applied_hp?"%+d PF":"Temp %+d",current.applied_hp?current.applied_hp:current.applied_temporary);
        unsigned color=current.applied_hp<0||(!current.applied_hp&&current.applied_temporary<0)?RED:MINT;
        feedback_label=text(surface,value,290,226,120,&lv_font_montserrat_20,color);
        if(current.applied_hp&&current.applied_temporary)lv_obj_set_height(feedback_label,50);
        lv_obj_set_style_bg_color(feedback_label,lv_color_hex(INK),0);lv_obj_set_style_bg_opa(feedback_label,220,0);
        for(unsigned i=0;i<3;i++)motes[i]=block(surface,168+i*61,232+i*15,4,4,color,255,0);
    }
    if(current.storage_error||current.notice) {
        lv_obj_t *warning=block(surface,100,163,266,37,INK,245,6);
        text(warning,current.notice?current.notice:"Errore memoria: non salvato",6,10,254,&lv_font_montserrat_16,current.storage_error?RED:GOLD);
    }
}
static void menu(void) {
    block(surface,0,0,466,466,INK,210,0);
    if(current.storage_error||current.notice){
        lv_obj_t *warning=text(surface,current.notice?current.notice:"Errore memoria: non salvato",104,49,258,&lv_font_montserrat_16,current.storage_error?RED:GOLD);
        lv_label_set_long_mode(warning,LV_LABEL_LONG_WRAP);lv_obj_set_height(warning,44);
    }else text(surface,"AZIONI",133,56,200,&lv_font_montserrat_26,IVORY);
    const char *names[]={"Attacco","Dadi","Magie","Zaino","Riposo","Scheda"};
    const lv_image_dsc_t *images[]={&companion_sword,&companion_dice,&companion_magic,&companion_pack,&companion_rest,&companion_sheet};
    const unsigned colors[]={0x5b322c,0x164553,0x493353,0x54412b,0x253b57,0x284d41};
    const companion_action actions[]={COMPANION_ATTACK,COMPANION_DICE,COMPANION_MAGIC,COMPANION_PACK,COMPANION_REST,COMPANION_SHEET};
    for(unsigned i=0;i<6;i++) {
        lv_obj_t *b=control(names[i],81+108*(i%3),113+120*(i/3),88,100,actions[i],colors[i]);
        sprite(b,images[i],12,5);
    }
    lv_obj_t *uses=control("",99,361,112,56,COMPANION_RESOURCES,INK);
    text(uses,"Usi",2,16,108,&lv_font_montserrat_20,IVORY);
    lv_obj_t *back=control("",255,361,112,56,COMPANION_OPEN_HOME,INK);
    text(back,"Eroe",2,16,108,&lv_font_montserrat_20,IVORY);
}
static void vitals(void) {
    char value[80];block(surface,0,0,466,466,INK,220,0);
    text(surface,current.storage_error?"Errore memoria":"Punti ferita",98,69,270,&lv_font_montserrat_26,IVORY);
    snprintf(value,sizeof value,"%u / %u",current.hp,current.maximum);
    text(surface,value,108,112,250,&lv_font_montserrat_32,current.hp?MINT:RED);
    snprintf(value,sizeof value,"Temporanei: %u",current.temporary);
    text(surface,value,113,159,240,&lv_font_montserrat_20,IVORY);
    text(surface,"+/-1: correzione manuale",98,184,270,&lv_font_montserrat_16,GOLD);
    const char *names[]={"-1 PF","+1 PF","Cura","Temp.","Danni","Stato","Annulla","Eroe"};
    const companion_action actions[]={COMPANION_MINUS,COMPANION_PLUS,COMPANION_HEAL,COMPANION_TEMPORARY,
        COMPANION_DAMAGE,COMPANION_STATUS,COMPANION_UNDO,COMPANION_OPEN_HOME};
    for(unsigned i=0;i<8;i++){
        lv_obj_t *b=control("",105+(i%2)*144,207+(i/2)*54,112,44,actions[i],i==0?0x4b3032:i==1||i==2?0x214c43:INK);
        text(b,names[i],2,11,108,&lv_font_montserrat_20,IVORY);
        if((i==0&&!current.can_hurt)||((i==1||i==2)&&!current.can_heal)||(i==6&&!current.can_undo))lv_obj_add_state(b,LV_STATE_DISABLED);
    }
}
static void tick(lv_timer_t *unused) {
    (void)unused;if(current.paused)return;
    uint32_t elapsed=lv_tick_get()-started;
    uint32_t receipt_age=elapsed+current.feedback_age;
    if(hero) {
        int y=current.motion&&((elapsed/480)%2)?-2:0;
        if(feedback_label&&current.motion&&current.applied_hp>0&&receipt_age<600)
            y=receipt_age<300?-(int)(receipt_age/50):-(int)((600-receipt_age)/50);
        if(feedback_label&&current.motion&&current.applied_hp<0&&receipt_age<300)
            lv_obj_set_x(hero,hero_x+(receipt_age/60%2?2:-2));
        else lv_obj_set_x(hero,hero_x);
        lv_obj_set_y(hero,hero_y+y);
    }
    if(feedback_label) {
        if(receipt_age>=2000) {
            companion_clear_feedback();
        }else if(current.motion) {
            lv_obj_set_y(feedback_label,226-(int)(receipt_age/160));
            for(unsigned i=0;i<3;i++)lv_obj_set_y(motes[i],232+i*15-(receipt_age/80)%12);
        }
    }
    if(!current.motion&&!feedback_label){lv_timer_delete(timer);timer=NULL;}
}
void companion_close(void) {
    if(timer){lv_timer_delete(timer);timer=NULL;}
    surface=hero=feedback_label=NULL;memset(motes,0,sizeof motes);memset(targets,0,sizeof targets);
    target_count=focused=0;receiver=NULL;receiver_context=NULL;
}
void companion_show(lv_obj_t *root,const companion_view *view,companion_handler handler,void *context) {
    companion_close();lv_obj_clean(root);surface=root;current=*view;receiver=handler;receiver_context=context;
    /* The panel supplies the circular aperture. Keep LVGL's display rectangular,
     * as in the production renderer, so repeated refreshes repaint every pixel. */
    lv_obj_set_style_bg_color(root,lv_color_hex(INK),0);lv_obj_set_style_radius(root,0,0);
    lv_obj_set_style_clip_corner(root,false,0);lv_obj_remove_flag(root,LV_OBJ_FLAG_SCROLLABLE);
    sprite(root,current.background?current.background:&companion_camp,0,0);
    if(current.page==COMPANION_HOME)home();else if(current.page==COMPANION_MENU)menu();else vitals();
    started=lv_tick_get();focused=target_count; /* touch use has no artificial focused target */
    if((hero&&current.motion)||feedback_label)timer=lv_timer_create(tick,80,NULL);
    if(current.paused&&timer)lv_timer_pause(timer);
}
void companion_pause(bool paused) {
    if(!surface||current.paused==paused)return;
    current.paused=paused;
    if(paused) {
        if(timer)lv_timer_pause(timer);
        /* Expire cosmetic feedback at sleep; never replay it on wake. */
        companion_clear_feedback();
        if(hero){lv_obj_set_x(hero,hero_x);lv_obj_set_y(hero,hero_y);}
    }else if(timer){started=lv_tick_get();lv_timer_resume(timer);}
}
void companion_focus_step(int direction) {
    if(current.paused||!target_count)return;
    int step=direction<0?-1:1;
    if(focused>=target_count)focused=step>0?target_count-1:0;
    for(unsigned i=0;i<target_count;i++) {
        focused=(focused+target_count+step)%target_count;
        if(!lv_obj_has_state(targets[focused],LV_STATE_DISABLED))break;
    }
    focus();
}
void companion_activate(void) {
    if(!current.paused&&focused<target_count&&!lv_obj_has_state(targets[focused],LV_STATE_DISABLED))
        lv_obj_send_event(targets[focused],LV_EVENT_CLICKED,NULL);
}
unsigned companion_focus_index(void){return focused;}
lv_obj_t *companion_avatar_object(void){return hero;}
bool companion_feedback_visible(void){return feedback_label!=NULL;}
void companion_clear_feedback(void){
    if(feedback_label){lv_obj_delete(feedback_label);feedback_label=NULL;}
    for(unsigned i=0;i<3;i++)if(motes[i]){lv_obj_delete(motes[i]);motes[i]=NULL;}
}
