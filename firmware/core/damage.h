#ifndef PD_DAMAGE_H
#define PD_DAMAGE_H
#include "pocketdd.h"
#include "checks.h"
enum { PD_CONCENTRATION_UNRESOLVED,PD_CONCENTRATION_KEEP,PD_CONCENTRATION_END };
typedef struct { unsigned type,amount,concentration; } pd_damage_plan;
typedef struct {
    unsigned flags,adjusted,absorbed,hp_after,temp_after,concentration_dc,ended_effects,max_hp_after;
    bool configured,allowed,at_zero,massive,needs_concentration,ends_concentration;
} pd_damage_summary;
#define PD_DAMAGE_COMPONENTS 6
enum { PD_SOURCE_MAGICAL=1,PD_SOURCE_ATTACK=2,PD_SOURCE_SILVERED=4,PD_SOURCE_ADAMANTINE=8 };
/* Unknown is distinct from false; values must be a subset of known. */
typedef struct { uint8_t known,values; } pd_damage_context;
typedef struct { unsigned type,amount; } pd_damage_component;
/* One source/impact, not a sequence of separate hits. Count 1..6, amount 0..9999.
   Same type + resolved defensive flags are summed before rounding. */
typedef struct { unsigned count,concentration; pd_damage_component components[PD_DAMAGE_COMPONENTS]; pd_damage_context contexts[PD_DAMAGE_COMPONENTS]; } pd_damage_event;
typedef struct { unsigned type,amount,adjusted,flags; bool configured; uint8_t unresolved; /* Property bits not decided by the known source facts. */ } pd_damage_group;
typedef struct {
    pd_damage_summary total; /* flags meaningful only when there is one group. */
    unsigned raw,count;
    uint16_t missing; /* Unconfigured damage-type bits. */
    uint8_t context_missing,needed[PD_DAMAGE_COMPONENTS]; /* Row bits, then needed trait bits per row. */
    pd_damage_group groups[PD_DAMAGE_COMPONENTS]; /* Sorted by type, effective flags and unresolved properties. */
} pd_damage_event_summary;
/* Any unknown profile or relevant source fact blocks the whole event, even for zero raw damage. In that
   case total HP/temp stay unchanged and total.adjusted is not a partial sum. */
bool pd_damage_event_preview(const pd_state *state,const pd_damage_event *event,pd_damage_event_summary *out);
bool pd_damage_event_apply(pd_state *state,const pd_damage_event *event,pd_damage_event_summary *out);
/* One CON save on the combined effective damage. Same validation/frozen-candidate
   contract as the single-component APIs below; no new persistence fields. */
bool pd_concentration_event_info(const pd_state *state,const pd_damage_event *event,int mode,int extra,pd_check_info *out,pd_damage_event_summary *damage,bool cap30);
bool pd_concentration_event_roll(const pd_state *state,const pd_damage_event *event,int mode,int extra,pd_random random,void *context,pd_state *next,pd_roll *roll,pd_damage_event_summary *damage,bool cap30);
const char *pd_damage_name(unsigned type);
/* A valid preview can be blocked; inspect allowed. No mutation, RNG or inference
   from imported descriptions. hp_after includes the cap from ending bound effects;
   max_hp_after is the final effective maximum. Output untouched for malformed input. */
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
