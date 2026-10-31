#include "theme.h"
#include "../room_cache.h"
#include "visual.h"

static const pd_theme_spec themes[]={
    [PD_THEME_SHEET]={"Scheda",0xb3dccb,&theme_sheet,&theme_room_study},
    [PD_THEME_DICE]={"Dadi",0x85d9e3,&theme_dice,&theme_room_study},
    [PD_THEME_USES]={"Risorse",0xd2b2ef,&theme_crystal,&theme_room_magic},
    [PD_THEME_PACK]={"Zaino",0xe7c389,&theme_pack,&theme_room_pack},
    [PD_THEME_COMBAT]={"Attacco",0xeeae94,&theme_sword,&theme_room_pack},
    [PD_THEME_MAGIC]={"Magie",0xc8adea,&theme_magic,&theme_room_magic},
    [PD_THEME_REST]={"Riposo",0xedc894,&theme_rest,&theme_room_rest},
    [PD_THEME_STATUS]={"Stato",0xf0b3b2,&theme_heart,&theme_room_study},
    [PD_THEME_EFFECTS]={"Effetti",0xc4b2ef,&theme_notes,&theme_room_magic},
    [PD_THEME_DAMAGE]={"Difese",0x9ec9ef,&theme_shield,&theme_room_study},
    [PD_THEME_TIME]={"Tempo",0xe3c487,&theme_time,&theme_room_study},
    [PD_THEME_APPEARANCE]={"Compagno",0xa6dbc9,&theme_hero,&theme_room_rest},
    [PD_THEME_SCREEN]={"Schermo",0xe8d08d,&theme_sun,&theme_room_study},
    [PD_THEME_KEYS]={"Tasti",0xb5d4da,&theme_gear,&theme_room_study},
    [PD_THEME_WALLET]={"Monete",0xe7c389,&theme_pack,&theme_room_pack},
    [PD_THEME_HISTORY]={"Storico",0xe3c487,&theme_notes,&theme_room_study},
    [PD_THEME_BACKUP]={"Backup",0x8ad9e2,&theme_usb,&theme_room_study},
    [PD_THEME_CHECK]={"Collaudo",0xb5d4da,&theme_gear,&theme_room_study},
    [PD_THEME_IMPORT]={"Copia USB",0x8ad9e2,&theme_usb,&theme_room_study},
    [PD_THEME_LIBRARY]={"Raccolta",0xa6dbc9,&theme_hero,&theme_room_rest}
};
static const pd_theme_spec *current;
static lv_obj_t *crest,*panel;
const pd_theme_spec *pd_theme_get(pd_theme_kind kind){return kind>PD_THEME_NONE&&kind<=PD_THEME_LIBRARY?&themes[kind]:NULL;}
static lv_obj_t *rect(lv_obj_t *root,int x,int y,int w,int h,uint32_t color,lv_opa_t opacity,int radius){
    lv_obj_t *o=lv_obj_create(root);lv_obj_remove_style_all(o);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
    lv_obj_set_style_bg_color(o,lv_color_hex(color),0);lv_obj_set_style_bg_opa(o,opacity,0);lv_obj_set_style_radius(o,radius,0);
    lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);return o;
}
void pd_theme_begin(lv_obj_t *root,pd_theme_kind kind,bool dice_tray){
    current=pd_theme_get(kind);if(!current)return;
    lv_obj_t *bg=lv_image_create(root);lv_image_set_src(bg,pd_room_image(current->room));
    lv_obj_remove_flag(bg,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    rect(root,0,0,466,466,0x061923,80,0);
    panel=rect(root,74,96,318,kind==PD_THEME_HISTORY||kind==PD_THEME_LIBRARY?261:249,0x081c27,230,10);
    lv_obj_set_style_border_width(panel,1,0);lv_obj_set_style_border_color(panel,lv_color_hex(0x49616b),0);
    rect(root,121,24,230,47,0x081c27,230,8);
    rect(root,91,72,284,20,0x081c27,235,4);
    if(dice_tray){
        lv_obj_t *tray=rect(root,69,134,328,134,PD_INK,255,6);
        lv_obj_set_style_border_width(tray,1,0);lv_obj_set_style_border_color(tray,lv_color_hex(current->accent),0);
    }
    crest=lv_image_create(root);lv_image_set_src(crest,current->icon);lv_obj_set_pos(crest,312,31);
    lv_obj_remove_flag(crest,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    rect(root,208,426,50,2,current->accent,190,0);
}
void pd_theme_hide_crest(void){if(crest)lv_obj_add_flag(crest,LV_OBJ_FLAG_HIDDEN);}
void pd_theme_button(lv_obj_t *b,bool primary){
    unsigned accent=current?current->accent:PD_MINT;
    lv_obj_set_style_radius(b,6,0);
    lv_obj_set_style_bg_color(b,lv_color_hex(primary?0x285852:0x17313e),0);
    lv_obj_set_style_bg_grad_color(b,lv_color_hex(primary?0x173d3b:0x102631),0);
    lv_obj_set_style_bg_grad_dir(b,LV_GRAD_DIR_VER,0);lv_obj_set_style_bg_opa(b,245,0);
    lv_obj_set_style_border_width(b,1,0);lv_obj_set_style_border_color(b,lv_color_hex(primary?PD_MINT:0x557782),0);
    lv_obj_set_style_text_color(b,lv_color_hex(PD_TEXT),0);
    lv_obj_set_style_text_align(b,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_pad_all(b,4,0);
    lv_obj_set_style_shadow_width(b,0,0);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x385563),LV_STATE_PRESSED);
    lv_obj_set_style_border_color(b,lv_color_hex(accent),LV_STATE_PRESSED);
    lv_obj_set_style_border_width(b,2,LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x101f2a),LV_STATE_DISABLED);
    lv_obj_set_style_bg_grad_dir(b,LV_GRAD_DIR_NONE,LV_STATE_DISABLED);
    lv_obj_set_style_border_color(b,lv_color_hex(0x344953),LV_STATE_DISABLED);
    lv_obj_set_style_text_color(b,lv_color_hex(0x8e9ca5),LV_STATE_DISABLED);
}
static void crest_opacity(void *o,int32_t value){lv_obj_set_style_opa(o,(lv_opa_t)value,0);}
static void panel_opacity(void *o,int32_t value){lv_obj_set_style_border_opa(o,(lv_opa_t)value,0);}
void pd_theme_pause(void){
    if(crest){lv_anim_delete(crest,crest_opacity);crest_opacity(crest,255);}
    if(panel){lv_anim_delete(panel,panel_opacity);panel_opacity(panel,255);}
}
void pd_theme_close(void){pd_theme_pause();crest=panel=NULL;current=NULL;}
void pd_theme_finish(bool motion,bool changed){
    if(!current||!motion||!changed)return;
    lv_anim_t a;lv_anim_init(&a);lv_anim_set_duration(&a,160);lv_anim_set_values(&a,200,255);
    lv_anim_set_var(&a,crest);lv_anim_set_exec_cb(&a,crest_opacity);lv_anim_start(&a);
    lv_anim_set_values(&a,100,255);
    lv_anim_set_var(&a,panel);lv_anim_set_exec_cb(&a,panel_opacity);lv_anim_start(&a);
}
