#include "combat.h"
#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls,face;
static uint32_t random_face(void *unused){(void)unused;calls++;return face;}
int main(void){
 pd_state s;pd_demo(&s);s.level=6;s.abilities[0]=15;s.abilities[5]=17;s.items[0].count=2;s.items[0].sides=6;s.items[0].attack_modifier=1;s.items[0].damage_modifier=1;s.magic.count=1;s.magic.powers[0]=(pd_power){.name="True Strike",.flags=PD_PREPARED,.ability=5,.effect=PD_TRUE_STRIKE_2024};assert(pd_valid(&s));
 pd_weapon_info w;assert(pd_true_strike_stats(&s,0,false,&w)&&w.attack_bonus==7&&w.damage_bonus==4&&w.radiant_dice==1);assert(!pd_magic_use(&s,0,0,false));
 for(unsigned level=1;level<=20;level++){s.level=level;assert(pd_true_strike_stats(&s,0,false,&w)&&w.radiant_dice==(level<5?0:level<11?1:level<17?2:3));}s.level=6;
 pd_strike strike;face=19;assert(pd_true_strike_attack(&s,0,true,30,0,(pd_roll_context){0},false,random_face,NULL,&strike)&&strike.critical&&strike.hits&&strike.weapon.radiant_weapon);pd_weapon_damage_parts damage;face=0;unsigned previous=calls;assert(pd_weapon_damage_roll(&strike,random_face,NULL,&damage));assert(damage.weapon.count==4&&damage.radiant.count==2&&damage.weapon.total==8&&damage.radiant.total==2&&damage.total==10&&calls==previous+6);
 face=0;assert(pd_true_strike_attack(&s,0,false,1,0,(pd_roll_context){0},false,random_face,NULL,&strike)&&!strike.hits);previous=calls;assert(!pd_weapon_damage_roll(&strike,random_face,NULL,&damage)&&calls==previous);
 pd_strike before=strike;s.magic.powers[0].flags=0;previous=calls;assert(!pd_true_strike_attack(&s,0,false,16,0,(pd_roll_context){0},false,random_face,NULL,&strike)&&!memcmp(&before,&strike,sizeof strike)&&calls==previous);
 s.magic.powers[0].flags=PD_PREPARED;s.combat.rage_bonus=2;s.combat.raging=true;assert(!pd_true_strike_stats(&s,0,false,&w));s.combat.raging=false;s.hp=0;assert(!pd_true_strike_stats(&s,0,false,&w));
 puts("True Strike 2024: casting ability, weapon proficiency/magic, whole-character scaling, radiant choice, critical mixed components, miss/unprepared/rage/zero-HP guards, no detached cast or rejected RNG passed; synthetic only.");
}
