#pragma once
#include "lvgl.h"
#include "screen.h"
/* All LVGL calls, including board initialization, belong to the main task. */
void pd_board_init(void);
void pd_board_tick(void);
uint32_t pd_board_random(void *unused);

/* PWR and BOOT normalized levels: -1 unavailable, 0 released, 1 pressed. */
void pd_board_buttons(int levels[2]);

void pd_board_screen(pd_screen *controller);
bool pd_board_brightness(unsigned percent,void *unused);
