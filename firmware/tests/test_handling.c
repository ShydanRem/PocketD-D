#include "combat.h"
#include "backup.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pd_state s,before,next,decoded;
static uint8_t record[PD_RECORD_SIZE];
static unsigned calls;
static uint32_t fixed(void *unused){(void)unused;calls++;return 19;}
static void setup(void){pd_demo(&s);s.item_count=3;s.equipped=0;memset(s.gear,0,sizeof s.gear);s.items[0]=(pd_item){.name="Test weapon",.quantity=1,.kind=PD_WEAPON,.count=1,.sides=8,.flags=3,.reach=1,.thrown_range=20};s.items[1]=(pd_item){.name="Shield",.quantity=1};s.items[2]=(pd_item){.name="Arrows",.quantity=5};s.combat.raging=true;s.combat.rage_bonus=2;assert(pd_valid(&s));}
int main(int argc,char **argv){
 unsigned sides[]={0,6,8,10,12},cases=0;
 for(unsigned flags=0;flags<8;flags++)for(unsigned grip=0;grip<2;grip++)for(unsigned d=0;d<5;d++)for(unsigned str=1;str<=30;str+=7)for(unsigned dex=1;dex<=30;dex+=7)for(unsigned shield=0;shield<2;shield++)for(unsigned thrown=0;thrown<2;thrown++){
  setup();s.abilities[0]=str;s.abilities[1]=dex;s.combat.thrown=thrown;if(shield)s.gear[1]=(pd_gear){.kind=PD_WEAR_SHIELD,.armor=2,.equipped=true};before=next=s;
  pd_handling h={.flags=flags,.grip=grip,.sides=sides[d]};bool valid=!(flags&1&&flags&2)&&((flags&2)?sides[d]>8:!sides[d]&&!grip);
  bool two=(flags&1)||((flags&2)&&grip&&!thrown),allowed=valid&&!(shield&&two);
  assert(pd_handling_set(&next,0,h)==allowed);
  if(!allowed)assert(!memcmp(&next,&before,sizeof next));
  else{
   before.handling[0]=h;assert(!memcmp(&next,&before,sizeof next)&&pd_valid(&next));pd_weapon_info w;assert(pd_weapon_stats(&next,0,&w));
   unsigned ability=flags&4?(str>=dex?0:1):0;int base=pd_modifier(next.abilities[ability]);
   assert(w.ability==ability&&w.base==base&&w.sides==((flags&2)&&two?sides[d]:8)&&w.damage_bonus==base+(!thrown&&ability==0?2:0));
   unsigned entropy=calls;pd_strike hit;assert(pd_attack(&next,15,0,fixed,NULL,&hit)&&calls==entropy+1&&hit.critical);pd_pool damage;assert(pd_weapon_damage(&hit,fixed,NULL,&damage)&&damage.count==2&&damage.sides==w.sides);
  }
  if(valid&&shield&&two){s.handling[0]=h;assert(pd_valid(&s));pd_strike hit={0},keep=hit;unsigned entropy=calls;assert(!pd_attack(&s,15,0,fixed,NULL,&hit)&&calls==entropy&&!memcmp(&hit,&keep,sizeof hit));}
  cases++;
 }
 setup();s.handling[0]=(pd_handling){.flags=PD_VERSATILE,.grip=1,.sides=10};s.combat.thrown=true;s.gear[1]=(pd_gear){.kind=PD_WEAR_SHIELD,.armor=2};s.unarmored_rule=1;before=s;assert(!pd_gear_equip(&s,1,true)&&!memcmp(&s,&before,sizeof s));s.gear[1].equipped=true;before=s;assert(!pd_equip(&s,0)&&!memcmp(&s,&before,sizeof s));
 setup();s.combat.raging=false;s.handling[0]=(pd_handling){.flags=PD_VERSATILE|PD_FINESSE,.grip=1,.sides=10,.ammo=3};s.magic.available=true;s.magic.count=120;
 for(unsigned i=0;i<120;i++)s.magic.powers[i]=(pd_power){.name="Spell",.flags=PD_PREPARED};assert(pd_valid(&s));
 uint32_t gen;size_t n=pd_encode(&s,112,record);assert(n==PD_HANDLING_RECORD_SIZE&&!memcmp(record,"PDDg",4)&&pd_decode(record,n,&decoded,&gen)&&gen==112&&!memcmp(&decoded,&s,sizeof s));
 pd_history history;pd_history_reset(&history,s.history_epoch);size_t length;char *raw=pd_backup_encode(&s,&history,&length);pd_backup *backup=malloc(sizeof *backup);assert(raw&&backup&&pd_backup_decode(raw,length,backup)&&!memcmp(&backup->state,&s,sizeof s));
 if(argc>1){FILE *f=fopen(argv[1],"wb");assert(f&&fwrite(raw,1,length,f)==length);fclose(f);}free(raw);free(backup);
 unsigned tail=n-PD_METADATA_BYTES-4;for(unsigned field=0;field<4;field++){uint8_t old=record[tail+field];record[tail+field]=(unsigned[]){8,2,7,61}[field];uint32_t crc=pd_crc32(record,n-4);for(unsigned k=0;k<4;k++)record[n-4+k]=crc>>(8*k);before=decoded;assert(!pd_decode(record,n,&decoded,&gen)&&!memcmp(&decoded,&before,sizeof decoded));record[tail+field]=old;}
 strcpy(s.beyond_id,"167361077");n=pd_encode(&s,112,record);assert(pd_decode(record,n,&decoded,&gen)&&!strcmp(decoded.beyond_id,s.beyond_id));
 memset(s.handling,0,sizeof s.handling);n=pd_encode(&s,112,record);assert(!memcmp(record,"PDDg",4)&&pd_decode(record,n,&decoded,&gen)&&!strcmp(decoded.beyond_id,s.beyond_id));
 memset(s.beyond_id,0,sizeof s.beyond_id);n=pd_encode(&s,112,record);assert(n==PD_LEGACY_RECORD_SIZE&&!memcmp(record,"PDDe",4));
 printf("Handling: %u independent flags/grip/dice/abilities/shield/thrown cases, exact atomic setters, rage/finesse/critical math, blocked hands before RNG, max PDDg/native backup and CRC-valid malformed metadata passed; synthetic.\n",cases);
}
