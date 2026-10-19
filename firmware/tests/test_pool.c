#include "dice_pool.h"
#include "dice_animation.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static uint32_t values[32];static unsigned used;
static uint32_t random_value(void *context) { (void)context;assert(used<32);return values[used++]; }
static struct { uint32_t before;uint16_t pixels[PD_DICE_PIXELS];uint32_t after; } guarded;
static void paint(pd_dice_animation *a) {
    guarded.before=0x12345678;guarded.after=0xabcdef01;
    pd_dice_paint(a,guarded.pixels);
    assert(guarded.before==0x12345678&&guarded.after==0xabcdef01);
}
int main(void) {
    unsigned sides[]={4,6,8,10,12,20};pd_pool r,before;pd_dice_animation a;
    for(unsigned s=0;s<6;s++)for(unsigned count=1;count<=6;count++) {
        for(unsigned i=0;i<32;i++)values[i]=sides[s]-1;
        used=0;assert(pd_roll_pool(sides[s],count,30,false,random_value,NULL,&r));
        assert(used==count&&r.sum==(int)(count*sides[s])&&r.total==r.sum+30);
        for(unsigned i=0;i<count;i++)assert(r.dice[i]==sides[s]);
        pd_dice_pool_start(&a,&r,UINT32_MAX-100);before=r;
        for(unsigned t=0;t<pd_dice_duration(&a);t+=213) {
            assert(!pd_dice_tick(&a,(UINT32_MAX-100)+t));paint(&a);
            assert(!memcmp(&a.pool,&before,sizeof before));
            for(unsigned i=0;i<count;i++) { pd_dice_pose p=pd_dice_position(&a,i);assert(p.frame>=0&&p.frame<24); }
        }
        assert(pd_dice_tick(&a,(UINT32_MAX-100)+pd_dice_duration(&a)));paint(&a);
        uint32_t final=pd_crc32((uint8_t*)guarded.pixels,sizeof guarded.pixels);
        pd_dice_pool_start(&a,&r,0);pd_dice_tick(&a,140);pd_dice_finish(&a);paint(&a);
        assert(pd_crc32((uint8_t*)guarded.pixels,sizeof guarded.pixels)==final);
        /* Every possible face is rendered at rest for every quantity and shape. */
        for(unsigned face=1;face<=sides[s];face++) {
            for(unsigned i=0;i<count;i++)r.dice[i]=face;
            pd_dice_pool_start(&a,&r,0);pd_dice_finish(&a);paint(&a);
        }
        memset(values,0,sizeof values);used=0;
        assert(pd_roll_pool(sides[s],count,-15,false,random_value,NULL,&r));assert(r.total==(int)count-15);
        used=0;assert(pd_roll_pool(sides[s],count,-15,true,random_value,NULL,&r));assert(r.total==0&&r.sum==(int)count);
        /* Exact upper bound is accepted for d4/d8; other dice reject the bias tail. */
        values[0]=UINT32_MAX;values[1]=sides[s]-1;used=0;
        assert(pd_roll_pool(sides[s],1,0,false,random_value,NULL,&r));
        assert(used==((sides[s]==4||sides[s]==8)?1:2));assert(r.dice[0]==sides[s]);
    }
    before=r;unsigned calls=used;
    assert(!pd_roll_pool(3,1,0,false,random_value,NULL,&r));
    assert(!pd_roll_pool(6,0,0,false,random_value,NULL,&r));
    assert(!pd_roll_pool(6,7,0,false,random_value,NULL,&r));
    assert(!pd_roll_pool(6,1,INT_MIN,false,random_value,NULL,&r));
    assert(!pd_roll_pool(6,1,INT_MAX,false,random_value,NULL,&r));
    assert(!pd_roll_pool(6,1,0,false,NULL,NULL,&r));
    assert(!pd_roll_pool(6,1,0,false,random_value,NULL,NULL));
    assert(used==calls&&!memcmp(&before,&r,sizeof r));
    puts("Poly dice: six shapes, 1..6 dice, every face, signed sums, damage floor, bias-tail/power-of-two, invalid input, wrap, buffer guards and skip passed");
}
