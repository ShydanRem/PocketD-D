/* Synthetic bridge: test the actual wire parser and C rules against PC results. */
#include "pocketdd.h"
#include "combat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint32_t face;
static uint32_t rng(void *unused) {(void)unused;return face;}
int main(int argc,char **argv) {
    assert(argc==2);FILE *f=fopen(argv[1],"r");assert(f);char line[PD_PACKET_MAX+2];
    while(fgets(line,sizeof line,f)) {
        pd_state s;assert(pd_parse_packet(line,strlen(line),&s));
        printf("{\"items\":[");
        for(unsigned i=0;i<s.item_count;i++) {
            if(i)printf(",");
            pd_weapon_info w;
            if(!pd_weapon_stats(&s,i,&w)){printf("null");continue;}
            printf("{\"attack\":%d,\"damage\":%d,\"reach\":%u,\"criticalDice\":%u,\"melee\":%d}",w.attack_bonus,w.damage_bonus,w.reach,w.critical_dice,w.melee);
        }
        printf("],\"attacks\":[");
        for(int mode=-1;mode<=1;mode++)for(unsigned natural=0;natural<3;natural++) {
            if(mode!=-1||natural)printf(",");
            face=natural==0?0:natural==1?19:9;pd_strike hit;pd_pool damage;
            if(!pd_attack(&s,15,mode,rng,NULL,&hit)){printf("null");continue;}
            printf("[%u,%d,%d,%d,%u",hit.roll.count,hit.roll.total,hit.hits,hit.critical,hit.roll.natural);
            face=0;
            if(pd_weapon_damage(&hit,rng,NULL,&damage))printf(",%u,%d",damage.count,damage.total);else printf(",0,0");
            printf("]");
        }
        printf("],\"uses\":[");face=0;
        for(unsigned i=0;i<s.item_count;i++) {
            if(i)printf(",");
            pd_state next;pd_pool roll;unsigned gained;
            if(!pd_use_item(&s,i,rng,NULL,&next,&roll,&gained)){printf("null");continue;}
            printf("[%u,%u,%u,%u,%u,%d,%d]",next.hp,next.temp_hp,next.items[i].quantity,gained,roll.count,roll.total,next.combat.thrown);
        }
        printf("]}\n");
    }
    fclose(f);
}
