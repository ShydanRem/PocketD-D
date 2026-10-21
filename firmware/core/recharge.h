#ifndef PD_RECHARGE_H
#define PD_RECHARGE_H
#include "dice_pool.h"
#define PD_RECHARGE_MAX_DICE 6
/* One explicitly authorized event. No inference of dawn, rest or item rules. */
typedef struct { unsigned count,sides; int bonus; } pd_recharge_plan;
/* Produces one candidate; caller saves before presenting the roll.
   Floor recovery at zero, cap at the pool maximum; quantity/HP/effects unchanged.
   No RNG at full balance, absent item/pool or invalid input. Rejection preserves outputs. */
bool pd_item_recharge(const pd_state *s,unsigned index,const pd_recharge_plan *plan,
                      pd_random random,void *context,pd_state *next,pd_pool *roll,unsigned *gained);
#endif
