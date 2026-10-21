#ifndef PD_AVATAR_H
#define PD_AVATAR_H
#include "pocketdd.h"
#define PD_AVATAR_WIDTH 48
#define PD_AVATAR_HEIGHT 60
#define PD_AVATAR_IMAGE_BYTES (64+PD_AVATAR_WIDTH*PD_AVATAR_HEIGHT/2)
#define PD_AVATAR_LARGE_BYTES (64+PD_AVATAR_WIDTH*PD_AVATAR_HEIGHT*2)
#define PD_AVATAR_HERO_BYTES (64+72*90/2)
/* Pure presentation, four poses. BGRA palette + I4 pixels, no RNG or persistence. */
bool pd_avatar_image(const pd_avatar *avatar,unsigned pose,uint8_t out[PD_AVATAR_IMAGE_BYTES]);
/* Exact pixel replication, supplied at native size to LVGL (no indexed-image transform). */
bool pd_avatar_large(const pd_avatar *avatar,unsigned pose,uint8_t out[PD_AVATAR_LARGE_BYTES]);
bool pd_avatar_hero(const pd_avatar *avatar,unsigned pose,uint8_t out[PD_AVATAR_HERO_BYTES]);
unsigned pd_avatar_pose(uint32_t elapsed);
#endif
