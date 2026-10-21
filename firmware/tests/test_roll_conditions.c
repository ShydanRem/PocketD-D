#include "combat.h"
#include "status.h"
#include "rest.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;
static uint32_t rng(void *p){(void)p;return calls++%2?19:0;}
/* Independent complete-context oracle: failed sight checks have no d20 outcome. */
static unsigned oracle(unsigned flags,unsigned scope,unsigned ability,int requested,bool poison,bool rage,bool ignore,unsigned facts){
 if(ignore){flags=0;poison=rage=false;}
 if(scope<PD_SAVE&&(flags&PD_BLINDED)&&(facts&1))return 3;
 bool adv=requested>0||(scope!=PD_ROLL_ATTACK&&rage&&ability==0);
 bool dis=requested<0||(poison&&scope!=PD_SAVE);
 if(scope==PD_ROLL_ATTACK)dis|=(flags&(PD_BLINDED|PD_PRONE|PD_RESTRAINED))!=0;
 if(scope==PD_SAVE)dis|=(flags&PD_RESTRAINED)&&ability==1;
 if(scope!=PD_SAVE&&(flags&PD_FRIGHTENED)&&(facts&2))dis=true;
 return adv==dis?1:adv?2:0;
}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
 static pd_state s,before,out,expected;pd_demo(&s);unsigned cases=0;
 const unsigned abilities[]={1,4,3,0,5,3,4,5,3,4,3,4,5,5,3,1,1,4};
 for(unsigned flags=0;flags<16;flags++)for(unsigned scope=0;scope<4;scope++)for(unsigned index=0;index<(scope==PD_SKILL?18:scope==PD_ROLL_ATTACK?1:6);index++)for(int requested=-1;requested<=1;requested++)for(unsigned poison=0;poison<2;poison++)for(unsigned rage=0;rage<2;rage++)for(unsigned ignore=0;ignore<2;ignore++)for(unsigned known=0;known<4;known++)for(unsigned values=0;values<4;values++){
  if(values&~known)continue;unsigned ability=scope==PD_SKILL?abilities[index]:scope==PD_ROLL_ATTACK?0:index;
  s.roll_conditions=flags;s.training.poisoned=poison;s.training.strength_advantage=rage;before=s;pd_roll_context context={known,values};unsigned possible=0,needed=0;
  for(unsigned facts=0;facts<4;facts++)if((facts&known)==values){unsigned result=oracle(flags,scope,ability,requested,poison,rage,ignore,facts);possible|=1u<<result;for(unsigned bit=1;bit<=2;bit<<=1)if(!(known&bit)&&result!=oracle(flags,scope,ability,requested,poison,rage,ignore,facts^bit))needed|=bit;}
  bool allowed=(possible&(possible-1))==0&&possible!=8;int mode=possible==1?-1:possible==4?1:0;calls=0;
  if(scope==PD_ROLL_ATTACK){
   pd_attack_resolution r;assert(pd_attack_resolve(&s,requested,context,ignore,&r)&&r.allowed==allowed&&r.needed==needed);if(allowed)assert(r.mode==mode);
   pd_strike hit;memset(&hit,0xa5,sizeof hit);pd_strike untouched=hit;assert(pd_attack_with_context(&s,15,requested,context,ignore,rng,NULL,&hit)==allowed);
   if(allowed)assert(calls==(mode?2:1)&&hit.roll.natural==(mode==1?20:1));else assert(!calls&&!memcmp(&hit,&untouched,sizeof hit));
  }else{
   pd_check_resolution r;assert(pd_check_resolve(&s,scope,index,requested,ignore,context,&r)&&r.allowed==allowed&&r.needed==needed&&r.automatic_failure==(possible==8));if(allowed)assert(r.mode==mode);
   pd_roll roll;memset(&roll,0xa5,sizeof roll);pd_roll untouched=roll;assert(pd_check_roll(&s,scope,index,requested,ignore,context,rng,NULL,&roll)==allowed);
   if(allowed)assert(calls==(mode?2:1)&&roll.natural==(mode==1?20:1)&&roll.total==r.info.total+(mode==1?20:1));else assert(!calls&&!memcmp(&roll,&untouched,sizeof roll));
  }
  assert(!memcmp(&s,&before,sizeof s));cases++;
 }
 /* Notes never activate mechanical flags; changing flags never rewrites notes or HP. */
 pd_demo(&s);assert(pd_condition_add(&s,"Prono",0)&&pd_condition_add(&s,"Accecato",0)&&!s.roll_conditions);before=s;assert(pd_condition_effects_set(&s,15));expected=before;expected.roll_conditions=15;assert(!memcmp(&s,&expected,sizeof s));before=s;assert(!pd_condition_effects_set(&s,16)&&!pd_condition_effects_set(&s,UINT_MAX)&&!pd_condition_effects_set(NULL,0)&&!memcmp(&s,&before,sizeof s));
 pd_round_summary summary;assert(pd_round_advance(&s,&summary)&&s.roll_conditions==15&&pd_time_advance(&s,60,&summary)&&s.roll_conditions==15&&pd_rest_apply(&s,&(pd_rest_plan){.long_rest=true})&&s.roll_conditions==15);
 /* Imported unknown bonuses remain blocked even with explicit exclusion of effects. */
 pd_demo(&s);s.roll_conditions=15;s.training.blocked=1;before=s;pd_roll roll;memset(&roll,0xa5,sizeof roll);pd_roll untouched=roll;calls=0;assert(!pd_check_roll(&s,PD_CHECK,0,1,true,(pd_roll_context){3,0},rng,NULL,&roll)&&!calls&&!memcmp(&roll,&untouched,sizeof roll)&&!memcmp(&s,&before,sizeof s));
 s.training.blocked=0;s.training.available=false;assert(!pd_check_roll(&s,PD_SAVE,0,0,true,(pd_roll_context){0},rng,NULL,&roll)&&!calls);assert(pd_check_roll(&s,PD_CHECK,0,0,true,(pd_roll_context){0},rng,NULL,&roll)&&calls==1);
 /* Malformed contexts and requests reject before RNG and leave outputs exact. */
 pd_demo(&s);s.roll_conditions=15;const pd_roll_context invalid[]={{4,0},{0,1},{1,2},{3,4}};
 for(unsigned i=0;i<4;i++){calls=0;roll=untouched;pd_check_resolution r;memset(&r,0xa5,sizeof r);pd_check_resolution original=r;assert(!pd_check_resolve(&s,PD_CHECK,0,0,false,invalid[i],&r)&&!memcmp(&r,&original,sizeof r));assert(!pd_check_roll(&s,PD_CHECK,0,0,false,invalid[i],rng,NULL,&roll)&&!calls&&!memcmp(&roll,&untouched,sizeof roll));}
 calls=0;assert(!pd_check_roll(&s,PD_CHECK,0,2,false,(pd_roll_context){0},rng,NULL,&roll)&&!pd_check_roll(NULL,PD_CHECK,0,0,false,(pd_roll_context){0},rng,NULL,&roll)&&!pd_check_roll(&s,PD_CHECK,0,0,false,(pd_roll_context){0},NULL,NULL,&roll)&&!pd_check_roll(&s,PD_CHECK,0,0,false,(pd_roll_context){0},rng,NULL,NULL)&&!calls);
 /* Known failures need no fear declaration, and duplicate disadvantage cancels once. */
 pd_check_resolution r;assert(pd_check_resolve(&s,PD_CHECK,0,1,false,(pd_roll_context){1,1},&r)&&r.automatic_failure&&!r.needed&&!r.allowed);s.roll_conditions=PD_FRIGHTENED|PD_PRONE;pd_attack_resolution attack;assert(pd_attack_resolve(&s,1,(pd_roll_context){0},false,&attack)&&attack.allowed&&!attack.needed&&!attack.mode);
 /* One-byte PDD23 extension: every mask, CRC-valid invalid flags, all legacy data stable. */
 pd_demo(&s);s.magic.count=120;for(unsigned i=0;i<120;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};static uint8_t data[PD_RECORD_SIZE];uint32_t generation;
 for(unsigned flags=0;flags<16;flags++){s.roll_conditions=flags;size_t n=pd_encode(&s,230,data);assert(n==PD_RECORD_SIZE&&!memcmp(data,"PDDN",4)&&pd_decode(data,n,&out,&generation)&&generation==230&&out.roll_conditions==flags);}
 for(unsigned flags=16;flags<256;flags++){size_t n=pd_encode(&s,230,data);data[n-5]=flags;crc(data,n);before=out;generation=9;assert(!pd_decode(data,n,&out,&generation)&&generation==9&&!memcmp(&out,&before,sizeof out));}
 printf("Roll conditions: %u independent flags/scope/ability/mode/poison/rage/exclusion/partial-context cases, complete-context oracle, exact RNG guards, note separation and strict PDD23 passed\n",cases);
}
