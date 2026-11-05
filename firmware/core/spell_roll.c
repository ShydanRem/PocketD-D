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
bool pd_spell_attack_gate(const pd_state *s,int requested,pd_roll_context context,int *mode,unsigned *needed){
    pd_condition_result effect;
    if(!s||!mode||!needed||requested< -1||requested>1||!pd_condition_evaluate(s,PD_ROLL_ATTACK,0,context,false,&effect))return false;
    bool adv=requested>0,dis=requested<0||s->training.poisoned||effect.disadvantage;
    *mode=adv==dis?0:adv?1:-1;*needed=dis?effect.needed&~PD_CONTEXT_FEAR:effect.needed;return true;
}
bool pd_spell_battle_prepare(const pd_state *s,const pd_spell_roll_plan *p,const pd_spell_battle_plan *b,pd_state *next){
    if(!s||!p||!b||!next||next==s||p->kind!=PD_SPELL_DAMAGE)return false;
    int attack,dc,mode;unsigned needed;
    if(!pd_valid(s)||!pd_magic_stats(s,p->power,&attack,&dc))return false;
    if(b->mode==PD_SPELL_ATTACK){
        if(b->armor<1||b->armor>40||b->success||b->save_ability||!pd_spell_attack_gate(s,b->requested,b->context,&mode,&needed)||needed)return false;
    }else if(b->mode==PD_SPELL_TARGET_SAVE){
        if(b->armor||b->requested||b->context.known||b->context.values||b->save_ability>5||b->success<PD_SPELL_SAVE_ZERO||b->success>PD_SPELL_SAVE_HALF)return false;
    }else return false;
    return pd_spell_roll_prepare(s,p,next);
}
bool pd_spell_attack_cast(const pd_state *s,const pd_spell_roll_plan *p,const pd_spell_battle_plan *b,pd_random random,void *context,pd_state *next,pd_spell_attack_result *out){
    if(!random||!out||!b||b->mode!=PD_SPELL_ATTACK||!pd_spell_battle_prepare(s,p,b,next))return false;
    int attack,dc,mode;unsigned needed;pd_magic_stats(s,p->power,&attack,&dc);pd_spell_attack_gate(s,b->requested,b->context,&mode,&needed);
    pd_spell_attack_result r={.mode=mode};r.roll=pd_d20(attack,mode,random,context);r.critical=r.roll.natural==20;r.hits=r.roll.natural!=1&&(r.critical||r.roll.total>=(int)b->armor);*out=r;return true;
}
bool pd_spell_impact_roll(const pd_spell_roll_plan *p,bool critical,unsigned impact,pd_random random,void *context,pd_spell_impact *out){
    if(!p||!out||p->kind!=PD_SPELL_DAMAGE||p->count<1||p->count>PD_POOL_MAX||!pd_pool_sides(p->sides)||p->bonus<PD_POOL_MIN_BONUS||p->bonus>PD_POOL_MAX_BONUS||impact>PD_SPELL_ZERO||(critical&&impact!=PD_SPELL_FULL)||(impact!=PD_SPELL_ZERO&&!random))return false;
    pd_spell_impact r={.impact=impact,.critical=critical};
    if(impact!=PD_SPELL_ZERO){
        unsigned count=p->count*(critical?2:1),first=count>PD_POOL_MAX?PD_POOL_MAX:count;
        if(!pd_roll_pool(p->sides,first,p->bonus,true,random,context,&r.first))return false;
        if(count>first&&!pd_roll_pool(p->sides,count-first,0,true,random,context,&r.second))return false;
        /* A split first pool has 18 dice: even its minimum minus 15 is positive.
           Flooring the first pool therefore also floors the complete sum. */
        r.rolled=r.first.total+r.second.total;r.applicable=impact==PD_SPELL_HALF?r.rolled/2:r.rolled;
    }
    *out=r;return true;
}
