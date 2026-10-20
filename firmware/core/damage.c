#include "damage.h"
const char *pd_damage_name(unsigned type) {
    static const char *const names[]={"Contundente","Perforante","Tagliente","Acido","Freddo","Fuoco","Forza","Fulmine","Necrotico","Veleno","Psichico","Radiante","Tuono"};
    return type<PD_DAMAGE_TYPES?names[type]:NULL;
}
static unsigned adjusted(unsigned amount,unsigned flags) {
    if(flags&PD_IMMUNITY)return 0;
    if(flags&PD_RESISTANCE)amount/=2;
    if(flags&PD_VULNERABILITY)amount*=2;
    return amount;
}
bool pd_damage_event_preview(const pd_state *s,const pd_damage_event *event,pd_damage_event_summary *out) {
    if(!s||!event||!out||!pd_valid(s)||!event->count||event->count>PD_DAMAGE_COMPONENTS||event->concentration>PD_CONCENTRATION_END)return false;
    unsigned amounts[PD_DAMAGE_TYPES]={0},present=0;
    pd_damage_event_summary report={0};
    for(unsigned i=0;i<event->count;i++) {
        const pd_damage_component *c=&event->components[i];if(c->type>=PD_DAMAGE_TYPES||c->amount>9999)return false;
        amounts[c->type]+=c->amount;present|=1u<<c->type;report.raw+=c->amount;
    }
    unsigned total=0;
    /* Canonical grouping prevents row order/splitting from changing rounding.
       All defences are read from the state before this single damage event. */
    for(unsigned type=0;type<PD_DAMAGE_TYPES;type++)if(present&(1u<<type)) {
        pd_damage_group *g=&report.groups[report.count++];
        g->type=type;g->amount=amounts[type];g->flags=pd_defence_flags(s,type);g->configured=(s->defences.configured&(1u<<type))!=0;
        if(g->configured){g->adjusted=adjusted(g->amount,g->flags);total+=g->adjusted;}
        else report.missing|=1u<<type;
    }
    pd_damage_summary *r=&report.total;r->configured=!report.missing;r->hp_after=s->hp;r->temp_after=s->temp_hp;r->max_hp_after=pd_max_hp(s);
    if(report.count==1)r->flags=report.groups[0].flags;
    if(r->configured){
        r->adjusted=total;r->absorbed=total<s->temp_hp?total:s->temp_hp;r->temp_after-=r->absorbed;
        unsigned remainder=total-r->absorbed;r->hp_after=remainder<s->hp?s->hp-remainder:0;
        r->at_zero=!s->hp&&total>0;
        r->massive=s->hp>0&&remainder>=s->hp&&remainder-s->hp>=pd_max_hp(s);
        r->needs_concentration=s->magic.concentration&&total>0&&r->hp_after>0;
        if(r->needs_concentration){r->concentration_dc=total/2;if(r->concentration_dc<10)r->concentration_dc=10;}
        r->ends_concentration=s->magic.concentration&&total>0&&(!r->hp_after||(r->needs_concentration&&event->concentration==PD_CONCENTRATION_END));
        if(r->ends_concentration){r->ended_effects=pd_concentration_effects(s);r->max_hp_after=pd_unconcentrated_max_hp(s);if(r->hp_after>r->max_hp_after)r->hp_after=r->max_hp_after;}
        r->allowed=!r->at_zero&&!r->massive&&(!r->needs_concentration||event->concentration!=PD_CONCENTRATION_UNRESOLVED);
    }
    *out=report;return true;
}
bool pd_damage_event_apply(pd_state *s,const pd_damage_event *event,pd_damage_event_summary *out) {
    pd_damage_event_summary report;if(!out||!pd_damage_event_preview(s,event,&report)||!report.total.allowed)return false;
    if(report.total.adjusted)pd_hp(s,-(int)report.total.adjusted);
    if(report.total.ends_concentration)pd_concentration_end(s);
    *out=report;return true;
}
bool pd_concentration_event_info(const pd_state *s,const pd_damage_event *event,int mode,int extra,pd_check_info *out,pd_damage_event_summary *damage,bool cap30) {
    pd_damage_event_summary report;pd_check_info info;
    if(!out||!damage||mode< -1||mode>1||extra< -30||extra>30||!pd_damage_event_preview(s,event,&report)||
       event->concentration!=PD_CONCENTRATION_UNRESOLVED||!report.total.needs_concentration||report.total.at_zero||report.total.massive||
       !pd_describe_check(s,PD_SAVE,2,&info)||info.blocked)return false;
    if(cap30&&report.total.concentration_dc>30)report.total.concentration_dc=30;
    *out=info;*damage=report;return true;
}
bool pd_concentration_event_roll(const pd_state *s,const pd_damage_event *event,int mode,int extra,pd_random random,void *context,pd_state *next,pd_roll *roll,pd_damage_event_summary *damage,bool cap30) {
    pd_check_info info;pd_damage_event_summary report;
    if(!next||s==next||!roll||!random||!damage||!pd_concentration_event_info(s,event,mode,extra,&info,&report,cap30))return false;
    pd_roll result=pd_d20(info.total+extra,pd_check_mode(s,PD_SAVE,2,mode,false),random,context);
    pd_damage_event resolved=*event;resolved.concentration=result.total>=(int)report.total.concentration_dc?PD_CONCENTRATION_KEEP:PD_CONCENTRATION_END;
    unsigned dc=report.total.concentration_dc;*next=*s;if(!pd_damage_event_apply(next,&resolved,&report))return false;
    report.total.concentration_dc=dc;*roll=result;*damage=report;return true;
}
/* Single-component API is retained and uses the same event calculation. */
static pd_damage_event single(const pd_damage_plan *p){return (pd_damage_event){.count=1,.concentration=p->concentration,.components={{p->type,p->amount}}};}
bool pd_damage_preview(const pd_state *s,const pd_damage_plan *p,pd_damage_summary *out) {
    if(!p||!out)return false;
    pd_damage_event event=single(p);pd_damage_event_summary report;if(!pd_damage_event_preview(s,&event,&report))return false;
    *out=report.total;return true;
}
bool pd_damage_apply(pd_state *s,const pd_damage_plan *p,pd_damage_summary *out) {
    if(!p||!out)return false;
    pd_damage_event event=single(p);pd_damage_event_summary report;if(!pd_damage_event_apply(s,&event,&report))return false;
    *out=report.total;return true;
}
bool pd_concentration_info(const pd_state *s,const pd_damage_plan *p,int mode,int extra,pd_check_info *out,pd_damage_summary *damage,bool cap30) {
    if(!p||!damage)return false;
    pd_damage_event event=single(p);pd_damage_event_summary report;if(!pd_concentration_event_info(s,&event,mode,extra,out,&report,cap30))return false;
    *damage=report.total;return true;
}
bool pd_concentration_roll(const pd_state *s,const pd_damage_plan *p,int mode,int extra,pd_random random,void *context,pd_state *next,pd_roll *roll,pd_damage_summary *damage,bool cap30) {
    if(!p||!damage)return false;
    pd_damage_event event=single(p);pd_damage_event_summary report;if(!pd_concentration_event_roll(s,&event,mode,extra,random,context,next,roll,&report,cap30))return false;
    *damage=report.total;return true;
}
