#include "combat.h"
#include "checks.h"
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
int pd_attack_mode(const pd_state *s,int requested) { return s->training.poisoned?(requested>0?0:-1):requested; }
bool pd_attack(const pd_state *s,unsigned armor,int mode,pd_random random,void *context,pd_strike *out) {
    if(!s||!out||!random||armor<1||armor>30||mode< -1||mode>1)return false;
    pd_strike hit={.armor=armor};if(!pd_weapon_stats(s,s->equipped,&hit.weapon))return false;
    hit.roll=pd_d20(hit.weapon.attack_bonus,pd_attack_mode(s,mode),random,context);
    hit.critical=hit.roll.natural==20;hit.hits=hit.roll.natural!=1&&(hit.critical||hit.roll.total>=(int)armor);
    *out=hit;return true;
}
bool pd_weapon_damage(const pd_strike *hit,pd_random random,void *context,pd_pool *out) {
    if(!hit||!hit->hits)return false;
    unsigned count=hit->weapon.count*(hit->critical?2:1)+(hit->critical?hit->weapon.critical_dice:0);
    return pd_roll_pool(hit->weapon.sides,count,hit->weapon.damage_bonus,true,random,context,out);
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
