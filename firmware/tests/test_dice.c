#include "dice_animation.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
/* Renderer guards cover all 1..20 faces, modes, flight phases, bursts and uint32 wrap. */
static struct { uint32_t before;uint16_t pixels[PD_DICE_PIXELS];uint32_t after; } guarded;
static uint32_t hash(void) { return pd_crc32((const uint8_t*)guarded.pixels,sizeof guarded.pixels); }
static void paint(pd_dice_animation *a) {
    guarded.before=0x12345678;guarded.after=0xabcdef01;
    pd_dice_paint(a,guarded.pixels);assert(guarded.before==0x12345678&&guarded.after==0xabcdef01);
}
int main(void) {
    pd_dice_animation a;pd_dice_idle(&a,1);paint(&a);
    assert(a.preview&&!a.active&&pd_dice_position(&a,0).x==160);
    for(unsigned count=1;count<=2;count++)for(unsigned n=1;n<=20;n++) {
        pd_roll r={.count=count,.dice={n,21-n},.natural=n,.bonus=3,.total=n+3};
        pd_dice_start(&a,&r,UINT32_MAX-100);
        assert(!pd_dice_tick(&a,UINT32_MAX-100));paint(&a);uint32_t first=hash();
        for(uint32_t t=0;t<pd_dice_duration(&a);t+=17) {
            assert(!pd_dice_tick(&a,(UINT32_MAX-100)+t));paint(&a);
            assert(a.active);assert(!memcmp(&a.roll,&r,sizeof r));
            for(unsigned d=0;d<count;d++) { pd_dice_pose p=pd_dice_position(&a,d);assert(p.frame>=0&&p.frame<24&&p.y>=38&&p.y<=68); }
        }
        assert(pd_dice_tick(&a,(UINT32_MAX-100)+pd_dice_duration(&a)));
        assert(!a.active);paint(&a);assert(first!=hash());
        for(unsigned d=0;d<count;d++) { pd_dice_pose p=pd_dice_position(&a,d);assert(p.settled&&p.frame==0); }
        uint32_t final=hash();assert(!pd_dice_tick(&a,123456));paint(&a);assert(hash()==final);
        pd_dice_start(&a,&r,0);pd_dice_tick(&a,333);pd_dice_finish(&a);paint(&a);
        assert(hash()==final&&!memcmp(&a.roll,&r,sizeof r));
    }
    /* Ties consistently choose the first die; a higher second die highlights that die. */
    pd_roll tied={.count=2,.dice={10,10},.natural=10,.bonus=0,.total=10};
    pd_dice_start(&a,&tied,0);pd_dice_finish(&a);paint(&a);
    assert(guarded.pixels[121*PD_DICE_WIDTH+102]!=guarded.pixels[0]);
    assert(guarded.pixels[121*PD_DICE_WIDTH+218]==guarded.pixels[0]);
    tied.dice[1]=20;tied.natural=20;tied.total=20;
    pd_dice_start(&a,&tied,0);pd_dice_finish(&a);paint(&a);
    assert(guarded.pixels[121*PD_DICE_WIDTH+218]!=guarded.pixels[0]);
    assert(guarded.pixels[121*PD_DICE_WIDTH+102]==guarded.pixels[0]);
    puts("Dice presentation: all faces, one/two dice, wrap, bounds guards, motion, final frame, skip preserves outcome passed");
}
