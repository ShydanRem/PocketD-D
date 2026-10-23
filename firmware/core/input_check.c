#include "input_check.h"
#include <string.h>

void pd_input_check_begin(pd_input_check *c,uint32_t now){
    memset(c,0,sizeof *c);c->mode=PD_INPUT_HOME;c->started=now;
}
bool pd_input_check_select(pd_input_check *c,pd_input_check_mode mode,uint32_t now){
    if(c->mode==PD_INPUT_IDLE||mode<PD_INPUT_HOME||mode>PD_INPUT_KEYS)return false;
    c->mode=mode;c->started=now;c->keys=(pd_buttons){0};
    if(mode==PD_INPUT_TOUCH)c->touched=0;
    if(mode==PD_INPUT_KEYS){memset(c->counts,0,sizeof c->counts);memset(c->available,0,sizeof c->available);}
    return true;
}
bool pd_input_check_touch(pd_input_check *c,unsigned target){
    if(c->mode!=PD_INPUT_TOUCH||target!=c->touched||target>=PD_INPUT_CHECK_TARGETS)return false;
    c->touched++;return true;
}
bool pd_input_check_keys(pd_input_check *c,int pwr,int boot,uint32_t now,bool enabled){
    if(c->mode!=PD_INPUT_KEYS)return false;
    bool pwr_ok=pwr==0||pwr==1,boot_ok=boot==0||boot==1;
    bool changed=c->available[0]!=pwr_ok||c->available[1]!=boot_ok;
    c->available[0]=pwr_ok;c->available[1]=boot_ok;
    unsigned gestures=pd_buttons_sample(&c->keys,pwr,boot,now,enabled);
    for(unsigned i=0;i<3;i++)if((gestures&(1u<<i))&&c->counts[i]<PD_INPUT_CHECK_COUNT_MAX){c->counts[i]++;changed=true;}
    return changed;
}
void pd_input_check_close(pd_input_check *c){c->mode=PD_INPUT_IDLE;c->keys=(pd_buttons){0};}
bool pd_input_check_tick(pd_input_check *c,uint32_t now){
    if(c->mode==PD_INPUT_IDLE||(uint32_t)(now-c->started)<PD_INPUT_CHECK_TIMEOUT_MS)return false;
    pd_input_check_close(c);return true;
}
