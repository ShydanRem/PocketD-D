#pragma once
#include "buttons.h"

#define PD_INPUT_CHECK_TIMEOUT_MS 120000u
#define PD_INPUT_CHECK_TARGETS 5u
#define PD_INPUT_CHECK_COUNT_MAX 999u
typedef enum { PD_INPUT_IDLE,PD_INPUT_HOME,PD_INPUT_TOUCH,PD_INPUT_KEYS } pd_input_check_mode;
/* Temporary observations only: never part of the character or its backups. */
typedef struct {
    pd_input_check_mode mode;
    uint32_t started;
    unsigned touched;
    unsigned counts[3];
    bool available[2];
    pd_buttons keys;
} pd_input_check;
void pd_input_check_begin(pd_input_check *c,uint32_t now);
bool pd_input_check_select(pd_input_check *c,pd_input_check_mode mode,uint32_t now);
bool pd_input_check_touch(pd_input_check *c,unsigned target);
/* Uses the same debounce/short/long policy as configured gameplay buttons. */
bool pd_input_check_keys(pd_input_check *c,int pwr,int boot,uint32_t now,bool enabled);
bool pd_input_check_tick(pd_input_check *c,uint32_t now);
void pd_input_check_close(pd_input_check *c);
