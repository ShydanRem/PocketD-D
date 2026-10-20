#ifndef PD_DAMAGE_H
#define PD_DAMAGE_H
#include "pocketdd.h"
#include "checks.h"
enum { PD_CONCENTRATION_UNRESOLVED,PD_CONCENTRATION_KEEP,PD_CONCENTRATION_END };
typedef struct { unsigned type,amount,concentration; } pd_damage_plan;
typedef struct {
    unsigned flags,adjusted,absorbed,hp_after,temp_after,concentration_dc;
    bool configured,allowed,at_zero,massive,needs_concentration,ends_concentration;
} pd_damage_summary;
const char *pd_damage_name(unsigned type);
/* A valid preview can be blocked; inspect allowed. No mutation, RNG or inference
   from imported descriptions. Output untouched for malformed input. */
bool pd_damage_preview(const pd_state *s,const pd_damage_plan *plan,pd_damage_summary *out);
/* Rejected damage leaves state and output untouched. */
bool pd_damage_apply(pd_state *s,const pd_damage_plan *plan,pd_damage_summary *out);
/* A concentration roll is permitted only for a needed, unresolved check with
   known/unblocked CON-save data. Extra is an explicit additional -30..30.
   cap30 selects a maximum DC of 30; it does not select an entire ruleset. */
bool pd_concentration_info(const pd_state *s,const pd_damage_plan *plan,int mode,int extra,pd_check_info *out,pd_damage_summary *damage,bool cap30);
/* Validates before RNG. Resulting damage and concentration are one candidate;
   the caller must save it before presenting the roll. Natural 1/20 use total vs DC. */
bool pd_concentration_roll(const pd_state *s,const pd_damage_plan *plan,int mode,int extra,pd_random random,void *context,pd_state *next,pd_roll *roll,pd_damage_summary *damage,bool cap30);
#endif
