#include "status.h"
#include "rest.h"
#include "combat.h"
#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state s,next,before;
static char line[PD_PACKET_MAX+2];
static unsigned calls,value;
static uint32_t rng(void *unused){(void)unused;calls++;return value;}
static void output(const pd_state *v){printf("[%u,%u,%u,%u,%d,%d]",v->hp,v->temp_hp,v->status.successes,v->status.failures,v->status.stabilized,v->status.available);}
int main(int argc,char **argv){
 assert(argc==2);FILE *f=fopen(argv[1],"r");assert(f);
 while(fgets(line,sizeof line,f)){
  assert(pd_parse_packet(line,strlen(line),&s));before=s;printf("{\"rolls\":[");
  for(unsigned face=1;face<=20;face++){
   if(face>1)printf(",");
   next=s;calls=0;value=face-1;pd_roll roll={0};bool ok=pd_death_roll(&s,rng,NULL,&next,&roll);
   if(!ok){assert(!calls&&!memcmp(&s,&next,sizeof s));printf("null");}
   else{assert(calls==1&&roll.count==1&&roll.bonus==0&&roll.natural==face&&roll.total==(int)face);output(&next);}
  }
  printf("],\"changes\":[");for(unsigned action=0;action<7;action++){
   if(action){printf(",");}next=s;if(pd_death_change(&next,action))output(&next);else{assert(!memcmp(&next,&s,sizeof s));printf("null");}
  }
  printf("],\"hp\":[");for(int delta=-1;delta<=1;delta++){
   if(delta> -1){printf(",");}next=s;pd_hp(&next,delta);output(&next);
  }
  printf("],\"notes\":[");
  for(unsigned i=0;i<s.status.count;i++){
   if(i){printf(",");}printf("[");for(int delta=-7;delta<=7;delta++){
    if(delta> -7){printf(",");}next=s;assert(pd_condition_level(&next,i,delta)&&next.training.poisoned==s.training.poisoned);printf("%u",next.status.conditions[i].level);
   }printf("]");
  }
  next=s;bool recovered=pd_rest_apply(&next,&(pd_rest_plan){.long_rest=true});
  printf("],\"long\":");if(recovered)output(&next);else{assert(!memcmp(&next,&s,sizeof s));printf("null");}
  pd_pool pool;unsigned gained;calls=0;next=s;bool healed=pd_use_item(&s,1,rng,NULL,&next,&pool,&gained);
  printf(",\"potion\":");if(healed)output(&next);else{assert(!calls&&!memcmp(&next,&s,sizeof s));printf("null");}
  assert(!memcmp(&s,&before,sizeof s));printf("}\n");
 }
 fclose(f);
}
