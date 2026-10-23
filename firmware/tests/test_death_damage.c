#include "damage.h"
#include "status.h"
#include "rest.h"
#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned calls;
static uint32_t rng(void *unused){(void)unused;calls++;return 0;}
static unsigned reduced(unsigned n,unsigned flags){if(flags&4)return 0;if(flags&1)n/=2;if(flags&2)n*=2;return n;}
int main(void){
    static pd_state s,before,expected,next,decoded;
    pd_death_damage_summary r,original;unsigned cases=0;
    const unsigned hps[]={0,1,24,32},temps[]={0,3,32,99},amounts[]={0,1,2,3,31,32,33,55,56,57,9999};
    for(unsigned hp=0;hp<4;hp++)for(unsigned t=0;t<4;t++)for(unsigned a=0;a<11;a++)for(unsigned flags=0;flags<8;flags++)for(unsigned registry=0;registry<8;registry++)for(unsigned critical=0;critical<3;critical++)for(unsigned verified=0;verified<2;verified++){
        pd_demo(&s);s.hp=hps[hp];s.temp_hp=temps[t];assert(pd_defence_set(&s,0,true,flags));
        s.status.available=registry!=0;
        if(registry==2)s.status.failures=2;
        if(registry==3){s.status.successes=2;s.status.failures=1;}
        if(registry==4)s.status.stabilized=true;
        if(registry==5)s.status.failures=3;
        if(registry==6)s.status.successes=3;
        if(registry==7){s.status.stabilized=true;s.status.failures=1;}
        before=s;expected=s;assert(pd_valid(&s));
        unsigned damage=reduced(amounts[a],flags),absorbed=damage<s.temp_hp?damage:s.temp_hp;
        unsigned leftover=damage-absorbed;
        bool eligible=damage&&(!s.hp||leftover>=s.hp+32u);
        unsigned excess=s.hp?(leftover>s.hp?leftover-s.hp:0):damage;
        bool instant=eligible&&excess>=32;
        bool conflict=!s.hp&&(registry==6||registry==7),dead=!s.hp&&registry==5;
        bool allowed=eligible&&registry&&!conflict&&!dead&&verified&&(instant||critical);
        pd_damage_event event={.count=1,.components={{0,amounts[a]}}};
        assert(pd_death_damage_preview(&s,&event,(pd_death_damage_choice){critical,verified},&r));
        assert(r.eligible==eligible&&r.allowed==allowed&&r.instant==instant&&r.registry_conflict==conflict&&r.already_dead==dead);
        assert(r.impact.total.adjusted==damage&&r.impact.total.absorbed==absorbed&&r.maximum==32&&!memcmp(&s,&before,sizeof s));
        original=r;
        if(allowed){
            expected.hp=0;expected.temp_hp-=absorbed;expected.status.stabilized=false;
            if(instant){expected.status.failures=3;expected.status.successes=0;}
            else{unsigned n=expected.status.failures+(critical==PD_HIT_CRITICAL?2:1);expected.status.failures=n>3?3:n;}
            assert(pd_death_damage_apply(&s,&event,(pd_death_damage_choice){critical,verified},&r));
            assert(!memcmp(&s,&expected,sizeof s)&&!memcmp(&r,&original,sizeof r)&&pd_valid(&s));
        }else assert(!pd_death_damage_apply(&s,&event,(pd_death_damage_choice){critical,verified},&r)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&original,sizeof r));
        if(eligible){pd_damage_event_summary old;next=before;assert(!pd_damage_event_apply(&next,&event,&old)&&!memcmp(&next,&before,sizeof next));}
        cases++;
    }
    /* Six components of one impact cause one critical failure event, not six. */
    pd_demo(&s);s.hp=0;s.temp_hp=99;s.status.available=true;s.status.successes=2;s.defences.configured=8191;
    pd_damage_event event={.count=6};for(unsigned i=0;i<6;i++)event.components[i]=(pd_damage_component){i,1};
    assert(pd_death_damage_apply(&s,&event,(pd_death_damage_choice){PD_HIT_CRITICAL,true},&r)&&r.failures_added==2&&s.status.failures==2&&s.status.successes==2&&s.temp_hp==93);
    /* At zero, even temporary absorption counts; instant threshold uses the
       effective damage. Above zero, only excess beyond temporary/current HP. */
    event=(pd_damage_event){.count=1,.components={{0,32}}};s.temp_hp=99;
    assert(pd_death_damage_apply(&s,&event,(pd_death_damage_choice){0,true},&r)&&r.instant&&s.status.failures==3&&s.temp_hp==67);
    before=s;pd_hp(&s,32);assert(!memcmp(&s,&before,sizeof s));
    pd_rest_plan rest={.long_rest=true};assert(!pd_rest_apply(&s,&rest)&&!memcmp(&s,&before,sizeof s));
    pd_roll roll;assert(!pd_death_roll(&s,rng,NULL,&next,&roll)&&!calls);
    static uint8_t data[PD_RECORD_SIZE];uint32_t gen;size_t size=pd_encode(&s,123,data);
    assert(pd_decode(data,size,&decoded,&gen)&&gen==123&&!memcmp(&s,&decoded,sizeof s));
    /* End linked max-HP/defence effects only after evaluating the hit. */
    pd_demo(&s);s.status.available=true;s.temp_hp=3;s.magic.available=true;s.magic.count=1;
    s.magic.powers[0]=(pd_power){.name="Concentrazione",.flags=PD_PREPARED|PD_CONCENTRATES};s.magic.concentration=1;s.defences.configured=8191;
    assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=1}));
    event=(pd_damage_event){.count=1,.components={{0,69}}};
    assert(pd_death_damage_apply(&s,&event,(pd_death_damage_choice){0,true},&r)&&r.maximum==42&&r.excess==42&&r.impact.total.max_hp_after==32&&!s.magic.concentration&&!s.effects[0].active&&s.status.failures==3&&pd_valid(&s));
    /* Existing three-expense cost/reaction machinery is used exactly once. */
    pd_demo(&s);s.status.available=true;s.defences.configured=8191;s.magic.available=true;
    s.magic.slots[0]=(pd_slot){3,3};s.items[1].charges=(pd_charges){3,3,0};
    s.reductions[0]=(pd_reduction_profile){.value=1,.types=8191,.active=true,.stage=1,.reaction=true,.cost_kind=PD_COST_COUNTER,.cost_index=0,.cost=1,
        .extra_costs={{PD_COST_SLOT,0,1},{PD_COST_ITEM,1,1}}};
    event=(pd_damage_event){.count=1,.components={{0,57}},.profile=1};assert(pd_valid(&s));
    before=s;assert(pd_death_damage_apply(&s,&event,(pd_death_damage_choice){0,true},&r)&&r.instant&&s.status.failures==3&&!s.resources[0].current&&s.magic.slots[0].current==2&&s.items[1].charges.current==2&&s.reaction_spent);
    s=before;s.hp=0;assert(pd_death_damage_preview(&s,&event,(pd_death_damage_choice){PD_HIT_NORMAL,true},&r)&&r.reaction_at_zero&&!r.allowed);
    before=s;assert(!pd_death_damage_apply(&s,&event,(pd_death_damage_choice){1,true},&r)&&!memcmp(&s,&before,sizeof s));
    s.reductions[0].reaction=false;s.resources[0].current=0;before=s;
    assert(pd_death_damage_preview(&s,&event,(pd_death_damage_choice){1,true},&r)&&r.impact.cost_missing&&!r.allowed&&!r.eligible);
    assert(!pd_death_damage_apply(&s,&event,(pd_death_damage_choice){1,true},&r)&&!memcmp(&s,&before,sizeof s));
    /* Unknown defences/source facts, malformed events and invalid choices never
       change either output or state. No random source participates in apply. */
    pd_demo(&s);s.hp=0;s.status.available=true;event=(pd_damage_event){.count=1,.components={{0,1}}};before=s;
    assert(pd_death_damage_preview(&s,&event,(pd_death_damage_choice){1,true},&r)&&!r.eligible&&!r.allowed);
    assert(pd_defence_set(&s,0,true,PD_RESISTANCE)&&pd_defence_rule_set(&s,0,false,0,PD_RULE_NONMAGICAL));
    assert(pd_death_damage_preview(&s,&event,(pd_death_damage_choice){1,true},&r)&&r.impact.context_missing&&!r.allowed);
    before=s;memset(&r,0x55,sizeof r);original=r;
    assert(!pd_death_damage_preview(&s,&event,(pd_death_damage_choice){3,true},&r)&&!memcmp(&r,&original,sizeof r));
    event.count=7;assert(!pd_death_damage_apply(&s,&event,(pd_death_damage_choice){1,true},&r)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&original,sizeof r));
    assert(!pd_death_damage_preview(NULL,&event,(pd_death_damage_choice){0},&r)&&!pd_death_damage_preview(&s,NULL,(pd_death_damage_choice){0},&r)&&!pd_death_damage_apply(&s,&event,(pd_death_damage_choice){0},NULL));
    printf("Death damage: %u independent HP/temp/type/registry/critical/confirmation cases, one impact/six parts, exact instant thresholds, linked effects, atomic three costs, reaction-at-zero guards, persistence and dead recovery blocks passed\n",cases);
}
