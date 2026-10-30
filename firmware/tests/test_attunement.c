#include "pocketdd.h"
#include "magic.h"
#include "combat.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
static pd_state base,s,off,on,out;static uint8_t record[PD_RECORD_SIZE];
static void crc(size_t n){uint32_t c=pd_crc32(record,n-4);for(unsigned i=0;i<4;i++)record[n-4+i]=c>>(i*8);}
int main(void){
 unsigned cases=0;uint32_t gen;
 for(unsigned ability=0;ability<6;ability++)for(unsigned level=1;level<=20;level++)for(unsigned worn=0;worn<2;worn++){
  pd_demo(&base);base.level=level;base.max_hp=200;base.hp=100;base.unarmored_rule=1;base.armor=13;base.item_count=4;base.items[3]=(pd_item){.name="Anonymous charm",.quantity=1};
  base.items[0].ability=ability;base.magic.count=1;base.magic.powers[0]=(pd_power){.name="Anonymous spell",.ability=ability,.flags=PD_PREPARED};
  pd_gear gear={.kind=PD_WEAR_ITEM,.equipped=worn,.attunement=1,.ac=1,.attack=1,.damage=2,.spell_attack=2,.spell_dc=1,.saves=1,.checks=2,.max_hp=3};gear.ability[ability]=2;
  s=base;assert(pd_gear_configure(&s,3,gear,1,false));off=s;assert(s.hp==base.hp&&s.max_hp==base.max_hp&&s.armor==base.armor&&!memcmp(s.abilities,base.abilities,6));
  assert(pd_gear_attune(&s,3,true));on=s;
  assert(s.gear[3].attunement==2&&s.gear[3].equipped==worn&&s.hp==100);
  assert(s.abilities[ability]==base.abilities[ability]+(worn?2:0));assert(s.max_hp==200+(worn?3+(ability==2?level:0):0));assert(s.armor==13+(worn?1+(ability==1?1:0):0));
  pd_weapon_info a,b;assert(pd_weapon_stats(&base,0,&a)&&pd_weapon_stats(&s,0,&b));assert(b.attack_bonus==a.attack_bonus+(worn?2:0));assert(b.damage_bonus==a.damage_bonus+(worn?3:0));
  int attack,dc;assert(pd_magic_stats(&s,0,&attack,&dc));assert(attack==pd_modifier(base.abilities[ability])+pd_proficiency(level)+(worn?3:0));
  size_t n=pd_encode(&s,86,record);assert(!memcmp(record,"PDDc",4)&&pd_decode(record,n,&out,&gen)&&gen==86&&!memcmp(&s,&out,sizeof s));
  record[3]='b';crc(n);out=base;assert(!pd_decode(record,n,&out,&gen)&&!memcmp(&out,&base,sizeof out));
  assert(pd_gear_attune(&s,3,false));assert(!memcmp(&s,&off,sizeof s));
  s=on;assert(pd_item_quantity_set(&s,3,0)&&s.gear[3].attunement==1&&!s.gear[3].equipped&&s.max_hp==200&&s.hp==100);assert(!pd_gear_attune(&s,3,true));
  s=on;assert(pd_gear_equip(&s,3,false)&&s.gear[3].attunement==2&&s.max_hp==200);assert(pd_gear_equip(&s,3,true)&&s.max_hp==200+3+(ability==2?level:0));
  cases++;
 }
 pd_demo(&s);size_t n=pd_encode(&s,85,record);record[3]='b';crc(n);assert(pd_decode(record,n,&out,&gen)&&!memcmp(&out,&s,sizeof s));
 puts("Attunement: all six abilities/20 levels, independent worn/attuned gates, no healing, weapon/spell/check/HP/AC propagation, removal, old PDDb migration and no downgrade smuggling passed (synthetic)");printf("Attunement cases: %u\n",cases);
}
