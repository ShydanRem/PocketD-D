#include "recharge.h"
bool pd_item_recharge(const pd_state *s,unsigned index,const pd_recharge_plan *plan,
                      pd_random random,void *context,pd_state *next,pd_pool *roll,unsigned *gained){
    if(!s||!next||s==next||!plan||!roll||!gained||!random||!pd_valid(s)||!s->inventory_available||index>=s->item_count||
       !plan->count||plan->count>PD_RECHARGE_MAX_DICE||!pd_pool_sides(plan->sides)||plan->bonus<PD_POOL_MIN_BONUS||plan->bonus>PD_POOL_MAX_BONUS)return false;
    const pd_item *item=&s->items[index];const pd_charges *c=&item->charges;
    if(!item->quantity||!c->maximum||c->current>=c->maximum)return false;
    pd_pool result;if(!pd_roll_pool(plan->sides,plan->count,plan->bonus,true,random,context,&result))return false;
    unsigned space=c->maximum-c->current,recovered=(unsigned)result.total<space?(unsigned)result.total:space;
    *next=*s;next->items[index].charges.current+=recovered;*roll=result;*gained=recovered;return true;
}
