#pragma once
#include "pocketdd.h"
#include "lvgl.h"
const lv_image_dsc_t *pd_item_icon(const pd_state *state,unsigned index);
const lv_image_dsc_t *pd_power_icon(const pd_power *power);

/* Cosmetic setup hint: never applies numeric properties. */
unsigned pd_item_setup_hint(const pd_state *state,unsigned index);
