#include "combat.h"
#include "checks.h"
#include "magic.h"
#include <stdio.h>
bool pd_weapon_stats(const pd_state *s,unsigned index,pd_weapon_info *out) {
    if(!s||!out||!s->inventory_available||index>=s->item_count)return false;
    const pd_item *v=&s->items[index];if(v->kind!=PD_WEAPON||!v->quantity)return false;
    pd_weapon_info w={.mastery=s->handling[index].flags>>4,.graze=(s->handling[index].flags>>4)==1,.index=index,.ability=v->ability,.base=pd_modifier(s->abilities[v->ability]),.trained=(v->flags&1)?pd_proficiency(s->level):0,.count=v->count,.sides=v->sides};
    if(v->ability<2&&(s->handling[index].flags&PD_FINESSE)){w.ability=s->abilities[0]>=s->abilities[1]?0:1;w.base=pd_modifier(s->abilities[w.ability]);}
    if((s->handling[index].flags&PD_VERSATILE)&&pd_weapon_two_hands(s,index))w.sides=s->handling[index].sides;
    bool thrown=s->combat.thrown&&index==s->equipped&&v->thrown_range;
    w.melee_weapon=!(v->flags&4);w.melee=w.melee_weapon&&!thrown;w.reach=thrown?v->thrown_range:v->reach;
    w.power_attack=s->combat.power_attack&&w.melee&&(v->flags&8)&&(v->flags&1);
    w.attack_extra=v->attack_modifier-(w.power_attack?5:0);
    w.damage_base=(v->flags&2)?w.base:0;
    w.damage_extra=v->damage_modifier+(w.power_attack?10:0)+(s->combat.raging&&w.melee&&w.ability==0?s->combat.rage_bonus:0);
    w.critical_dice=w.melee?s->combat.critical_dice:0;
    w.attack_bonus=w.base+w.trained+w.attack_extra;w.damage_bonus=w.damage_base+w.damage_extra;
    *out=w;return true;
}
bool pd_dual_stats_for(const pd_state *s,unsigned method,pd_weapon_info *out){
    if(method>PD_EXTRA_FEAT||!s||!out||!pd_valid(s)||!s->hp||!s->dual.weapon||s->equipped>=s->item_count)return false;
    unsigned other=s->dual.weapon-1;const pd_item *a=&s->items[s->equipped],*b=&s->items[other];
    if(other==s->equipped||a->kind!=PD_WEAPON||!a->quantity||!b->quantity||s->combat.thrown)return false;
    if(!(s->handling[s->equipped].flags&PD_LIGHT)||(method!=PD_EXTRA_FEAT&&!(s->handling[other].flags&PD_LIGHT))||pd_weapon_two_hands(s,s->equipped)||pd_weapon_two_hands(s,other))return false;
    for(unsigned i=0;i<s->item_count;i++)if(s->gear[i].kind==PD_WEAR_SHIELD&&s->gear[i].equipped)return false;
    if(s->dual.edition==1&&((a->flags|b->flags)&4))return false;
    if(method==PD_EXTRA_NICK&&(s->dual.edition!=2||(s->handling[other].flags>>4)!=2))return false;
    if(method==PD_EXTRA_FEAT&&(s->dual.edition!=2||!s->dual.feat||(b->flags&4)))return false;
    pd_weapon_info w;if(!pd_weapon_stats(s,other,&w))return false;
    w.damage_base=s->dual.style?w.base:w.base<0?w.base:0;w.damage_bonus=w.damage_base+w.damage_extra;*out=w;return true;
}
bool pd_dual_prepare_for(const pd_state *s,unsigned method,bool trigger,pd_state *next){
    pd_weapon_info w;if(!trigger||!pd_dual_stats_for(s,method,&w)||next==s)return false;
    if(method==PD_EXTRA_NICK?s->dual.light_spent:method==PD_EXTRA_FEAT?(s->dual.bonus_spent||s->dual.feat_spent):(s->dual.bonus_spent||s->dual.light_spent))return false;
    unsigned binding=s->handling[w.index].ammo;
    if(!w.melee&&binding){unsigned ammo=binding-1;if(!pd_ammunition_eligible(s,ammo)||!s->items[ammo].quantity)return false;}
    if(next){*next=*s;if(method!=PD_EXTRA_NICK)next->dual.bonus_spent=1;if(method==PD_EXTRA_FEAT)next->dual.feat_spent=1;else next->dual.light_spent=1;if(!w.melee&&binding&&!pd_item_quantity_set(next,binding-1,s->items[binding-1].quantity-1))return false;}
    return true;
}
bool pd_dual_attack_for(const pd_state *s,unsigned method,bool trigger,unsigned armor,int mode,pd_roll_context context,bool ignore,pd_random random,void *random_context,pd_strike *out){
    if(!out||!random||armor<1||armor>30||!pd_dual_prepare_for(s,method,trigger,NULL))return false;
    pd_strike hit={.armor=armor};pd_attack_resolution r;
    if(!pd_dual_stats_for(s,method,&hit.weapon)||!pd_attack_resolve(s,mode,context,ignore,&r)||!r.allowed)return false;
    hit.roll=pd_d20(hit.weapon.attack_bonus,r.mode,random,random_context);hit.critical=hit.roll.natural==20;hit.hits=hit.roll.natural!=1&&(hit.critical||hit.roll.total>=(int)armor);*out=hit;return true;
}
bool pd_dual_stats(const pd_state *s,pd_weapon_info *out){return pd_dual_stats_for(s,PD_EXTRA_LIGHT,out);}
bool pd_dual_prepare(const pd_state *s,bool trigger,pd_state *next){return pd_dual_prepare_for(s,PD_EXTRA_LIGHT,trigger,next);}
bool pd_dual_attack(const pd_state *s,bool trigger,unsigned armor,int mode,pd_roll_context c,bool ignore,pd_random r,void *ctx,pd_strike *out){return pd_dual_attack_for(s,PD_EXTRA_LIGHT,trigger,armor,mode,c,ignore,r,ctx,out);}
bool pd_cleave_stats(const pd_state *s,const pd_strike *origin,pd_weapon_info *out){
    if(!s||!out||!origin||!pd_valid(s)||!s->hp||!origin->hits||!origin->weapon.melee||origin->weapon.mastery!=3||(origin->weapon.index!=s->equipped&&s->dual.weapon!=origin->weapon.index+1)||!pd_weapon_hands_ready(s,origin->weapon.index))return false;
    pd_weapon_info w;if(!pd_weapon_stats(s,origin->weapon.index,&w)||w.mastery!=3||!w.melee)return false;
    w.damage_base=w.base<0?w.base:0;w.damage_bonus=w.damage_base+w.damage_extra;*out=w;return true;
}
bool pd_cleave_prepare(const pd_state *s,const pd_strike *origin,bool target,pd_state *next){
    pd_weapon_info w;if(!target||!pd_cleave_stats(s,origin,&w)||s->dual.cleave_spent||next==s)return false;
    if(next){*next=*s;next->dual.cleave_spent=1;}return true;
}
bool pd_cleave_attack(const pd_state *s,const pd_strike *origin,bool target,unsigned armor,int mode,pd_roll_context c,bool ignore,pd_random random,void *ctx,pd_strike *out){
    if(!out||!random||armor<1||armor>30||!pd_cleave_prepare(s,origin,target,NULL))return false;
    pd_strike hit={.armor=armor};pd_attack_resolution r;if(!pd_cleave_stats(s,origin,&hit.weapon)||!pd_attack_resolve(s,mode,c,ignore,&r)||!r.allowed)return false;
    hit.roll=pd_d20(hit.weapon.attack_bonus,r.mode,random,ctx);hit.critical=hit.roll.natural==20;hit.hits=hit.roll.natural!=1&&(hit.critical||hit.roll.total>=(int)armor);*out=hit;return true;
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
    pd_strike hit={.armor=armor};if(!pd_weapon_hands_ready(s,s->equipped)||!pd_weapon_stats(s,s->equipped,&hit.weapon))return false;
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
    w.true_strike=true;w.radiant_weapon=radiant;w.ability=p->ability;w.base=pd_modifier(s->abilities[p->ability]);
    w.damage_base=w.base; /* True Strike explicitly adds the casting modifier. */
    w.attack_extra+=p->attack_modifier; /* Explicit per-profile corrections only. */
    w.damage_extra=s->items[s->equipped].damage_modifier+(w.power_attack?10:0)+p->save_modifier;
    w.attack_bonus=w.base+w.trained+w.attack_extra;w.damage_bonus=w.damage_base+w.damage_extra;
    w.radiant_dice=s->level>=17?3:s->level>=11?2:s->level>=5?1:0;*out=w;return true;
}
bool pd_true_strike_attack(const pd_state *s,unsigned power,bool radiant,unsigned armor,int mode,pd_roll_context context,bool ignore,pd_random random,void *random_context,pd_strike *out){
    if(!out||!random||armor<1||armor>30||mode< -1||mode>1)return false;
    pd_strike hit={.armor=armor};pd_attack_resolution r;
    if(!pd_weapon_hands_ready(s,s->equipped)||!pd_true_strike_stats(s,power,radiant,&hit.weapon)||!pd_attack_resolve(s,mode,context,ignore,&r)||!r.allowed)return false;
    hit.roll=pd_d20(hit.weapon.attack_bonus,r.mode,random,random_context);hit.critical=hit.roll.natural==20;hit.hits=hit.roll.natural!=1&&(hit.critical||hit.roll.total>=(int)armor);*out=hit;return true;
}
bool pd_graze_damage(const pd_strike *hit,unsigned *damage){
    if(!hit||!damage||hit->hits||!hit->weapon.graze)return false;
    *damage=hit->weapon.base>0?(unsigned)hit->weapon.base:0;return true;
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
    pd_weapon_info w;if(index!=PD_NO_WEAPON&&(!pd_weapon_equip_ready(s,index)||!pd_weapon_stats(s,index,&w)))return false;
    s->equipped=index;s->combat.thrown=false;return true;
}
bool pd_ammunition_eligible(const pd_state *s,unsigned index){
    return s&&s->inventory_available&&index<PD_ITEMS&&index<s->item_count&&index!=s->equipped&&s->items[index].kind<=PD_CONSUMABLE&&!s->gear[index].kind&&!s->items[index].charges.maximum;
}
bool pd_ammunition_prepare(const pd_state *s,unsigned index,pd_state *next){
    pd_weapon_info weapon;
    if(!s||!next||s==next||!pd_valid(s)||!pd_ammunition_eligible(s,index)||!s->items[index].quantity||s->combat.thrown||!pd_weapon_stats(s,s->equipped,&weapon)||!(s->items[s->equipped].flags&4))return false;
    *next=*s;return pd_item_quantity_set(next,index,s->items[index].quantity-1);
}
bool pd_weapon_damage_with_effect(const pd_state *s,const pd_strike *hit,bool target,pd_random random,void *context,pd_weapon_damage_parts *out){
    if(!s||!hit||!random||!out||!pd_valid(s))return false;
    int power=target?pd_on_hit_spell(s):-1;if(target&&power<0)return false;
    pd_weapon_damage_parts result;
    if(!pd_weapon_damage_roll(hit,random,context,&result))return false;
    if(target){
        if(!pd_roll_pool(6,hit->critical?2:1,0,true,random,context,&result.extra))return false;
        result.extra_effect=s->magic.powers[power].effect;result.total+=result.extra.total;
    }
    *out=result;return true;
}
bool pd_use_item(const pd_state *s,unsigned index,pd_random random,void *context,pd_state *next,pd_pool *roll,unsigned *gained) {
    if(!s||!next||!roll||!gained||!s->inventory_available||index>=s->item_count)return false;
    const pd_item *v=&s->items[index];if(!v->quantity||(v->kind!=PD_CONSUMABLE&&v->kind!=PD_HEALING))return false;
    pd_pool r={0};if(v->kind==PD_HEALING&&(s->hp==pd_max_hp(s)||(!s->hp&&s->status.available&&s->status.failures==3)||!pd_roll_pool(v->sides,v->count,v->healing_modifier,false,random,context,&r)))return false;
    *next=*s;next->items[index].quantity--;next->combat.thrown=false;unsigned old=s->hp;
    if(v->kind==PD_HEALING)pd_hp(next,r.total);
    *gained=next->hp-old;*roll=r;return true;
}
bool pd_smite_prepare(const pd_state *s,const pd_strike *hit,unsigned index,unsigned slot,bool pact,bool fiend,pd_state *next,unsigned *dice){
    if(!s||!hit||!pd_valid(s)||!s->hp||s->combat.raging||s->dual.bonus_spent||!hit->hits||!hit->weapon.melee_weapon||!s->magic.available||index>=s->magic.count)return false;
    const pd_power *p=&s->magic.powers[index];
    if(p->effect!=PD_DIVINE_SMITE_2024||!(p->flags&PD_PREPARED)||((p->flags&PD_LIMITED)&&!p->remaining))return false;
    if(p->flags&PD_USES_SLOT){if(slot<1||slot>9||!(pact?s->magic.pact:s->magic.slots)[slot-1].current)return false;}
    else if(slot||pact||!(p->flags&PD_LIMITED))return false;
    unsigned n=((slot?slot:1)+1+(fiend?1:0))*(hit->critical?2:1);
    if(next){*next=*s;next->dual.bonus_spent=1;if(p->flags&PD_USES_SLOT)(pact?next->magic.pact:next->magic.slots)[slot-1].current--;if(p->flags&PD_LIMITED)next->magic.powers[index].remaining--;}
    if(dice)*dice=n;
    return true;
}
bool pd_smite_damage(const pd_state *s,const pd_strike *hit,unsigned index,unsigned slot,bool pact,bool fiend,bool target,pd_random random,void *context,pd_weapon_damage_parts *out){
    unsigned dice;if(!random||!out||!pd_smite_prepare(s,hit,index,slot,pact,fiend,NULL,&dice)||(target&&pd_on_hit_spell(s)<0))return false;
    pd_weapon_damage_parts result;if(!pd_weapon_damage_with_effect(s,hit,target,random,context,&result))return false;
    unsigned first=dice>PD_POOL_MAX?PD_POOL_MAX:dice,extra=dice-first;
    if(!pd_roll_pool(8,first,0,true,random,context,&result.smite))return false;
    if(extra&&!pd_roll_pool(8,extra,0,true,random,context,&result.smite_overflow))return false;
    result.total+=result.smite.total+result.smite_overflow.total;*out=result;return true;
}

const char *pd_mastery_name(unsigned m){static const char *const names[]={"Nessuna","Scalfire","Nick","Cleave","Push","Sap","Slow","Topple","Vex"};return m<9?names[m]:"Non supportata";}
bool pd_mastery_text(const pd_strike *hit,unsigned level,char *text,size_t cap){
    if(!hit||!text||!cap||level<1||level>20||hit->weapon.mastery<1||hit->weapon.mastery>8)return false;
    const char *note="";unsigned m=hit->weapon.mastery;int n=0;
    if(!hit->hits&&m!=1&&m!=2)note="Mancato: maestria non applicabile";
    else if(m==1){unsigned damage=0;if(pd_graze_damage(hit,&damage))n=snprintf(text,cap,"Mancato: Scalfire %u. Stesso tipo dell'arma; difese al tavolo.",damage);else note="Colpito: usa i danni normali, senza Scalfire.";}
    else if(m==2)note="Nick permette l'extra Light in Attacco senza bonus. Il limite per turno vale anche sul mancato.";
    else if(m==3)note="Cleave: altro nemico entro 1,5 m dal primo e nella tua portata. Un extra per turno.";
    else if(m==4)note="Push: puoi spingere fino a 3 m, diritto lontano da te; solo taglia Grande o inferiore.";
    else if(m==5)note="Sap: svantaggio al prossimo attacco del nemico prima dell'inizio del tuo prossimo turno.";
    else if(m==6)note="Slow: solo se infliggi danni, velocita -3 m fino all'inizio del tuo prossimo turno. Non si somma.";
    else if(m==7)n=snprintf(text,cap,"Topple: TS COS CD %d del bersaglio. Su fallimento diventa Prono; esito dichiarato dal DM.",8+hit->weapon.base+pd_proficiency(level));
    else note="Vex: solo se infliggi danni. Vantaggio al tuo prossimo attacco contro lo stesso nemico, entro fine del tuo prossimo turno.";
    if(!n)n=snprintf(text,cap,"%s",note);
    if(n<0||(size_t)n>=cap){text[0]=0;return false;}return true;
}
