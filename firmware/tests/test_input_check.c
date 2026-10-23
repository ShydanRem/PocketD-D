#include "input_check.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint32_t now;
static void sample(pd_input_check *c,int pwr,int boot,unsigned delay,bool enabled){now+=delay;pd_input_check_keys(c,pwr,boot,now,enabled);}
static void release(pd_input_check *c){sample(c,0,0,0,true);sample(c,0,0,31,true);}
static void gesture(pd_input_check *c,unsigned which,unsigned held){
    release(c);int pwr=which==0,boot=which!=0;
    sample(c,pwr,boot,0,true);sample(c,pwr,boot,31,true);
    assert(held>=31);sample(c,pwr,boot,held-31,true);release(c);
}
int main(void){
    pd_input_check c={0},before=c;
    assert(!pd_input_check_select(&c,PD_INPUT_KEYS,0)&&!pd_input_check_touch(&c,0)&&!pd_input_check_keys(&c,0,0,0,true)&&!pd_input_check_tick(&c,120000));
    assert(!memcmp(&c,&before,sizeof c));pd_input_check_begin(&c,123);
    assert(c.mode==PD_INPUT_HOME&&!pd_input_check_select(&c,PD_INPUT_IDLE,0)&&!pd_input_check_select(&c,99,0));
    assert(pd_input_check_select(&c,PD_INPUT_TOUCH,200));
    for(unsigned i=0;i<PD_INPUT_CHECK_TARGETS;i++){
        before=c;assert(!pd_input_check_touch(&c,i+1)&&!memcmp(&c,&before,sizeof c));
        assert(pd_input_check_touch(&c,i)&&c.touched==i+1);
        assert(!pd_input_check_touch(&c,i));
    }
    assert(pd_input_check_select(&c,PD_INPUT_HOME,300)&&c.touched==5);
    assert(pd_input_check_select(&c,PD_INPUT_TOUCH,400)&&!c.touched);
    for(unsigned start=0;start<2;start++){
        now=start?UINT32_MAX-500:0;assert(pd_input_check_select(&c,PD_INPUT_KEYS,now));
        /* Already held on entry: no gesture until released and freshly pressed. */
        sample(&c,1,1,0,true);sample(&c,1,1,1000,true);release(&c);
        assert(!c.counts[0]&&!c.counts[1]&&!c.counts[2]);
        gesture(&c,0,100);gesture(&c,1,100);gesture(&c,2,800);
        assert(c.counts[0]==1&&c.counts[1]==1&&c.counts[2]==1);
        gesture(&c,0,900);assert(c.counts[0]==1); /* PWR long is reserved. */
        /* A contact shorter than debounce does not count. */
        sample(&c,1,0,0,true);sample(&c,0,0,10,true);release(&c);assert(c.counts[0]==1);
        sample(&c,0,1,0,true);sample(&c,0,1,31,true);sample(&c,0,1,100,false);release(&c);
        assert(c.counts[1]==1&&c.counts[2]==1);
        sample(&c,-1,0,0,true);assert(!c.available[0]&&c.available[1]);
        sample(&c,2,-1,0,true);assert(!c.available[0]&&!c.available[1]);release(&c);
        gesture(&c,1,799);assert(c.counts[1]==2&&c.counts[2]==1);
        gesture(&c,2,800);assert(c.counts[1]==2&&c.counts[2]==2);
        for(unsigned i=0;i<3;i++)c.counts[i]=PD_INPUT_CHECK_COUNT_MAX;
        gesture(&c,0,100);gesture(&c,1,100);gesture(&c,2,800);
        for(unsigned i=0;i<3;i++)assert(c.counts[i]==PD_INPUT_CHECK_COUNT_MAX);
        unsigned touched=c.touched;assert(pd_input_check_select(&c,PD_INPUT_HOME,now)&&c.counts[0]==999&&c.touched==touched);
    }
    now=UINT32_MAX-60000;pd_input_check_begin(&c,now);
    assert(!pd_input_check_tick(&c,now+119999));assert(pd_input_check_tick(&c,now+120000)&&c.mode==PD_INPUT_IDLE);
    before=c;assert(!pd_input_check_touch(&c,0)&&!pd_input_check_keys(&c,0,0,now,true)&&!memcmp(&c,&before,sizeof c));
    puts("Input check: ordered touch targets, held/bounce/disabled/unavailable inputs, three gestures, reserved PWR long, saturation, session reset and timeout/wrap passed (synthetic)");
}
