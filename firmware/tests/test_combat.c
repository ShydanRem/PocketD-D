#include "combat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;static uint32_t value;
static uint32_t rng(void *unused){(void)unused;calls++;return value;}
int main(void){
 pd_state s,next,before;pd_demo(&s);pd_strike hit={0},old=hit;pd_pool roll;unsigned gained;
 assert(!pd_attack(&s,0,0,rng,NULL,&hit)&&!pd_attack(&s,31,0,rng,NULL,&hit)&&!pd_attack(&s,15,2,rng,NULL,&hit));
 assert(!calls&&!memcmp(&hit,&old,sizeof hit));
 assert(!pd_weapon_damage(&hit,rng,NULL,&roll)&&!calls);
 before=next=s;assert(!pd_use_item(&s,0,rng,NULL,&next,&roll,&gained)&&!memcmp(&next,&before,sizeof next)&&!calls);
 s.hp=s.max_hp;assert(!pd_use_item(&s,1,rng,NULL,&next,&roll,&gained)&&!calls&&s.items[1].quantity==2);
 s.hp=31;s.temp_hp=9;assert(pd_use_item(&s,1,rng,NULL,&next,&roll,&gained));assert(calls==2&&gained==1&&next.hp==32&&next.temp_hp==9&&next.items[1].quantity==1&&s.items[1].quantity==2);
 s.items[2].quantity=0;assert(!pd_use_item(&s,2,rng,NULL,&next,&roll,&gained)&&calls==2);
 assert(!pd_equip(&s,1)&&s.equipped==0);assert(pd_equip(&s,PD_NO_WEAPON)&&s.equipped==PD_NO_WEAPON);assert(!pd_attack(&s,15,0,rng,NULL,&hit)&&calls==2);
 assert(pd_equip(&s,0));s.items[0].count=6;s.combat.critical_dice=6;value=19;assert(pd_attack(&s,30,0,rng,NULL,&hit)&&hit.hits&&hit.critical);
 value=0;assert(pd_weapon_damage(&hit,rng,NULL,&roll)&&roll.count==18&&roll.total==20);
 assert(pd_attack(&s,1,0,rng,NULL,&hit)&&!hit.hits&&!hit.critical);
 s.items[0].quantity=0;assert(!pd_valid(&s));s.equipped=PD_NO_WEAPON;assert(pd_valid(&s));
 puts("Combat: validation before RNG, untouched outputs, full-HP no consumption, atomic healing, equipment, natural 1/20 and 18-die critical passed");
}
