#pragma once
#include "lvgl.h"
/* All LVGL calls, including board initialization, belong to the main task. */
void pd_board_init(void);
void pd_board_tick(void);
uint32_t pd_board_random(void *unused);
