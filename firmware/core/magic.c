#include "magic.h"
#include "checks.h"
#include <string.h>
static bool printable(const char *v,size_t cap,bool empty) {
    size_t n=0;while(n<cap&&v[n]){if((unsigned char)v[n]<32||(unsigned char)v[n]>126)return false;n++;}
    return n<cap&&(empty||n>0);
}
bool pd_magic_valid(const pd_state *s) {
    const pd_magic *m=&s->magic;
    if(m->count>PD_POWERS||m->concentration>m->count||(!m->available&&(m->count||m->concentration)))return false;
    for(int i=0;i<9;i++)if(m->slots[i].maximum>9||m->slots[i].current>m->slots[i].maximum||m->pact[i].maximum>9||m->pact[i].current>m->pact[i].maximum)return false;
    for(int i=0;i<m->count;i++) {
        const pd_power *v=&m->powers[i];
        if(!printable(v->name,sizeof v->name,false)||!printable(v->description,sizeof v->description,true)||v->kind>1||v->level>9||v->flags>63||v->ability>5||v->maximum>99||v->remaining>v->maximum||v->recovery>2||v->effect>2||v->attack_modifier< -15||v->attack_modifier>30||v->save_modifier< -15||v->save_modifier>30)return false;
        if(v->kind==0&&(v->effect||(v->flags&PD_PASSIVE)))return false;
        if(v->kind==1&&((v->flags&PD_PASSIVE)?(v->maximum||v->remaining||v->effect):!v->maximum))return false;
        if(v->kind==0&&(v->flags&PD_LIMITED)&&!v->maximum)return false;
    }
    if(m->concentration&&(!(m->powers[m->concentration-1].flags&PD_CONCENTRATES)||m->powers[m->concentration-1].kind||!s->hp||s->combat.raging))return false;
    return true;
}
bool pd_magic_stats(const pd_state *s,unsigned index,int *attack,int *dc) {
    if(!s||!attack||!dc||!s->magic.available||index>=s->magic.count||s->magic.powers[index].kind)return false;
    const pd_power *v=&s->magic.powers[index];int base=pd_modifier(s->abilities[v->ability])+pd_proficiency(s->level);
    *attack=base+v->attack_modifier;*dc=8+base+v->save_modifier;return true;
}
bool pd_magic_use(pd_state *s,unsigned index,unsigned slot,bool pact) {
    if(!s||!pd_valid(s)||!s->magic.available||index>=s->magic.count)return false;
    pd_power *v=&s->magic.powers[index];
    if(v->kind==1) {
        if((v->flags&PD_PASSIVE)||!v->remaining)return false;
        if(v->effect==1&&(!s->combat.rage_bonus||!s->hp||s->combat.raging))return false;
        if(v->effect==2&&(s->hp||(s->status.available&&s->status.failures==3)))return false;
        v->remaining--;
        if(v->effect==1){pd_concentration_end(s);s->combat.raging=true;s->training.strength_advantage=true;}
        if(v->effect==2)pd_hp(s,1);
        return true;
    }
    if(s->combat.raging||!(v->flags&PD_PREPARED)||((v->flags&PD_CONCENTRATES)&&!s->hp)||((v->flags&PD_LIMITED)&&!v->remaining))return false;
    pd_slot *cost=NULL;
    if(v->level&&(v->flags&PD_USES_SLOT)) {
        if(slot<v->level||slot>9)return false;
        cost=&(pact?s->magic.pact:s->magic.slots)[slot-1];if(!cost->current)return false;
    }
    if(cost)cost->current--;
    if(v->flags&PD_LIMITED)v->remaining--;
    if(v->flags&PD_CONCENTRATES){pd_concentration_end(s);s->magic.concentration=index+1;}
    return true;
}
bool pd_magic_recover(pd_state *s,unsigned index,int delta) {
    if(!s||!s->magic.available||index>=s->magic.count)return false;
    pd_power *v=&s->magic.powers[index];if((v->flags&PD_PASSIVE)||(!v->kind&&!(v->flags&PD_LIMITED)))return false;
    int64_t n=(int64_t)v->remaining+delta;v->remaining=n<0?0:n>v->maximum?v->maximum:n;return true;
}
bool pd_slot_change(pd_state *s,unsigned level,bool pact,int delta) {
    if(!s||!s->magic.available||level<1||level>9)return false;
    pd_slot *v=&(pact?s->magic.pact:s->magic.slots)[level-1];int64_t n=(int64_t)v->current+delta;
    v->current=n<0?0:n>v->maximum?v->maximum:n;return true;
}
void pd_end_rage(pd_state *s) {s->combat.raging=false;s->training.strength_advantage=false;}
