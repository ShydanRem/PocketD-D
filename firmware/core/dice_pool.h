#ifndef PD_DICE_POOL_H
#define PD_DICE_POOL_H
#include "pocketdd.h"
#define PD_POOL_MAX 18
#define PD_POOL_MIN_BONUS (-15)
#define PD_POOL_MAX_BONUS 60
/* Transient roll data only: USB and NVS character formats do not change. */
typedef struct {
    uint8_t dice[PD_POOL_MAX],count,sides;
    int sum,bonus,total;
    bool damage;
} pd_pool;
bool pd_pool_sides(unsigned sides);
/* Validate before drawing entropy. Invalid input leaves output untouched. */
bool pd_roll_pool(unsigned sides,unsigned count,int bonus,bool damage,
                  pd_random random,void *context,pd_pool *out);
#endif
