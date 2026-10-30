#include "pocketdd.h"
#include "checks.h"
#include "combat.h"
#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void){
 pd_state s,before,out;pd_demo(&s);s.level=6;s.armor=16;s.hp=35;s.max_hp=53;s.abilities[1]=10;s.abilities[2]=16;s.abilities[5]=17;s.item_count=8;
 s.items[3]=(pd_item){.name="Chain mail",.quantity=1};s.items[4]=(pd_item){.name="Shield",.quantity=1};s.items[5]=(pd_item){.name="Ability charm",.quantity=1};s.items[6]=(pd_item){.name="Leather",.quantity=1};s.items[7]=(pd_item){.name="Focus",.quantity=1};
 s.magic.count=2;s.magic.powers[0]=(pd_power){.name="Spell",.flags=PD_PREPARED,.ability=5};s.magic.powers[1]=(pd_power){.name="Initiative",.kind=1,.level=6,.flags=PD_PASSIVE,.effect=5};assert(pd_valid(&s));
 pd_gear chain={.kind=PD_WEAR_HEAVY,.armor=16,.equipped=true};assert(pd_gear_configure(&s,3,chain,1,true));assert(s.armor==16&&s.hp==35);assert(pd_gear_equip(&s,3,false)&&s.armor==10);assert(pd_gear_equip(&s,3,true)&&s.armor==16);
 pd_gear shield={.kind=PD_WEAR_SHIELD,.armor=2,.equipped=true};assert(pd_gear_configure(&s,4,shield,1,false)&&s.armor==18);
 pd_gear charm={.kind=PD_WEAR_ITEM,.equipped=true,.ability={0,2,2,0,0,2},.attack=1,.damage=2,.spell_attack=1,.spell_dc=2,.checks=1,.saves=2,.max_hp=4};
 assert(pd_gear_configure(&s,5,charm,1,false));assert(s.armor==18&&s.max_hp==63&&s.hp==35&&s.abilities[1]==12&&s.abilities[2]==18&&s.abilities[5]==19);
 int attack,dc;assert(pd_magic_stats(&s,0,&attack,&dc)&&attack==8&&dc==17);pd_check_info check;assert(pd_describe_check(&s,PD_SKILL,0,&check)&&check.total==2);assert(pd_describe_check(&s,PD_SAVE,2,&check)&&check.total==6);assert(s.magic.powers[1].attack_modifier==2);
 pd_weapon_info w;assert(pd_weapon_stats(&s,0,&w)&&w.attack_extra==1&&w.damage_extra==2);
 assert(pd_gear_equip(&s,3,false)&&s.armor==13);pd_gear leather={.kind=PD_WEAR_LIGHT,.armor=11,.equipped=true};assert(pd_gear_configure(&s,6,leather,1,false)&&s.armor==14);
 assert(pd_gear_equip(&s,3,true)&&s.armor==18&&!s.gear[6].equipped);s.hp=63;assert(pd_gear_equip(&s,5,false)&&s.max_hp==53&&s.hp==53&&s.armor==18&&s.abilities[2]==16&&s.magic.powers[1].attack_modifier==0);
 assert(pd_item_quantity_set(&s,4,0)&&!s.gear[4].equipped&&s.armor==16);assert(!pd_gear_equip(&s,4,true));
 before=s;pd_gear bad={.kind=PD_WEAR_ITEM,.equipped=true,.ability={0,0,-10,0,0,0},.max_hp=-999};assert(!pd_gear_configure(&s,7,bad,1,false)&&!memcmp(&s,&before,sizeof s));bad=charm;bad.ability[5]=10;s.abilities[5]=29;before=s;assert(!pd_gear_configure(&s,7,bad,1,false)&&!memcmp(&s,&before,sizeof s));s.abilities[5]=17;
 uint8_t *record=malloc(PD_RECORD_SIZE);assert(record);size_t n=pd_encode(&s,42,record);uint32_t gen;assert(n==pd_encoded_size(&s)&&pd_decode(record,n,&out,&gen)&&gen==42&&!memcmp(&s,&out,sizeof s));assert(!pd_record_known((const uint8_t *)"PDDb",4));record[3]='b';assert(!pd_record_known(record,n-PD_EQUIPMENT_BYTES));free(record);
 s.unarmored_rule=0;before=s;assert(pd_gear_configure(&s,3,s.gear[3],2,false)&&s.armor==before.armor);
 pd_demo(&s);s.item_count=4;s.items[3]=(pd_item){.name="Dexterity item",.quantity=1};before=s;bad=(pd_gear){.kind=PD_WEAR_ITEM,.equipped=true,.ability={0,2,0,0,0,0}};assert(!pd_gear_configure(&s,3,bad,0,false)&&!memcmp(&s,&before,sizeof s));
 puts("Loadout: imported-value adoption, exclusive armor/shield, DEX AC, CON max HP without healing, checks/saves/weapon/spell/initiative propagation, quantity removal, atomic invalid rollback and native roundtrip passed; synthetic only.");
}
