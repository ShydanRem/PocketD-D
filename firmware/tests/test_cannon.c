#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state s,before,next,decoded;
static uint8_t record[PD_RECORD_SIZE];
static void setup(unsigned remaining){
 pd_demo(&s);memset(&s.magic,0,sizeof s.magic);s.magic.available=true;s.magic.count=1;
 s.magic.powers[0]=(pd_power){.name="Cannon synthetic",.kind=1,.flags=PD_PREPARED,.maximum=1,.remaining=remaining,.recovery=2,.effect=3};
 s.resource_count=2;s.resources[0]=(pd_resource){.name="Manual A",.current=35,.maximum=50};s.resources[1]=(pd_resource){.name="Manual B",.current=1,.maximum=4};
}
static void crc(size_t size){uint32_t c=pd_crc32(record,size-4);for(unsigned i=0;i<4;i++)record[size-4+i]=c>>(8*i);}
int main(void){
 unsigned cases=0;
 for(unsigned hp=0;hp<3;hp++)for(unsigned rage=0;rage<2;rage++)for(unsigned free=0;free<2;free++)for(unsigned slot=0;slot<=10;slot++)for(unsigned pact=0;pact<2;pact++)for(unsigned balance=0;balance<3;balance++){
  setup(free);s.hp=(unsigned[]){0,1,24}[hp];s.combat.raging=rage;s.combat.rage_bonus=2;s.training.strength_advantage=rage;
  for(unsigned i=0;i<9;i++)s.magic.slots[i]=s.magic.pact[i]=(pd_slot){.current=(unsigned[]){0,1,9}[balance],.maximum=9};
  before=s;next=s;bool allowed=s.hp&&(free?slot==0&&!pact:slot>=1&&slot<=9&&balance);
  assert(pd_magic_use(&s,0,slot,pact)==allowed);
  if(allowed){if(free)next.magic.powers[0].remaining--;else (pact?next.magic.pact:next.magic.slots)[slot-1].current--;}
  assert(!memcmp(&s,&next,sizeof s)&&pd_valid(&s));cases++;
 }
 setup(0);s.magic.count=2;s.magic.powers[1]=(pd_power){.name="Concentration retained",.flags=PD_PREPARED|PD_CONCENTRATES};s.magic.concentration=2;s.magic.slots[8]=(pd_slot){1,1};before=s;
 assert(pd_magic_use(&s,0,9,false)&&s.magic.concentration==2&&!s.magic.slots[8].current);before.magic.slots[8].current=0;assert(!memcmp(&s,&before,sizeof s));
 for(unsigned count=1;count<=120;count+=119){
  setup(0);s.magic.count=count;for(unsigned i=1;i<count;i++)s.magic.powers[i]=(pd_power){.name="Other spell",.flags=PD_PREPARED};
  size_t n=pd_encode(&s,77,record);uint32_t gen=0;assert(n&&pd_decode(record,n,&decoded,&gen)&&gen==77&&!memcmp(&s,&decoded,sizeof s));
  record[3]='Y';crc(n);before=decoded;gen=55;assert(!pd_decode(record,n,&decoded,&gen)&&gen==55&&!memcmp(&decoded,&before,sizeof before));
  s.magic.powers[0].effect=0;pd_encode(&s,77,record);record[3]='Y';crc(n);assert(pd_decode(record,n,&decoded,&gen)&&!memcmp(&s,&decoded,sizeof s));
 }
 puts("Cannon: free/paid ordinary/pact/zero HP/rage/empty-slot boundaries, exact state preservation, retained concentration, PDD35 roundtrip and PDD34 migration/no smuggled effect passed");printf("Cannon independent states: %u\n",cases);
}
