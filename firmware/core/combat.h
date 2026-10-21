#ifndef PD_COMBAT_H
#define PD_COMBAT_H
#include "dice_pool.h"
#include "checks.h"
typedef struct {
    int base,trained,attack_extra,damage_base,damage_extra,attack_bonus,damage_bonus;
    unsigned count,sides,critical_dice,reach;
    bool melee,power_attack;
} pd_weapon_info;
typedef struct { pd_roll roll; pd_weapon_info weapon; unsigned armor; bool hits,critical; } pd_strike;
bool pd_weapon_stats(const pd_state *s,unsigned index,pd_weapon_info *out);
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
