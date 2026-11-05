#pragma once
#include "magic.h"
#include "dice_pool.h"
#include "checks.h"
/* Per-cast, explicit formula. No inference from names, text or edition and no
   persistent character/protocol fields. Combat resolution is explicit; target
   saving throws and unrepresented rules remain at the table. */
enum {PD_SPELL_DAMAGE=1,PD_SPELL_HEAL_SELF,PD_SPELL_HEAL_OTHER};
typedef struct {
    unsigned power,slot,kind,count,sides;
    int bonus;
    bool pact;
} pd_spell_roll_plan;
/* Resolve a saved BASE formula for this payment. Reject overflow, absent or
   invalid data; never clamp away dice or silently fall back to another roll. */
bool pd_spell_formula_resolve(const pd_state *,unsigned,unsigned,bool,pd_spell_roll_plan *);
/* Preview all costs and concentration consequences before RNG. Only generic
   spell rows (effect=0) are eligible; special weapon/feature flows stay intact.
   A slot-free spell must use slot=0/pact=false: never silently ignore payment. */
bool pd_spell_roll_prepare(const pd_state *,const pd_spell_roll_plan *,pd_state *);
/* Freeze one pool and a complete candidate. Both damage and healing floor at
   zero; self healing caps at the effective maximum after casting. Other-target
   rolls never alter local HP. Invalid requests leave every output untouched. */
bool pd_spell_roll(const pd_state *,const pd_spell_roll_plan *,pd_random,void *,pd_state *,pd_pool *,unsigned *gained);
enum {PD_SPELL_ATTACK=1,PD_SPELL_TARGET_SAVE};
enum {PD_SPELL_SAVE_UNSET,PD_SPELL_SAVE_ZERO,PD_SPELL_SAVE_HALF};
typedef struct {unsigned mode,armor,save_ability,success;int requested;pd_roll_context context;} pd_spell_battle_plan;
typedef struct {pd_roll roll;int mode;bool hits,critical;} pd_spell_attack_result;
typedef struct {pd_pool first,second;int rolled,applicable;unsigned impact;bool critical;} pd_spell_impact;
enum {PD_SPELL_FULL,PD_SPELL_HALF,PD_SPELL_ZERO};
/* Shared condition facts, no weapon/STR proficiency or spell-save inference. */
bool pd_spell_attack_gate(const pd_state *,int,pd_roll_context,int *mode,unsigned *needed);
/* Preview validates costs and every decision before touching the candidate.
   Target saves consume no entropy: the caller later declares the actual result. */
bool pd_spell_battle_prepare(const pd_state *,const pd_spell_roll_plan *,const pd_spell_battle_plan *,pd_state *);
bool pd_spell_attack_cast(const pd_state *,const pd_spell_roll_plan *,const pd_spell_battle_plan *,pd_random,void *,pd_state *,pd_spell_attack_result *);
/* Frozen paid cast only; caller ensures one use. Crit doubles dice, never the
   bonus. Up to 36 dice split into two pools; half applies to their whole sum. */
bool pd_spell_impact_roll(const pd_spell_roll_plan *,bool critical,unsigned impact,pd_random,void *,pd_spell_impact *);
