#include "spell_roll.h"
#include "combat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state s,before,next,expected;
static unsigned calls,value;
static uint32_t fixed(void *unused){(void)unused;calls++;return value;}
static void setup(void){pd_demo(&s);s.hp=10;s.magic=(pd_magic){.available=true,.count=1};s.magic.powers[0]=(pd_power){.name="Synthetic spell",.level=1,.ability=3,.flags=PD_PREPARED|PD_USES_SLOT|PD_LIMITED,.maximum=2,.remaining=1,.attack_modifier=2,.save_modifier=1};s.magic.slots[0]=(pd_slot){2,2};s.magic.pact[0]=(pd_slot){1,1};assert(pd_valid(&s));}
int main(void){
    pd_spell_roll_plan p={.power=0,.slot=1,.kind=PD_SPELL_DAMAGE,.count=2,.sides=8,.bonus=3};unsigned cases=0;
    for(unsigned ability=0;ability<6;ability++)for(unsigned level=1;level<=20;level+=5)for(unsigned natural=1;natural<=20;natural++)for(int mode=-1;mode<=1;mode++)for(unsigned armor=1;armor<=40;armor+=13)for(unsigned pact=0;pact<2;pact++){
        setup();s.level=level;s.magic.powers[0].ability=ability;s.abilities[ability]=18;before=s;p.pact=pact;value=natural-1;unsigned draws=calls;
        pd_spell_battle_plan b={.mode=PD_SPELL_ATTACK,.armor=armor,.requested=mode};pd_spell_attack_result hit;
        expected=s;assert(pd_magic_use(&expected,0,1,pact));assert(pd_spell_attack_cast(&s,&p,&b,fixed,NULL,&next,&hit));
        int bonus=4+pd_proficiency(level)+2;assert(hit.roll.bonus==bonus&&hit.roll.total==(int)natural+bonus&&hit.roll.natural==natural&&hit.roll.count==(mode?2:1));
        assert(hit.critical==(natural==20)&&hit.hits==(natural!=1&&(natural==20||(int)natural+bonus>=(int)armor))&&calls==draws+(mode?2u:1u)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&next,&expected,sizeof next));cases++;
    }
    /* Spell/weapon attack gates share all represented conditions and Boolean
       advantage cancellation; no STR rage or weapon bonuses enter a spell. */
    setup();for(unsigned flags=0;flags<16;flags++)for(unsigned poison=0;poison<2;poison++)for(int requested=-1;requested<=1;requested++)for(unsigned fear=0;fear<3;fear++){
        s.roll_conditions=flags;s.training.poisoned=poison;pd_roll_context c={.known=fear?PD_CONTEXT_FEAR:0,.values=fear==1?PD_CONTEXT_FEAR:0};pd_attack_resolution weapon;int effective;unsigned needed;
        assert(pd_attack_resolve(&s,requested,c,false,&weapon)&&pd_spell_attack_gate(&s,requested,c,&effective,&needed)&&effective==weapon.mode&&needed==weapon.needed);
    }
    setup();p.pact=false;s.roll_conditions=PD_FRIGHTENED|PD_BLINDED;s.training.poisoned=true;
    for(unsigned ability=0;ability<6;ability++)for(unsigned success=1;success<=2;success++){
        pd_spell_battle_plan b={.mode=PD_SPELL_TARGET_SAVE,.save_ability=ability,.success=success};before=s;unsigned draws=calls;assert(pd_spell_battle_prepare(&s,&p,&b,&next)&&calls==draws&&!memcmp(&s,&before,sizeof s)&&next.magic.slots[0].current==1&&next.magic.powers[0].remaining==0);
    }
    setup();before=s;pd_spell_attack_result untouched={.roll={.total=999}},hit=untouched;pd_spell_battle_plan b={.mode=PD_SPELL_ATTACK,.armor=15};
    for(unsigned bad=0;bad<10;bad++){
        pd_spell_battle_plan q=b;pd_spell_roll_plan r=p;
        switch(bad){case 0:q.armor=0;break;case 1:q.armor=41;break;case 2:q.requested=2;break;case 3:q.context.known=4;break;case 4:q.success=1;break;case 5:q.save_ability=1;break;case 6:r.kind=PD_SPELL_HEAL_SELF;break;case 7:r.power=120;break;case 8:r.count=0;break;case 9:r.bonus=61;break;}
        next=before;unsigned draws=calls;assert(!pd_spell_attack_cast(&s,&r,&q,fixed,NULL,&next,&hit)&&calls==draws&&!memcmp(&next,&before,sizeof next)&&!memcmp(&hit,&untouched,sizeof hit));
    }
    s.roll_conditions=PD_FRIGHTENED;before=s;next=s;unsigned draws=calls;assert(!pd_spell_attack_cast(&s,&p,&b,fixed,NULL,&next,&hit)&&calls==draws&&!memcmp(&next,&before,sizeof next));
    /* Concentration costs can lower HP/max before an attack, on hits or misses. */
    setup();s.max_hp=30;s.hp=32;s.magic.count=2;s.magic.powers[1]=(pd_power){.name="Old concentration",.flags=PD_PREPARED|PD_CONCENTRATES};s.magic.concentration=2;s.magic.powers[0].flags|=PD_CONCENTRATES;s.effect_count=1;s.effects[0]=(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=2};value=0;
    assert(pd_valid(&s)&&pd_spell_attack_cast(&s,&p,&b,fixed,NULL,&next,&hit)&&!hit.hits&&next.hp==30&&pd_max_hp(&next)==30&&next.magic.concentration==1&&next.magic.slots[0].current==1);
    unsigned impacts=0;for(unsigned die=0;die<6;die++)for(unsigned count=1;count<=18;count++)for(int bonus=-15;bonus<=60;bonus+=15)for(unsigned critical=0;critical<2;critical++)for(unsigned impact=0;impact<3;impact++){
        unsigned sides=(const unsigned[]){4,6,8,10,12,20}[die];p=(pd_spell_roll_plan){.kind=PD_SPELL_DAMAGE,.count=count,.sides=sides,.bonus=bonus};pd_spell_impact result={.rolled=999},old=result;value=0;unsigned draws=calls;
        bool ok=pd_spell_impact_roll(&p,critical,impact,fixed,NULL,&result);
        if(critical&&impact){assert(!ok&&calls==draws&&!memcmp(&result,&old,sizeof result));continue;}
        assert(ok);if(impact==PD_SPELL_ZERO)assert(!result.first.count&&!result.second.count&&!result.applicable&&calls==draws);
        else {unsigned dice=count*(critical?2:1);int total=(int)dice+bonus;if(total<0)total=0;assert(result.rolled==total&&result.applicable==(impact==PD_SPELL_HALF?total/2:total)&&calls==draws+dice&&result.first.bonus==bonus&&result.second.bonus==0&&result.first.count+result.second.count==dice);}
        impacts++;
    }
    printf("Spell battle: %u attack/payment cases, condition parity, external save/no RNG, atomic rejection, concentration cap loss and %u full/half/zero/critical impacts with bonus once and split36 passed; synthetic.\n",cases,impacts);
}
