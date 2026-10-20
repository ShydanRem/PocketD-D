#include "pocketdd.h"
#include "combat.h"
#include "rest.h"
#include "status.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
static unsigned calls;
static uint32_t rng(void *unused){(void)unused;calls++;return 7;}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
 static pd_state s,before,next;unsigned comparisons=0;
 for(unsigned base=1;base<=40;base++)for(int delta=-20;delta<=20;delta++)for(unsigned active=0;active<2;active++){
  pd_demo(&s);s.armor=base;before=s;int expected=base+(active?delta:0);bool valid=expected>=1&&expected<=40;
  assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_ARMOR,.value=delta,.active=active})==valid);
  if(valid)assert(pd_valid(&s)&&pd_armor(&s)==(unsigned)expected&&s.armor==base&&s.hp==before.hp&&pd_max_hp(&s)==32);
  else {assert(!memcmp(&s,&before,sizeof s));}
  comparisons++;
 }
 const int deltas[]={-999,-31,-1,0,1,31,999};
 for(unsigned base=1;base<=999;base++)for(unsigned d=0;d<7;d++)for(unsigned active=0;active<2;active++){
  pd_demo(&s);s.hp=s.max_hp=base;s.temp_hp=5;before=s;int expected=base+(active?deltas[d]:0);bool valid=expected>=1&&expected<=999;
  assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=deltas[d],.active=active})==valid);
  if(valid){assert(pd_valid(&s)&&pd_max_hp(&s)==(unsigned)expected&&s.hp==(base<(unsigned)expected?base:(unsigned)expected)&&s.max_hp==base&&s.temp_hp==5);pd_hp(&s,INT_MAX);assert(s.hp==(unsigned)expected);}
  else {assert(!memcmp(&s,&before,sizeof s));}
  comparisons++;
 }
 pd_demo(&s);s.hp=32;assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true})&&s.hp==32&&pd_max_hp(&s)==42);
 pd_pool roll;unsigned gained;assert(pd_use_item(&s,1,rng,NULL,&next,&roll,&gained)&&next.hp==42&&gained==10&&next.items[1].quantity==1);
 s.recovery=(pd_recovery){.available=true,.count=1,.pools={{.name="Classe",.die=8,.maximum=1}}};
 assert(pd_spend_hit_die(&s,0,rng,NULL,&next,&roll,&gained)&&gained==10&&next.hp==42&&next.recovery.pools[0].used==1);
 pd_rest_plan plan={.long_rest=true};assert(pd_rest_apply(&s,&plan)&&s.hp==42&&s.effect_count==1&&s.effects[0].active&&s.max_hp==32);
 before=s;assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=false})&&s.hp==32&&pd_max_hp(&s)==32);s=before;
 assert(pd_effect_remove(&s,0)&&s.hp==32&&!s.effect_count&&s.max_hp==32);
 /* Negative effects persist through a long rest and determine the healing cap. */
 assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=-10,.active=true})&&s.hp==22);s.hp=10;assert(pd_rest_apply(&s,&plan)&&s.hp==22&&s.max_hp==32);
 assert(pd_effect_remove(&s,0)&&s.hp==22&&pd_max_hp(&s)==32); /* Removal is not healing. */
 s.hp=0;s.status.available=true;s.status.failures=3;assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true})&&!s.hp&&s.status.failures==3);pd_hp(&s,100);assert(!s.hp);
 assert(pd_death_change(&s,PD_DEATH_MINUS_FAILURE)&&pd_death_face(&s,20)&&s.hp==1&&s.effects[0].active);
 pd_demo(&s);assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_ARMOR,.value=20,.active=true})&&pd_effect_set(&s,1,(pd_effect){.target=PD_EFFECT_ARMOR,.value=-20,.active=true}));before=s;
 assert(!pd_effect_remove(&s,0)&&!memcmp(&s,&before,sizeof s)); /* The remaining penalty would leave CA below one. */
 assert(!pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_ARMOR,.value=20,.active=false})&&!memcmp(&s,&before,sizeof s));
 pd_demo(&s);
 for(unsigned i=0;i<PD_EFFECTS;i++)assert(pd_effect_set(&s,i,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=(int)i+1,.active=true}));
 assert(pd_max_hp(&s)==68&&s.max_hp==32);before=s;assert(!pd_effect_set(&s,8,(pd_effect){0})&&!memcmp(&s,&before,sizeof s));
 assert(pd_effect_remove(&s,2)&&s.effect_count==7&&s.effects[2].value==4&&s.effects[7].value==0&&!s.effects[7].active);
 before=s;const pd_effect bad[]={{.target=2,.value=1,.active=true},{.target=PD_EFFECT_ARMOR,.value=21,.active=true},{.target=PD_EFFECT_MAX_HP,.value=1000,.active=true},{.target=PD_EFFECT_MAX_HP,.value=-1000,.active=false}};
 for(unsigned i=0;i<4;i++)assert(!pd_effect_set(&s,0,bad[i])&&!memcmp(&s,&before,sizeof s));
 /* Annotated conditions never create or remove numeric effects. */
 assert(pd_condition_add(&s,"CA +99",6)&&pd_armor(&s)==16&&pd_max_hp(&s)==65);assert(pd_condition_remove(&s,0)&&s.effect_count==7);
 s.magic.count=PD_POWERS;for(unsigned i=0;i<PD_POWERS;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};
 s.shortcuts[0]=10;pd_hp(&s,999);static uint8_t data[PD_RECORD_SIZE];uint32_t generation;
 size_t size=pd_encode(&s,127,data);assert(size==PD_RECORD_SIZE&&pd_decode(data,size,&next,&generation)&&generation==127&&next.hp==65&&next.max_hp==32&&pd_max_hp(&next)==65&&next.shortcuts[0]==10&&next.effects[2].value==4);
 /* CRC-valid malformed fields, invalid effective totals and out-of-range HP fail atomically. */
 size_t start=size-134;const unsigned offsets[]={0,1,3,4};const unsigned corrupt[]={9,2,255,2};
 for(unsigned i=0;i<4;i++){pd_encode(&s,127,data);data[start+offsets[i]]=corrupt[i];crc(data,size);before=next;generation=99;assert(!pd_decode(data,size,&next,&generation)&&generation==99&&!memcmp(&next,&before,sizeof next));}
 pd_encode(&s,127,data);data[57]=231;data[58]=3;crc(data,size);assert(!pd_decode(data,size,&next,&generation));
 pd_demo(&s);s.effect_count=1;s.effects[0]=(pd_effect){.target=PD_EFFECT_ARMOR,.value=-20,.active=true};pd_encode(&s,1,data);size=pd_encoded_size(&s);assert(!pd_decode(data,size,&next,&generation));
 printf("Effects: %u independent base/delta cases, atomic limits, caps without healing, item/hit-die/rest integration, death guards, annotation separation, 8 effects and strict PDD13 roundtrip/corruption passed\n",comparisons);
}
