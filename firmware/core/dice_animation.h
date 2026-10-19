#ifndef PD_DICE_ANIMATION_H
#define PD_DICE_ANIMATION_H
#include "pocketdd.h"
#define PD_DICE_WIDTH 320
#define PD_DICE_HEIGHT 128
#define PD_DICE_PIXELS (PD_DICE_WIDTH*PD_DICE_HEIGHT)
#define PD_DICE_FLIGHT_MS 1100
#define PD_DICE_STAGGER_MS 140
#define PD_DICE_SPARK_MS 260
/* Pure presentation. Never calls an RNG, persists state or recomputes a roll. */
typedef struct {
    pd_roll roll;
    uint32_t started,elapsed;
    bool active,preview;
} pd_dice_animation;
typedef struct { int x,y,frame,height; bool settled; } pd_dice_pose;
void pd_dice_idle(pd_dice_animation *a,unsigned count);
void pd_dice_start(pd_dice_animation *a,const pd_roll *roll,uint32_t now);
uint32_t pd_dice_duration(const pd_dice_animation *a);
bool pd_dice_tick(pd_dice_animation *a,uint32_t now);
void pd_dice_finish(pd_dice_animation *a);
pd_dice_pose pd_dice_position(const pd_dice_animation *a,unsigned die);
/* Deterministic RGB565 canvas, width 320 x height 128; caller owns the buffer. */
void pd_dice_paint(const pd_dice_animation *a,uint16_t pixels[PD_DICE_PIXELS]);
#endif
