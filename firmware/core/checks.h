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
int pd_proficiency(unsigned level);
/* Unknown legacy training never becomes a fabricated untrained skill/save. */
bool pd_describe_check(const pd_state *s,pd_check_kind kind,unsigned index,pd_check_info *out);
int pd_check_mode(const pd_state *s,pd_check_kind kind,unsigned index,int requested,bool ignore_effects);
#endif
