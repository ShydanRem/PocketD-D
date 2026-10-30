/* Synthetic JS -> strict USB18 -> native deltas and PDDb/backup roundtrip. */
#include "backup.h"
#include "combat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pd_state state,next,decoded;
static void numbers(const pd_state *s){
 printf("{\"armor\":%u,\"hp\":%u,\"maxHp\":%u,\"abilities\":[",s->armor,s->hp,s->max_hp);
 for(unsigned i=0;i<6;i++)printf("%s%u",i?",":"",s->abilities[i]);
 printf("],\"weapons\":[");for(unsigned i=0;i<s->item_count;i++)printf("%s[%d,%d]",i?",":"",s->items[i].attack_modifier,s->items[i].damage_modifier);
 printf("],\"powers\":[");for(unsigned i=0;i<s->magic.count;i++)printf("%s[%d,%d]",i?",":"",s->magic.powers[i].attack_modifier,s->magic.powers[i].save_modifier);
 printf("],\"checks\":[");for(unsigned i=0;i<6;i++)printf("%s%d",i?",":"",s->training.check_bonus[i]);
 printf("],\"saves\":[");for(unsigned i=0;i<6;i++)printf("%s%d",i?",":"",s->training.save_bonus[i]);
 printf("],\"skills\":[");for(unsigned i=0;i<18;i++)printf("%s%d",i?",":"",s->training.skill_bonus[i]);
 printf("],\"equipped\":[");for(unsigned i=0;i<s->item_count;i++)printf("%s%d",i?",":"",s->gear[i].equipped);printf("]}");
 uint32_t generation;static uint8_t data[PD_RECORD_SIZE];assert(pd_encode(s,1,data));assert(!memcmp(data,"PDDb",4));assert(pd_decode(data,pd_encoded_size(s),&decoded,&generation));assert(!memcmp(s,&decoded,sizeof *s));
}
int main(int argc,char **argv){
 assert(argc==3);FILE *f=fopen(argv[1],"r");assert(f);static char line[PD_PACKET_MAX+2];unsigned count=0;
 while(fgets(line,sizeof line,f)){
  if(!pd_parse_packet(line,strlen(line),&state)){puts("{\"valid\":false}");continue;}
  printf("{\"valid\":true,\"rows\":[");numbers(&state);
  for(unsigned i=0;i<state.item_count;i++){next=state;if(!state.gear[i].kind)continue;printf(",");if(pd_gear_equip(&next,i,!next.gear[i].equipped))numbers(&next);else printf("null");}
  puts("]}");
  if(!count++){pd_history h;pd_history_reset(&h,state.history_epoch);size_t size;char *raw=pd_backup_encode(&state,&h,&size);assert(raw);FILE *b=fopen(argv[2],"w");assert(b&&fwrite(raw,1,size,b)==size);fclose(b);free(raw);}
 }
 fclose(f);return 0;
}
