#include "checks.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    pd_state s;pd_demo(&s);pd_check_info info;
    assert(pd_proficiency(0)==0&&pd_proficiency(21)==0);
    for(int level=1;level<=20;level++)for(int score=1;score<=30;score++)for(int rank=0;rank<=2;rank++) {
        s.level=level;s.abilities[1]=score;s.training.ranks[0]=rank;s.training.half_skills=1;
        for(int extra=-30;extra<=30;extra+=30) {
            s.training.skill_bonus[0]=extra;assert(pd_describe_check(&s,PD_SKILL,0,&info));
            int prof=2+(level-1)/4,trained=rank?rank*prof:prof/2;
            assert(info.base==pd_modifier(score)&&info.trained==trained&&info.total==info.base+trained+extra);
        }
    }
    s.level=20;s.abilities[1]=30;s.training.ranks[0]=2;s.training.skill_bonus[0]=30;
    assert(pd_describe_check(&s,PD_SKILL,0,&info)&&info.total==52);
    s.training.half_skills=0;s.abilities[1]=1;s.training.ranks[0]=0;s.training.skill_bonus[0]=-30;
    assert(pd_describe_check(&s,PD_SKILL,0,&info)&&info.total==-35);
    s.training.blocked=(1u<<0)|(1u<<6)|(1u<<24);
    for(int kind=0;kind<3;kind++)assert(pd_describe_check(&s,kind,0,&info)&&info.blocked);
    s.training.poisoned=true;s.training.strength_advantage=true;
    assert(pd_check_mode(&s,PD_CHECK,0,0,false)==0); /* Both sources cancel once. */
    assert(pd_check_mode(&s,PD_SAVE,0,0,false)==1); /* Poison never affects saves. */
    assert(pd_check_mode(&s,PD_CHECK,1,1,false)==0);
    assert(pd_check_mode(&s,PD_CHECK,1,-1,true)==-1);
    s.training.available=false;assert(!pd_describe_check(&s,PD_SKILL,0,&info));
    assert(!pd_describe_check(&s,PD_SAVE,0,&info));assert(pd_describe_check(&s,PD_CHECK,0,&info)&&!info.blocked&&!info.trained&&!info.extra);
    assert(!pd_describe_check(&s,PD_CHECK,6,&info));assert(!pd_describe_check(NULL,PD_CHECK,0,&info));
    puts("Checks: levels 1..20, scores 1..30, ranks 0..2, extras -30..30, extrema -35/+52, effects, blocks and legacy unknown training passed");
}
