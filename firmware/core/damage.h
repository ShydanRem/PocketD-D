#ifndef PD_DAMAGE_H
#define PD_DAMAGE_H
#include "pocketdd.h"
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
#endif
