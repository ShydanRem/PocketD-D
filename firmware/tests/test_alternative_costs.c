#include "damage.h"
#include "backup.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pd_state s,seed,before,expected,next;
static pd_backup restored;
static uint8_t record[PD_RECORD_SIZE];
static unsigned calls;
static uint32_t rng(void *unused){(void)unused;calls++;return 0;}
static void setup(void){
    pd_demo(&s);s.hp=s.max_hp=100;s.temp_hp=3;s.resources[0].current=s.resources[0].maximum=3;
    s.magic.available=true;s.magic.count=1;s.magic.powers[0]=(pd_power){.name="Limited power",.kind=1,.maximum=3,.remaining=3};
    s.magic.slots[0]=s.magic.pact[0]=(pd_slot){3,3};s.items[2].quantity=1;s.items[2].charges=(pd_charges){3,3,0};s.defences.configured=8191;
}
static void balance(pd_state *state,unsigned kind,unsigned n){
    if(kind==PD_COST_COUNTER)state->resources[0].current=n;
    else if(kind==PD_COST_POWER)state->magic.powers[0].remaining=n;
    else if(kind==PD_COST_SLOT)state->magic.slots[0].current=n;
    else if(kind==PD_COST_PACT)state->magic.pact[0].current=n;
    else state->items[2].charges.current=n;
}
static void crc(size_t n){uint32_t c=pd_crc32(record,n-4);for(unsigned i=0;i<4;i++)record[n-4+i]=c>>(8*i);}
int main(int argc,char **argv){
    assert(argc==1||argc==2);unsigned cases=0;const unsigned kinds[]={PD_COST_COUNTER,PD_COST_POWER,PD_COST_SLOT,PD_COST_PACT,PD_COST_ITEM},values[]={0,1,3};
    pd_damage_event_summary r,out,untouched={.raw=123};
    for(unsigned rotation=0;rotation<5;rotation++)for(unsigned a=0;a<3;a++)for(unsigned b=0;b<3;b++)for(unsigned c=0;c<3;c++)
    for(unsigned choice=0;choice<4;choice++)for(unsigned reaction=0;reaction<2;reaction++)for(unsigned spent=0;spent<2;spent++)
    for(unsigned stage=0;stage<2;stage++)for(unsigned amount=0;amount<3;amount++)for(unsigned parts=1;parts<=6;parts+=5){
        setup();unsigned balances[]={values[a],values[b],values[c]};pd_reduction_profile p={.value=3,.types=8191,.stage=stage,.active=true,.reaction=reaction,.payment=PD_PAYMENT_CHOOSE};
        for(unsigned row=0;row<3;row++){unsigned kind=kinds[(rotation+row)%5];pd_profile_cost_set(&p,row,(pd_cost){kind,kind==PD_COST_ITEM?2:0,2});balance(&s,kind,balances[row]);}
        s.reaction_spent=spent;assert(pd_reduction_profile_set(&s,0,p));before=s;expected=s;
        unsigned raw=(const unsigned[]){0,5,20}[amount],damage=raw>3?raw-3:0;bool allowed=choice&&balances[choice-1]>=2&&(!reaction||!spent);
        pd_damage_event e={.count=parts,.components={{0,raw}},.profile=1,.payment_choice=choice};if(!stage)e.allocation[0]=3;
        assert(pd_damage_event_preview(&s,&e,&r)&&r.total.allowed==allowed&&r.payment_missing==!choice);
        for(unsigned row=0;row<3;row++){assert(r.costs[row].selected==(choice==row+1)&&r.costs[row].before==balances[row]);assert(r.costs[row].after==(allowed&&choice==row+1?balances[row]-2:balances[row]));}
        if(allowed){balance(&expected,kinds[(rotation+choice-1)%5],balances[choice-1]-2);expected.reaction_spent=spent||reaction;expected.temp_hp=damage<3?3-damage:0;expected.hp=100-(damage>3?damage-3:0);}
        out=untouched;assert(pd_damage_event_apply(&s,&e,&out)==allowed&&!memcmp(&s,&expected,sizeof s));if(!allowed)assert(!memcmp(&out,&untouched,sizeof out));cases++;
    }
    setup();pd_reduction_profile p={.value=3,.types=8191,.stage=1,.active=true,.payment=PD_PAYMENT_CHOOSE};
    pd_profile_cost_set(&p,0,(pd_cost){PD_COST_COUNTER,0,2});before=s;assert(!pd_reduction_profile_set(&s,0,p)&&!memcmp(&s,&before,sizeof s));
    pd_profile_cost_set(&p,2,(pd_cost){PD_COST_ITEM,2,2});assert(pd_reduction_profile_set(&s,0,p));s.items[2].quantity=0;
    pd_damage_event e={.count=1,.components={{0,5}},.profile=1,.payment_choice=1};assert(pd_damage_event_preview(&s,&e,&r)&&r.total.allowed&&r.costs[2].unavailable&&!r.cost_unavailable&&!r.costs[2].selected);
    e.payment_choice=2;assert(pd_damage_event_preview(&s,&e,&r)&&r.payment_missing&&!r.total.allowed);
    e.payment_choice=3;assert(pd_damage_event_preview(&s,&e,&r)&&r.cost_unavailable&&!r.total.allowed);
    e.payment_choice=4;out=untouched;assert(!pd_damage_event_preview(&s,&e,&out)&&!memcmp(&out,&untouched,sizeof out));
    e.payment_choice=1;e.profile=0;assert(!pd_damage_event_preview(&s,&e,&out));e.profile=1;
    p.payment=PD_PAYMENT_ALL;assert(pd_reduction_profile_set(&s,0,p));assert(!pd_damage_event_preview(&s,&e,&out));e.payment_choice=0;assert(pd_damage_event_preview(&s,&e,&r)&&!r.total.allowed&&r.cost_unavailable);
    /* Missing choice or source facts block CON RNG; an unavailable unselected
       item cannot block the explicitly chosen power. */
    setup();s.magic.count=2;s.magic.powers[1]=(pd_power){.name="Concentration",.flags=PD_PREPARED|PD_CONCENTRATES};s.magic.concentration=2;
    p=(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.concentration=2,.payment=PD_PAYMENT_CHOOSE};
    pd_profile_cost_set(&p,0,(pd_cost){PD_COST_POWER,0,2});pd_profile_cost_set(&p,1,(pd_cost){PD_COST_ITEM,2,2});assert(pd_reduction_profile_set(&s,0,p));s.items[2].quantity=0;
    e=(pd_damage_event){.count=1,.components={{0,20}},.profile=1};pd_roll roll={.total=321},old_roll=roll;next=s;seed=next;out=untouched;calls=0;
    assert(!pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&out,false)&&!calls&&!memcmp(&next,&seed,sizeof next)&&!memcmp(&roll,&old_roll,sizeof roll)&&!memcmp(&out,&untouched,sizeof out));
    e.payment_choice=1;before=s;assert(pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&calls==1&&!memcmp(&s,&before,sizeof s)&&next.magic.powers[0].remaining==1&&next.items[2].charges.current==3&&!next.magic.concentration&&!next.reductions[0].active&&r.costs[0].selected&&!r.costs[1].selected);
    s=before;s.reductions[0].rule=PD_RULE_MAGICAL;out=untouched;calls=0;assert(!pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&out,false)&&!calls);
    /* Zero-HP/instant damage use the same selected payment once. */
    setup();s.hp=0;s.status.available=true;p=(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.payment=PD_PAYMENT_CHOOSE};pd_profile_cost_set(&p,0,(pd_cost){PD_COST_COUNTER,0,2});pd_profile_cost_set(&p,1,(pd_cost){PD_COST_ITEM,2,2});assert(pd_reduction_profile_set(&s,0,p));s.items[2].quantity=0;
    e=(pd_damage_event){.count=6,.components={{0,5}},.profile=1,.payment_choice=1};pd_death_damage_summary death;
    assert(pd_death_damage_apply(&s,&e,(pd_death_damage_choice){PD_HIT_NORMAL,true},&death)&&s.resources[0].current==1&&s.temp_hp==1&&s.status.failures==1&&s.items[2].charges.current==3);
    setup();s.hp=1;s.temp_hp=0;s.status.available=true;assert(pd_reduction_profile_set(&s,0,p));e.components[0].amount=104;assert(pd_death_damage_apply(&s,&e,(pd_death_damage_choice){0,true},&death)&&death.instant&&s.status.failures==3&&s.resources[0].current==1&&s.items[2].charges.current==3);
    /* Three profile modes pack into the old stage byte. No size/partition change. */
    setup();s.magic.count=120;for(unsigned i=1;i<120;i++)s.magic.powers[i]=(pd_power){.name="Synthetic spell",.flags=PD_PREPARED};
    for(unsigned i=0;i<3;i++){p.stage=i?1:0;p.payment=i==1?PD_PAYMENT_ALL:PD_PAYMENT_CHOOSE;assert(pd_reduction_profile_set(&s,i,p));}
    assert(pd_turn_name_set(&s,1,"Ally A"));size_t n=pd_encode(&s,60,record);uint32_t generation=0;assert(n==PD_RECORD_SIZE&&!memcmp(record,"PDDX",4)&&pd_decode(record,n,&next,&generation)&&generation==60&&!memcmp(&s,&next,sizeof s));
    pd_history h;pd_history_reset(&h,s.history_epoch);size_t size;char *json=pd_backup_encode(&s,&h,&size);assert(json&&pd_backup_decode(json,size,&restored)&&!memcmp(&s,&restored.state,sizeof s));if(argc==2){FILE *f=fopen(argv[1],"wb");assert(f&&fwrite(json,1,size,f)==size&&!fclose(f));}free(json);
    for(unsigned profile=0;profile<3;profile++){pd_encode(&s,60,record);record[8333+552*120+profile*10]=4;crc(n);before=next;generation=59;assert(!pd_decode(record,n,&next,&generation)&&generation==59&&!memcmp(&next,&before,sizeof next));}
    pd_encode(&s,60,record);record[3]='R';n-=4;crc(n);assert(!pd_decode(record,n,&next,&generation));
    for(unsigned i=0;i<3;i++){record[8333+552*120+i*10]&=1;}crc(n);assert(pd_decode(record,n,&next,&generation)&&!next.reductions[0].payment&&!next.reductions[2].payment&&!strcmp(next.turn_names[0],"Ally A"));
    printf("Alternative costs: %u independent source/balance/choice/reaction/stage/damage/component cases, no fallback, holes, no-RNG blocks, CON/death paths, exact PDD28 and PDD27 migration passed\n",cases);
}
