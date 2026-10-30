#include "combat.h"
#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state s,before,restored;static uint8_t data[PD_RECORD_SIZE];
static unsigned calls,face;static uint32_t random_face(void *ctx){(void)ctx;calls++;return face;}
static void crc(size_t n){uint32_t v=pd_crc32(data,n-4);for(unsigned i=0;i<4;i++)data[n-4+i]=v>>(8*i);}
int main(void){unsigned cases=0;uint32_t generation;
 for(unsigned effect=PD_HEX_2024;effect<=PD_HUNTERS_MARK_2024;effect++)for(unsigned ability=0;ability<6;ability++)for(unsigned level=1;level<=20;level++)for(unsigned critical=0;critical<2;critical++){
  pd_demo(&s);s.level=level;s.magic.count=2;s.magic.concentration=0;s.magic.powers[0]=(pd_power){.name="Anonymous cantrip",.effect=PD_TRUE_STRIKE_2024,.ability=ability,.flags=PD_PREPARED};s.magic.powers[1]=(pd_power){.name="Anonymous rider",.effect=effect,.level=1,.ability=ability,.flags=PD_PREPARED|PD_CONCENTRATES|PD_USES_SLOT};s.magic.slots[0]=(pd_slot){1,1};s.magic.pact[2]=(pd_slot){1,1};assert(pd_valid(&s)&&pd_on_hit_spell(&s)<0);before=s;
  assert(!pd_magic_use(&s,1,2,true)&&!memcmp(&s,&before,sizeof s));assert(pd_magic_use(&s,1,3,true));assert(s.magic.pact[2].current==0&&s.magic.slots[0].current==1&&pd_on_hit_spell(&s)==1);
  pd_strike hit;face=critical?19:9;assert(pd_true_strike_attack(&s,0,false,1,0,(pd_roll_context){0},false,random_face,NULL,&hit));assert(hit.hits&&hit.critical==(bool)critical);face=0;unsigned old=calls;before=s;pd_weapon_damage_parts result;
  assert(pd_weapon_damage_with_effect(&s,&hit,true,random_face,NULL,&result));assert(result.extra_effect==effect&&result.extra.count==1+critical&&result.extra.total==(int)(1+critical)&&result.extra.sides==6&&result.extra.bonus==0);assert(result.total==result.weapon.total+result.radiant.total+result.extra.total&&calls==old+result.weapon.count+result.radiant.count+result.extra.count&&!memcmp(&s,&before,sizeof s));
  assert(pd_magic_prepare(&s,1,false)&&pd_on_hit_spell(&s)==1);pd_concentration_end(&s);old=calls;pd_weapon_damage_parts saved=result;assert(!pd_weapon_damage_with_effect(&s,&hit,true,random_face,NULL,&result)&&calls==old&&!memcmp(&result,&saved,sizeof result));assert(pd_weapon_damage_with_effect(&s,&hit,false,random_face,NULL,&result)&&!result.extra.count);
  s=before;size_t n=pd_encode(&s,88,data);assert(!memcmp(data,"PDDd",4)&&pd_decode(data,n,&restored,&generation)&&generation==88&&!memcmp(&s,&restored,sizeof s));for(unsigned tag='a';tag<='c';tag++){data[3]=tag;crc(n);restored=s;assert(!pd_decode(data,n,&restored,&generation)&&!memcmp(&s,&restored,sizeof s));}
  s=before;pd_end_rage(&s);pd_concentration_end(&s);s.hp=0;assert(pd_on_hit_spell(&s)<0);cases++;
 }
 pd_demo(&s);s.magic.count=1;s.magic.powers[0]=(pd_power){.name="Synthetic",.level=1,.effect=PD_HEX_2024,.flags=PD_PREPARED|PD_CONCENTRATES,.maximum=1,.remaining=1,.recovery=2};s.magic.powers[0].flags|=PD_LIMITED;assert(pd_magic_use(&s,0,0,false)&&s.magic.powers[0].remaining==0&&pd_on_hit_spell(&s)==0);assert(!pd_magic_use(&s,0,0,false));
 puts("On-hit spells: 480 effect/ability/level/critical combinations, explicit target, active concentration, ordinary/pact isolation, finite innate use, frozen three-part damage, no slot on hit, no invalid RNG/output mutation and PDDd/old-tag rejection passed; synthetic.");printf("Cases: %u\n",cases);
}
