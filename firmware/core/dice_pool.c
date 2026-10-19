#include "dice_pool.h"
bool pd_pool_sides(unsigned sides) {
    return sides==4 || sides==6 || sides==8 || sides==10 || sides==12 || sides==20;
}
bool pd_roll_pool(unsigned sides,unsigned count,int bonus,bool damage,
                  pd_random random,void *context,pd_pool *out) {
    if(!out || !random || !pd_pool_sides(sides) || count<1 || count>PD_POOL_MAX ||
       bonus<PD_POOL_MIN_BONUS || bonus>PD_POOL_MAX_BONUS) return false;
    pd_pool r={.count=count,.sides=sides,.bonus=bonus,.damage=damage};
    /* 2^32 is intentional for power-of-two dice; do not truncate this limit to uint32. */
    uint64_t limit=(UINT64_C(4294967296)/sides)*sides;
    for(unsigned i=0;i<count;i++) {
        uint32_t n;do { n=random(context); } while((uint64_t)n>=limit);
        r.dice[i]=n%sides+1;r.sum+=r.dice[i];
    }
    r.total=r.sum+bonus;
    if(damage && r.total<0)r.total=0;
    *out=r;return true;
}
