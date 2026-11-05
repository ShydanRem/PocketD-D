#include "spell_roll.h"
bool pd_spell_formula_resolve(const pd_state *s,unsigned index,unsigned slot,bool pact,pd_spell_roll_plan *out){
    if(!s||!out||!pd_valid(s)||!s->magic.available||index>=s->magic.count)return false;
    const pd_power *p=&s->magic.powers[index];pd_spell_formula f=s->formulas[index];
    if(!f.kind)return false;
    if(p->level&&(p->flags&PD_USES_SLOT)){if(slot<p->level||slot>9)return false;}
    else if(slot||pact)return false;
    unsigned level=slot?slot:p->level;
    unsigned extra=f.scaling==1?level-p->level:f.scaling==2?(unsigned)((s->level>=5)+(s->level>=11)+(s->level>=17)):0u;
    unsigned count=f.count+f.step*extra;int bonus=f.bonus+(f.ability?pd_modifier(s->abilities[p->ability]):0);
    if(count>PD_POOL_MAX||bonus<PD_POOL_MIN_BONUS||bonus>PD_POOL_MAX_BONUS)return false;
    *out=(pd_spell_roll_plan){.power=index,.slot=slot,.pact=pact,.kind=f.kind==1?PD_SPELL_DAMAGE:PD_SPELL_HEAL_OTHER,.count=count,.sides=f.sides,.bonus=bonus};return true;
}
bool pd_spell_roll_prepare(const pd_state *s,const pd_spell_roll_plan *plan,pd_state *next){
    if(!s||!plan||!next||next==s||!pd_valid(s)||!s->magic.available||plan->power>=s->magic.count||plan->kind<PD_SPELL_DAMAGE||plan->kind>PD_SPELL_HEAL_OTHER||plan->count<1||plan->count>PD_POOL_MAX||!pd_pool_sides(plan->sides)||plan->bonus<PD_POOL_MIN_BONUS||plan->bonus>PD_POOL_MAX_BONUS)return false;
    const pd_power *p=&s->magic.powers[plan->power];
    if(p->kind||p->effect)return false;
    if(!(p->level&&(p->flags&PD_USES_SLOT))&&(plan->slot||plan->pact))return false;
    /* Do not allocate another large state on an ESP32 task stack. All possible
       failures precede writing the caller's candidate. */
    if(!s->hp||s->combat.raging||!(p->flags&PD_PREPARED)||((p->flags&PD_LIMITED)&&!p->remaining))return false;
    if(p->level&&(p->flags&PD_USES_SLOT)&&(plan->slot<p->level||plan->slot>9||!(plan->pact?s->magic.pact:s->magic.slots)[plan->slot-1].current))return false;
    if(plan->kind==PD_SPELL_HEAL_SELF&&s->hp==pd_max_hp(s))return false;
    *next=*s;
    return pd_magic_use(next,plan->power,plan->slot,plan->pact);
}
bool pd_spell_roll(const pd_state *s,const pd_spell_roll_plan *plan,pd_random random,void *context,pd_state *next,pd_pool *out,unsigned *gained){
    if(!next||!out||!gained||!random||next==s)return false;
    /* Prepare is also the UI's read-only preview. Output state is written only
       after validation, before the one infallible validated pool draw. */
    if(!pd_spell_roll_prepare(s,plan,next))return false;
    pd_pool roll;
    if(!pd_roll_pool(plan->sides,plan->count,plan->bonus,true,random,context,&roll))return false;
    unsigned old=next->hp;
    if(plan->kind==PD_SPELL_HEAL_SELF)pd_hp(next,roll.total);
    *gained=next->hp-old;*out=roll;return true;
}
