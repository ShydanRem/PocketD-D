#include "pocketdd.h"
#include "damage.h"
#include "magic.h"
#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;
static uint32_t face;
static uint32_t rng(void *ctx){(void)ctx;calls++;return face;}
static void setup(pd_state *s){pd_demo(s);s->hp=32;s->magic.count=2;s->magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};s->magic.powers[1]=(pd_power){.name="Ira",.kind=1,.effect=1,.maximum=1,.remaining=1};s->combat.rage_bonus=2;s->magic.concentration=1;}
static unsigned adjusted(unsigned amount,unsigned flags){if(flags&4)return 0;if(flags&1)amount/=2;if(flags&2)amount*=2;return amount;}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
 static pd_state s,before,next,decoded;pd_damage_summary damage;pd_round_summary round;unsigned cases=0,ticks=0;pd_roll roll;
 /* Independent combinations of base/additional flags for all 13 types, including
    same-flag non-stacking, inactive grants, unknown bases and odd damage. */
 for(unsigned type=0;type<13;type++)for(unsigned base=0;base<8;base++)for(unsigned extra=1;extra<8;extra++)for(unsigned active=0;active<2;active++)for(unsigned amount=0;amount<22;amount++){
  setup(&s);assert(pd_defence_set(&s,type,true,base));pd_defence_temporary d={.flags=extra,.active=active,.timed=true,.rounds=1,.concentration=1};assert(pd_defence_temporary_set(&s,type,d));
  unsigned effective=base|(active?extra:0),value=adjusted(amount,effective);assert(pd_defence_flags(&s,type)==effective&&pd_defence_base_flags(&s,type)==base);
  before=s;pd_damage_plan plan={.type=type,.amount=amount,.concentration=PD_CONCENTRATION_KEEP};assert(pd_damage_preview(&s,&plan,&damage)&&damage.configured&&damage.flags==effective&&damage.adjusted==value&&!memcmp(&s,&before,sizeof s));
  assert(pd_defence_set(&s,type,false,0));before=s;assert(pd_damage_preview(&s,&plan,&damage)&&!damage.configured&&!damage.allowed&&pd_defence_flags(&s,type)==(active?extra:0));assert(!pd_damage_apply(&s,&plan,&damage)&&!memcmp(&s,&before,sizeof s));
  assert(pd_round_advance(&s,&round)&&round.defences_advanced==(active?1u<<type:0)&&round.defences_expired==(active?1u<<type:0)&&!s.defences.configured&&pd_defence_flags(&s,type)==0);
  assert(s.defences.temporary[type].flags==extra&&s.defences.temporary[type].rounds==(active?0:1)&&!s.defences.temporary[type].active&&s.magic.concentration==1);cases++;
 }
 /* Every timer value, each type, pause/manual, expiry and preserved bound source. */
 for(unsigned type=0;type<13;type++)for(unsigned duration=1;duration<=99;duration++){
  setup(&s);assert(pd_defence_set(&s,type,true,PD_VULNERABILITY));assert(pd_defence_temporary_set(&s,type,(pd_defence_temporary){.flags=PD_RESISTANCE,.active=true,.timed=true,.rounds=duration,.concentration=1}));
  for(unsigned tick=0;tick<duration;tick++){
   assert(pd_round_advance(&s,&round));bool ends=tick+1==duration;assert(round.defences_advanced==(1u<<type)&&round.defences_expired==(ends?1u<<type:0)&&s.defences.temporary[type].rounds==duration-tick-1&&pd_defence_flags(&s,type)==(ends?2:3)&&s.magic.concentration==1&&s.defences.temporary[type].concentration==1&&pd_valid(&s));ticks++;
  }
  before=s;assert(pd_round_advance(&s,&round)&&!round.defences_advanced&&!memcmp(&s,&before,sizeof s));
 }
 /* One round is a single atomic change for numeric effects and all defences. */
 setup(&s);for(unsigned i=0;i<13;i++){assert(pd_defence_set(&s,i,true,i%8));assert(pd_defence_temporary_set(&s,i,(pd_defence_temporary){.flags=4,.active=true,.timed=true,.rounds=i%2?1:2}));}
 assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.timed=true,.rounds=1}));s.hp=41;
 assert(pd_round_advance(&s,&round)&&round.effects_expired==1&&round.defences_advanced==8191&&round.defences_expired==2730&&s.hp==32);
 for(unsigned i=0;i<13;i++)assert(s.defences.temporary[i].rounds==(i%2?0:1)&&pd_defence_base_flags(&s,i)==i%8);
 setup(&s);assert(pd_effect_set(&s,0,(pd_effect){.value=20,.active=true,.timed=true,.rounds=1}));assert(pd_effect_set(&s,1,(pd_effect){.value=-20,.active=true,.timed=true,.rounds=2}));
 assert(pd_defence_temporary_set(&s,12,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1}));before=s;memset(&round,0x55,sizeof round);pd_round_summary saved=round;
 assert(!pd_round_advance(&s,&round)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&round,&saved,sizeof round));
 /* All end triggers clear bound grants together with numeric bonuses, leaving
    unbound grants, base profiles, remaining time and source flags unchanged. */
 for(unsigned event=0;event<7;event++)for(unsigned mask=0;mask<8192;mask+=17){
  setup(&s);for(unsigned i=0;i<13;i++){assert(pd_defence_set(&s,i,true,i%8));assert(pd_defence_temporary_set(&s,i,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=3,.concentration=(mask&(1u<<i))?1:0}));}
  assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=1}));s.hp=41;unsigned expected=1;
  for(unsigned i=0;i<13;i++){if(mask&(1u<<i))expected++;}
  assert(pd_concentration_effects(&s)==expected);
  if(event==0)pd_concentration_end(&s);
  if(event==1)assert(pd_magic_use(&s,0,0,false)&&s.magic.concentration==1);
  if(event==2)assert(pd_magic_use(&s,1,0,false));
  if(event==3)assert(pd_rest_apply(&s,&(pd_rest_plan){.long_rest=true}));
  if(event==4)pd_hp(&s,-99);
  if(event==5||event==6){assert(pd_defence_set(&s,0,true,0));pd_defence_temporary zero={0};assert(pd_defence_temporary_set(&s,0,zero));s.temp_hp=10;expected=1;for(unsigned i=1;i<13;i++)if(mask&(1u<<i))expected++;
   if(event==5)assert(pd_damage_apply(&s,&(pd_damage_plan){.amount=1,.concentration=PD_CONCENTRATION_END},&damage));
   else{face=0;calls=0;assert(pd_concentration_roll(&s,&(pd_damage_plan){.amount=1},0,0,rng,NULL,&next,&roll,&damage,false)&&calls==1);s=next;}
   assert(damage.ended_effects==expected&&damage.hp_after==32&&damage.adjusted==1);
  }
  assert(pd_valid(&s)&&s.hp==(event==4?0:32)&&!s.effects[0].active&&!s.effects[0].concentration);
  for(unsigned i=(event>=5?1:0);i<13;i++){const pd_defence_temporary *d=&s.defences.temporary[i];assert(d->active==!(mask&(1u<<i))&&!d->concentration&&d->rounds==3&&d->flags==1&&pd_defence_base_flags(&s,i)==i%8);}
 }
 /* A hit uses defences present at impact; losing concentration changes the NEXT hit.
    Duplicate resistance is once only; CON DC uses reduced damage before temp HP. */
 setup(&s);s.temp_hp=99;assert(pd_defence_set(&s,0,true,1));assert(pd_defence_temporary_set(&s,0,(pd_defence_temporary){.flags=1,.active=true,.concentration=1}));
 face=0;calls=0;assert(pd_concentration_roll(&s,&(pd_damage_plan){.amount=49},0,0,rng,NULL,&next,&roll,&damage,false)&&calls==1&&damage.adjusted==24&&damage.concentration_dc==12&&next.temp_hp==75&&!next.defences.temporary[0].active&&pd_defence_flags(&next,0)==1);
 assert(pd_defence_set(&s,0,true,0));assert(pd_concentration_roll(&s,&(pd_damage_plan){.amount=49},0,0,rng,NULL,&next,&roll,&damage,false)&&damage.adjusted==24&&damage.concentration_dc==12&&next.temp_hp==75&&pd_defence_flags(&next,0)==0);
 assert(pd_damage_preview(&next,&(pd_damage_plan){.amount=49},&damage)&&damage.adjusted==49);
 /* Base editing must not copy the effective additional flags into permanent flags. */
 setup(&s);assert(pd_defence_set(&s,5,true,1));assert(pd_defence_temporary_set(&s,5,(pd_defence_temporary){.flags=4,.active=true}));assert(pd_defence_set(&s,5,true,2)&&pd_defence_base_flags(&s,5)==2&&pd_defence_flags(&s,5)==6);
 /* Inactive/manual entries do not tick or end on unrelated events. */
 assert(pd_defence_temporary_set(&s,1,(pd_defence_temporary){.flags=1,.active=false,.timed=true,.rounds=9}));before=s;assert(pd_round_advance(&s,&round)&&!round.defences_advanced&&!memcmp(&s,&before,sizeof s));assert(pd_rest_apply(&s,&(pd_rest_plan){0})&&!memcmp(&s,&before,sizeof s));
 /* Invalid flags/timers/references reject atomically; no entropy for bad state. */
 const pd_defence_temporary bad[]={{.flags=8},{.active=true},{.flags=1,.active=true,.timed=true},{.timed=true,.rounds=100},{.rounds=1},{.concentration=2}};
 before=s;for(unsigned i=0;i<sizeof bad/sizeof bad[0];i++)assert(!pd_defence_temporary_set(&s,0,bad[i])&&!memcmp(&s,&before,sizeof s));
 assert(!pd_defence_temporary_set(&s,13,(pd_defence_temporary){0})&&!pd_round_advance(NULL,&round)&&!pd_round_advance(&s,NULL));
 s.defences.temporary[0]=(pd_defence_temporary){.flags=1,.active=true,.timed=true};calls=0;assert(!pd_concentration_roll(&s,&(pd_damage_plan){.amount=5},0,0,rng,NULL,&next,&roll,&damage,false)&&!calls);
 /* Maximum-size PDD13 and every new field: CRC-valid malformed bytes reject. */
 setup(&s);s.magic.count=PD_POWERS;for(unsigned i=2;i<PD_POWERS;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};
 for(unsigned i=0;i<13;i++)assert(pd_defence_temporary_set(&s,i,(pd_defence_temporary){.flags=1+i%7,.active=i%2,.timed=true,.rounds=99,.concentration=1}));
 static uint8_t data[PD_RECORD_SIZE];size_t n=pd_encode(&s,135,data);uint32_t gen;
 assert(n==PD_RECORD_SIZE&&!memcmp(data,"PDDD",4)&&pd_decode(data,n,&decoded,&gen)&&gen==135);
 for(unsigned i=0;i<13;i++)assert(decoded.defences.temporary[i].flags==1+i%7&&decoded.defences.temporary[i].rounds==99&&decoded.defences.temporary[i].concentration==1&&decoded.defences.temporary[i].active==i%2);
 const unsigned invalid[]={8,2,2,100,2};for(unsigned type=0;type<13;type++)for(unsigned field=0;field<5;field++){pd_encode(&s,135,data);data[n-69+5*type+field]=invalid[field];crc(data,n);before=decoded;gen=17;assert(!pd_decode(data,n,&decoded,&gen)&&gen==17&&!memcmp(&decoded,&before,sizeof decoded));}
 printf("Temporary defences: %u independent flag/type/damage cases, %u timer ticks, unified atomic expiry, concentration transitions, impact snapshot, unknown bases, strict PDD13 and corrupt fields passed\n",cases,ticks);
}
