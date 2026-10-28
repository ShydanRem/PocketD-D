#include "status.h"
#include "rest.h"
#include "combat.h"
#include "magic.h"
#include "checks.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;static uint32_t value;
static uint32_t rng(void *unused){(void)unused;calls++;return value;}
int main(void){
 static pd_state s,next,before;pd_demo(&s);s.hp=0;assert(!pd_death_ready(&s));assert(pd_condition_add(&s,"Nota locale",1)&&!s.status.available&&pd_valid(&s)&&pd_condition_remove(&s,0));assert(pd_death_change(&s,PD_DEATH_RESET)&&pd_death_ready(&s));
 s.status.successes=2;s.status.failures=2;before=s;assert(pd_death_face(&s,10)&&s.status.stabilized&&!s.status.successes&&!s.status.failures);
 s=before;assert(pd_death_face(&s,1)&&s.status.failures==3&&!pd_death_ready(&s));calls=0;assert(!pd_death_roll(&s,rng,NULL,&next,(pd_roll[1]){{0}})&&!calls);
 next=s;pd_hp(&s,1);assert(!memcmp(&s,&next,sizeof s));pd_pool pool;unsigned gained;assert(!pd_use_item(&s,1,rng,NULL,&next,&pool,&gained)&&!calls);
 s.magic.count=1;s.magic.powers[0]=(pd_power){.name="Resistenza",.kind=1,.effect=2,.maximum=1,.remaining=1};assert(!pd_magic_use(&s,0,0,false));
 pd_rest_plan plan={.long_rest=true};before=s;assert(!pd_rest_apply(&s,&plan)&&!memcmp(&s,&before,sizeof s));
 assert(pd_death_change(&s,PD_DEATH_MINUS_FAILURE)&&s.status.failures==2);value=19;pd_roll r;assert(pd_death_roll(&s,rng,NULL,&next,&r)&&calls==1&&r.natural==20&&r.bonus==0&&r.count==1&&next.hp==1&&!next.status.successes&&!next.status.failures&&!next.status.stabilized);
 s=next;s.status.failures=2;pd_hp(&s,-1);assert(!s.hp&&!s.status.failures);assert(pd_death_change(&s,PD_DEATH_STABILIZE)&&s.status.stabilized);assert(pd_death_change(&s,PD_DEATH_FAILURE)&&!s.status.stabilized&&s.status.failures==1);
 before=s;assert(!pd_death_face(&s,0)&&!pd_death_face(&s,21)&&!memcmp(&s,&before,sizeof s));
 s.status.successes=3;before=s;calls=0;assert(!pd_death_roll(&s,rng,NULL,&next,&r)&&!calls&&!memcmp(&s,&before,sizeof s));
 assert(pd_death_change(&s,PD_DEATH_RESET));s.training.poisoned=true;s.training.available=false;assert(pd_check_mode(&s,PD_CHECK,1,0,false)==-1);
 assert(pd_condition_add(&s,"Avvelenato",0)&&s.training.poisoned);assert(pd_condition_level(&s,0,99)&&s.status.conditions[0].level==6);assert(pd_condition_remove(&s,0)&&s.training.poisoned);
 char long_name[62];memset(long_name,'W',61);long_name[61]=0;assert(!pd_condition_add(&s,long_name,0)&&!pd_condition_add(&s,"",0)&&!pd_condition_add(&s,"Nota",7));
 for(unsigned i=0;i<30;i++)assert(pd_condition_add(&s,"Nota sintetica",i%7));before=s;assert(!pd_condition_add(&s,"Troppo",0)&&!memcmp(&s,&before,sizeof s));
 s.magic.count=PD_POWERS;for(unsigned i=0;i<PD_POWERS;i++)s.magic.powers[i]=(pd_power){.name="Magia sintetica",.flags=PD_PREPARED};
 static uint8_t data[PD_RECORD_SIZE];uint32_t gen;s.shortcuts[0]=8;size_t length=pd_encode(&s,22,data);assert(length==PD_RECORD_SIZE&&pd_decode(data,length,&next,&gen)&&next.magic.count==PD_POWERS&&gen==22&&next.shortcuts[0]==8&&next.status.count==30&&next.status.conditions[29].level==1);
 unsigned offsets[]={0,1,2,3,4,66},bad[]={2,31,4,4,2,7};size_t start=length-2918;
 for(unsigned i=0;i<6;i++){length=pd_encode(&s,22,data);data[start+offsets[i]]=bad[i];uint32_t crc=pd_crc32(data,length-4);for(unsigned j=0;j<4;j++)data[length-4+j]=crc>>(8*j);before=next;gen=99;assert(!pd_decode(data,length,&next,&gen)&&gen==99&&!memcmp(&next,&before,sizeof next));}
 puts("Status: base death saves, 1/20, stabilization, explicit corrections, HP recovery/dead guards, poison, 30 notes and strict PDD24 persistence passed");
}
