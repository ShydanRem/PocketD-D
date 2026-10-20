#include "buttons.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>
static pd_buttons keys;
static unsigned sample(int p,int b,uint32_t t){return pd_buttons_sample(&keys,p,b,t,true);}
static void arm(uint32_t t){assert(!sample(0,0,t));assert(!sample(0,0,t+31));}
int main(void){
 arm(0);assert(!sample(1,0,100));assert(!sample(0,0,110));assert(!sample(0,0,141)); /* Bounce only. */
 assert(!sample(1,0,200));assert(!sample(1,0,231));assert(!sample(0,0,400));assert(sample(0,0,431)==1);assert(!sample(0,0,500));
 assert(!sample(1,0,600));assert(!sample(1,0,631));assert(!sample(1,0,1500));assert(!sample(0,0,1600));assert(!sample(0,0,1631)); /* Long PWR reserved. */
 assert(!sample(0,1,2000));assert(!sample(0,1,2031));assert(!sample(0,0,2200));assert(sample(0,0,2231)==2);
 assert(!sample(0,1,3000));assert(!sample(0,1,3031));assert(!sample(0,1,4000));assert(!sample(0,0,4100));assert(sample(0,0,4131)==4);
 keys=(pd_buttons){0};assert(!sample(1,1,0));assert(!sample(1,1,1000));assert(!sample(0,0,1100));assert(!sample(0,0,1131)); /* Held at boot. */
 assert(!sample(0,1,1200));assert(!sample(0,1,1231));assert(!pd_buttons_sample(&keys,0,1,1300,false));assert(!sample(0,0,1400));assert(!sample(0,0,1431));
 assert(!sample(0,1,1500));assert(!sample(0,1,1531));assert(!sample(-1,-1,1600));assert(!sample(0,0,1700));assert(!sample(0,0,1731));
 keys=(pd_buttons){0};arm(UINT_MAX-400);assert(!sample(0,1,UINT_MAX-200));assert(!sample(0,1,UINT_MAX-169));assert(!sample(0,0,100));assert(sample(0,0,131)==2);
 puts("Buttons: debounce, boot-held/reconnect/modal suppression, short/long separation, no repeats, reserved PWR long and timer wrap passed");
}
