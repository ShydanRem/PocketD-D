#include "damage.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
 static pd_state s,before,next;pd_damage_summary r,output_before;pd_damage_plan plan={0};unsigned cases=0;
 pd_demo(&s);before=s;memset(&r,0x5a,sizeof r);output_before=r;
 /* Every subset: explicit normal completion changes only the configured mask.
    Conditional base rules, all three grants and unrelated state stay byte-exact. */
 for(unsigned mask=0;mask<8192;mask++){
  pd_demo(&s);
  for(unsigned type=0;type<PD_DAMAGE_TYPES;type++){
   if(mask&(1u<<type)){assert(pd_defence_set(&s,type,true,type%8));if(type%8&1)assert(pd_defence_rule_set(&s,type,false,0,PD_RULE_NONMAGICAL));}
   for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++)assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=1+type%7,.active=true,.timed=true,.rounds=slot+1}));
  }
  before=s;next=s;next.defences.configured=8191;assert(pd_defence_complete_normals(&s)&&pd_valid(&s)&&!memcmp(&s,&next,sizeof s));
  assert(pd_defence_complete_normals(&s)&&!memcmp(&s,&next,sizeof s));
  if(mask%127==0){static uint8_t record[PD_RECORD_SIZE];uint32_t generation;size_t size=pd_encode(&s,12,record);assert(pd_decode(record,size,&next,&generation)&&generation==12&&!memcmp(&s,&next,sizeof s));}
 }
 assert(!pd_defence_complete_normals(NULL));s.level=0;before=s;assert(!pd_defence_complete_normals(&s)&&!memcmp(&s,&before,sizeof s));
 pd_demo(&s);before=s;
 assert(!pd_damage_apply(&s,&plan,&r)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&output_before,sizeof r));
 assert(pd_damage_preview(&s,&plan,&r)&&!r.configured&&!r.allowed&&r.hp_after==24);
 const unsigned hps[]={0,1,24},temps[]={0,1,7};
 for(unsigned type=0;type<PD_DAMAGE_TYPES;type++)for(unsigned flags=0;flags<8;flags++)for(unsigned amount=0;amount<64;amount++)for(unsigned h=0;h<3;h++)for(unsigned t=0;t<3;t++){
  pd_demo(&s);s.hp=hps[h];s.temp_hp=temps[t];s.status.available=true;s.status.failures=s.hp?0:1;
  assert(pd_defence_set(&s,type,true,flags));before=s;plan=(pd_damage_plan){.type=type,.amount=amount};
  unsigned expected=(flags&4)?0:(flags&1)?amount/2:amount;if(!(flags&4)&&(flags&2))expected*=2;
  unsigned absorbed=expected<temps[t]?expected:temps[t],remainder=expected-absorbed,hp_after=remainder<hps[h]?hps[h]-remainder:0;
  bool at_zero=!hps[h]&&expected>0,massive=hps[h]>0&&remainder>=hps[h]+32,allowed=!at_zero&&!massive;
  assert(pd_damage_preview(&s,&plan,&r)&&r.configured&&r.allowed==allowed&&r.adjusted==expected&&r.absorbed==absorbed&&r.hp_after==hp_after&&r.temp_after==temps[t]-absorbed&&r.at_zero==at_zero&&r.massive==massive&&!r.needs_concentration&&!memcmp(&s,&before,sizeof s));
  memset(&r,0x5a,sizeof r);output_before=r;assert(pd_damage_apply(&s,&plan,&r)==allowed);
  if(allowed){assert(s.hp==hp_after&&s.temp_hp==temps[t]-absorbed&&pd_valid(&s));if(!expected)assert(!memcmp(&s,&before,sizeof s));}
  else {assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&output_before,sizeof r));}
  assert(s.defences.configured==before.defences.configured&&s.max_hp==32);cases++;
 }
 /* All supported amounts, including odd rounding and extreme doubling. */
 for(unsigned amount=0;amount<=9999;amount++)for(unsigned flags=0;flags<8;flags++){
  pd_demo(&s);assert(pd_defence_set(&s,5,true,flags));plan=(pd_damage_plan){.type=5,.amount=amount};
  unsigned expected=amount;if(flags&1)expected=expected/2;if(flags&2)expected=expected*2;if(flags&4)expected=0;
  assert(pd_damage_preview(&s,&plan,&r)&&r.adjusted==expected);cases++;
 }
 /* Temporary HP absorbs damage, but does not eliminate the concentration check. */
 pd_demo(&s);s.temp_hp=99;s.magic.count=1;s.magic.powers[0]=(pd_power){.name="Concentrazione",.flags=PD_PREPARED|PD_CONCENTRATES};s.magic.concentration=1;
 assert(pd_defence_set(&s,5,true,0));plan=(pd_damage_plan){.type=5,.amount=25};before=s;
 assert(pd_damage_preview(&s,&plan,&r)&&r.needs_concentration&&r.concentration_dc==12&&!r.allowed&&r.hp_after==24&&r.temp_after==74);
 assert(!pd_damage_apply(&s,&plan,&r)&&!memcmp(&s,&before,sizeof s));plan.concentration=PD_CONCENTRATION_KEEP;
 assert(pd_damage_apply(&s,&plan,&r)&&s.magic.concentration==1&&s.hp==24&&s.temp_hp==74);s=before;plan.concentration=PD_CONCENTRATION_END;
 assert(pd_damage_apply(&s,&plan,&r)&&!s.magic.concentration&&s.temp_hp==74);s=before;
 assert(pd_defence_set(&s,5,true,PD_IMMUNITY));plan.concentration=PD_CONCENTRATION_END;before=s;
 assert(pd_damage_apply(&s,&plan,&r)&&!memcmp(&s,&before,sizeof s)&&s.magic.concentration==1&&!r.needs_concentration);
 /* Dropping to zero clears rage/concentration and starts the existing death registry. */
 pd_demo(&s);s.magic.count=1;s.magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};s.magic.concentration=1;
 assert(pd_defence_set(&s,0,true,0));s.status.available=true;s.status.failures=2;plan=(pd_damage_plan){.amount=24};
 assert(pd_damage_apply(&s,&plan,&r)&&!s.hp&&!s.magic.concentration&&!s.status.failures&&r.ends_concentration);
 /* Effective max HP sets massive-damage boundary; raw corrections stay separate. */
 pd_demo(&s);assert(pd_defence_set(&s,0,true,0)&&pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.timed=true,.rounds=2}));
 plan=(pd_damage_plan){.amount=65};assert(pd_damage_preview(&s,&plan,&r)&&!r.massive&&r.allowed);plan.amount=66;before=s;
 assert(pd_damage_preview(&s,&plan,&r)&&r.massive&&!r.allowed&&!pd_damage_apply(&s,&plan,&r)&&!memcmp(&s,&before,sizeof s));
 /* Defence profiles are explicit and persist independently from round effects and rage. */
 s.combat.rage_bonus=2;s.combat.raging=true;plan.amount=10;assert(pd_damage_apply(&s,&plan,&r)&&r.adjusted==10&&s.effects[0].rounds==2);
 uint8_t a,e;assert(pd_effect_advance(&s,&a,&e)&&pd_defence_flags(&s,0)==0&&s.defences.configured==1);
 next=s;plan.amount=s.hp;assert(pd_damage_apply(&s,&plan,&r)&&!s.hp&&!s.combat.raging&&!s.training.strength_advantage);s=next;
 before=s;assert(!pd_defence_set(&s,13,true,0)&&!pd_defence_set(&s,0,false,1)&&!pd_defence_set(&s,0,true,8)&&!memcmp(&s,&before,sizeof s));
 assert(pd_defence_set(&s,0,false,0)&&!s.defences.configured&&pd_defence_flags(&s,0)==0);
 /* Strict input validation and output preservation. */
 before=s;plan=(pd_damage_plan){.amount=10000};memset(&r,0x5a,sizeof r);output_before=r;
 assert(!pd_damage_preview(&s,&plan,&r)&&!pd_damage_apply(&s,&plan,&r)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&output_before,sizeof r));
 plan=(pd_damage_plan){.type=13};assert(!pd_damage_preview(&s,&plan,&r));plan=(pd_damage_plan){.concentration=3};assert(!pd_damage_preview(&s,&plan,&r));
 plan=(pd_damage_plan){.amount=UINT_MAX};assert(!pd_damage_preview(&s,&plan,&r));assert(!pd_damage_preview(NULL,&plan,&r)&&!pd_damage_preview(&s,NULL,&r)&&!pd_damage_preview(&s,&plan,NULL));
 assert(!pd_damage_name(13)&&!pd_damage_name(UINT_MAX)&&!strcmp(pd_damage_name(12),"Tuono"));
 /* Full profile and largest PDD23 record roundtrip, including unknown vs normal. */
 pd_demo(&s);for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)assert(pd_defence_set(&s,i,true,i%8));
 s.magic.count=PD_POWERS;for(unsigned i=0;i<PD_POWERS;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};s.shortcuts[0]=12;
 static uint8_t data[PD_RECORD_SIZE];size_t n=pd_encode(&s,131,data);uint32_t gen;
 assert(n==PD_RECORD_SIZE&&!memcmp(data,PD_RECORD_MAGIC,4)&&pd_decode(data,n,&next,&gen)&&gen==131&&next.defences.configured==8191&&next.shortcuts[0]==12);
 for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)assert(pd_defence_flags(&next,i)==i%8);
 size_t start=n-1002;const unsigned offsets[]={1,3,5,7};
 for(unsigned i=0;i<4;i++){pd_encode(&s,131,data);data[start+offsets[i]]|=0x80;crc(data,n);before=next;gen=99;assert(!pd_decode(data,n,&next,&gen)&&gen==99&&!memcmp(&next,&before,sizeof next));}
 pd_encode(&s,131,data);data[start]=0;data[start+1]=0;crc(data,n);assert(!pd_decode(data,n,&next,&gen));
 printf("Damage: %u independent typed/flag/amount/HP/temp cases, explicit configuration, rounding, immunity, concentration decisions, death/massive guards, atomic rejection and PDD24 persistence passed\n",cases);
}
