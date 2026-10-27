#include "damage.h"
#include "magic.h"
#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;
static uint32_t rng(void *p){(void)p;calls++;return 0;}
static void setup(pd_state *s){
    pd_demo(s);s->hp=s->max_hp=999;s->temp_hp=7;s->defences.configured=8191;
    for(unsigned i=0;i<9;i++){s->magic.slots[i]=(pd_slot){9,9};s->magic.pact[i]=(pd_slot){9,9};}
    s->magic.count=1;s->magic.powers[0]=(pd_power){.name="Magia sintetica",.flags=PD_PREPARED|PD_CONCENTRATES};s->magic.concentration=1;
}
static unsigned sub(unsigned a,unsigned b){return a>b?a-b:0;}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
    static pd_state s,before,next,out;pd_damage_event_summary r,unchanged;unsigned cases=0;const unsigned amounts[]={0,3,31};
    /* Independent costs/balances for both pools, every level/profile/phase and reaction. */
    for(unsigned slot=0;slot<3;slot++)for(unsigned stage=0;stage<2;stage++)for(unsigned kind=PD_COST_SLOT;kind<=PD_COST_PACT;kind++)for(unsigned level=0;level<9;level++)for(unsigned cost=1;cost<=9;cost++)for(unsigned balance=0;balance<=9;balance++)for(unsigned reaction=0;reaction<2;reaction++)for(unsigned spent=0;spent<2;spent++)for(unsigned a=0;a<3;a++)for(unsigned immune=0;immune<2;immune++){
        setup(&s);(kind==PD_COST_PACT?s.magic.pact:s.magic.slots)[level].current=balance;s.reaction_spent=spent;if(immune)assert(pd_defence_set(&s,0,true,PD_IMMUNITY));
        assert(pd_reduction_profile_set(&s,slot,(pd_reduction_profile){.value=3,.types=8191,.stage=stage,.active=true,.cost_kind=kind,.cost_index=level,.cost=cost,.reaction=reaction}));before=s;
        pd_damage_event e={.count=1,.concentration=PD_CONCENTRATION_KEEP,.components={{0,amounts[a]}},.profile=slot+1,.allocation={stage?0:3}};
        assert(pd_damage_event_preview(&s,&e,&r));bool enough=balance>=cost&&(!reaction||!spent);unsigned damage=immune?0:sub(amounts[a],3);
        assert(r.cost_missing==(balance<cost)&&r.reaction_missing==(reaction&&spent)&&r.total.configured==enough&&r.costs[0].before==balance&&r.costs[0].after==(enough?balance-cost:balance)&&r.reaction_after==(enough?(spent||reaction):spent));
        char name[41];snprintf(name,sizeof name,"Slot %s livello %u",kind==PD_COST_PACT?"patto":"ordinario",level+1);assert(!strcmp(name,r.costs[0].name));
        if(enough){assert(r.total.allowed&&r.total.adjusted==damage);assert(pd_damage_event_apply(&s,&e,&r));next=before;(kind==PD_COST_PACT?next.magic.pact:next.magic.slots)[level].current-=cost;next.reaction_spent=spent||reaction;next.temp_hp=sub(7,damage);next.hp=sub(999,sub(damage,7));assert(!memcmp(&s,&next,sizeof s));}
        else{unchanged=r;next=s;pd_roll roll;calls=0;e.concentration=0;assert(!pd_damage_event_apply(&s,&e,&r)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&unchanged,sizeof r));assert(!pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&!calls&&!memcmp(&next,&before,sizeof next));}
        cases++;
    }
    /* Capacity limits and unavailable/nonexistent levels, including magic not imported. */
    for(unsigned kind=PD_COST_SLOT;kind<=PD_COST_PACT;kind++)for(unsigned level=0;level<9;level++)for(unsigned maximum=0;maximum<=9;maximum++)for(unsigned cost=0;cost<=10;cost++){
        setup(&s);(kind==PD_COST_PACT?s.magic.pact:s.magic.slots)[level]=(pd_slot){0,maximum};before=s;
        bool valid=maximum&&cost&&cost<=maximum;
        assert(pd_reduction_cost_max(&s,kind,level)==maximum&&!pd_reduction_cost_current(&s,kind,level));
        assert(pd_reduction_profile_set(&s,0,(pd_reduction_profile){.cost_kind=kind,.cost_index=level,.cost=cost})==valid);if(!valid)assert(!memcmp(&s,&before,sizeof s));
    }
    setup(&s);for(unsigned kind=PD_COST_SLOT;kind<=PD_COST_PACT;kind++){assert(!pd_reduction_cost_max(&s,kind,9)&&!pd_reduction_cost_current(&s,kind,255)&&!pd_reduction_cost_name(&s,kind,9));}
    assert(!pd_reduction_cost_max(NULL,PD_COST_SLOT,0)&&!pd_reduction_cost_current(NULL,PD_COST_PACT,0)&&!pd_reduction_cost_name(NULL,PD_COST_SLOT,0)&&!pd_reduction_cost_name(&s,PD_COST_COUNT,0));
    s.magic.count=s.magic.concentration=0;s.magic.available=false;before=s;for(unsigned kind=PD_COST_SLOT;kind<=PD_COST_PACT;kind++)assert(!pd_reduction_cost_max(&s,kind,0)&&!pd_reduction_profile_set(&s,0,(pd_reduction_profile){.cost_kind=kind,.cost=1})&&!memcmp(&s,&before,sizeof s));
    /* Selected empty slots never fall back to another level or to the other pool. */
    setup(&s);s.magic.slots[0].current=0;assert(pd_reduction_profile_set(&s,0,(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.cost_kind=PD_COST_SLOT,.cost=1}));pd_damage_event e={.count=1,.concentration=PD_CONCENTRATION_KEEP,.components={{0,11}},.profile=1};before=s;assert(pd_damage_event_preview(&s,&e,&r)&&r.cost_missing&&!pd_damage_event_apply(&s,&e,&r)&&!memcmp(&s,&before,sizeof s));
    /* One rolled candidate pays two level-5 pact slots and ends the bound profile. */
    setup(&s);s.magic.pact[4]=(pd_slot){2,2};assert(pd_reduction_profile_set(&s,0,(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.concentration=1,.cost_kind=PD_COST_PACT,.cost_index=4,.cost=2,.reaction=true}));before=s;e=(pd_damage_event){.count=1,.components={{0,31}},.profile=1};pd_roll roll;calls=0;
    assert(pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&calls==1&&!memcmp(&s,&before,sizeof s)&&next.hp==978&&!next.temp_hp&&!next.magic.pact[4].current&&next.magic.slots[4].current==9&&next.reaction_spent&&!next.magic.concentration&&!next.reductions[0].active&&r.costs[0].before==2&&!r.costs[0].after&&!strcmp(r.costs[0].name,"Slot patto livello 5"));
    /* Round/turn do not replenish slots; short/long rest preserve their separate policies. */
    s=next;s.magic.slots[4].current=2;pd_round_summary round;assert(pd_round_advance(&s,&round)&&!s.magic.pact[4].current&&s.magic.slots[4].current==2&&s.reaction_spent);assert(pd_reaction_change(&s,false)&&!s.magic.pact[4].current&&s.magic.slots[4].current==2);assert(pd_rest_apply(&s,&(pd_rest_plan){0})&&s.magic.pact[4].current==2&&s.magic.slots[4].current==2);s.magic.pact[4].current=0;assert(pd_rest_apply(&s,&(pd_rest_plan){.long_rest=true})&&s.magic.pact[4].current==2&&s.magic.slots[4].current==9);
    /* Strict PDD23 cost enums; old PDD17 rejects new meanings. */
    static uint8_t data[PD_RECORD_SIZE];uint32_t gen;setup(&s);s.magic.count=120;for(unsigned i=1;i<120;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};
    for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&s,i,(pd_reduction_profile){.value=3,.types=8191,.active=true,.cost_kind=i==1?PD_COST_PACT:PD_COST_SLOT,.cost_index=i*4,.cost=9,.reaction=true}));
    size_t n=pd_encode(&s,180,data);assert(n==PD_RECORD_SIZE&&!memcmp(data,"PDDV",4)&&pd_decode(data,n,&out,&gen)&&gen==180&&out.reductions[2].cost_index==8);
    for(unsigned i=0;i<3;i++)for(unsigned field=0;field<4;field++){pd_encode(&s,180,data);data[n-586+i*5+field]=(const unsigned[]){PD_COST_COUNT,9,10,255}[field];crc(data,n);before=out;gen=17;assert(!pd_decode(data,n,&out,&gen)&&!memcmp(&out,&before,sizeof out)&&gen==17);}
    pd_encode(&s,180,data);size_t old_n=n-566;data[3]='H';crc(data,old_n);assert(pd_record_known(data,old_n)&&!pd_decode(data,old_n,&out,&gen));data[3]='Q';crc(data,old_n);assert(!pd_record_known(data,old_n)&&!pd_decode(data,old_n,&out,&gen));
    printf("Slot costs: %u independent slot/stage/pool/level/cost/balance/reaction/damage cases, capacity validation, no fallback, one CON candidate, rest policies and strict PDD24 passed\n",cases);
}
