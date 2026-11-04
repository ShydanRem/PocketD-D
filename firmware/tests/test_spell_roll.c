#include "spell_roll.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state s,before,next,expected,untouched;
static unsigned calls;
static uint32_t maximum(void *context){calls++;return *(unsigned *)context-1;}
static void fixture(void){
    pd_demo(&s);s.max_hp=30;s.hp=12;s.temp_hp=5;
    s.magic=(pd_magic){.available=true,.count=2};
    s.magic.powers[0]=(pd_power){.name="Explicit synthetic spell",.level=1,.flags=PD_PREPARED|PD_USES_SLOT|PD_LIMITED,.maximum=2,.remaining=1};
    s.magic.powers[1]=(pd_power){.name="Old concentration",.flags=PD_PREPARED|PD_CONCENTRATES};
    for(unsigned i=0;i<9;i++){s.magic.slots[i]=(pd_slot){2,3};s.magic.pact[i]=(pd_slot){1,1};}
    assert(pd_valid(&s));before=s;
}
static void rejected(pd_spell_roll_plan plan){
    pd_pool r={.total=4321},old=r;unsigned gained=8765,draws=calls,sides=6;
    next=untouched;
    assert(!pd_spell_roll(&s,&plan,maximum,&sides,&next,&r,&gained));
    assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&next,&untouched,sizeof next)&&!memcmp(&r,&old,sizeof r)&&gained==8765&&draws==calls);
}
int main(void){
    unsigned cases=0;const unsigned sizes[]={4,6,8,10,12,20};
    for(unsigned die=0;die<6;die++)for(unsigned count=1;count<=PD_POOL_MAX;count++)for(unsigned kind=1;kind<=3;kind++)for(unsigned pact=0;pact<2;pact++){
        fixture();unsigned sides=sizes[die];pd_spell_roll_plan p={.power=0,.slot=9,.kind=kind,.count=count,.sides=sides,.bonus=3,.pact=pact};
        expected=s;assert(pd_magic_use(&expected,0,9,pact));unsigned gained=0,draws=calls;pd_pool r;
        assert(pd_spell_roll_prepare(&s,&p,&next)&&!memcmp(&s,&before,sizeof s)&&calls==draws&&!memcmp(&next,&expected,sizeof next));
        if(kind==PD_SPELL_HEAL_SELF){unsigned old=expected.hp;pd_hp(&expected,count*sides+3);gained=expected.hp-old;}
        unsigned actual=999;assert(pd_spell_roll(&s,&p,maximum,&sides,&next,&r,&actual)&&actual==gained&&pd_valid(&next)&&!memcmp(&expected,&next,sizeof next)&&!memcmp(&s,&before,sizeof s));
        assert(r.total==(int)(count*sides+3)&&r.count==count&&r.sides==sides&&r.damage&&calls==draws+count&&next.temp_hp==5);
        for(unsigned i=0;i<count;i++)assert(r.dice[i]==sides);
        cases++;
    }
    /* Slot-free/limited casting, negative healing floored at zero; zero does
       not reset death counters or become an unsigned healing amount. */
    fixture();s.magic.powers[0].flags=PD_PREPARED|PD_LIMITED;before=s;pd_spell_roll_plan p={.power=0,.kind=PD_SPELL_HEAL_SELF,.count=1,.sides=4,.bonus=-15};
    pd_pool r;unsigned gained,sides=4;assert(pd_spell_roll(&s,&p,maximum,&sides,&next,&r,&gained)&&!r.total&&!gained&&next.hp==s.hp&&!next.magic.powers[0].remaining&&next.magic.slots[0].current==2);
    /* Ending old concentration can lower both maximum and current HP. Report
       real healing after the cost, without reinterpreting the clamp as damage
       or a negative unsigned gain. */
    fixture();s.magic.concentration=2;s.magic.powers[0].flags|=PD_CONCENTRATES;s.effect_count=1;s.effects[0]=(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=2};s.hp=32;before=s;
    p=(pd_spell_roll_plan){.power=0,.slot=1,.kind=PD_SPELL_HEAL_SELF,.count=1,.sides=8,.bonus=3};sides=8;
    assert(pd_spell_roll(&s,&p,maximum,&sides,&next,&r,&gained)&&next.magic.concentration==1&&next.max_hp==30&&pd_max_hp(&next)==30&&next.hp==30&&!gained&&r.total==11&&pd_valid(&next));
    /* Every rejection happens before entropy and leaves candidate/result intact. */
    fixture();pd_demo(&untouched);p=(pd_spell_roll_plan){.power=0,.slot=1,.kind=PD_SPELL_DAMAGE,.count=1,.sides=6};
    for(unsigned bad=0;bad<13;bad++){
        pd_spell_roll_plan q=p;
        switch(bad){case 0:q.power=PD_POWERS;break;case 1:q.kind=0;break;case 2:q.kind=4;break;case 3:q.count=0;break;case 4:q.count=PD_POOL_MAX+1;break;case 5:q.sides=7;break;case 6:q.bonus=-16;break;case 7:q.bonus=61;break;case 8:q.slot=0;break;case 9:q.slot=10;break;case 10:q.power=1;q.slot=1;break;case 11:q.power=1;q.pact=true;q.slot=0;break;case 12:q.power=1;q.kind=0;break;}
        rejected(q);
    }
    for(unsigned bad=0;bad<9;bad++){
        fixture();switch(bad){case 0:s.magic.powers[0].flags&=~PD_PREPARED;break;case 1:s.magic.slots[0].current=0;break;case 2:s.magic.powers[0].remaining=0;break;case 3:s.hp=0;break;case 4:s.hp=s.max_hp;break;case 5:s.magic.available=false;s.magic.count=0;break;case 6:s.magic.powers[0].effect=PD_DIVINE_SMITE_2024;break;case 7:s.magic.powers[0].effect=255;break;case 8:s.magic.powers[0]=(pd_power){.name="Feature",.kind=1,.maximum=2,.remaining=1};break;}
        before=s;pd_spell_roll_plan q=p;if(bad==4)q.kind=PD_SPELL_HEAL_SELF;rejected(q);
    }
    fixture();unsigned draws=calls;assert(!pd_spell_roll(&s,&p,NULL,&sides,&next,&r,&gained)&&calls==draws);assert(!pd_spell_roll_prepare(&s,&p,&s)&&!memcmp(&s,&before,sizeof s));
    printf("Spell roll: %u explicit formula/payment/target cases, exact atomic candidates, caps/temp HP, slot-free use, zero floor, concentration cap loss, invalid/no-RNG/no-output mutation passed.\n",cases);
}
