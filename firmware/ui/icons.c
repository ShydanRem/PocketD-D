/* Original pixel control glyphs. Existing approved emblems are reused below. */
#include "icons.h"
#include <string.h>
#include "theme/theme_assets.h"
#include "companion/companion_assets.h"
#include "control_icons.inc"
const lv_image_dsc_t *pd_ui_icon(pd_ui_icon_id id){
 static const lv_image_dsc_t *const icons[]={&menu,&back,&next,&search,&ready,&pouch,&undo,&theme_sword,&theme_dice,&theme_magic,&theme_pack,&theme_rest,&theme_sheet,&theme_heart,&theme_shield,&theme_crystal,&theme_gear,&theme_hero};
 return (unsigned)id<PD_UI_COUNT?icons[id]:NULL;
}
const lv_image_dsc_t *pd_ui_menu_icon(pd_ui_icon_id id){
 switch(id){case PD_UI_ATTACK:return &companion_sword;case PD_UI_DICE:return &companion_dice;case PD_UI_MAGIC:return &companion_magic;case PD_UI_PACK:return &companion_pack;case PD_UI_REST:return &companion_rest;case PD_UI_SHEET:return &companion_sheet;default:return pd_ui_icon(id);}
}

static lv_obj_t *hint;
static lv_timer_t *hint_timer;
void pd_ui_hints_close(void){if(hint_timer){lv_timer_delete(hint_timer);hint_timer=NULL;}if(hint){lv_obj_delete(hint);hint=NULL;}}
static void hint_expire(lv_timer_t *timer){(void)timer;pd_ui_hints_close();}
static void explain_icon(lv_event_t *e){
 lv_obj_t *button=lv_event_get_target_obj(e);if(lv_obj_has_state(button,LV_STATE_DISABLED))return;
 lv_indev_t *input=lv_indev_active();if(input)lv_indev_wait_release(input);
 const char *name=lv_obj_get_user_data(button);if(!name||!*name)return;
 if(!strcmp(name,"<"))name="Pagina precedente";else if(!strcmp(name,">"))name="Pagina successiva";else if(!strcmp(name,"Pronte"))name="Preparazione, slot e utilizzi";else if(!strcmp(name,"Consumabili"))name="Solo consumabili e cariche";
 pd_ui_hints_close();hint=lv_obj_create(lv_screen_active());lv_obj_remove_style_all(hint);lv_obj_set_pos(hint,91,157);lv_obj_set_size(hint,284,48);lv_obj_set_style_bg_color(hint,lv_color_hex(0x081c25),0);lv_obj_set_style_bg_opa(hint,LV_OPA_COVER,0);lv_obj_set_style_radius(hint,6,0);lv_obj_set_style_border_width(hint,2,0);lv_obj_set_style_border_color(hint,lv_color_hex(0xf2c882),0);lv_obj_remove_flag(hint,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
 lv_obj_t *caption=lv_label_create(hint);lv_obj_set_style_text_font(caption,&lv_font_montserrat_16,0);lv_obj_set_style_text_color(caption,lv_color_hex(0xffefd2),0);lv_label_set_text(caption,name);lv_obj_center(caption);lv_obj_remove_flag(caption,LV_OBJ_FLAG_CLICKABLE);hint_timer=lv_timer_create(hint_expire,2000,NULL);
}
void pd_ui_name(lv_obj_t *button,const char *name){lv_obj_set_user_data(button,(void*)name);lv_obj_add_event_cb(button,explain_icon,LV_EVENT_LONG_PRESSED,NULL);}
