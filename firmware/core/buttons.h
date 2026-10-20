#ifndef PD_BUTTONS_H
#define PD_BUTTONS_H
#include <stdbool.h>
#include <stdint.h>
#define PD_BUTTON_DEBOUNCE_MS 30u
#define PD_BUTTON_LONG_MS 800u
/* Levels are normalized: -1 unavailable, 0 released, 1 pressed. No boot-held action. */
typedef struct { bool known,armed,pressed;int raw;uint32_t changed,started; } pd_key;
typedef struct { pd_key keys[2]; } pd_buttons;
/* Bits 0 PWR short, 1 BOOT short, 2 BOOT long. Long PWR is never an app action. */
unsigned pd_buttons_sample(pd_buttons *keys,int pwr,int boot,uint32_t now,bool enabled);
#endif
