#include "damage.h"
const char *pd_damage_name(unsigned type) {
    static const char *const names[]={"Contundente","Perforante","Tagliente","Acido","Freddo","Fuoco","Forza","Fulmine","Necrotico","Veleno","Psichico","Radiante","Tuono"};
    return type<PD_DAMAGE_TYPES?names[type]:NULL;
}
bool pd_damage_preview(const pd_state *s,const pd_damage_plan *plan,pd_damage_summary *out) {
    if(!s||!plan||!out||!pd_valid(s)||plan->type>=PD_DAMAGE_TYPES||plan->amount>9999||plan->concentration>PD_CONCENTRATION_END)return false;
    pd_damage_summary r={.configured=(s->defences.configured&(1u<<plan->type))!=0,.hp_after=s->hp,.temp_after=s->temp_hp,.max_hp_after=pd_max_hp(s)};
    r.flags=pd_defence_flags(s,plan->type);
    if(r.configured){
        r.adjusted=plan->amount;
        if(r.flags&PD_IMMUNITY)r.adjusted=0;
        else {if(r.flags&PD_RESISTANCE)r.adjusted/=2;if(r.flags&PD_VULNERABILITY)r.adjusted*=2;}
        r.absorbed=r.adjusted<s->temp_hp?r.adjusted:s->temp_hp;r.temp_after-=r.absorbed;
        unsigned remainder=r.adjusted-r.absorbed;r.hp_after=remainder<s->hp?s->hp-remainder:0;
        r.at_zero=!s->hp&&r.adjusted>0;
        r.massive=s->hp>0&&remainder>=s->hp&&remainder-s->hp>=pd_max_hp(s);
        r.needs_concentration=s->magic.concentration&&r.adjusted>0&&r.hp_after>0;
        if(r.needs_concentration){r.concentration_dc=r.adjusted/2;if(r.concentration_dc<10)r.concentration_dc=10;}
        r.ends_concentration=s->magic.concentration&&r.adjusted>0&&(!r.hp_after||(r.needs_concentration&&plan->concentration==PD_CONCENTRATION_END));
        if(r.ends_concentration){r.ended_effects=pd_concentration_effects(s);r.max_hp_after=pd_unconcentrated_max_hp(s);if(r.hp_after>r.max_hp_after)r.hp_after=r.max_hp_after;}
        r.allowed=!r.at_zero&&!r.massive&&(!r.needs_concentration||plan->concentration!=PD_CONCENTRATION_UNRESOLVED);
    }
    *out=r;return true;
}
bool pd_damage_apply(pd_state *s,const pd_damage_plan *plan,pd_damage_summary *out) {
    pd_damage_summary r;if(!out||!pd_damage_preview(s,plan,&r)||!r.allowed)return false;
    if(r.adjusted)pd_hp(s,-(int)r.adjusted);
    if(r.ends_concentration)pd_concentration_end(s);
    *out=r;return true;
}
bool pd_concentration_info(const pd_state *s,const pd_damage_plan *plan,int mode,int extra,pd_check_info *out,pd_damage_summary *damage,bool cap30) {
    pd_damage_summary r;pd_check_info info;
    if(!out||!damage||mode< -1||mode>1||extra< -30||extra>30||!pd_damage_preview(s,plan,&r)||
       plan->concentration!=PD_CONCENTRATION_UNRESOLVED||!r.needs_concentration||r.at_zero||r.massive||
       !pd_describe_check(s,PD_SAVE,2,&info)||info.blocked)return false;
    if(cap30&&r.concentration_dc>30)r.concentration_dc=30;
    *out=info;*damage=r;return true;
}
bool pd_concentration_roll(const pd_state *s,const pd_damage_plan *plan,int mode,int extra,pd_random random,void *context,pd_state *next,pd_roll *roll,pd_damage_summary *damage,bool cap30) {
    pd_check_info info;pd_damage_summary r;
    if(!next||s==next||!roll||!random||!pd_concentration_info(s,plan,mode,extra,&info,&r,cap30)||!damage)return false;
    pd_roll result=pd_d20(info.total+extra,pd_check_mode(s,PD_SAVE,2,mode,false),random,context);
    pd_damage_plan resolved=*plan;resolved.concentration=result.total>=(int)r.concentration_dc?PD_CONCENTRATION_KEEP:PD_CONCENTRATION_END;
    unsigned dc=r.concentration_dc;*next=*s;if(!pd_damage_apply(next,&resolved,&r))return false;
    r.concentration_dc=dc;
    *roll=result;*damage=r;return true;
}
