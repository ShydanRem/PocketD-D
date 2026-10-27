#ifndef PD_AVATAR_H
#define PD_AVATAR_H
#include "pocketdd.h"
#define PD_AVATAR_WIDTH 48
#define PD_AVATAR_HEIGHT 60
#define PD_AVATAR_IMAGE_BYTES (1024+PD_AVATAR_WIDTH*PD_AVATAR_HEIGHT)
#define PD_AVATAR_LARGE_BYTES (1024+PD_AVATAR_WIDTH*PD_AVATAR_HEIGHT*4)
#define PD_AVATAR_HERO_BYTES (1024+72*90)
/* Pure presentation, four poses (idle/blink/heal/hurt). BGRA palette + I8 pixels, no RNG or persistence. */
bool pd_avatar_image(const pd_avatar *avatar,unsigned pose,uint8_t out[PD_AVATAR_IMAGE_BYTES]);
/* Pre-expanded sizes for LVGL: 2x home; nearest 3/2 appearance; no image transform. */
bool pd_avatar_large(const pd_avatar *avatar,unsigned pose,uint8_t out[PD_AVATAR_LARGE_BYTES]);
bool pd_avatar_hero(const pd_avatar *avatar,unsigned pose,uint8_t out[PD_AVATAR_HERO_BYTES]);
const char *pd_species_surface(unsigned lineage);
const char *pd_species_name(unsigned lineage);
const char *pd_body_name(unsigned body);
bool pd_species_hair(unsigned lineage);
const char *pd_species_choice(unsigned lineage,unsigned field,unsigned value);
bool pd_avatar_variant_image(const pd_avatar *avatar,unsigned lineage,unsigned body,unsigned pose,uint8_t out[PD_AVATAR_IMAGE_BYTES]);
bool pd_avatar_variant_large(const pd_avatar *avatar,unsigned lineage,unsigned body,unsigned pose,uint8_t out[PD_AVATAR_LARGE_BYTES]);
bool pd_avatar_variant_hero(const pd_avatar *avatar,unsigned lineage,unsigned body,unsigned pose,uint8_t out[PD_AVATAR_HERO_BYTES]);
unsigned pd_avatar_pose(uint32_t elapsed);
/* Actual committed HP/temp deltas; cosmetic only, no state or random draw. */
unsigned pd_avatar_reaction(int hp_delta,int temp_delta,uint32_t age);
#endif
