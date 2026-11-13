#include "combat.h"
#include "backup.h"
#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pd_state s,before,next,decoded;
static uint8_t record[PD_RECORD_SIZE];
static unsigned calls;
static uint32_t fixed(void *p){(void)p;calls++;return 19;}
static void setup(void){
 pd_demo(&s);s.item_count=4;s.equipped=0;s.combat=(pd_combat){0};memset(s.gear,0,sizeof s.gear);
 s.items[0]=(pd_item){.name="First",.quantity=1,.kind=PD_WEAPON,.ability=0,.count=1,.sides=6,.flags=3,.reach=1};
 s.items[1]=(pd_item){.name="Second",.quantity=1,.kind=PD_WEAPON,.ability=0,.count=1,.sides=4,.flags=3,.reach=1,.damage_modifier=2};
 s.items[2]=(pd_item){.name="Ammo",.quantity=3};s.items[3]=(pd_item){.name="Shield",.quantity=1};
 s.handling[0].flags=s.handling[1].flags=PD_LIGHT|PD_FINESSE;s.dual=(pd_dual){.weapon=2,.edition=1};assert(pd_valid(&s));
}
int main(int argc,char **argv){
 unsigned cases=0;
 for(unsigned str=1;str<=30;str+=3)for(unsigned dex=1;dex<=30;dex+=3)for(unsigned style=0;style<2;style++)for(unsigned rage=0;rage<2;rage++)for(unsigned shield=0;shield<2;shield++)for(unsigned spent=0;spent<2;spent++)for(unsigned light=0;light<2;light++){
  setup();s.abilities[0]=str;s.abilities[1]=dex;s.dual.style=style;s.dual.bonus_spent=spent;s.combat.raging=rage;s.combat.rage_bonus=2;if(shield)s.gear[3]=(pd_gear){.kind=PD_WEAR_SHIELD,.armor=2,.equipped=true};if(!light)s.handling[1].flags=PD_FINESSE;
  before=s;next=s;pd_weapon_info w;bool valid=!shield&&light;assert(pd_dual_stats(&s,&w)==valid);
  if(valid){int base=pd_modifier(str>=dex?str:dex),damage=(style?base:base<0?base:0)+2+(rage&&str>=dex?2:0);assert(w.index==1&&w.damage_base==(style?base:base<0?base:0)&&w.damage_bonus==damage&&w.attack_bonus==base+pd_proficiency(s.level));}
  pd_strike hit={0},keep=hit;unsigned draws=calls;bool allowed=valid&&!spent;assert(pd_dual_prepare(&s,true,&next)==allowed);assert(pd_dual_attack(&s,true,30,0,(pd_roll_context){0},false,fixed,NULL,&hit)==allowed);assert(calls==draws+(allowed?1:0));
  if(allowed){before.dual.bonus_spent=1;assert(!memcmp(&next,&before,sizeof next));before=s;pd_pool damage;assert(hit.critical&&hit.hits&&pd_weapon_damage(&hit,fixed,NULL,&damage)&&damage.count==2&&damage.bonus==w.damage_bonus);}
  else assert(!memcmp(&next,&s,sizeof next)&&!memcmp(&hit,&keep,sizeof hit));
  assert(!memcmp(&s,&before,sizeof s));draws=calls;assert(!pd_dual_attack(&s,false,15,0,(pd_roll_context){0},false,fixed,NULL,&hit)&&calls==draws);cases++;
 }
 unsigned graze_cases=0;
 for(unsigned score=1;score<=30;score++)for(unsigned ability=0;ability<6;ability++){
  setup();s.handling[0].flags=PD_GRAZE;s.abilities[0]=score;s.abilities[ability]=score;s.magic.count=1;s.magic.available=true;s.magic.powers[0]=(pd_power){.name="Cantrip",.effect=PD_TRUE_STRIKE_2024,.ability=ability,.flags=PD_PREPARED};s.items[0].damage_modifier=10;s.combat.rage_bonus=4;s.combat.critical_dice=3;
  pd_strike miss={0};unsigned entropy=calls,value=99;assert(pd_true_strike_stats(&s,0,false,&miss.weapon));miss.hits=false;assert(pd_graze_damage(&miss,&value)&&value==(unsigned)(pd_modifier(score)>0?pd_modifier(score):0)&&calls==entropy);miss.hits=true;value=99;assert(!pd_graze_damage(&miss,&value)&&value==99&&calls==entropy);miss.hits=false;miss.weapon.graze=false;assert(!pd_graze_damage(&miss,&value)&&value==99);graze_cases++;
 }
 setup();before=s;unsigned entropy=calls;assert(!pd_dual_prepare(&s,true,&s)&&!memcmp(&s,&before,sizeof s)&&calls==entropy);
 setup();s.dual.edition=2;s.items[1].flags|=4;s.handling[1].ammo=3;before=s;assert(pd_dual_prepare(&s,true,&next)&&next.dual.bonus_spent&&next.items[2].quantity==2&&next.equipped==0&&!memcmp(&s,&before,sizeof s));s.items[2].quantity=0;before=next=s;unsigned draws=calls;pd_strike hit;assert(!pd_dual_prepare(&s,true,&next)&&!pd_dual_attack(&s,true,15,0,(pd_roll_context){0},false,fixed,NULL,&hit)&&calls==draws&&!memcmp(&next,&before,sizeof next));
 setup();s.dual.bonus_spent=1;s.reaction_spent=true;pd_round_summary summary;assert(pd_turn_advance_subject(&s,1,true,&summary)&&s.dual.bonus_spent);assert(pd_turn_advance(&s,false,&summary)&&s.dual.bonus_spent);assert(pd_round_advance(&s,&summary)&&s.dual.bonus_spent);assert(pd_turn_advance(&s,true,&summary)&&!s.dual.bonus_spent&&!s.reaction_spent);
 setup();s.magic.available=true;s.magic.count=2;s.magic.powers[0]=(pd_power){.name="Hex",.level=1,.flags=PD_PREPARED|PD_CONCENTRATES|PD_USES_SLOT,.effect=PD_HEX_2024};s.magic.powers[1]=(pd_power){.name="Smite",.level=1,.flags=PD_PREPARED|PD_USES_SLOT,.effect=PD_DIVINE_SMITE_2024};s.magic.slots[0]=(pd_slot){3,3};assert(pd_magic_use(&s,0,1,false)&&s.dual.bonus_spent&&s.magic.slots[0].current==2);before=s;assert(!pd_magic_use(&s,0,1,false)&&!memcmp(&s,&before,sizeof s));pd_strike smite_hit;assert(pd_attack(&s,1,0,fixed,NULL,&smite_hit));pd_weapon_damage_parts parts;draws=calls;assert(!pd_smite_damage(&s,&smite_hit,1,1,false,false,false,fixed,NULL,&parts)&&calls==draws);assert(pd_turn_advance(&s,true,&summary)&&!s.dual.bonus_spent);assert(pd_smite_prepare(&s,&smite_hit,1,1,false,false,&next,NULL)&&next.dual.bonus_spent&&next.magic.slots[0].current==1&&!s.dual.bonus_spent);
 setup();s.magic.available=true;s.magic.count=120;for(unsigned i=0;i<120;i++)s.magic.powers[i]=(pd_power){.name="Spell",.flags=PD_PREPARED};uint32_t gen;size_t n=pd_encode(&s,113,record);assert(n==PD_RECORD_SIZE&&!memcmp(record,"PDDh",4)&&pd_decode(record,n,&decoded,&gen)&&gen==113&&!memcmp(&s,&decoded,sizeof s));
 pd_history history;pd_history_reset(&history,s.history_epoch);size_t length;char *raw=pd_backup_encode(&s,&history,&length);pd_backup *backup=malloc(sizeof *backup);assert(raw&&backup&&pd_backup_decode(raw,length,backup)&&!memcmp(&backup->state,&s,sizeof s));if(argc>1){FILE *f=fopen(argv[1],"wb");assert(f&&fwrite(raw,1,length,f)==length);fclose(f);}free(raw);free(backup);
 for(unsigned i=0;i<4;i++){uint8_t old=record[n-8+i];record[n-8+i]=(unsigned[]){61,3,2,2}[i];uint32_t crc=pd_crc32(record,n-4);for(unsigned k=0;k<4;k++)record[n-4+k]=crc>>(8*k);before=decoded;assert(!pd_decode(record,n,&decoded,&gen)&&!memcmp(&decoded,&before,sizeof decoded));record[n-8+i]=old;}
 setup();s.dual=(pd_dual){0};memset(s.handling,0,sizeof s.handling);assert(!memcmp((pd_encode(&s,1,record),record),"PDDe",4));
 printf("Graze: %u ability/casting-ability cases, only positive ability modifier or zero, no weapon bonus/rider/RNG, hit rejection unchanged; synthetic.\n",graze_cases);
 printf("Dual wield: %u independent ability/style/rage/shield/bonus/Light cases, no-RNG rejection, exact atomic cost, ranged ammo, own-turn-only reset, max PDDh/backup and CRC-valid malformed metadata passed; synthetic.\n",cases);
}
