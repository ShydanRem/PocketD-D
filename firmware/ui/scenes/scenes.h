#pragma once
#include "lvgl.h"
/* Three device-local scenes. Existing camp remains byte-for-byte unchanged.
 * Indexed variants expand once into shared PSRAM; borrowed descriptors remain
 * valid until another scene is selected. Call after destroying the old view. */
const lv_image_dsc_t *pd_scene_image(unsigned scene);
const lv_image_dsc_t *pd_scene_preview(unsigned scene);
