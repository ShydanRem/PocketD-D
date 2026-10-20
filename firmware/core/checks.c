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
    pd_check_info r={.ability=kind==PD_SKILL?skill_ability[index]:index};
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
int pd_check_mode(const pd_state *s,pd_check_kind kind,unsigned index,int requested,bool ignore_effects) {
    pd_check_info r;
    if(!pd_describe_check(s,kind,index,&r))return 0;
    bool automatic=s->training.available&&!ignore_effects;
    bool advantage=requested>0||(automatic&&s->training.strength_advantage&&r.ability==0);
    bool disadvantage=requested<0||(!ignore_effects&&s->training.poisoned&&kind!=PD_SAVE);
    return advantage==disadvantage?0:advantage?1:-1;
}
