#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 static pd_state s,before,out;pd_demo(&s);s.magic.count=6;
 s.magic.powers[0]=(pd_power){.name="Luce sintetica",.flags=PD_PREPARED|PD_USES_SLOT,.ability=3};
 s.magic.powers[1]=(pd_power){.name="Scudo sintetico",.level=1,.flags=PD_PREPARED|PD_USES_SLOT|PD_CONCENTRATES,.ability=4,.attack_modifier=2,.save_modifier=-1};
 s.magic.powers[2]=(pd_power){.name="Innato sintetico",.level=2,.flags=PD_PREPARED|PD_LIMITED,.maximum=1,.remaining=1};
 s.magic.powers[3]=(pd_power){.name="Ira sintetica",.kind=1,.effect=1,.maximum=2,.remaining=2};s.combat.rage_bonus=2;
 s.magic.powers[4]=(pd_power){.name="Nota sintetica",.kind=1,.flags=PD_PASSIVE};
 s.magic.powers[5]=(pd_power){.name="Resistenza sintetica",.kind=1,.effect=2,.maximum=1,.remaining=1};
 s.magic.slots[0]=(pd_slot){1,2};s.magic.pact[1]=(pd_slot){1,1};assert(pd_valid(&s));
 int attack,dc;assert(pd_magic_stats(&s,1,&attack,&dc)&&attack==5&&dc==10);
 before=s;assert(!pd_magic_use(&s,1,0,false)&&!memcmp(&s,&before,sizeof s));
 assert(pd_magic_use(&s,1,2,true)&&!s.magic.pact[1].current&&s.magic.concentration==2&&s.magic.slots[0].current==1);
 assert(pd_magic_use(&s,0,0,false)&&s.magic.concentration==2);assert(pd_magic_use(&s,2,0,false)&&!s.magic.powers[2].remaining&&s.magic.concentration==2);
 before=s;assert(!pd_magic_use(&s,2,0,false)&&!pd_magic_use(&s,4,0,false)&&!pd_magic_use(&s,5,0,false)&&!memcmp(&s,&before,sizeof s));
 assert(pd_magic_use(&s,3,0,false)&&s.combat.raging&&s.training.strength_advantage&&!s.magic.concentration&&s.magic.powers[3].remaining==1);
 before=s;assert(!pd_magic_use(&s,0,0,false)&&!pd_magic_use(&s,3,0,false)&&!memcmp(&s,&before,sizeof s));pd_end_rage(&s);assert(!s.training.strength_advantage);
 s.magic.powers[0].flags&=~PD_PREPARED;before=s;assert(!pd_magic_use(&s,0,0,false)&&!memcmp(&s,&before,sizeof s));
 s.hp=0;assert(pd_magic_use(&s,5,0,false)&&s.hp==1&&!s.magic.powers[5].remaining);
 assert(pd_magic_recover(&s,5,1)&&s.magic.powers[5].remaining==1);assert(!pd_magic_recover(&s,4,1));
 assert(pd_slot_change(&s,1,false,100)&&s.magic.slots[0].current==2);assert(pd_slot_change(&s,1,false,-100)&&!s.magic.slots[0].current);
 assert(pd_slot_change(&s,1,false,1));assert(pd_magic_use(&s,1,1,false));pd_hp(&s,-999);assert(!s.magic.concentration&&!s.combat.raging);
 s.shortcuts[0]=7;s.shortcuts[1]=0;s.shortcuts[2]=4;
 static uint8_t data[PD_RECORD_SIZE];size_t length=pd_encode(&s,88,data);uint32_t gen;assert(pd_decode(data,length,&out,&gen)&&gen==88&&out.magic.count==6&&out.magic.powers[5].remaining==1&&out.shortcuts[0]==7&&!out.shortcuts[1]&&out.shortcuts[2]==4);
 s.magic=(pd_magic){.available=true,.count=PD_POWERS};s.hp=24;
 for(unsigned i=0;i<PD_POWERS;i++)s.magic.powers[i]=(pd_power){.name="Magia massima",.flags=PD_PREPARED};
 assert(pd_valid(&s)&&pd_encode(&s,99,data)==PD_RECORD_SIZE&&pd_decode(data,PD_RECORD_SIZE,&out,&gen)&&out.magic.count==PD_POWERS&&!strcmp(out.magic.powers[119].name,"Magia massima"));
 puts("Magic: stats, slots/pact, cantrips, innate costs, preparation, concentration, rage/endurance, manual recovery, HP zero and PDD16 persistence passed");
}
