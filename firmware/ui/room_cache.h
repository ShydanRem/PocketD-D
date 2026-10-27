#pragma once
#include "lvgl.h"
/* Two live full-screen RGB565 backgrounds; no transform or per-draw allocation.
 * Source descriptors at 233x233 store exact repeated 2x2 native pixels.
 * Other dimensions pass through unchanged. Callers must not retain >2 folded
 * backgrounds on one screen. Scene thumbnails/I8 sources use separate buffers. */
const lv_image_dsc_t *pd_room_image(const lv_image_dsc_t *source);
