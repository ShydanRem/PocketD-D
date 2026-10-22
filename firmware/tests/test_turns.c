#include "pocketdd.h"
#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void setup(pd_state *s){pd_demo(s);s->hp=32;s->magic.count=1;s->magic.powers[0]=(pd_power){.name="Magia sintetica",.flags=PD_PREPARED|PD_CONCENTRATES};s->magic.concentration=1;s->magic.slots[0]=(pd_slot){2,4};}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
 static pd_state s,before,expected,out;pd_round_summary r;unsigned cases=0;
 for(unsigned index=0;index<50;index++)for(unsigned tick=0;tick<3;tick++)for(unsigned n=0;n<3;n++)for(unsigned active=0;active<2;active++)for(unsigned action=0;action<3;action++)for(unsigned spent=0;spent<2;spent++)for(unsigned linked=0;linked<2;linked++){
  setup(&s);unsigned count=(const unsigned[]){1,2,99}[n];s.reaction_spent=spent;uint8_t *remaining;bool *enabled;uint64_t bit=0;unsigned numeric=0;
  if(index<8){s.effect_count=index+1;assert(pd_effect_set(&s,index,(pd_effect){.target=index%2,.value=4,.active=active,.timed=true,.rounds=count,.concentration=linked,.tick=tick}));remaining=&s.effects[index].rounds;enabled=&s.effects[index].active;numeric=1u<<index;}
  else if(index<47){unsigned at=index-8,type=at%13,slot=at/13;assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=1,.active=active,.timed=true,.rounds=count,.concentration=linked,.tick=tick}));pd_defence_temporary *d=slot?&s.defences.additional[type][slot-1]:&s.defences.temporary[type];remaining=&d->rounds;enabled=&d->active;bit=UINT64_C(1)<<at;}
  else{unsigned at=index-47;assert(pd_reduction_profile_set(&s,at,(pd_reduction_profile){.value=3,.types=8191,.active=active,.timed=true,.rounds=count,.concentration=linked,.cost_kind=PD_COST_SLOT,.cost=1,.reaction=true,.tick=tick}));remaining=&s.reductions[at].rounds;enabled=&s.reductions[at].active;bit=UINT64_C(1)<<(39+at);}
  if(pd_max_hp(&s)>32)s.hp=35;before=s;bool match=active&&tick==action;
  if(match){--*remaining;if(!*remaining)*enabled=false;}if(s.hp>pd_max_hp(&s))s.hp=pd_max_hp(&s);if(action==PD_TICK_START)s.reaction_spent=false;expected=s;s=before;
  assert((action==0?pd_round_advance(&s,&r):pd_turn_advance(&s,action==PD_TICK_START,&r))&&!memcmp(&s,&expected,sizeof s)&&pd_valid(&s));
  assert(r.effects_advanced==(match?numeric:0)&&r.effects_expired==(match&&count==1?numeric:0)&&r.defences_advanced==(match?bit:0)&&r.defences_expired==(match&&count==1?bit:0));cases++;
 }
 /* Fifty simultaneous expiries, high bits included; unrelated edges do nothing. */
 setup(&s);s.reaction_spent=true;for(unsigned i=0;i<8;i++)assert(pd_effect_set(&s,i,(pd_effect){.target=i%2,.value=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));s.hp=36;
 for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));
 for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&s,i,(pd_reduction_profile){.value=3,.types=8191,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));before=s;
 assert(pd_round_advance(&s,&r)&&!r.effects_advanced&&!r.defences_advanced&&!memcmp(&s,&before,sizeof s));assert(pd_turn_advance(&s,false,&r)&&!r.effects_advanced&&!r.defences_advanced&&!memcmp(&s,&before,sizeof s));
 assert(pd_turn_advance(&s,true,&r)&&r.effects_expired==255&&r.defences_expired==((UINT64_C(1)<<42)-1)&&s.hp==32&&!s.reaction_spent&&s.magic.slots[0].current==2&&s.magic.concentration==1);
 /* Invalid numeric expiry rejects before any defence/profile or reaction change. */
 setup(&s);s.armor=1;s.reaction_spent=true;assert(pd_effect_set(&s,0,(pd_effect){.value=2,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));assert(pd_effect_set(&s,1,(pd_effect){.value=-2,.active=true,.timed=true,.rounds=2,.tick=PD_TICK_END}));assert(pd_defence_grant_set(&s,12,2,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));assert(pd_reduction_profile_set(&s,2,(pd_reduction_profile){.value=3,.types=8191,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));before=s;memset(&r,0xa5,sizeof r);pd_round_summary old=r;assert(!pd_turn_advance(&s,true,&r)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&old,sizeof r));assert(!pd_turn_advance(NULL,true,&r)&&!pd_turn_advance(&s,true,NULL));
 /* Positive and negative HP maximum expiry caps without healing; no concentration loss. */
 setup(&s);assert(pd_effect_set(&s,0,(pd_effect){.target=1,.value=-5,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_END}));s.hp=20;assert(pd_turn_advance(&s,false,&r)&&pd_max_hp(&s)==32&&s.hp==20&&s.magic.concentration==1);
 /* Rests do not tick unbound timers; long rest ends bound ones via concentration. */
 setup(&s);assert(pd_effect_set(&s,0,(pd_effect){.value=1,.active=true,.timed=true,.rounds=2,.tick=PD_TICK_START}));assert(pd_defence_grant_set(&s,0,0,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=2,.tick=PD_TICK_END,.concentration=1}));s.reaction_spent=true;assert(pd_rest_apply(&s,&(pd_rest_plan){0})&&s.effects[0].rounds==2&&s.defences.temporary[0].rounds==2&&s.defences.temporary[0].active&&s.reaction_spent);assert(pd_rest_apply(&s,&(pd_rest_plan){.long_rest=true})&&s.effects[0].active&&s.effects[0].rounds==2&&!s.defences.temporary[0].active&&s.defences.temporary[0].rounds==2&&!s.defences.temporary[0].concentration&&s.reaction_spent);
 /* Phase fields require a timer; invalid definitions leave all state intact. */
 setup(&s);before=s;for(unsigned tick=1;tick<4;tick++){assert(!pd_effect_set(&s,0,(pd_effect){.tick=tick})&&!pd_defence_grant_set(&s,0,0,(pd_defence_temporary){.tick=tick})&&!pd_reduction_profile_set(&s,0,(pd_reduction_profile){.tick=tick})&&!memcmp(&s,&before,sizeof s));}
 /* Every appended selector rejects invalid values with a valid CRC. */
 setup(&s);s.effect_count=8;for(unsigned i=0;i<8;i++)s.effects[i]=(pd_effect){.timed=true,.rounds=2,.tick=i%3};for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.timed=true,.rounds=2,.tick=(type+slot)%3}));for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&s,i,(pd_reduction_profile){.timed=true,.rounds=2,.tick=i}));
 s.magic.count=120;for(unsigned i=1;i<120;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};static uint8_t data[PD_RECORD_SIZE];size_t n=pd_encode(&s,190,data);uint32_t gen;assert(n==PD_RECORD_SIZE&&!memcmp(data,"PDDP",4)&&pd_decode(data,n,&out,&gen)&&out.effects[7].tick==1&&out.reductions[2].tick==2);
 for(unsigned i=0;i<50;i++){pd_encode(&s,190,data);data[n-504+i]=3;crc(data,n);before=out;gen=7;assert(!pd_decode(data,n,&out,&gen)&&gen==7&&!memcmp(&out,&before,sizeof out));}
 /* Valid selectors without timers and hidden unused numeric selectors also reject. */
 setup(&s);n=pd_encode(&s,1,data);for(unsigned i=0;i<50;i++){pd_encode(&s,1,data);data[n-504+i]=1;crc(data,n);assert(!pd_decode(data,n,&out,&gen));}
 printf("Turns: %u independent timer/family/index/phase/count/pause/reaction/binding cases, 50 simultaneous expiries, atomic rejection, HP caps and strict PDD24 selectors passed\n",cases);
}
