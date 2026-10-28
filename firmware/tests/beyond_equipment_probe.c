#include "pocketdd.h"
#include "damage.h"
#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state state,before,decoded;
static char line[PD_PACKET_MAX+2];
static uint8_t record[PD_RECORD_SIZE];
int main(int argc,char **argv){
 assert(argc==2);FILE *f=fopen(argv[1],"r");assert(f);
 while(fgets(line,sizeof line,f)){
  pd_demo(&state);before=state;bool valid=pd_parse_packet(line,strlen(line),&state);
  if(!valid){assert(!memcmp(&state,&before,sizeof state));puts("{\"valid\":false}");continue;}
  uint32_t gen=0;size_t n=pd_encode(&state,71,record);assert(pd_decode(record,n,&decoded,&gen)&&gen==71&&!memcmp(&state,&decoded,sizeof state));
  printf("{\"valid\":true,\"hp\":%u,\"maxHp\":%u,\"armor\":%u,\"abilities\":[%u,%u,%u,%u,%u,%u],\"defences\":[%u,%u,%u,%u],\"avatar\":[%u,%u,%u],\"items\":[",state.hp,state.max_hp,state.armor,state.abilities[0],state.abilities[1],state.abilities[2],state.abilities[3],state.abilities[4],state.abilities[5],state.defences.configured,state.defences.resistance,state.defences.vulnerability,state.defences.immunity,state.lineage,state.body,state.outfit);
  for(unsigned i=0;i<state.item_count;i++){const pd_item *v=&state.items[i];printf("%s[%u,%u,%u,%u,%d,%u,%u,%u]",i?",":"",v->kind,v->quantity,v->count,v->sides,v->healing_modifier,v->charges.current,v->charges.maximum,v->charges.recovery);}
  printf("],\"resources\":[");
  for(unsigned i=0;i<state.resource_count;i++)printf("%s[%u,%u]",i?",":"",state.resources[i].current,state.resources[i].maximum);
  printf("],\"powers\":[");
  for(unsigned i=0;i<state.magic.count;i++){const pd_power *p=&state.magic.powers[i];printf("%s[%u,%u,%u,%u,%u]",i?",":"",p->kind,p->flags,p->maximum,p->remaining,p->recovery);}
  printf("],\"cannonUses\":[");
  for(unsigned i=0;i<state.magic.count;i++)if(state.magic.powers[i].effect==3){
   for(unsigned choice=0;choice<19;choice++){
    unsigned slot=choice?choice<=9?choice:choice-9:0;bool pact=choice>9;decoded=state;bool ok=pd_magic_use(&decoded,i,slot,pact);before=state;
    if(ok){before.magic.powers[i].remaining=decoded.magic.powers[i].remaining;if(slot)(pact?before.magic.pact:before.magic.slots)[slot-1].current=(pact?decoded.magic.pact:decoded.magic.slots)[slot-1].current;}
    assert(!memcmp(&before,&decoded,sizeof before));printf("%s[%s,%u,%u]",choice?",":"",ok?"true":"false",decoded.magic.powers[i].remaining,slot?(pact?decoded.magic.pact:decoded.magic.slots)[slot-1].current:0);
   }break;
  }
  printf("],\"damage\":[");
  for(unsigned i=0;i<13;i++){pd_damage_summary d;assert(pd_damage_preview(&state,&(pd_damage_plan){.type=i,.amount=9,.concentration=PD_CONCENTRATION_KEEP},&d));printf("%s[%s,%u,%u,%s]",i?",":"",d.configured?"true":"false",d.flags,d.adjusted,d.allowed?"true":"false");}
  puts("]}");
 }
 assert(!fclose(f));
}
