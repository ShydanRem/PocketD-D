#include "rest.h"
#include "magic.h"
#include <string.h>
unsigned pd_hit_budget(const pd_state *s,unsigned policy) {
    if(!s||!s->recovery.available||s->recovery.count>PD_HIT_POOLS||policy==PD_HD_MANUAL||policy>PD_HD_2024)return 0;
    unsigned maximum=0,used=0;
    for(unsigned i=0;i<s->recovery.count;i++){maximum+=s->recovery.pools[i].maximum;used+=s->recovery.pools[i].used;}
    unsigned budget=policy==PD_HD_2024?used:maximum/2;
    if(policy==PD_HD_2014&&maximum&&!budget)budget=1;
    return budget<used?budget:used;
}
bool pd_rest_apply(pd_state *s,const pd_rest_plan *plan) {
    if(!s||!plan||!pd_valid(s)||plan->policy>PD_HD_2024||(!plan->long_rest&&plan->policy!=PD_HD_MANUAL))return false;
    if(plan->long_rest&&!s->hp&&s->status.available&&s->status.failures==3)return false;
    unsigned regain=0;
    for(unsigned i=0;i<PD_HIT_POOLS;i++) {
        unsigned selected=plan->regain[i];
        if((i>=s->recovery.count||!s->recovery.available||!plan->long_rest||plan->policy!=PD_HD_2014)?selected!=0:selected>s->recovery.pools[i].used)return false;
        regain+=selected;
    }
    if(regain>pd_hit_budget(s,plan->policy))return false;
    if(plan->long_rest){pd_hp(s,s->max_hp);s->temp_hp=0;s->magic.concentration=0;s->status.successes=0;s->status.failures=0;s->status.stabilized=false;}
    pd_end_rage(s);s->combat.power_attack=false;
    if(s->magic.available) {
        for(unsigned i=0;i<9;i++) {
            s->magic.pact[i].current=s->magic.pact[i].maximum;
            if(plan->long_rest)s->magic.slots[i].current=s->magic.slots[i].maximum;
        }
        for(unsigned i=0;i<s->magic.count;i++) {
            pd_power *p=&s->magic.powers[i];
            if(!(p->flags&PD_PASSIVE)&&(p->kind||(p->flags&PD_LIMITED))&&(p->recovery==1||(plan->long_rest&&p->recovery==2)))p->remaining=p->maximum;
        }
    }
    if(plan->long_rest&&s->recovery.available)for(unsigned i=0;i<s->recovery.count;i++) {
        pd_hit_die *h=&s->recovery.pools[i];
        if(plan->policy==PD_HD_2024)h->used=0;
        else if(plan->policy==PD_HD_2014)h->used-=plan->regain[i];
    }
    return true;
}
bool pd_rest_preview(const pd_state *s,const pd_rest_plan *plan,pd_state *scratch,pd_rest_summary *out) {
    if(!s||!scratch||s==scratch||!out)return false;
    *scratch=*s;if(!pd_rest_apply(scratch,plan))return false;
    pd_rest_summary result={.hp=scratch->hp-s->hp,.clears_temp=s->temp_hp>scratch->temp_hp,
        .ends_rage=s->combat.raging&&!scratch->combat.raging,.ends_concentration=s->magic.concentration&&!scratch->magic.concentration};
    for(unsigned i=0;i<9;i++){result.ordinary+=scratch->magic.slots[i].current-s->magic.slots[i].current;result.pact+=scratch->magic.pact[i].current-s->magic.pact[i].current;}
    for(unsigned i=0;i<s->magic.count;i++)if(scratch->magic.powers[i].remaining>s->magic.powers[i].remaining)result.powers++;
    for(unsigned i=0;i<s->recovery.count;i++)result.hit_dice+=s->recovery.pools[i].used-scratch->recovery.pools[i].used;
    *out=result;return true;
}
bool pd_spend_hit_die(const pd_state *s,unsigned index,pd_random random,void *context,pd_state *next,pd_pool *roll,unsigned *gained) {
    if(!s||!next||s==next||!roll||!gained||!random||!pd_valid(s)||!s->recovery.available||index>=s->recovery.count||!s->hp||s->hp==s->max_hp)return false;
    const pd_hit_die *h=&s->recovery.pools[index];if(h->used==h->maximum)return false;
    pd_pool result;if(!pd_roll_pool(h->die,1,pd_modifier(s->abilities[2]),true,random,context,&result))return false;
    *next=*s;next->recovery.pools[index].used++;pd_hp(next,result.total);
    *roll=result;*gained=next->hp-s->hp;return true;
}
