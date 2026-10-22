#pragma once
#include "lvgl.h"
#include "theme_assets.h"
typedef enum { PD_THEME_NONE,PD_THEME_SHEET,PD_THEME_DICE,PD_THEME_USES,PD_THEME_PACK,
    PD_THEME_COMBAT,PD_THEME_MAGIC,PD_THEME_REST,PD_THEME_STATUS,PD_THEME_EFFECTS,
    PD_THEME_DAMAGE,PD_THEME_TIME,PD_THEME_APPEARANCE,PD_THEME_SCREEN,PD_THEME_KEYS,
    PD_THEME_WALLET,PD_THEME_IMPORT } pd_theme_kind;
typedef struct { const char *title;uint32_t accent;const lv_image_dsc_t *icon,*room; } pd_theme_spec;
const pd_theme_spec *pd_theme_get(pd_theme_kind kind);
void pd_theme_begin(lv_obj_t *root,pd_theme_kind kind,bool dice_tray);
void pd_theme_button(lv_obj_t *button,bool primary);
void pd_theme_finish(bool motion,bool changed);
void pd_theme_pause(void);
void pd_theme_close(void);
