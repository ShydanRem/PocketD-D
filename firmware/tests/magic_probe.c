#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state state,next;
static char line[PD_PACKET_MAX+2];
int main(int argc,char **argv){
 assert(argc==2);FILE *f=fopen(argv[1],"r");assert(f);
 while(fgets(line,sizeof line,f)){
  assert(pd_parse_packet(line,strlen(line),&state));printf("[");
  for(unsigned i=0;i<state.magic.count;i++){
   if(i)printf(",");
   int attack=0,dc=0;bool stats=pd_magic_stats(&state,i,&attack,&dc);
   printf("{\"stats\":[%d,%d,%d],\"uses\":[",stats,attack,dc);
   for(unsigned option=0;option<19;option++){
    if(option)printf(",");
    unsigned slot=option>9?option-9:option;bool pact=option>9;next=state;
    bool ok=pd_magic_use(&next,i,slot,pact);if(!ok){assert(!memcmp(&next,&state,sizeof state));printf("null");continue;}
    printf("[%u,%u,%d,%d,%u,%u,%u]",next.hp,next.magic.concentration,next.combat.raging,next.training.strength_advantage,slot?next.magic.slots[slot-1].current:0,slot?next.magic.pact[slot-1].current:0,next.magic.powers[i].remaining);
   }
   printf("]}");
  }
  printf("]\n");
 }
 fclose(f);
}
