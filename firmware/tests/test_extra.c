#include "combat.h"
#include "backup.h"
#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pd_state s,before,next,out;
static uint8_t data[PD_RECORD_SIZE];
static unsigned calls;
static uint32_t face(void *p){(void)p;calls++;return 19;}
static void setup(void){
 pd_demo(&s);memset(s.gear,0,sizeof s.gear);s.combat=(pd_combat){0};s.item_count=3;s.equipped=0;
 s.items[0]=(pd_item){.name="First",.kind=PD_WEAPON,.quantity=1,.ability=0,.count=1,.sides=6,.flags=3,.reach=1};
 s.items[1]=(pd_item){.name="Other",.kind=PD_WEAPON,.quantity=1,.ability=0,.count=1,.sides=4,.flags=3,.reach=1,.damage_modifier=2};s.items[2]=(pd_item){.name="Ammo",.quantity=2};
 s.handling[0]=(pd_handling){.flags=PD_LIGHT};s.handling[1]=(pd_handling){.flags=PD_LIGHT|PD_NICK};s.dual=(pd_dual){.weapon=2,.edition=2,.feat=1};assert(pd_valid(&s));
}
int main(int argc,char **argv){
 unsigned cases=0;
 for(unsigned edition=1;edition<=2;edition++)for(unsigned feat=0;feat<2;feat++)for(unsigned first=0;first<2;first++)for(unsigned second=0;second<2;second++)for(unsigned nick=0;nick<2;nick++)for(unsigned bonus=0;bonus<2;bonus++)for(unsigned light=0;light<2;light++)for(unsigned used=0;used<2;used++)for(unsigned method=0;method<3;method++)for(unsigned score=1;score<=30;score+=7){
  if(edition==1&&feat)continue;
  setup();s.dual.edition=edition;s.dual.feat=feat;s.dual.bonus_spent=bonus;s.dual.light_spent=light;s.dual.feat_spent=used;s.handling[0].flags=first?PD_LIGHT:0;s.handling[1].flags=(second?PD_LIGHT:0)|(nick?PD_NICK:0);s.abilities[0]=score;assert(pd_valid(&s));before=next=s;
  bool eligible=first&&(method==2?(edition==2&&feat):second)&&(method!=1||(edition==2&&nick));
  bool allowed=eligible&&(method==1?!light:method==2?!bonus&&!used:!bonus&&!light);
  pd_weapon_info w;assert(pd_dual_stats_for(&s,method,&w)==eligible);unsigned entropy=calls;pd_strike hit={0},keep=hit;
  assert(pd_dual_prepare_for(&s,method,true,&next)==allowed);assert(pd_dual_attack_for(&s,method,true,30,0,(pd_roll_context){0},false,face,NULL,&hit)==allowed);assert(calls==entropy+(allowed?1:0));
  if(allowed){before=s;if(method!=1)before.dual.bonus_spent=1;if(method==2)before.dual.feat_spent=1;else before.dual.light_spent=1;assert(!memcmp(&next,&before,sizeof next));int mod=pd_modifier(score);assert(hit.weapon.damage_bonus==(mod<0?mod:0)+2&&hit.critical);before=s;}
  else assert(!memcmp(&next,&s,sizeof s)&&!memcmp(&hit,&keep,sizeof hit));assert(!memcmp(&s,&before,sizeof s));cases++;
 }
 setup();assert(pd_dual_prepare_for(&s,PD_EXTRA_NICK,true,&next)&&!next.dual.bonus_spent&&next.dual.light_spent);s=next;assert(pd_dual_prepare_for(&s,PD_EXTRA_FEAT,true,&next)&&next.dual.bonus_spent&&next.dual.feat_spent);s=next;assert(!pd_dual_prepare_for(&s,PD_EXTRA_NICK,true,&next)&&!pd_dual_prepare_for(&s,PD_EXTRA_LIGHT,true,&next));
 setup();assert(pd_dual_prepare_for(&s,PD_EXTRA_FEAT,true,&next));s=next;assert(pd_dual_prepare_for(&s,PD_EXTRA_NICK,true,&next)&&next.dual.bonus_spent&&next.dual.feat_spent&&next.dual.light_spent);
 setup();s.handling[0].flags=PD_CLEAVE;s.abilities[0]=8;s.items[0].damage_modifier=2;pd_strike original;assert(pd_attack(&s,1,0,face,NULL,&original));pd_weapon_info w;assert(pd_cleave_stats(&s,&original,&w)&&w.base==-1&&w.damage_bonus==1&&!w.true_strike&&!w.radiant_dice);assert(pd_cleave_prepare(&s,&original,true,&next)&&next.dual.cleave_spent&&!next.dual.bonus_spent);s=next;unsigned entropy=calls;pd_strike other;assert(!pd_cleave_attack(&s,&original,true,15,0,(pd_roll_context){0},false,face,NULL,&other)&&calls==entropy);
 pd_round_summary turn;assert(pd_turn_advance_subject(&s,1,true,&turn)&&s.dual.cleave_spent);assert(pd_turn_advance(&s,false,&turn)&&s.dual.cleave_spent);assert(pd_turn_advance(&s,true,&turn)&&!s.dual.cleave_spent);
 setup();s.level=5;s.abilities[0]=8;s.abilities[3]=20;s.handling[0].flags=PD_CLEAVE;memset(&s.magic,0,sizeof s.magic);s.magic.available=true;s.magic.count=1;s.magic.powers[0]=(pd_power){.name="Cantrip",.effect=PD_TRUE_STRIKE_2024,.ability=3,.flags=PD_PREPARED};pd_strike spell_hit={.hits=true};assert(pd_true_strike_stats(&s,0,false,&spell_hit.weapon)&&spell_hit.weapon.base==5);assert(pd_cleave_stats(&s,&spell_hit,&w)&&w.base==-1&&!w.true_strike&&!w.radiant_dice&&w.damage_base==-1);s.handling[0].flags=7u<<4;assert(pd_true_strike_stats(&s,0,false,&spell_hit.weapon));char topple[220];assert(pd_mastery_text(&spell_hit,s.level,topple,sizeof topple)&&strstr(topple,"CD 16"));
 setup();for(unsigned m=1;m<=8;m++){s.handling[0].flags=m<<4;assert(pd_weapon_stats(&s,0,&w)&&w.mastery==m&&w.graze==(m==1));pd_strike hit={.weapon=w,.hits=true};char note[220];entropy=calls;assert(pd_mastery_text(&hit,s.level,note,sizeof note)&&note[0]&&calls==entropy);hit.hits=false;assert(pd_mastery_text(&hit,s.level,note,sizeof note)&&calls==entropy);if(m==2)assert(strstr(note,"Attacco")&&strstr(note,"mancato"));}
 setup();memset(&s.magic,0,sizeof s.magic);s.magic.available=true;s.magic.count=120;for(unsigned i=0;i<120;i++)s.magic.powers[i]=(pd_power){.name="Spell",.flags=PD_PREPARED};s.dual.light_spent=s.dual.feat_spent=s.dual.cleave_spent=1;s.dual.bonus_spent=1;uint32_t gen;size_t n=pd_encode(&s,114,data);assert(n==PD_RECORD_SIZE&&!memcmp(data,"PDDi",4)&&pd_decode(data,n,&out,&gen)&&gen==114&&!memcmp(&s,&out,sizeof s));
 pd_history h;pd_history_reset(&h,s.history_epoch);size_t length;char *raw=pd_backup_encode(&s,&h,&length);pd_backup *backup=malloc(sizeof *backup);assert(raw&&backup&&pd_backup_decode(raw,length,backup)&&!memcmp(&backup->state,&s,sizeof s));if(argc>1){FILE *f=fopen(argv[1],"wb");assert(f&&fwrite(raw,1,length,f)==length);fclose(f);}free(raw);free(backup);
 for(unsigned field=0;field<4;field++){uint8_t old=data[n-8+field];data[n-8+field]=2;uint32_t crc=pd_crc32(data,n-4);for(unsigned k=0;k<4;k++)data[n-4+k]=crc>>(8*k);before=out;assert(!pd_decode(data,n,&out,&gen)&&!memcmp(&out,&before,sizeof out));data[n-8+field]=old;}
 printf("Extra attacks: %u independent edition/feat/Light/Nick/cost/ability cases, both Nick/feat orders, once-per-turn Cleave, own-turn reset, eight no-RNG mastery reports, max PDDi/backup and malformed counters passed; synthetic.\n",cases);
}
