#include "pocketdd.h"
#include "magic.h"
#include "damage.h"
#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;
static uint32_t face;
static uint32_t rng(void *ctx){(void)ctx;calls++;return face;}
static void setup(pd_state *s){
 pd_demo(s);s->armor=20;s->hp=32;s->magic.count=4;s->magic.slots[0]=(pd_slot){3,3};s->combat.rage_bonus=2;
 s->magic.powers[0]=(pd_power){.name="Magia A",.level=1,.flags=PD_PREPARED|PD_CONCENTRATES|PD_USES_SLOT};
 s->magic.powers[1]=s->magic.powers[0];strcpy(s->magic.powers[1].name,"Magia B");
 s->magic.powers[2]=(pd_power){.name="Ira",.kind=1,.effect=1,.maximum=2,.remaining=2};
 s->magic.powers[3]=(pd_power){.name="Trucchetto",.flags=PD_PREPARED};
 s->magic.concentration=1;assert(pd_defence_set(s,0,true,0));assert(pd_valid(s));
}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
 static pd_state s,before,next,decoded;pd_damage_summary d;pd_roll roll;unsigned cases=0;pd_rest_summary summary;
 /* Independent final totals for all bound subsets, both targets, all end triggers.
    Paired deltas also ensure caps occur once, after every bound effect has ended. */
 for(unsigned mask=0;mask<256;mask++)for(unsigned target=0;target<2;target++)for(unsigned event=0;event<9;event++){
  setup(&s);s.effect_count=8;int unbound=target==PD_EFFECT_ARMOR?20:32;unsigned ended=0;
  for(unsigned i=0;i<8;i++){
   int value=i%2?-1:1;bool linked=mask&(1u<<i);
   s.effects[i]=(pd_effect){.target=target,.value=value,.active=true,.timed=true,.rounds=7,.concentration=linked?1:0};
   if(linked)ended++;else unbound+=value;
  }
  assert(pd_valid(&s));before=s;unsigned expected_hp=target==PD_EFFECT_MAX_HP&&unbound<32?(unsigned)unbound:32;
  switch(event){
   case 0:pd_concentration_end(&s);break;
   case 1:assert(pd_magic_use(&s,0,1,false)&&s.magic.slots[0].current==2&&s.magic.concentration==1);break; /* same instance index */
   case 2:assert(pd_magic_use(&s,1,1,false)&&s.magic.slots[0].current==2&&s.magic.concentration==2);break;
   case 3:assert(pd_magic_use(&s,2,0,false)&&s.magic.powers[2].remaining==1&&s.combat.raging);break;
   case 4:assert(pd_rest_preview(&s,&(pd_rest_plan){.long_rest=true},&next,&summary)&&summary.hp==(target==PD_EFFECT_MAX_HP?unbound:32)-32);s=next;expected_hp=target==PD_EFFECT_MAX_HP?unbound:32;break;
   case 5:pd_hp(&s,-32);expected_hp=0;break;
   case 6:{s.temp_hp=10;pd_damage_plan p={.amount=1,.concentration=PD_CONCENTRATION_END};assert(pd_damage_preview(&s,&p,&d)&&d.hp_after==expected_hp&&d.ended_effects==ended&&pd_damage_apply(&s,&p,&d));assert(s.temp_hp==9);break;}
   case 8:{pd_damage_plan p={.amount=32};assert(pd_damage_preview(&s,&p,&d)&&d.ends_concentration&&d.ended_effects==ended&&!d.hp_after&&pd_damage_apply(&s,&p,&d));expected_hp=0;break;}
   case 7:{s.temp_hp=10;face=0;calls=0;assert(pd_concentration_roll(&s,&(pd_damage_plan){.amount=1},0,0,rng,NULL,&next,&roll,&d,false)&&calls==1&&d.ended_effects==ended&&d.hp_after==expected_hp);s=next;break;}
  }
  assert(pd_valid(&s)&&s.hp==expected_hp&&pd_armor(&s)==(unsigned)(target==PD_EFFECT_ARMOR?unbound:20)&&pd_max_hp(&s)==(unsigned)(target==PD_EFFECT_MAX_HP?unbound:32));
  assert(s.magic.concentration==(event==1?1:event==2?2:0));
  for(unsigned i=0;i<8;i++)assert(s.effects[i].active==!(mask&(1u<<i))&&!s.effects[i].concentration&&s.effects[i].rounds==7&&s.effects[i].value==before.effects[i].value);
  cases++;
 }
 /* Kept concentration, non-concentrated spells, short rests and immune damage preserve bindings. */
 setup(&s);assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=1}));s.hp=41;s.temp_hp=10;
 before=s;assert(pd_magic_use(&s,3,0,false)&&!memcmp(&s,&before,sizeof s));assert(pd_rest_apply(&s,&(pd_rest_plan){0})&&!memcmp(&s,&before,sizeof s));
 face=19;calls=0;assert(pd_concentration_roll(&s,&(pd_damage_plan){.amount=1},0,0,rng,NULL,&next,&roll,&d,false)&&calls==1&&next.hp==41&&next.effects[0].active&&next.effects[0].concentration==1&&!d.ended_effects&&d.max_hp_after==42);
 assert(pd_defence_set(&s,0,true,PD_IMMUNITY));before=s;assert(pd_damage_apply(&s,&(pd_damage_plan){.amount=9999},&d)&&!memcmp(&s,&before,sizeof s));
 assert(pd_rest_preview(&s,&(pd_rest_plan){.long_rest=true},&next,&summary)&&summary.hp==-9&&summary.hp<0&&next.hp==32);
 /* Expiring the timer does not end concentration or reanimate on recast. */
 pd_effect e=s.effects[0];e.timed=true;e.rounds=1;assert(pd_effect_set(&s,0,e));uint8_t advanced,expired;
 assert(pd_effect_advance(&s,&advanced,&expired)&&advanced==1&&expired==1&&s.magic.concentration==1&&s.hp==32&&!s.effects[0].active&&s.effects[0].concentration==1);
 assert(pd_magic_use(&s,0,1,false)&&s.magic.concentration==1&&!s.effects[0].concentration&&!s.effects[0].active&&!s.effects[0].rounds);
 /* Removing a negative bound maximum increases maximum but never heals. */
 setup(&s);assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=-10,.active=true,.concentration=1})&&s.hp==22);pd_concentration_end(&s);assert(s.hp==22&&pd_max_hp(&s)==32);
 /* Reject cross-spell/stale links, including inactive ones, with no mutation. */
 setup(&s);before=s;assert(!pd_effect_set(&s,0,(pd_effect){.concentration=2})&&!memcmp(&s,&before,sizeof s));pd_concentration_end(&s);before=s;assert(!pd_effect_set(&s,0,(pd_effect){.concentration=1})&&!memcmp(&s,&before,sizeof s));
 /* Totals must remain legal when every bound effect ends. Cover lower/upper bounds. */
 for(unsigned target=0;target<2;target++)for(unsigned upper=0;upper<2;upper++){
  setup(&s);s.armor=upper?40:1;s.max_hp=upper?999:1;s.hp=1;
  assert(pd_effect_set(&s,0,(pd_effect){.target=target,.value=upper?-1:1,.active=true,.concentration=1}));
  before=s;assert(!pd_effect_set(&s,1,(pd_effect){.target=target,.value=upper?1:-1,.active=true})&&!memcmp(&s,&before,sizeof s));
 }
 /* Malformed state is rejected before entropy; insufficient spell costs keep everything. */
 setup(&s);s.effects[0]=(pd_effect){.concentration=2};s.effect_count=1;before=s;calls=0;memset(&next,0x55,sizeof next);decoded=next;memset(&d,0x33,sizeof d);pd_damage_summary old_d=d;
 assert(!pd_concentration_roll(&s,&(pd_damage_plan){.amount=1},0,0,rng,NULL,&next,&roll,&d,false)&&!calls&&!memcmp(&next,&decoded,sizeof next)&&!memcmp(&d,&old_d,sizeof d));
 assert(!pd_magic_use(&s,0,1,false)&&!memcmp(&s,&before,sizeof s));
 setup(&s);assert(pd_effect_set(&s,0,(pd_effect){.value=2,.active=true,.concentration=1}));s.magic.slots[0].current=0;before=s;assert(!pd_magic_use(&s,1,1,false)&&!memcmp(&s,&before,sizeof s));
 /* Full-size persistence, every link byte, dangling/unused references with valid CRC. */
 setup(&s);s.magic.count=PD_POWERS;for(unsigned i=4;i<PD_POWERS;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};
 for(unsigned i=0;i<PD_EFFECTS;i++)assert(pd_effect_set(&s,i,(pd_effect){.value=1,.active=i%2,.concentration=1}));
 static uint8_t data[PD_RECORD_SIZE];size_t n=pd_encode(&s,133,data);uint32_t gen;
 assert(n==PD_RECORD_SIZE&&!memcmp(data,"PDDW",4)&&pd_decode(data,n,&decoded,&gen)&&gen==133&&decoded.effects[7].concentration==1&&decoded.effects[0].concentration==1);
 for(unsigned i=0;i<PD_EFFECTS;i++){pd_encode(&s,133,data);data[n-975+i]=2;crc(data,n);before=decoded;gen=77;assert(!pd_decode(data,n,&decoded,&gen)&&gen==77&&!memcmp(&decoded,&before,sizeof decoded));}
 setup(&s);n=pd_encode(&s,1,data);data[n-975]=1;crc(data,n);assert(!pd_decode(data,n,&decoded,&gen));
 printf("Links: %u independent subsets/targets/end triggers, same-spell recast, costs, caps, rolls, preserved paths, expiration, stale links, invariant rejection and strict PDD24 persistence passed\n",cases);
}
