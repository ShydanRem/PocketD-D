#include "buttons.h"
static unsigned sample(pd_key *k,int level,uint32_t now,bool enabled,bool boot) {
    if(!enabled||level<0||level>1){*k=(pd_key){0};return 0;}
    if(!k->known){k->known=true;k->raw=level;k->changed=now;return 0;}
    if(level!=k->raw){k->raw=level;k->changed=now;return 0;}
    if(now-k->changed<PD_BUTTON_DEBOUNCE_MS)return 0;
    if(!k->armed){if(!level)k->armed=true;return 0;}
    if(level&&!k->pressed){k->pressed=true;k->started=k->changed;return 0;}
    if(!level&&k->pressed){
        k->pressed=false;
        bool long_press=k->changed-k->started>=PD_BUTTON_LONG_MS;
        return boot?(long_press?4u:2u):(long_press?0u:1u);
    }
    return 0;
}
unsigned pd_buttons_sample(pd_buttons *keys,int pwr,int boot,uint32_t now,bool enabled) {
    if(!keys)return 0;
    unsigned a=sample(&keys->keys[0],pwr,now,enabled,false),b=sample(&keys->keys[1],boot,now,enabled,true);
    return a|b;
}
