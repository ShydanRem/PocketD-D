#ifndef PD_REST_H
#define PD_REST_H
#include "dice_pool.h"
/* Explicit recovery policy; no inference of a character's rules edition. */
enum { PD_HD_MANUAL, PD_HD_2014, PD_HD_2024 };
typedef struct { bool long_rest; uint8_t policy,regain[PD_HIT_POOLS]; } pd_rest_plan;
typedef struct { unsigned hp,ordinary,pact,powers,hit_dice; bool clears_temp,ends_rage,ends_concentration; } pd_rest_summary;
unsigned pd_hit_budget(const pd_state *s,unsigned policy);
/* Validate the entire plan before any mutation; generic counters stay manual. */
bool pd_rest_apply(pd_state *s,const pd_rest_plan *plan);
bool pd_rest_preview(const pd_state *s,const pd_rest_plan *plan,pd_state *scratch,pd_rest_summary *out);
/* One basic hit die + CON, minimum zero. No entropy at full/zero HP or exhaustion.
   Caller must establish a short rest and commit next before displaying the roll. */
bool pd_spend_hit_die(const pd_state *s,unsigned index,pd_random random,void *context,pd_state *next,pd_pool *roll,unsigned *gained);
#endif
