#ifndef PD_CHECKS_H
#define PD_CHECKS_H
#include "pocketdd.h"
typedef enum { PD_CHECK,PD_SKILL,PD_SAVE } pd_check_kind;
extern const char *const pd_skill_ids[PD_SKILLS];
extern const char *const pd_skill_names[PD_SKILLS];
extern const char *const pd_abilities[6];
typedef struct {
    unsigned ability,rank;
    int base,trained,extra,total;
    bool half,blocked;
} pd_check_info;
enum { PD_ROLL_ATTACK=3,PD_CONTEXT_SIGHT=1,PD_CONTEXT_FEAR=2 };
/* SIGHT: check requires sight. FEAR: frightening source is within line of sight. Unknown != no. */
typedef struct { uint8_t known,values; } pd_roll_context;
typedef struct { bool disadvantage,automatic_failure; uint8_t needed; } pd_condition_result;
typedef struct { pd_check_info info; int mode; bool allowed,automatic_failure; uint8_t needed; } pd_check_resolution;
bool pd_condition_evaluate(const pd_state *s,unsigned scope,unsigned ability,pd_roll_context context,bool ignore,pd_condition_result *out);
bool pd_check_resolve(const pd_state *s,pd_check_kind kind,unsigned index,int requested,bool ignore,pd_roll_context context,pd_check_resolution *out);
/* Rejects blocked/unknown/automatic failures before RNG; outputs untouched. */
bool pd_check_roll(const pd_state *s,pd_check_kind kind,unsigned index,int requested,bool ignore,pd_roll_context context,pd_random random,void *random_context,pd_roll *out);
int pd_proficiency(unsigned level);
/* Unknown legacy training never becomes a fabricated untrained skill/save. */
bool pd_describe_check(const pd_state *s,pd_check_kind kind,unsigned index,pd_check_info *out);
int pd_check_mode(const pd_state *s,pd_check_kind kind,unsigned index,int requested,bool ignore_effects);
#endif
