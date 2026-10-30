#include "combat.h"
#include "checks.h"
#include "magic.h"
bool pd_weapon_stats(const pd_state *s,unsigned index,pd_weapon_info *out) {
    if(!s||!out||!s->inventory_available||index>=s->item_count)return false;
    const pd_item *v=&s->items[index];if(v->kind!=PD_WEAPON||!v->quantity)return false;
    pd_weapon_info w={.base=pd_modifier(s->abilities[v->ability]),.trained=(v->flags&1)?pd_proficiency(s->level):0,.count=v->count,.sides=v->sides};
    bool thrown=s->combat.thrown&&index==s->equipped&&v->thrown_range;
    w.melee=!(v->flags&4)&&!thrown;w.reach=thrown?v->thrown_range:v->reach;
    w.power_attack=s->combat.power_attack&&w.melee&&(v->flags&8)&&(v->flags&1);
    w.attack_extra=v->attack_modifier-(w.power_attack?5:0);
    w.damage_base=(v->flags&2)?w.base:0;
    w.damage_extra=v->damage_modifier+(w.power_attack?10:0)+(s->combat.raging&&w.melee&&v->ability==0?s->combat.rage_bonus:0);
    w.critical_dice=w.melee?s->combat.critical_dice:0;
    w.attack_bonus=w.base+w.trained+w.attack_extra;w.damage_bonus=w.damage_base+w.damage_extra;
    *out=w;return true;
}
bool pd_attack_resolve(const pd_state *s,int requested,pd_roll_context context,bool ignore,pd_attack_resolution *out){
    pd_condition_result effect;if(!out||requested< -1||requested>1||!pd_condition_evaluate(s,PD_ROLL_ATTACK,0,context,ignore,&effect))return false;
    bool advantage=requested>0,disadvantage=requested<0||(!ignore&&s->training.poisoned)||effect.disadvantage;
    if(disadvantage)effect.needed&=~PD_CONTEXT_FEAR;
    *out=(pd_attack_resolution){.mode=advantage==disadvantage?0:advantage?1:-1,.allowed=!effect.needed,.needed=effect.needed};return true;
}
int pd_attack_mode(const pd_state *s,int requested){pd_attack_resolution r;return pd_attack_resolve(s,requested,(pd_roll_context){0},false,&r)?r.mode:0;}
bool pd_attack(const pd_state *s,unsigned armor,int mode,pd_random random,void *context,pd_strike *out){return pd_attack_with_context(s,armor,mode,(pd_roll_context){0},false,random,context,out);}
bool pd_attack_with_context(const pd_state *s,unsigned armor,int mode,pd_roll_context context,bool ignore,pd_random random,void *random_context,pd_strike *out) {
    if(!s||!out||!random||armor<1||armor>30||mode< -1||mode>1)return false;
    pd_strike hit={.armor=armor};if(!pd_weapon_stats(s,s->equipped,&hit.weapon))return false;
    pd_attack_resolution r;if(!pd_attack_resolve(s,mode,context,ignore,&r)||!r.allowed)return false;
    hit.roll=pd_d20(hit.weapon.attack_bonus,r.mode,random,random_context);
    hit.critical=hit.roll.natural==20;hit.hits=hit.roll.natural!=1&&(hit.critical||hit.roll.total>=(int)armor);
    *out=hit;return true;
}
bool pd_true_strike_stats(const pd_state *s,unsigned power,bool radiant,pd_weapon_info *out){
    if(!s||!out||!pd_valid(s)||!s->hp||s->combat.raging||!s->magic.available||power>=s->magic.count)return false;
    const pd_power *p=&s->magic.powers[power];
    if(p->effect!=PD_TRUE_STRIKE_2024||!(p->flags&PD_PREPARED))return false;
    pd_weapon_info w;if(!pd_weapon_stats(s,s->equipped,&w))return false;
    w.true_strike=true;w.radiant_weapon=radiant;w.base=pd_modifier(s->abilities[p->ability]);
    w.damage_base=w.base; /* True Strike explicitly adds the casting modifier. */
    w.attack_extra+=p->attack_modifier; /* Explicit per-profile corrections only. */
    w.damage_extra=s->items[s->equipped].damage_modifier+(w.power_attack?10:0)+p->save_modifier;
    w.attack_bonus=w.base+w.trained+w.attack_extra;w.damage_bonus=w.damage_base+w.damage_extra;
    w.radiant_dice=s->level>=17?3:s->level>=11?2:s->level>=5?1:0;*out=w;return true;
}
bool pd_true_strike_attack(const pd_state *s,unsigned power,bool radiant,unsigned armor,int mode,pd_roll_context context,bool ignore,pd_random random,void *random_context,pd_strike *out){
    if(!out||!random||armor<1||armor>30||mode< -1||mode>1)return false;
    pd_strike hit={.armor=armor};pd_attack_resolution r;
    if(!pd_true_strike_stats(s,power,radiant,&hit.weapon)||!pd_attack_resolve(s,mode,context,ignore,&r)||!r.allowed)return false;
    hit.roll=pd_d20(hit.weapon.attack_bonus,r.mode,random,random_context);hit.critical=hit.roll.natural==20;hit.hits=hit.roll.natural!=1&&(hit.critical||hit.roll.total>=(int)armor);*out=hit;return true;
}
bool pd_weapon_damage(const pd_strike *hit,pd_random random,void *context,pd_pool *out) {
    if(!hit||!hit->hits)return false;
    unsigned count=hit->weapon.count*(hit->critical?2:1)+(hit->critical?hit->weapon.critical_dice:0);
    return pd_roll_pool(hit->weapon.sides,count,hit->weapon.damage_bonus,true,random,context,out);
}
bool pd_weapon_damage_roll(const pd_strike *hit,pd_random random,void *context,pd_weapon_damage_parts *out){
    if(!hit||!out||!random||!hit->hits||hit->weapon.radiant_dice>3)return false;
    pd_weapon_damage_parts roll={0};
    /* Validate both pools before drawing either, then freeze both results. */
    unsigned weapon=hit->weapon.count*(hit->critical?2:1)+(hit->critical?hit->weapon.critical_dice:0),radiant=hit->weapon.radiant_dice*(hit->critical?2:1);
    if(!pd_pool_sides(hit->weapon.sides)||!weapon||weapon>PD_POOL_MAX||radiant>PD_POOL_MAX||hit->weapon.damage_bonus<PD_POOL_MIN_BONUS||hit->weapon.damage_bonus>PD_POOL_MAX_BONUS)return false;
    if(!pd_weapon_damage(hit,random,context,&roll.weapon))return false;
    if(radiant&&!pd_roll_pool(6,radiant,0,true,random,context,&roll.radiant))return false;
    roll.total=roll.weapon.total+roll.radiant.total;*out=roll;return true;
}
bool pd_equip(pd_state *s,unsigned index) {
    pd_weapon_info w;if(index!=PD_NO_WEAPON&&!pd_weapon_stats(s,index,&w))return false;
    s->equipped=index;s->combat.thrown=false;return true;
}
bool pd_use_item(const pd_state *s,unsigned index,pd_random random,void *context,pd_state *next,pd_pool *roll,unsigned *gained) {
    if(!s||!next||!roll||!gained||!s->inventory_available||index>=s->item_count)return false;
    const pd_item *v=&s->items[index];if(!v->quantity||(v->kind!=PD_CONSUMABLE&&v->kind!=PD_HEALING))return false;
    pd_pool r={0};if(v->kind==PD_HEALING&&(s->hp==pd_max_hp(s)||(!s->hp&&s->status.available&&s->status.failures==3)||!pd_roll_pool(v->sides,v->count,v->healing_modifier,false,random,context,&r)))return false;
    *next=*s;next->items[index].quantity--;next->combat.thrown=false;unsigned old=s->hp;
    if(v->kind==PD_HEALING)pd_hp(next,r.total);
    *gained=next->hp-old;*roll=r;return true;
}
