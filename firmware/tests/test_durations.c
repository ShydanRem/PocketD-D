#include "pocketdd.h"
#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
 static pd_state s,before,next;uint8_t advanced,expired;unsigned cases=0;
 /* Independent expectation for every supported timer, target and activation. */
 for(unsigned duration=1;duration<=99;duration++)for(unsigned target=0;target<2;target++)for(unsigned active=0;active<2;active++)for(int sign=-1;sign<=1;sign+=2){
  pd_demo(&s);s.hp=32;s.temp_hp=3;
  assert(pd_effect_set(&s,0,(pd_effect){.target=target,.value=sign,.active=active,.timed=true,.rounds=duration}));
  for(unsigned tick=1;tick<=duration+1;tick++){
   before=s;assert(pd_effect_advance(&s,&advanced,&expired));
   unsigned remaining=active?(tick>=duration?0:duration-tick):duration;
   assert(s.effects[0].rounds==remaining&&s.effects[0].active==(active&&remaining>0));
   assert(advanced==(active&&tick<=duration?1:0)&&expired==(active&&tick==duration?1:0));
   int bonus=active&&remaining?sign:0;
   assert(pd_armor(&s)==(unsigned)(16+(target==PD_EFFECT_ARMOR?bonus:0)));
   assert(pd_max_hp(&s)==(unsigned)(32+(target==PD_EFFECT_MAX_HP?bonus:0))&&s.temp_hp==3&&pd_valid(&s));
   if(!advanced)assert(!memcmp(&s,&before,sizeof s));
   cases++;
  }
 }
 /* All subsets of eight timers expire together, without order-dependent caps. */
 for(unsigned mask=0;mask<256;mask++){
  pd_demo(&s);for(unsigned i=0;i<PD_EFFECTS;i++)assert(pd_effect_set(&s,i,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=1,.active=true,.timed=true,.rounds=(mask&(1u<<i))?1:2}));
  s.hp=40;s.temp_hp=7;assert(pd_effect_advance(&s,&advanced,&expired)&&advanced==255&&expired==mask);
  unsigned left=0;for(unsigned i=0;i<PD_EFFECTS;i++){bool ends=mask&(1u<<i);assert(s.effects[i].rounds==(ends?0:1)&&s.effects[i].active==!ends);left+=!ends;}
  assert(s.hp==32+left&&pd_max_hp(&s)==32+left&&s.temp_hp==7);cases++;
 }
 /* Paired opposing effects must expire simultaneously, not one at a time. */
 pd_demo(&s);assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_ARMOR,.value=20,.active=true,.timed=true,.rounds=1}));
 assert(pd_effect_set(&s,1,(pd_effect){.target=PD_EFFECT_ARMOR,.value=-20,.active=true,.timed=true,.rounds=1}));
 assert(pd_effect_advance(&s,&advanced,&expired)&&advanced==3&&expired==3&&pd_armor(&s)==16);
 pd_demo(&s);assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=999,.active=false,.timed=true,.rounds=1}));
 /* Invalid final totals roll back every timer, flag, HP and output mask. */
 pd_demo(&s);assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_ARMOR,.value=20,.active=true,.timed=true,.rounds=1}));
 assert(pd_effect_set(&s,1,(pd_effect){.target=PD_EFFECT_ARMOR,.value=-20,.active=true,.timed=true,.rounds=2}));
 before=s;advanced=81;expired=72;assert(!pd_effect_advance(&s,&advanced,&expired)&&advanced==81&&expired==72&&!memcmp(&s,&before,sizeof s));
 pd_demo(&s);s.max_hp=1;s.hp=1;assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=1,.active=true,.timed=true,.rounds=1}));
 assert(pd_effect_set(&s,1,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=-1,.active=true}));before=s;
 assert(!pd_effect_advance(&s,&advanced,&expired)&&!memcmp(&s,&before,sizeof s));
 pd_demo(&s);s.hp=999;s.max_hp=999;assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=-1,.active=true,.timed=true,.rounds=1}));
 assert(pd_effect_set(&s,1,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=1,.active=true}));before=s;
 assert(!pd_effect_advance(&s,&advanced,&expired)&&!memcmp(&s,&before,sizeof s));
 /* Expiring a penalty increases the maximum without restoring lost current HP. */
 pd_demo(&s);s.hp=32;assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=-10,.active=true,.timed=true,.rounds=1})&&s.hp==22);
 assert(pd_effect_advance(&s,&advanced,&expired)&&s.hp==22&&pd_max_hp(&s)==32&&s.effects[0].timed&&!s.effects[0].active&&!s.effects[0].rounds);
 pd_effect e=s.effects[0];e.active=true;before=s;assert(!pd_effect_set(&s,0,e)&&!memcmp(&s,&before,sizeof s));e.rounds=2;assert(pd_effect_set(&s,0,e));
 /* Rests, zero HP, concentration and unrelated annotations do not advance timers. */
 pd_rest_plan plan={.long_rest=true};assert(pd_rest_apply(&s,&plan)&&s.effects[0].rounds==2&&s.effects[0].active);
 pd_hp(&s,-999);assert(!s.hp&&s.effects[0].rounds==2&&s.effects[0].active);s.status.available=true;s.status.failures=3;
 assert(pd_effect_advance(&s,&advanced,&expired)&&s.effects[0].rounds==1&&s.status.failures==3&&!s.hp);
 assert(pd_effect_advance(&s,&advanced,&expired)&&s.status.failures==3&&!s.hp&&pd_max_hp(&s)==32);
 assert(!pd_effect_advance(NULL,&advanced,&expired)&&!pd_effect_advance(&s,NULL,&expired)&&!pd_effect_advance(&s,&advanced,&advanced));
 const pd_effect bad[]={{.timed=false,.rounds=1},{.timed=true,.active=true},{.timed=true,.rounds=100}};
 before=s;for(unsigned i=0;i<3;i++)assert(!pd_effect_set(&s,0,bad[i])&&!memcmp(&s,&before,sizeof s));
 /* Timer fields survive the largest record; malformed data with valid CRC rejects atomically. */
 pd_demo(&s);for(unsigned i=0;i<PD_EFFECTS;i++)assert(pd_effect_set(&s,i,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=1,.active=i%2!=0,.timed=true,.rounds=i?99:0}));
 s.magic.count=PD_POWERS;for(unsigned i=0;i<PD_POWERS;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};s.shortcuts[0]=11;
 static uint8_t data[PD_RECORD_SIZE];size_t n=pd_encode(&s,129,data);uint32_t gen;
 assert(n==PD_RECORD_SIZE&&!memcmp(data,"PDDA",4)&&pd_decode(data,n,&next,&gen)&&gen==129&&next.effects[7].rounds==99&&next.effects[0].timed&&!next.effects[0].active&&!next.effects[0].rounds&&next.shortcuts[0]==11);
 size_t start=n-20;const unsigned offsets[]={0,1,2,3};const unsigned values[]={2,100,0,0};
 for(unsigned i=0;i<4;i++){pd_encode(&s,129,data);data[start+offsets[i]]=values[i];crc(data,n);before=next;gen=99;assert(!pd_decode(data,n,&next,&gen)&&gen==99&&!memcmp(&next,&before,sizeof next));}
 printf("Durations: %u independent advances/subsets, pause/expiry, simultaneous caps, atomic rejection, explicit reactivation, death/rest isolation and strict PDD10 persistence passed\n",cases);
}
