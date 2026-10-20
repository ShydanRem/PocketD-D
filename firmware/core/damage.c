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
/* Evaluate the 16 possible source descriptions compatible with known facts.
   This also resolves complementary grants (magical OR nonmagical) without
   asking for a fact that cannot change the applied property. */
static bool rule_applies(unsigned rule,unsigned values) {
    static const unsigned masks[]={0,1,1,3,7,11,15},wanted[]={0,1,0,2,2,2,2};
    return (values&masks[rule])==wanted[rule];
}
static void resolve(const pd_state *s,unsigned type,pd_damage_context context,unsigned *flags,unsigned *unresolved,unsigned *needed) {
    *flags=*unresolved=*needed=0;
    if(!pd_defence_conditional(s,type)){*flags=pd_defence_flags(s,type);return;}
    unsigned base=pd_defence_base_flags(s,type),possible=0,truth[3]={0};
    for(unsigned values=0;values<16;values++)if((values&context.known)==context.values){
        possible|=1u<<values;
        for(unsigned i=0;i<3;i++){
            bool applies=(base&(1u<<i))&&rule_applies(s->defences.conditions[type][i],values);
            for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++){const pd_defence_temporary *d=pd_defence_grant(s,type,slot);applies|=d->active&&(d->flags&(1u<<i))&&rule_applies(d->conditions[i],values);}
            if(applies)truth[i]|=1u<<values;
        }
    }
    for(unsigned i=0;i<3;i++){if(truth[i]==possible)*flags|=1u<<i;else if(truth[i])*unresolved|=1u<<i;}
    /* Definite immunity makes uncertainty in resistance/vulnerability irrelevant. */
    if(*flags&PD_IMMUNITY){*unresolved=0;return;}
    for(unsigned i=0;i<3;i++)if(*unresolved&(1u<<i))for(unsigned bit=1;bit<=8;bit<<=1)if(!(context.known&bit)){
        for(unsigned values=0;values<16;values++)if((possible&(1u<<values))&&(((truth[i]>>values)^(truth[i]>>(values^bit)))&1u)){*needed|=bit;break;}
    }
}
/* -1 unknown, 0 excluded, 1 eligible. Needed bits are influential unknown facts. */
static int eligible(unsigned rule,pd_damage_context c,unsigned *needed){
    unsigned truth=0,possible=0;*needed=0;
    for(unsigned v=0;v<16;v++)if((v&c.known)==c.values){possible|=1u<<v;if(rule_applies(rule,v))truth|=1u<<v;}
    if(!truth)return 0;
    if(truth==possible)return 1;
    for(unsigned bit=1;bit<=8;bit<<=1)if(!(c.known&bit))for(unsigned v=0;v<16;v++)if((possible&(1u<<v))&&(((truth>>v)^(truth>>(v^bit)))&1u)){*needed|=bit;break;}
    return -1;
}
bool pd_damage_event_preview(const pd_state *s,const pd_damage_event *event,pd_damage_event_summary *out) {
    if(!s||!event||!out||!pd_valid(s)||!event->count||event->count>PD_DAMAGE_COMPONENTS||event->concentration>PD_CONCENTRATION_END||event->reduction_after>9999||event->profile>PD_REDUCTION_PROFILES)return false;
    pd_damage_event_summary report={0};const pd_reduction_profile *profile=event->profile?&s->reductions[event->profile-1]:NULL;
    if(profile){report.profile=event->profile;report.profile_count=event->count;report.profile_snapshot=*profile;report.profile_unavailable=!profile->active;
        for(unsigned i=0;i<event->count;i++){if(event->allocation[i]>9999)return false;report.profile_rows[i]=event->allocation[i];report.profile_requested+=event->allocation[i];}
        if(profile->stage){report.profile_budget=report.profile_requested!=0;report.profile_requested=profile->value;}
        else report.profile_budget=!report.profile_requested||report.profile_requested>profile->value;
    }else for(unsigned i=0;i<event->count;i++)if(event->allocation[i])return false;
    for(unsigned i=0;i<event->count;i++) {
        const pd_damage_component *c=&event->components[i];pd_damage_context context=event->contexts[i];
        if(c->type>=PD_DAMAGE_TYPES||c->amount>9999||event->reductions[i]>9999||context.known>15||(context.values&~context.known))return false;
        unsigned flags,unresolved,needed;resolve(s,c->type,context,&flags,&unresolved,&needed);
        bool configured=(s->defences.configured&(1u<<c->type))!=0;
        if(!configured)report.missing|=1u<<c->type;
        else if(unresolved){report.context_missing|=1u<<i;report.needed[i]=needed;}
        if(profile&&(profile->stage||event->allocation[i])){
            unsigned traits=0;int match=(profile->types&(1u<<c->type))?eligible(profile->rule,context,&traits):0;
            if(match==0)report.profile_ineligible|=1u<<i;
            else if(match<0){report.context_missing|=1u<<i;report.needed[i]|=traits;}
        }
        unsigned group=0;while(group<report.count&&(report.groups[group].type!=c->type||report.groups[group].flags!=flags||report.groups[group].unresolved!=unresolved))group++;
        if(group==report.count){pd_damage_group *g=&report.groups[report.count++];g->type=c->type;g->flags=flags;g->unresolved=unresolved;g->configured=configured;}
        unsigned reduction=event->reductions[i]<c->amount?event->reductions[i]:c->amount;
        if(profile&&!profile->stage){unsigned left=c->amount-reduction,used=event->allocation[i]<left?event->allocation[i]:left;report.profile_used[i]=used;report.profile_applied+=used;reduction+=used;}
        report.groups[group].amount+=c->amount;report.groups[group].reduction+=reduction;
        report.raw+=c->amount;report.requested_before+=event->reductions[i]+(profile&&!profile->stage?event->allocation[i]:0);report.reduction_before+=reduction;
    }
    /* Same type + applied properties share one rounding. Different applicability
       stays separate, including magical/nonmagical portions of a single impact. */
    for(unsigned i=1;i<report.count;i++){
        pd_damage_group g=report.groups[i];unsigned j=i;
        while(j&&(report.groups[j-1].type>g.type||(report.groups[j-1].type==g.type&&(report.groups[j-1].flags>g.flags||(report.groups[j-1].flags==g.flags&&report.groups[j-1].unresolved>g.unresolved))))){report.groups[j]=report.groups[j-1];j--;}
        report.groups[j]=g;
    }
    unsigned total=0;
    for(unsigned i=0;i<report.count;i++){pd_damage_group *g=&report.groups[i];if(g->configured&&!g->unresolved){g->adjusted=adjusted(g->amount-g->reduction,g->flags);total+=g->adjusted;}}
    pd_damage_summary *r=&report.total;r->configured=!report.missing&&!report.context_missing&&!report.profile_unavailable&&!report.profile_budget&&!report.profile_ineligible;r->hp_after=s->hp;r->temp_after=s->temp_hp;r->max_hp_after=pd_max_hp(s);
    if(report.count==1)r->flags=report.groups[0].flags;
    report.requested_after=event->reduction_after+(profile&&profile->stage?profile->value:0);
    if(!r->configured){report.profile_applied=0;for(unsigned i=0;i<PD_DAMAGE_COMPONENTS;i++)report.profile_used[i]=0;}
    if(r->configured){
        report.mitigated=total;report.reduction_after=event->reduction_after<total?event->reduction_after:total;total-=report.reduction_after;
        if(profile&&profile->stage){report.profile_applied=profile->value<total?profile->value:total;report.reduction_after+=report.profile_applied;total-=report.profile_applied;}
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
