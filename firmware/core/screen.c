#include "screen.h"
void pd_screen_activity(pd_screen *s,uint32_t now) {
    s->activity=now;
    if(s->dimmed){s->dimmed=false;s->swallow_touch=true;s->swallow_keys=true;}
}
bool pd_screen_touch(pd_screen *s,bool valid,bool pressed,uint32_t now) {
    if(!valid)return false;
    s->touch_down=pressed;
    if(pressed)pd_screen_activity(s,now);
    if(s->swallow_touch){if(!pressed)s->swallow_touch=false;return false;}
    return pressed;
}
bool pd_screen_keys(pd_screen *s,int pwr,int boot,uint32_t now) {
    if(pwr==1||boot==1)pd_screen_activity(s,now);
    if(s->swallow_keys){if(pwr!=1&&boot!=1)s->swallow_keys=false;return false;}
    return true;
}
void pd_screen_step(pd_screen *s,unsigned brightness,unsigned timeout,bool locked,uint32_t now,pd_brightness_setter set,void *context) {
    static const uint32_t waits[]={0,30000,60000,120000};
    if(!s||!set||brightness<10||brightness>100||timeout>3)return;
    if(locked||s->touch_down||!timeout)pd_screen_activity(s,now);
    else if(brightness==10)s->dimmed=false;
    else if(now-s->activity>=waits[timeout])s->dimmed=true;
    unsigned desired=s->dimmed?10:brightness;
    if(desired==s->applied){s->failed=false;return;}
    if(s->failed&&s->attempted==desired&&now-s->attempted_at<1000)return;
    s->attempted=desired;s->attempted_at=now;
    s->failed=!set(desired,context);if(!s->failed)s->applied=desired;
}
