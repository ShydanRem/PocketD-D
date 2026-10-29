#pragma once
#include "lvgl.h"
#include "screen.h"
#include "diagnostics.h"
#include "power.h"
/* All LVGL calls, including board initialization, belong to the main task. */
void pd_board_init(void);
/* Optional SD catalog. Never formats a card. Main task only. */
void pd_board_assets_refresh(void);
void pd_board_assets_activate(void);
bool pd_board_assets_available(void);
bool pd_board_assets_space(uint64_t *available);
bool pd_board_assets_verify(const char *path,const char *sha256);
void pd_board_tick(void);
uint32_t pd_board_random(void *unused);

/* PWR and BOOT normalized levels: -1 unavailable, 0 released, 1 pressed. */
void pd_board_buttons(int levels[2]);

void pd_board_screen(pd_screen *controller);
bool pd_board_brightness(unsigned percent,void *unused);
/* Main-task snapshot; does not sample inputs or write peripheral registers. */
void pd_board_diagnostics(pd_diagnostics *out);
void pd_board_power(pd_power_info *out,uint32_t now,bool force);
bool pd_board_pause(bool paused);
enum { PD_POWER_WAIT,PD_POWER_WAKE,PD_POWER_ERROR };
/* Only while the panel is paused. Light sleep is allowed on battery only;
   GPIO0/11 wake immediately, PWR is checked at most every 20 ms. */
unsigned pd_board_power_wait(bool battery_only);
bool pd_board_power_off(void);
