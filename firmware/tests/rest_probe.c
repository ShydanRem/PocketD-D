#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state state,next,scratch;
static char line[PD_PACKET_MAX+2];
static unsigned calls,value;
static uint32_t rng(void *unused){(void)unused;calls++;return value;}
static void array_slots(const pd_slot *v){printf("[");for(unsigned i=0;i<9;i++)printf("%s%u",i?",":"",v[i].current);printf("]");}
static void output(const pd_state *s){
 printf("[%u,%u,%u,%d,%d,%d,",s->hp,s->temp_hp,s->magic.concentration,s->combat.raging,s->training.strength_advantage,s->combat.power_attack);
 array_slots(s->magic.slots);printf(",");array_slots(s->magic.pact);printf(",[");
 for(unsigned i=0;i<s->magic.count;i++)printf("%s%u",i?",":"",s->magic.powers[i].remaining);
 printf("],[");for(unsigned i=0;i<s->recovery.count;i++)printf("%s%u",i?",":"",s->recovery.pools[i].used);printf("]]");
}
int main(int argc,char **argv){
 assert(argc==2);FILE *f=fopen(argv[1],"r");assert(f);unsigned row=0;
 while(fgets(line,sizeof line,f)){
  assert(pd_parse_packet(line,strlen(line),&state));printf("{\"budget\":[%u,%u],\"rests\":[",pd_hit_budget(&state,PD_HD_2014),pd_hit_budget(&state,PD_HD_2024));
  for(unsigned option=0;option<4;option++) {
   if(option)printf(",");
   pd_rest_plan p={.long_rest=option>0,.policy=option>1?option-1:0};
   if(p.policy==PD_HD_2014){unsigned budget=pd_hit_budget(&state,p.policy);for(unsigned i=0;i<state.recovery.count;i++){unsigned at=(i+row)%state.recovery.count;unsigned n=state.recovery.pools[at].used;p.regain[at]=n<budget?n:budget;budget-=p.regain[at];}}
   pd_rest_summary summary;assert(pd_rest_preview(&state,&p,&scratch,&summary));next=state;assert(pd_rest_apply(&next,&p)&&!memcmp(&scratch,&next,sizeof next));
   assert(!memcmp(next.items,state.items,sizeof state.items)&&!memcmp(next.resources,state.resources,sizeof state.resources)&&!memcmp(next.shortcuts,state.shortcuts,3));
   output(&next);
  }
  printf("],\"spends\":[");bool first=true;
  for(unsigned i=0;i<state.recovery.count;i++)for(unsigned face=1;face<=state.recovery.pools[i].die;face++){
   if(!first)printf(",");
   first=false;
   pd_pool roll={0};unsigned gained=0;calls=0;value=face-1;next=state;
   bool ok=pd_spend_hit_die(&state,i,rng,NULL,&next,&roll,&gained);
   if(ok){assert(calls==1);printf("[%u,%u,%u,%u,%u,%u,%d]",calls,next.hp,next.temp_hp,next.recovery.pools[i].used,gained,roll.dice[0],roll.total);}
   else{assert(!calls&&!memcmp(&state,&next,sizeof state));printf("null");}
  }
  printf("]}\n");row++;
 }
 fclose(f);
}
