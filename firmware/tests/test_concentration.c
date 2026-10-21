#include "damage.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls,at;static uint32_t values[2];
static uint32_t rng(void *unused){(void)unused;calls++;return values[(at++)%2];}
static void setup(pd_state *s){pd_demo(s);s->temp_hp=99;s->magic.count=1;s->magic.powers[0]=(pd_power){.name="Concentrazione",.flags=PD_PREPARED|PD_CONCENTRATES};s->magic.concentration=1;assert(pd_defence_set(s,0,true,0));}
int main(void){
 static pd_state s,next,before;pd_roll roll,roll_before;pd_damage_summary summary,summary_before;pd_check_info info;pd_damage_plan plan={.amount=25};unsigned cases=0;
 const int extras[]={-30,0,30};
 for(unsigned level=1;level<=20;level++)for(unsigned score=1;score<=30;score++)for(unsigned rank=0;rank<2;rank++)for(unsigned e=0;e<3;e++)for(unsigned face=1;face<=20;face++){
  setup(&s);s.level=level;s.abilities[2]=score;s.training.saves=rank?4:0;s.training.save_bonus[2]=extras[e];s.training.poisoned=true;s.training.strength_advantage=true;
  int local=extras[(score+level)%3],base=(int)(score/2)-5,proficiency=rank?(2+(level-1)/4):0,bonus=base+proficiency+extras[e]+local;
  before=s;calls=at=0;values[0]=face-1;
  assert(pd_concentration_roll(&s,&plan,0,local,rng,NULL,&next,&roll,&summary,false)&&calls==1&&roll.count==1&&roll.natural==face&&roll.bonus==bonus&&roll.total==(int)face+bonus);
  bool pass=(int)face+bonus>=12;assert(next.magic.concentration==(pass?1:0)&&next.hp==24&&next.temp_hp==74&&summary.concentration_dc==12&&summary.ends_concentration==!pass&&!memcmp(&s,&before,sizeof s));cases++;
 }
 for(int mode=-1;mode<=1;mode+=2)for(unsigned a=1;a<=20;a++)for(unsigned b=1;b<=20;b++)for(unsigned e=0;e<3;e++){
  setup(&s);calls=at=0;values[0]=a-1;values[1]=b-1;
  unsigned natural=mode>0?(a>b?a:b):(a<b?a:b);int bonus=2+extras[e];
  assert(pd_concentration_roll(&s,&plan,mode,extras[e],rng,NULL,&next,&roll,&summary,false)&&calls==2&&roll.count==2&&roll.dice[0]==a&&roll.dice[1]==b&&roll.natural==natural&&roll.total==(int)natural+bonus);
  assert(next.magic.concentration==((int)natural+bonus>=12?1:0));cases++;
 }
 /* All defence profiles and multiple DCs, including damage absorbed by temp HP. */
 const unsigned amounts[]={1,19,20,21,25,40,100,998};
 for(unsigned d=0;d<8;d++)for(unsigned flags=0;flags<8;flags++)for(int mode=-1;mode<=1;mode++)for(unsigned face=1;face<=20;face++){
  setup(&s);s.hp=s.max_hp=999;s.temp_hp=999;assert(pd_defence_set(&s,0,true,flags));plan=(pd_damage_plan){.amount=amounts[d]};calls=at=0;values[0]=values[1]=face-1;
  unsigned damage=amounts[d];if(flags&1)damage/=2;if(flags&2)damage*=2;if(flags&4)damage=0;
  bool ready=damage>0;unsigned dc=damage/2;if(dc<10)dc=10;
  before=s;memset(&next,0x33,sizeof next);pd_state *untouched=&next; /* next is checked below when rejected. */
  assert(pd_concentration_roll(&s,&plan,mode,0,rng,NULL,&next,&roll,&summary,false)==ready);
  if(ready){assert(calls==(mode?2:1)&&summary.adjusted==damage&&summary.concentration_dc==dc&&next.magic.concentration==((int)face+2>=(int)dc?1:0));}
  else {assert(!calls&&!memcmp(&s,&before,sizeof s));const unsigned char *p=(const unsigned char*)untouched;for(size_t i=0;i<sizeof next;i++)assert(p[i]==0x33);}
  cases++;
 }
 /* Explicit capped-DC option at and above the 30 boundary. */
 const unsigned high[]={60,61,62,120,998};
 for(unsigned d=0;d<5;d++)for(unsigned cap=0;cap<2;cap++)for(int mode=-1;mode<=1;mode++)for(unsigned face=1;face<=20;face++){
  setup(&s);s.hp=s.max_hp=999;s.temp_hp=999;plan=(pd_damage_plan){.amount=high[d]};at=calls=0;values[0]=values[1]=face-1;
  unsigned dc=high[d]/2;if(cap&&dc>30)dc=30;
  assert(pd_concentration_roll(&s,&plan,mode,10,rng,NULL,&next,&roll,&summary,cap)&&summary.concentration_dc==dc&&roll.total==(int)face+12&&next.magic.concentration==((int)face+12>=(int)dc?1:0));cases++;
 }
 /* Natural one succeeds on total; natural twenty can fail. */
 setup(&s);s.abilities[2]=30;s.level=20;s.training.saves=4;s.training.save_bonus[2]=30;plan=(pd_damage_plan){.amount=25};calls=at=0;values[0]=0;
 assert(pd_concentration_roll(&s,&plan,0,30,rng,NULL,&next,&roll,&summary,false)&&roll.natural==1&&next.magic.concentration==1);
 s.abilities[2]=1;s.training.saves=0;s.training.save_bonus[2]=-30;calls=at=0;values[0]=19;
 assert(pd_concentration_roll(&s,&plan,0,-30,rng,NULL,&next,&roll,&summary,false)&&roll.natural==20&&!next.magic.concentration);
 /* Reject before RNG for missing/blocked CON training, no check, invalid options and aliases. */
 setup(&s);s.training.available=false;calls=0;assert(!pd_concentration_info(&s,&plan,0,0,&info,&summary,false)&&!pd_concentration_roll(&s,&plan,0,0,rng,NULL,&next,&roll,&summary,false)&&!calls);
 s.training.available=true;s.training.blocked=1u<<26;assert(!pd_concentration_roll(&s,&plan,0,0,rng,NULL,&next,&roll,&summary,false)&&!calls);
 s.training.blocked=1u<<25;assert(pd_concentration_info(&s,&plan,0,0,&info,&summary,false));s.training.blocked=0;
 const int modes[]={-2,2,0,0},locals[]={0,0,-31,31};before=s;next=before;memset(&roll,0x44,sizeof roll);roll_before=roll;memset(&summary,0x55,sizeof summary);summary_before=summary;
 for(unsigned i=0;i<4;i++)assert(!pd_concentration_roll(&s,&plan,modes[i],locals[i],rng,NULL,&next,&roll,&summary,false)&&!calls&&!memcmp(&s,&before,sizeof s)&&!memcmp(&next,&before,sizeof next)&&!memcmp(&roll,&roll_before,sizeof roll)&&!memcmp(&summary,&summary_before,sizeof summary));
 assert(!pd_concentration_roll(&s,&plan,0,0,NULL,NULL,&next,&roll,&summary,false)&&!pd_concentration_roll(&s,&plan,0,0,rng,NULL,&s,&roll,&summary,false)&&!calls);
 assert(!pd_concentration_roll(&s,&plan,0,0,rng,NULL,NULL,&roll,&summary,false)&&!pd_concentration_roll(&s,&plan,0,0,rng,NULL,&next,NULL,&summary,false)&&!pd_concentration_roll(&s,&plan,0,0,rng,NULL,&next,&roll,NULL,false)&&!calls);
 plan.concentration=PD_CONCENTRATION_KEEP;assert(!pd_concentration_roll(&s,&plan,0,0,rng,NULL,&next,&roll,&summary,false)&&!calls);plan.concentration=0;plan.amount=0;
 assert(!pd_concentration_roll(&s,&plan,0,0,rng,NULL,&next,&roll,&summary,false)&&!calls);plan.amount=24;s.temp_hp=0;
 assert(!pd_concentration_roll(&s,&plan,0,0,rng,NULL,&next,&roll,&summary,false)&&!calls);plan.amount=56;assert(!pd_concentration_roll(&s,&plan,0,0,rng,NULL,&next,&roll,&summary,false)&&!calls);
 /* The candidate roundtrips existing PDD18 with damage and concentration together. */
 setup(&s);plan=(pd_damage_plan){.amount=25};at=calls=0;values[0]=0;assert(pd_concentration_roll(&s,&plan,0,0,rng,NULL,&next,&roll,&summary,false)&&!next.magic.concentration&&next.temp_hp==74);
 static uint8_t data[PD_RECORD_SIZE];size_t n=pd_encode(&next,132,data);uint32_t gen;assert(pd_decode(data,n,&s,&gen)&&gen==132&&s.temp_hp==74&&!s.magic.concentration&&s.hp==24);
 printf("Concentration: %u independent CON/proficiency/bonus/faces/modes/DC cases, natural 1/20 by total, validation before RNG and atomic PDD18 result passed\n",cases);
}
