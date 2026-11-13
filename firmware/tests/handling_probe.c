#include "pocketdd.h"
#include "combat.h"
#include "backup.h"
#include <stdio.h>
#include <stdlib.h>
static pd_state state;
int main(int argc,char **argv){
 if(argc!=2)return 2;FILE *f=fopen(argv[1],"rb");if(!f)return 2;fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);if(n<1||n>PD_PACKET_MAX){fclose(f);return 2;}char *text=malloc(n+1);if(!text||fread(text,1,n,f)!=(size_t)n)return 2;fclose(f);text[n]=0;
 if(!pd_parse_packet(text,n,&state)){free(text);puts("null");return 0;}free(text);
 pd_history h;pd_history_reset(&h,state.history_epoch);size_t size;char *raw=pd_backup_encode(&state,&h,&size);if(!raw)return 2;
 printf("{\"handling\":[");for(unsigned i=0;i<state.item_count;i++){pd_handling v=state.handling[i];printf("%s[%u,%u,%u,%u]",i?",":"",v.flags,v.grip,v.sides,v.ammo);}pd_weapon_info w={0};pd_weapon_stats(&state,state.equipped,&w);
 printf("],\"methods\":[");for(unsigned m=0;m<3;m++){pd_weapon_info x={0};bool valid=pd_dual_stats_for(&state,m,&x);printf("%s{\"valid\":%s,\"ready\":%s,\"attack\":%d,\"damage\":%d}",m?",":"",valid?"true":"false",pd_dual_prepare_for(&state,m,true,NULL)?"true":"false",x.attack_bonus,x.damage_bonus);}
 pd_weapon_info secondary={0};bool dual=pd_dual_stats(&state,&secondary);printf("],\"dual\":{\"valid\":%s,\"attack\":%d,\"damage\":%d,\"sides\":%u}",dual?"true":"false",secondary.attack_bonus,secondary.damage_bonus,secondary.sides);
 printf(",\"counters\":[%u,%u,%u,%u],\"mastery\":%u,\"graze\":%d,\"stats\":{\"ability\":%u,\"sides\":%u,\"attack\":%d,\"damage\":%d},\"backup\":%s}\n",state.dual.feat,state.dual.light_spent,state.dual.feat_spent,state.dual.cleave_spent,w.mastery,w.graze?(w.base>0?w.base:0):-1,w.ability,w.sides,w.attack_bonus,w.damage_bonus,raw);free(raw);return 0;
}
