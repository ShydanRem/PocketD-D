#include "pocketdd.h"
#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void setup(pd_state *s){pd_demo(s);s->hp=32;s->magic.count=1;s->magic.powers[0]=(pd_power){.name="Magia sintetica",.flags=PD_PREPARED|PD_CONCENTRATES};s->magic.concentration=1;s->magic.slots[0]=(pd_slot){2,4};s->reaction_spent=true;}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
 static pd_state s,before,expected,out;pd_round_summary r;unsigned cases=0;const unsigned counts[]={1,2,59,60,61,99,119,120,5939,5940},steps[]={1,2,3,59,60,61,99,120,5940};
 for(unsigned index=0;index<50;index++)for(unsigned tick=PD_TICK_MINUTES;tick<=PD_TICK_HOURS;tick++)for(unsigned n=0;n<10;n++)for(unsigned active=0;active<2;active++)for(unsigned step=0;step<9;step++)for(unsigned spent=0;spent<2;spent++)for(unsigned linked=0;linked<2;linked++){
  unsigned minutes=counts[n];if(tick==PD_TICK_MINUTES&&minutes>99)continue;setup(&s);s.reaction_spent=spent;uint16_t *remaining;bool *enabled;unsigned numeric=0;uint64_t bit=0;
  if(index<8){s.effect_count=index+1;assert(pd_effect_set(&s,index,(pd_effect){.target=index%2,.value=4,.active=active,.timed=true,.tick=tick,.minutes=minutes,.concentration=linked}));remaining=&s.effects[index].minutes;enabled=&s.effects[index].active;numeric=1u<<index;}
  else if(index<47){unsigned at=index-8,type=at%13,slot=at/13;assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=1,.active=active,.timed=true,.tick=tick,.minutes=minutes,.concentration=linked}));pd_defence_temporary *d=slot?&s.defences.additional[type][slot-1]:&s.defences.temporary[type];remaining=&d->minutes;enabled=&d->active;bit=UINT64_C(1)<<at;}
  else{unsigned at=index-47;assert(pd_reduction_profile_set(&s,at,(pd_reduction_profile){.value=3,.types=8191,.active=active,.timed=true,.tick=tick,.minutes=minutes,.concentration=linked,.cost_kind=PD_COST_SLOT,.cost=1,.reaction=true}));remaining=&s.reductions[at].minutes;enabled=&s.reductions[at].active;bit=UINT64_C(1)<<(39+at);}
  if(pd_max_hp(&s)>32)s.hp=35;before=s;if(active){*remaining=minutes>steps[step]?minutes-steps[step]:0;if(!*remaining)*enabled=false;}if(s.hp>pd_max_hp(&s))s.hp=pd_max_hp(&s);expected=s;s=before;
  assert(pd_time_advance(&s,steps[step],&r)&&!memcmp(&s,&expected,sizeof s)&&pd_valid(&s));bool expired=active&&minutes<=steps[step];assert(r.effects_advanced==(active?numeric:0)&&r.effects_expired==(expired?numeric:0)&&r.defences_advanced==(active?bit:0)&&r.defences_expired==(expired?bit:0));cases++;
 }
 /* Earlier HP caps survive later increases, while simultaneous expiry is one step. */
 setup(&s);assert(pd_effect_set(&s,0,(pd_effect){.target=1,.value=10,.active=true,.timed=true,.tick=PD_TICK_MINUTES,.minutes=1}));assert(pd_effect_set(&s,1,(pd_effect){.target=1,.value=-10,.active=true,.timed=true,.tick=PD_TICK_MINUTES,.minutes=2}));before=s;assert(pd_time_advance(&s,2,&r)&&pd_max_hp(&s)==32&&s.hp==22&&r.effects_expired==3&&s.reaction_spent);s=before;s.effects[1].minutes=1;assert(pd_time_advance(&s,2,&r)&&s.hp==32);
 /* An invalid intermediate armour total rejects even when the final total is valid. */
 setup(&s);s.armor=1;assert(pd_effect_set(&s,0,(pd_effect){.value=2,.active=true,.timed=true,.tick=PD_TICK_MINUTES,.minutes=1}));assert(pd_effect_set(&s,1,(pd_effect){.value=-2,.active=true,.timed=true,.tick=PD_TICK_MINUTES,.minutes=2}));assert(pd_defence_grant_set(&s,12,2,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.tick=PD_TICK_MINUTES,.minutes=1}));assert(pd_reduction_profile_set(&s,2,(pd_reduction_profile){.value=3,.types=8191,.active=true,.timed=true,.tick=PD_TICK_MINUTES,.minutes=1}));before=s;memset(&r,0xa5,sizeof r);pd_round_summary sentinel=r;assert(!pd_time_advance(&s,2,&r)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&sentinel,sizeof r));s.effects[0].minutes=2;assert(pd_time_advance(&s,2,&r)&&pd_armor(&s)==1);
 /* Failure after an earlier valid HP cap rolls everything back. */
 s=before;s.effects[0].minutes=2;s.effects[1].minutes=3;assert(pd_effect_set(&s,2,(pd_effect){.target=1,.value=10,.active=true,.timed=true,.tick=PD_TICK_MINUTES,.minutes=1}));assert(pd_effect_set(&s,3,(pd_effect){.target=1,.value=-10,.active=true,.timed=true,.tick=PD_TICK_MINUTES,.minutes=4}));s.hp=32;before=s;assert(!pd_time_advance(&s,4,&r)&&!memcmp(&s,&before,sizeof s));
 /* Independent minute-by-minute oracle for every 1/2/3-minute schedule of eight effects. */
 unsigned histories=0;
 for(unsigned base=0;base<2;base++)for(unsigned code=0;code<6561;code++){
  setup(&s);s.armor=base?16:1;s.effect_count=8;unsigned digits=code;
  for(unsigned i=0;i<8;i++){s.effects[i]=(pd_effect){.target=i%2,.value=i<4?1:-1,.active=true,.timed=true,.tick=PD_TICK_HOURS,.minutes=1+digits%3};digits/=3;}assert(pd_valid(&s));before=s;expected=s;unsigned hp=32;bool valid=true;
  for(unsigned minute=1;minute<=3;minute++){
   int armor=expected.armor,maximum=32;
   for(unsigned i=0;i<8;i++){if(expected.effects[i].minutes==minute)expected.effects[i].active=false;if(expected.effects[i].active){if(expected.effects[i].target)maximum+=expected.effects[i].value;else armor+=expected.effects[i].value;}}
   if(armor<1||armor>40||maximum<1||maximum>999){valid=false;break;}if(hp>(unsigned)maximum)hp=maximum;
  }
  memset(&r,0xa5,sizeof r);pd_round_summary untouched=r;assert(pd_time_advance(&s,3,&r)==valid);
  if(valid){for(unsigned i=0;i<8;i++)expected.effects[i].minutes=0;expected.hp=hp;assert(!memcmp(&s,&expected,sizeof s)&&r.effects_advanced==255&&r.effects_expired==255);}
  else assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&untouched,sizeof r));histories++;
 }
 /* Fifty simultaneous timers, and a partial hour that survives serialization. */
 setup(&s);for(unsigned i=0;i<8;i++)assert(pd_effect_set(&s,i,(pd_effect){.target=i%2,.value=1,.active=true,.timed=true,.tick=PD_TICK_HOURS,.minutes=60}));s.hp=36;for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.tick=PD_TICK_HOURS,.minutes=60}));for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&s,i,(pd_reduction_profile){.value=3,.types=8191,.active=true,.timed=true,.tick=PD_TICK_HOURS,.minutes=60}));assert(pd_time_advance(&s,1,&r)&&s.effects[7].minutes==59&&s.reductions[2].minutes==59&&s.hp==36);static uint8_t data[PD_RECORD_SIZE];uint32_t gen;size_t size=pd_encode(&s,200,data);assert(pd_decode(data,size,&out,&gen)&&out.effects[7].minutes==59&&out.reductions[2].minutes==59);assert(pd_time_advance(&out,59,&r)&&out.hp==32&&r.effects_expired==255&&r.defences_expired==((UINT64_C(1)<<42)-1)&&out.reaction_spent&&out.magic.slots[0].current==2&&out.magic.concentration==1);
 /* Round, own-turn and rest operations never decrement elapsed-time counters. */
 before=s;assert(pd_round_advance(&s,&r)&&!memcmp(&s,&before,sizeof s));assert(pd_turn_advance(&s,false,&r)&&!memcmp(&s,&before,sizeof s));assert(pd_turn_advance(&s,true,&r)&&s.effects[0].minutes==59&&!s.reaction_spent);assert(pd_rest_apply(&s,&(pd_rest_plan){0})&&s.reductions[0].minutes==59);assert(pd_rest_apply(&s,&(pd_rest_plan){.long_rest=true})&&s.effects[0].minutes==59&&s.effects[0].active);
 setup(&s);assert(pd_effect_set(&s,0,(pd_effect){.value=1,.active=true,.timed=true,.rounds=2,.tick=PD_TICK_START}));before=s;assert(pd_time_advance(&s,5940,&r)&&!r.effects_advanced&&!memcmp(&s,&before,sizeof s));
 /* Validation rejects absent timers, invalid counts/units and elapsed-time bounds. */
 setup(&s);before=s;const pd_effect bad[]={ {.tick=PD_TICK_MINUTES,.minutes=1},{.timed=true,.tick=PD_TICK_MINUTES,.minutes=100},{.timed=true,.active=true,.tick=PD_TICK_HOURS},{.timed=true,.tick=PD_TICK_HOURS,.minutes=5941},{.timed=true,.tick=PD_TICK_HOURS,.minutes=1,.rounds=1},{.timed=true,.tick=PD_TICK_START,.minutes=1},{.minutes=1},{.timed=true,.tick=PD_TICK_COUNT} };for(unsigned i=0;i<sizeof bad/sizeof *bad;i++)assert(!pd_effect_set(&s,0,bad[i])&&!memcmp(&s,&before,sizeof s));assert(!pd_time_advance(&s,0,&r)&&!pd_time_advance(&s,5941,&r)&&!pd_time_advance(NULL,1,&r)&&!pd_time_advance(&s,1,NULL)&&!memcmp(&s,&before,sizeof s));
 /* All 100 appended bytes are checked independently with a valid CRC. */
 setup(&s);s.effect_count=8;for(unsigned i=0;i<8;i++)s.effects[i]=(pd_effect){.timed=true,.tick=PD_TICK_MINUTES,.minutes=1};for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.timed=true,.tick=PD_TICK_MINUTES,.minutes=1}));for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&s,i,(pd_reduction_profile){.timed=true,.tick=PD_TICK_MINUTES,.minutes=1}));s.magic.count=120;for(unsigned i=1;i<120;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};size=pd_encode(&s,200,data);assert(size==PD_RECORD_SIZE&&!memcmp(data,"PDDN",4)&&pd_decode(data,size,&out,&gen));
 for(unsigned i=0;i<100;i++){pd_encode(&s,200,data);data[size-429+i]=255;crc(data,size);before=out;gen=5;assert(!pd_decode(data,size,&out,&gen)&&gen==5&&!memcmp(&out,&before,sizeof out));}
 pd_encode(&s,200,data);data[3]='J';size_t previous=size-425;crc(data,previous);assert(pd_record_known(data,previous)&&!pd_decode(data,previous,&out,&gen));
 printf("Elapsed time: %u independent timer/family/unit/remaining/elapsed/pause/reaction/binding cases, %u minute-by-minute oracle histories, chronological HP caps/rollback, partial hours, 50 timers and strict PDD23 passed\n",cases,histories);
}
