#ifndef PD_SCREEN_H
#define PD_SCREEN_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    uint32_t activity,attempted_at;
    unsigned applied,attempted;
    bool dimmed,swallow_touch,swallow_keys,touch_down,failed;
} pd_screen;
typedef bool (*pd_brightness_setter)(unsigned percent,void *context);
void pd_screen_activity(pd_screen *s,uint32_t now);
/* Invalid touch samples cannot release a wake-up contact. */
bool pd_screen_touch(pd_screen *s,bool valid,bool pressed,uint32_t now);
/* False suppresses the complete physical gesture, regardless of its binding. */
bool pd_screen_keys(pd_screen *s,int pwr,int boot,uint32_t now);
void pd_screen_step(pd_screen *s,unsigned brightness,unsigned timeout,bool locked,uint32_t now,pd_brightness_setter set,void *context);
#endif
