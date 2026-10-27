#include "damage.h"
#include "magic.h"
#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;
static uint32_t rng(void *p){(void)p;calls++;return 0;}
static void setup(pd_state *s){pd_demo(s);s->hp=s->max_hp=999;s->temp_hp=7;s->defences.configured=8191;s->magic.count=1;s->magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};s->magic.concentration=1;}
static bool match(unsigned rule,unsigned t){bool m=t&1,a=t&2,s=t&4,d=t&8;switch(rule){case 0:return true;case 1:return m;case 2:return !m;case 3:return a&&!m;case 4:return a&&!m&&!s;case 5:return a&&!m&&!d;default:return a&&!m&&!s&&!d;}}
static unsigned sub(unsigned a,unsigned b){return a>b?a-b:0;}
static unsigned defence(unsigned a,unsigned f){if(f&4)return 0;if(f&1)a/=2;if(f&2)a*=2;return a;}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
static unsigned permutations;
static void permute(pd_state *s,pd_damage_event *e,unsigned pos,const pd_damage_event_summary *expected){
 if(pos==e->count){pd_damage_event_summary r;assert(pd_damage_event_preview(s,e,&r)&&!memcmp(&r.total,&expected->total,sizeof r.total)&&!memcmp(r.groups,expected->groups,sizeof r.groups)&&r.profile_applied==expected->profile_applied&&r.mitigated==expected->mitigated&&r.reduction_before==expected->reduction_before&&r.reduction_after==expected->reduction_after);for(unsigned i=0;i<e->count;i++)if(!s->reductions[0].stage)assert(r.profile_used[i]==(e->allocation[i]<e->components[i].amount-1?e->allocation[i]:e->components[i].amount-1));permutations++;return;}
 for(unsigned i=pos;i<e->count;i++){
  pd_damage_component c=e->components[pos];e->components[pos]=e->components[i];e->components[i]=c;unsigned a=e->allocation[pos];e->allocation[pos]=e->allocation[i];e->allocation[i]=a;
  permute(s,e,pos+1,expected);
  c=e->components[pos];e->components[pos]=e->components[i];e->components[i]=c;a=e->allocation[pos];e->allocation[pos]=e->allocation[i];e->allocation[i]=a;
 }
}
int main(void){
 static pd_state s,before,next,decoded;pd_damage_event_summary r,old;unsigned cases=0,allocations=0;
 for(unsigned slot=0;slot<3;slot++)for(unsigned type=0;type<13;type++)for(unsigned rule=0;rule<7;rule++)for(unsigned stage=0;stage<2;stage++)for(unsigned active=0;active<2;active++)for(unsigned includes=0;includes<2;includes++){
  setup(&s);unsigned flags=type%8;assert(pd_defence_set(&s,type,true,flags));pd_reduction_profile p={.value=3,.types=1u<<(includes?type:(type+1)%13),.stage=stage,.rule=rule,.active=active,.timed=true,.rounds=3,.concentration=1};assert(pd_reduction_profile_set(&s,slot,p));
  for(unsigned known=0;known<16;known++)for(unsigned values=0;values<16;values++)if(!(values&~known)){
   bool any=false,all=true;unsigned needed=0;for(unsigned full=0;full<16;full++)if((full&known)==values){bool yes=match(rule,full);any|=yes;all&=yes;for(unsigned bit=1;bit<=8;bit<<=1)if(!(known&bit)&&yes!=match(rule,full^bit))needed|=bit;}
   pd_damage_event e={.count=1,.components={{type,11}},.contexts={{known,values}},.reductions={2},.reduction_after=1,.profile=slot+1,.allocation={stage?0:3}};
   assert(pd_damage_event_preview(&s,&e,&r));bool allowed=active&&includes&&all;assert(r.total.configured==allowed&&r.profile_unavailable==!active&&!!r.profile_ineligible==(!includes||!any)&&!!r.context_missing==(includes&&any&&!all));if(r.context_missing)assert(r.needed[0]==needed);
   assert(r.profile==slot+1&&r.profile_snapshot.value==3&&r.profile_snapshot.rule==rule&&r.profile_requested==3);
   if(allowed){unsigned mitigated=defence(stage?9:6,flags),after=sub(mitigated,1),damage=stage?sub(after,3):after;assert(r.mitigated==mitigated&&r.total.adjusted==damage&&r.profile_applied==(stage?(after<3?after:3):3));}
   else{assert(!r.profile_applied);for(unsigned i=0;i<6;i++)assert(!r.profile_used[i]);before=s;next=s;old=r;calls=0;pd_roll roll;assert(!pd_damage_event_apply(&s,&e,&r)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&old,sizeof r));assert(!pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&!calls&&!memcmp(&next,&before,sizeof next));}
   cases++;
  }
 }
 /* One budget shared across rows; manual subtraction first, allocation caps locally. */
 for(unsigned stage=0;stage<2;stage++)for(unsigned budget=1;budget<=9;budget++)for(unsigned x=0;x<=10;x++)for(unsigned y=0;y<=10;y++){
  setup(&s);assert(pd_defence_set(&s,0,true,1));assert(pd_reduction_profile_set(&s,0,(pd_reduction_profile){.value=budget,.types=8191,.stage=stage,.active=true}));pd_damage_event e={.count=2,.components={{0,5},{0,6}},.reductions={1,2},.reduction_after=1,.profile=1,.allocation={x,y}};
  assert(pd_damage_event_preview(&s,&e,&r));bool valid=stage?!(x+y):(x+y>0&&x+y<=budget);assert(r.profile_budget==!valid&&r.total.configured==valid);
  if(valid){unsigned used=(x<4?x:4)+(y<4?y:4),mitigated=(8-used)/2;assert(r.total.adjusted==(stage?sub(3,budget):sub(mitigated,1)));}allocations++;
 }
 /* Before may target just one eligible row. After requires the whole impact eligible. */
 setup(&s);assert(pd_reduction_profile_set(&s,0,(pd_reduction_profile){.value=3,.types=1,.rule=PD_RULE_NONMAGICAL,.active=true}));pd_damage_event e={.count=2,.components={{0,11},{5,11}},.profile=1,.allocation={3,0},.contexts={{1,0},{0,0}}};assert(pd_damage_event_preview(&s,&e,&r)&&r.total.configured&&r.total.adjusted==19);
 s.reductions[0].stage=1;memset(e.allocation,0,sizeof e.allocation);assert(pd_damage_event_preview(&s,&e,&r)&&r.profile_ineligible==2&&!r.total.configured);
 s.reductions[0].types=8191;assert(pd_damage_event_preview(&s,&e,&r)&&r.context_missing==2);e.contexts[1]=(pd_damage_context){1,0};assert(pd_damage_event_preview(&s,&e,&r)&&r.total.adjusted==19);
 /* All 720 permutations in each phase preserve allocation ownership and totals. */
 for(unsigned stage=0;stage<2;stage++){setup(&s);assert(pd_reduction_profile_set(&s,0,(pd_reduction_profile){.value=6,.types=stage?8191:7,.stage=stage,.active=true}));e=(pd_damage_event){.count=6,.profile=1,.reduction_after=2};for(unsigned i=0;i<6;i++){assert(pd_defence_set(&s,i,true,i));e.components[i]=(pd_damage_component){i,i+5};e.reductions[i]=1;e.allocation[i]=!stage&&i<3?i+1:0;}assert(pd_damage_event_preview(&s,&e,&old)&&old.total.configured);permute(&s,&e,0,&old);}assert(permutations==1440);
 /* Every ending and inactive binding, same-spell recast, zero and selected expiry. */
 for(unsigned end=0;end<6;end++){
  setup(&s);for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&s,i,(pd_reduction_profile){.value=3,.types=8191,.active=i!=1,.timed=true,.rounds=2,.concentration=i<2?1:0}));assert(pd_concentration_effects(&s)==1);
  if(end==0)pd_concentration_end(&s);if(end==1)assert(pd_magic_use(&s,0,0,false));if(end==2)pd_hp(&s,-9999);if(end==3)assert(pd_rest_apply(&s,&(pd_rest_plan){.long_rest=true}));if(end==4){s.magic.count=2;s.magic.powers[1]=(pd_power){.name="Ira",.kind=1,.effect=1,.maximum=1,.remaining=1};s.combat.rage_bonus=2;assert(pd_magic_use(&s,1,0,false));}if(end==5){e=(pd_damage_event){.count=1,.components={{0,31}},.profile=1,.allocation={3}};pd_roll roll;calls=0;assert(pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&calls==1&&r.total.adjusted==28&&r.total.concentration_dc==14&&r.profile_applied==3&&r.profile_snapshot.active&&r.total.ended_effects==1);s=next;}
  assert(!s.reductions[0].active&&!s.reductions[0].concentration&&!s.reductions[1].concentration&&s.reductions[2].active&&pd_valid(&s));
 }
 setup(&s);for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&s,i,(pd_reduction_profile){.value=3,.types=8191,.active=true,.timed=true,.rounds=i+1}));pd_round_summary tick;assert(pd_round_advance(&s,&tick)&&tick.defences_advanced==(UINT64_C(7)<<39)&&tick.defences_expired==(UINT64_C(1)<<39)&&s.reductions[1].rounds==1&&s.reductions[2].rounds==2);
 e=(pd_damage_event){.count=1,.components={{0,11}},.profile=1,.allocation={3}};assert(pd_damage_event_preview(&s,&e,&r)&&r.profile_unavailable&&!r.total.configured);
 /* Numeric expiry is atomic with profile timers; no partial round. */
 setup(&s);assert(pd_effect_set(&s,0,(pd_effect){.value=10,.active=true,.timed=true,.rounds=1})&&pd_effect_set(&s,1,(pd_effect){.value=-20,.active=true}));assert(pd_reduction_profile_set(&s,2,(pd_reduction_profile){.value=3,.types=8191,.active=true,.timed=true,.rounds=1}));before=s;memset(&tick,0x55,sizeof tick);pd_round_summary prior=tick;assert(!pd_round_advance(&s,&tick)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&tick,&prior,sizeof tick));
 /* All 42 defensive timers and all 50 concentration-bound entries. */
 setup(&s);for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1,.concentration=1}));for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&s,i,(pd_reduction_profile){.value=3,.types=8191,.active=true,.timed=true,.rounds=1,.concentration=1}));for(unsigned i=0;i<8;i++)assert(pd_effect_set(&s,i,(pd_effect){.value=1,.active=true,.timed=true,.rounds=1,.concentration=1}));assert(pd_concentration_effects(&s)==50);before=s;assert(pd_round_advance(&s,&tick)&&tick.defences_advanced==((UINT64_C(1)<<42)-1)&&tick.defences_expired==tick.defences_advanced&&tick.effects_expired==255);s=before;pd_concentration_end(&s);assert(!pd_concentration_effects(&s)&&!s.reductions[2].active&&!s.reductions[2].concentration&&pd_valid(&s));
 /* Strict PDD23, largest record, all 30 bytes and CRC-valid semantic corruption. */
 setup(&s);s.magic.count=120;for(unsigned i=1;i<120;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&s,i,(pd_reduction_profile){.value=9999,.types=8191,.stage=i%2,.rule=6,.active=true,.timed=true,.rounds=99,.concentration=1}));static uint8_t data[PD_RECORD_SIZE];size_t n=pd_encode(&s,160,data);uint32_t gen;assert(n==PD_RECORD_SIZE&&!memcmp(data,"PDDW",4)&&pd_decode(data,n,&decoded,&gen)&&gen==160);for(unsigned i=0;i<3;i++){const pd_reduction_profile *a=&s.reductions[i],*b=&decoded.reductions[i];assert(a->value==b->value&&a->types==b->types&&a->stage==b->stage&&a->rule==b->rule&&a->active==b->active&&a->timed==b->timed&&a->rounds==b->rounds&&a->concentration==b->concentration&&!b->cost_kind&&!b->cost&&!b->reaction);}
 /* Each byte gets an invalid value or a semantically invalid field combination. */
 for(unsigned i=0;i<30;i++){pd_encode(&s,160,data);unsigned field=i%10;data[n-616+i]=(const unsigned[]){255,255,255,255,2,7,2,2,100,2}[field];if(field==0)data[n-616+i+1]=255;if(field==2)data[n-616+i+1]=255;crc(data,n);before=decoded;gen=17;assert(!pd_decode(data,n,&decoded,&gen)&&gen==17&&!memcmp(&decoded,&before,sizeof decoded));}
 before=s;assert(!pd_reduction_profile_set(&s,3,(pd_reduction_profile){0})&&!pd_reduction_profile_set(&s,0,(pd_reduction_profile){.active=true})&&!memcmp(&s,&before,sizeof s));e=(pd_damage_event){.count=1,.components={{0,11}},.profile=4};old=r;assert(!pd_damage_event_preview(&s,&e,&r)&&!memcmp(&r,&old,sizeof r));e.profile=0;e.allocation[0]=1;assert(!pd_damage_event_preview(&s,&e,&r));e.profile=1;e.allocation[0]=10000;assert(!pd_damage_event_preview(&s,&e,&r));
 printf("Reduction profiles: %u independent type/rule/partial-source/stage/slot/active cases, %u shared-budget allocations, 1440 permutations, eligibility, timers/endings, frozen impact, strict PDD24 and corrupt fields passed\n",cases,allocations);
}
