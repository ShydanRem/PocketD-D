#include "checks.h"
const char *const pd_abilities[]={"FOR","DES","COS","INT","SAG","CAR"};
const char *const pd_skill_ids[]={"acrobatics","animalHandling","arcana","athletics","deception","history","insight","intimidation","investigation","medicine","nature","perception","performance","persuasion","religion","sleightOfHand","stealth","survival"};
const char *const pd_skill_names[]={"Acrobazia","Addestrare animali","Arcano","Atletica","Inganno","Storia","Intuizione","Intimidire","Indagare","Medicina","Natura","Percezione","Intrattenere","Persuasione","Religione","Rapidita di mano","Furtivita","Sopravvivenza"};
static const uint8_t skill_ability[]={1,4,3,0,5,3,4,5,3,4,3,4,5,5,3,1,1,4};
int pd_proficiency(unsigned level) { return level>=1&&level<=20?2+(level-1)/4:0; }
bool pd_describe_check(const pd_state *s,pd_check_kind kind,unsigned index,pd_check_info *out) {
    if(!s||!out||kind<PD_CHECK||kind>PD_SAVE||index>=(kind==PD_SKILL?PD_SKILLS:6)||
       (kind!=PD_CHECK&&!s->training.available))return false;
    const pd_training *t=&s->training;
    if(kind==PD_SKILL&&t->skill_abilities[index]>6)return false;
    pd_check_info r={.ability=kind==PD_SKILL?(t->available&&t->skill_abilities[index]?(unsigned)t->skill_abilities[index]-1u:skill_ability[index]):index};
    r.base=pd_modifier(s->abilities[r.ability]);
    if(t->available) {
        r.rank=kind==PD_SKILL?t->ranks[index]:kind==PD_SAVE?((t->saves>>index)&1):0;
        r.half=kind==PD_CHECK?t->half_checks:kind==PD_SKILL&&!r.rank&&((t->half_skills>>index)&1);
        r.trained=r.rank*pd_proficiency(s->level)+(r.half?pd_proficiency(s->level)/2:0);
        r.extra=kind==PD_SKILL?t->skill_bonus[index]:kind==PD_SAVE?t->save_bonus[index]:t->check_bonus[index];
        r.blocked=(t->blocked>>(kind==PD_CHECK?index:kind==PD_SKILL?6+index:24+index))&1;
    }
    r.total=r.base+r.trained+r.extra;*out=r;return true;
}
bool pd_condition_evaluate(const pd_state *s,unsigned scope,unsigned ability,pd_roll_context context,bool ignore,pd_condition_result *out){
    if(!s||!out||scope>PD_ROLL_ATTACK||ability>5||s->roll_conditions>PD_ROLL_CONDITIONS||context.known>3||(context.values&~context.known))return false;
    pd_condition_result r={0};unsigned flags=ignore?0:s->roll_conditions;
    if(scope==PD_ROLL_ATTACK)r.disadvantage=(flags&(PD_BLINDED|PD_PRONE|PD_RESTRAINED))!=0;
    else if(scope==PD_SAVE)r.disadvantage=(flags&PD_RESTRAINED)&&ability==1;
    else if(flags&PD_BLINDED){if(!(context.known&PD_CONTEXT_SIGHT))r.needed|=PD_CONTEXT_SIGHT;else r.automatic_failure=(context.values&PD_CONTEXT_SIGHT)!=0;}
    if(scope!=PD_SAVE&&(flags&PD_FRIGHTENED)&&!r.automatic_failure&&!r.disadvantage){if(!(context.known&PD_CONTEXT_FEAR))r.needed|=PD_CONTEXT_FEAR;else r.disadvantage|=(context.values&PD_CONTEXT_FEAR)!=0;}
    *out=r;return true;
}
bool pd_check_resolve(const pd_state *s,pd_check_kind kind,unsigned index,int requested,bool ignore,pd_roll_context context,pd_check_resolution *out){
    pd_check_resolution r={0};pd_condition_result effect;
    if(!out||requested< -1||requested>1||!pd_describe_check(s,kind,index,&r.info)||!pd_condition_evaluate(s,kind,r.info.ability,context,ignore,&effect))return false;
    bool advantage=requested>0||(!ignore&&s->training.available&&s->training.strength_advantage&&r.info.ability==0);
    bool disadvantage=requested<0||(!ignore&&s->training.poisoned&&kind!=PD_SAVE)||effect.disadvantage;
    r.mode=advantage==disadvantage?0:advantage?1:-1;r.needed=disadvantage?effect.needed&~PD_CONTEXT_FEAR:effect.needed;r.automatic_failure=effect.automatic_failure;r.allowed=!r.info.blocked&&!r.needed&&!r.automatic_failure;*out=r;return true;
}
int pd_check_mode(const pd_state *s,pd_check_kind kind,unsigned index,int requested,bool ignore_effects){
    pd_check_resolution r;return pd_check_resolve(s,kind,index,requested,ignore_effects,(pd_roll_context){0},&r)?r.mode:0;
}
bool pd_check_roll(const pd_state *s,pd_check_kind kind,unsigned index,int requested,bool ignore,pd_roll_context context,pd_random random,void *random_context,pd_roll *out){
    pd_check_resolution r;if(!out||!random||!pd_check_resolve(s,kind,index,requested,ignore,context,&r)||!r.allowed)return false;
    *out=pd_d20(r.info.total,r.mode,random,random_context);return true;
}
