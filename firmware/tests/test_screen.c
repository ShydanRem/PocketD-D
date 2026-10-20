#include "screen.h"
#include <assert.h>
#include <stdio.h>
static unsigned calls,last;static bool fail;
static bool set(unsigned value,void *unused){(void)unused;calls++;last=value;return !fail;}
int main(void){
 pd_screen s={0};pd_screen_step(&s,50,1,false,0,set,0);assert(last==50&&calls==1&&!s.dimmed);
 pd_screen_step(&s,50,1,false,29999,set,0);assert(calls==1);pd_screen_step(&s,50,1,false,30000,set,0);assert(s.dimmed&&last==10&&calls==2);
 assert(!pd_screen_touch(&s,true,true,30001)&&!s.dimmed);pd_screen_step(&s,50,1,false,30001,set,0);assert(last==50);
 assert(!pd_screen_touch(&s,false,false,30002)&&!pd_screen_touch(&s,true,true,30003));
 assert(!pd_screen_touch(&s,true,false,30004));assert(pd_screen_touch(&s,true,true,30005));
 pd_screen_step(&s,50,1,false,90000,set,0);assert(!s.dimmed);pd_screen_touch(&s,true,false,90001);
 pd_screen_step(&s,50,1,true,150000,set,0);assert(!s.dimmed);pd_screen_step(&s,50,1,false,180000,set,0);assert(s.dimmed);
 assert(!pd_screen_keys(&s,-1,1,180001)&&!s.dimmed);assert(!pd_screen_keys(&s,-1,1,180002));assert(!pd_screen_keys(&s,-1,0,180003));assert(pd_screen_keys(&s,-1,0,180004));
 assert(!pd_screen_touch(&s,true,true,180005));pd_screen_touch(&s,true,false,180006);assert(pd_screen_touch(&s,true,true,180007));pd_screen_touch(&s,true,false,180008);
 s=(pd_screen){.activity=UINT32_MAX-10000};pd_screen_step(&s,70,1,false,19998,set,0);assert(!s.dimmed);pd_screen_step(&s,70,1,false,19999,set,0);assert(s.dimmed);
 pd_screen_step(&s,70,0,false,20000,set,0);assert(!s.dimmed&&last==70);
 fail=true;unsigned before=calls;pd_screen_step(&s,80,0,false,20001,set,0);assert(s.failed&&s.applied==70&&calls==before+1);
 pd_screen_step(&s,80,0,false,21000,set,0);assert(calls==before+1);fail=false;pd_screen_step(&s,80,0,false,21001,set,0);assert(!s.failed&&s.applied==80);
 before=calls;pd_screen_step(&s,0,0,false,40000,set,0);pd_screen_step(&s,80,4,false,40000,set,0);assert(calls==before);
 pd_screen_step(&s,10,1,false,100000,set,0);assert(!s.dimmed&&s.applied==10);
 for(unsigned policy=1;policy<=3;policy++){
  pd_screen t={0};unsigned ms=(const unsigned[]){0,30000,60000,120000}[policy];
  pd_screen_step(&t,60,policy,false,ms-1,set,0);assert(!t.dimmed);pd_screen_step(&t,60,policy,false,ms,set,0);assert(t.dimmed);
 }
 pd_screen_step(&s,60,1,false,130000,set,0);assert(s.dimmed);assert(!pd_screen_keys(&s,1,0,130001)&&!s.dimmed);
 assert(!pd_screen_keys(&s,0,1,130002));assert(!pd_screen_keys(&s,0,0,130003));assert(pd_screen_keys(&s,0,0,130004));
 puts("Screen: timeout boundaries/wrap, held touch, modal lock, wake quarantine, touch errors, both keys, retry and no repeated writes passed");
}
