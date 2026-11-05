#pragma once
#include "magic.h"
#include "dice_pool.h"
/* Per-cast, explicit formula. No inference from names, text or edition and no
   persistent character/protocol fields. Attack/save rules stay at the table. */
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
