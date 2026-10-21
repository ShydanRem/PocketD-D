#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;static uint32_t value;
static uint32_t rng(void *unused){(void)unused;calls++;return value;}
int main(void){
 static pd_state s,before,next,scratch;pd_demo(&s);s.temp_hp=5;s.combat.rage_bonus=2;s.combat.raging=true;s.training.strength_advantage=true;s.combat.power_attack=true;
 s.recovery=(pd_recovery){.available=true,.count=2,.pools={{.name="Classe A",.die=8,.maximum=3,.used=3},{.name="Classe B",.die=12,.maximum=2,.used=1}}};
 s.magic.count=4;s.magic.slots[0]=(pd_slot){0,3};s.magic.pact[1]=(pd_slot){0,2};
 for(unsigned i=0;i<3;i++)s.magic.powers[i]=(pd_power){.name="Risorsa",.kind=1,.maximum=2,.recovery=i};
 s.magic.powers[3]=(pd_power){.name="Innato",.flags=PD_PREPARED|PD_LIMITED,.maximum=1,.recovery=1};
 before=s;pd_rest_plan plan={0};pd_rest_summary summary;
 assert(pd_rest_preview(&s,&plan,&scratch,&summary)&&!memcmp(&s,&before,sizeof s));
 assert(summary.pact==2&&!summary.ordinary&&!summary.hp&&summary.powers==2&&summary.ends_rage);
 assert(pd_rest_apply(&s,&plan)&&!s.combat.raging&&!s.training.strength_advantage&&!s.combat.power_attack&&s.hp==24&&s.temp_hp==5);
 assert(!s.magic.slots[0].current&&s.magic.pact[1].current==2&&!s.magic.powers[0].remaining&&s.magic.powers[1].remaining==2&&!s.magic.powers[2].remaining&&s.magic.powers[3].remaining==1);
 assert(!memcmp(s.items,before.items,sizeof s.items)&&!memcmp(s.resources,before.resources,sizeof s.resources));
 assert(pd_hit_budget(&s,PD_HD_2014)==2&&pd_hit_budget(&s,PD_HD_2024)==4);
 before=s;plan=(pd_rest_plan){.long_rest=true,.policy=PD_HD_2014,.regain={3}};assert(!pd_rest_apply(&s,&plan)&&!memcmp(&s,&before,sizeof s));
 plan.regain[0]=1;plan.regain[1]=2;assert(!pd_rest_apply(&s,&plan)&&!memcmp(&s,&before,sizeof s));
 plan.regain[1]=1;assert(pd_rest_apply(&s,&plan)&&s.hp==32&&!s.temp_hp&&s.recovery.pools[0].used==2&&!s.recovery.pools[1].used&&s.magic.slots[0].current==3);
 plan=(pd_rest_plan){.long_rest=true,.policy=PD_HD_2024};assert(pd_rest_apply(&s,&plan)&&!s.recovery.pools[0].used);
 s.recovery.pools[0].maximum=1;s.recovery.pools[0].used=1;s.recovery.count=1;assert(pd_hit_budget(&s,PD_HD_2014)==1);
 s.hp=31;s.temp_hp=7;pd_pool roll={0};unsigned gained=99;calls=0;value=7;
 s.recovery.pools[0].used=0;assert(pd_spend_hit_die(&s,0,rng,NULL,&next,&roll,&gained)&&calls==1&&next.hp==32&&gained==1&&next.temp_hp==7&&next.recovery.pools[0].used==1);
 before=s;s.abilities[2]=1;value=0;assert(pd_spend_hit_die(&s,0,rng,NULL,&next,&roll,&gained)&&!roll.total&&!gained&&next.recovery.pools[0].used==1);
 s=before;s.hp=s.max_hp;calls=0;assert(!pd_spend_hit_die(&s,0,rng,NULL,&next,&roll,&gained)&&!calls);s.hp=0;assert(!pd_spend_hit_die(&s,0,rng,NULL,&next,&roll,&gained)&&!calls);
 s.hp=10;s.recovery.pools[0].used=1;assert(!pd_spend_hit_die(&s,0,rng,NULL,&next,&roll,&gained)&&!calls);
 s.recovery.available=false;s.recovery.count=0;plan=(pd_rest_plan){.long_rest=true,.policy=PD_HD_2014,.regain={1}};before=s;assert(!pd_rest_apply(&s,&plan)&&!memcmp(&s,&before,sizeof s));
 static uint8_t data[PD_RECORD_SIZE];uint32_t generation;s=before;s.recovery=(pd_recovery){.available=true,.count=20};
 for(unsigned i=0;i<20;i++)s.recovery.pools[i]=(pd_hit_die){.name="Classe",.die=12,.maximum=20,.used=i};
 size_t n=pd_encode(&s,15,data);assert(pd_decode(data,n,&next,&generation)&&generation==15&&next.recovery.count==20&&next.recovery.pools[19].used==19);
 /* CRC-valid malformed recovery data still fails without touching outputs. */
 size_t start=n-3665;unsigned offsets[]={0,1,43,44,45};unsigned bad[]={2,21,7,21,21};
 for(unsigned i=0;i<5;i++) {
  n=pd_encode(&s,15,data);data[start+offsets[i]]=bad[i];uint32_t crc=pd_crc32(data,n-4);for(unsigned j=0;j<4;j++)data[n-4+j]=crc>>(8*j);
  before=next;generation=77;assert(!pd_decode(data,n,&next,&generation)&&generation==77&&!memcmp(&next,&before,sizeof next));
 }
 puts("Rests: explicit recovery policies, atomic plan validation, resource scopes, multiclass budget, basic hit die healing, RNG guards and persistence passed");
}
