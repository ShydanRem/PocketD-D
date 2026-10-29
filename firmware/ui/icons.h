#pragma once
#include "lvgl.h"
typedef enum { PD_UI_MENU,PD_UI_BACK,PD_UI_NEXT,PD_UI_SEARCH,PD_UI_READY,PD_UI_POUCH,PD_UI_UNDO,PD_UI_ATTACK,PD_UI_DICE,PD_UI_MAGIC,PD_UI_PACK,PD_UI_REST,PD_UI_SHEET,PD_UI_HEART,PD_UI_SHIELD,PD_UI_USES,PD_UI_GEAR,PD_UI_HOME,PD_UI_COUNT } pd_ui_icon_id;
/* One semantic catalogue; all images static, no per-frame pixel allocation. */
const lv_image_dsc_t *pd_ui_icon(pd_ui_icon_id id);
const lv_image_dsc_t *pd_ui_menu_icon(pd_ui_icon_id id);
/* Stable semantic name and long-press help. A long press consumes the release;
 * it never activates the underlying command. Close before destroying a view. */
void pd_ui_name(lv_obj_t *button,const char *name);
void pd_ui_hints_close(void);
