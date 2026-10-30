#ifndef PD_COMBAT_H
#define PD_COMBAT_H
#include "dice_pool.h"
#include "checks.h"
typedef struct {
    int base,trained,attack_extra,damage_base,damage_extra,attack_bonus,damage_bonus;
    unsigned count,sides,critical_dice,reach,radiant_dice;
    bool melee,melee_weapon,power_attack,true_strike,radiant_weapon;
} pd_weapon_info;
typedef struct { pd_roll roll; pd_weapon_info weapon; unsigned armor; bool hits,critical; } pd_strike;
bool pd_weapon_stats(const pd_state *s,unsigned index,pd_weapon_info *out);
/* 2024 weapon cantrip: casting ability replaces STR/DEX, weapon proficiency
   and weapon bonuses remain; spell-attack bonuses do not become weapon bonuses. */
bool pd_true_strike_stats(const pd_state *s,unsigned power,bool radiant,pd_weapon_info *out);
bool pd_true_strike_attack(const pd_state *s,unsigned power,bool radiant,unsigned armor,int mode,pd_roll_context context,bool ignore,pd_random random,void *random_context,pd_strike *out);
typedef struct {pd_pool weapon,radiant,extra,smite,smite_overflow;uint8_t extra_effect;int total;} pd_weapon_damage_parts;
bool pd_weapon_damage_roll(const pd_strike *strike,pd_random random,void *context,pd_weapon_damage_parts *out);
/* Explicit confirmation that THIS hit targets the concentrated spell's target. */
bool pd_weapon_damage_with_effect(const pd_state *s,const pd_strike *strike,bool target,pd_random random,void *context,pd_weapon_damage_parts *out);
/* Predict/validate costs before any RNG or mutation. NULL next allows preview.
   Bonus action, verbal components and turn spell rules remain explicit at table. */
bool pd_smite_prepare(const pd_state *s,const pd_strike *hit,unsigned power,unsigned slot,bool pact,bool fiend,pd_state *next,unsigned *dice);
bool pd_smite_damage(const pd_state *s,const pd_strike *hit,unsigned power,unsigned slot,bool pact,bool fiend,bool target,pd_random random,void *context,pd_weapon_damage_parts *out);
typedef struct { int mode; bool allowed; uint8_t needed; } pd_attack_resolution;
bool pd_attack_resolve(const pd_state *s,int requested,pd_roll_context context,bool ignore,pd_attack_resolution *out);
bool pd_attack_with_context(const pd_state *s,unsigned armor,int mode,pd_roll_context context,bool ignore,pd_random random,void *random_context,pd_strike *out);
int pd_attack_mode(const pd_state *s,int requested);
bool pd_attack(const pd_state *s,unsigned armor,int mode,pd_random random,void *context,pd_strike *out);
bool pd_weapon_damage(const pd_strike *strike,pd_random random,void *context,pd_pool *out);
bool pd_equip(pd_state *s,unsigned index);
/* Consumable/healing only. No draw at full HP. Caller commits next atomically. */
bool pd_use_item(const pd_state *s,unsigned index,pd_random random,void *context,pd_state *next,pd_pool *roll,unsigned *gained);
#endif
