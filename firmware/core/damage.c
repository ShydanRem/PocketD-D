#include "damage.h"
const char *pd_damage_name(unsigned type) {
    static const char *const names[]={"Contundente","Perforante","Tagliente","Acido","Freddo","Fuoco","Forza","Fulmine","Necrotico","Veleno","Psichico","Radiante","Tuono"};
    return type<PD_DAMAGE_TYPES?names[type]:NULL;
}
bool pd_damage_preview(const pd_state *s,const pd_damage_plan *plan,pd_damage_summary *out) {
    if(!s||!plan||!out||!pd_valid(s)||plan->type>=PD_DAMAGE_TYPES||plan->amount>9999||plan->concentration>PD_CONCENTRATION_END)return false;
    pd_damage_summary r={.configured=(s->defences.configured&(1u<<plan->type))!=0,.hp_after=s->hp,.temp_after=s->temp_hp};
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
        r.allowed=!r.at_zero&&!r.massive&&(!r.needs_concentration||plan->concentration!=PD_CONCENTRATION_UNRESOLVED);
    }
    *out=r;return true;
}
bool pd_damage_apply(pd_state *s,const pd_damage_plan *plan,pd_damage_summary *out) {
    pd_damage_summary r;if(!out||!pd_damage_preview(s,plan,&r)||!r.allowed)return false;
    if(r.adjusted)pd_hp(s,-(int)r.adjusted);
    if(r.ends_concentration)s->magic.concentration=0;
    *out=r;return true;
}
