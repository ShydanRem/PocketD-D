#include "status.h"
#include <string.h>
bool pd_death_ready(const pd_state *s) {
    return s&&s->status.available&&!s->hp&&!s->status.stabilized&&s->status.successes<3&&s->status.failures<3;
}
bool pd_death_change(pd_state *s,unsigned action) {
    if(!s||!pd_valid(s)||action>PD_DEATH_MINUS_FAILURE)return false;
    pd_status *d=&s->status;
    if(action==PD_DEATH_RESET){d->available=true;d->successes=0;d->failures=0;d->stabilized=false;return true;}
    if(!d->available)return false;
    if(action==PD_DEATH_MINUS_SUCCESS){if(!d->successes)return false;d->successes--;d->stabilized=false;return true;}
    if(action==PD_DEATH_MINUS_FAILURE){if(!d->failures)return false;d->failures--;d->stabilized=false;return true;}
    if(s->hp||d->failures==3)return false;
    if(action==PD_DEATH_STABILIZE){d->successes=0;d->failures=0;d->stabilized=true;return true;}
    if(action==PD_DEATH_SUCCESS) {
        if(!pd_death_ready(s))return false;
        if(++d->successes==3){d->successes=0;d->failures=0;d->stabilized=true;}
    } else {d->stabilized=false;unsigned n=d->failures+(action==PD_DEATH_CRITICAL?2:1);d->failures=n>3?3:n;}
    return true;
}
bool pd_death_face(pd_state *s,unsigned face) {
    if(!s||!pd_valid(s)||!pd_death_ready(s)||face<1||face>20)return false;
    if(face==20){pd_hp(s,1);return true;}
    return pd_death_change(s,face==1?PD_DEATH_CRITICAL:face>=10?PD_DEATH_SUCCESS:PD_DEATH_FAILURE);
}
bool pd_death_roll(const pd_state *s,pd_random random,void *context,pd_state *next,pd_roll *roll) {
    if(!s||!next||s==next||!roll||!random||!pd_valid(s)||!pd_death_ready(s))return false;
    pd_roll r=pd_d20(0,0,random,context);*next=*s;
    if(!pd_death_face(next,r.natural))return false;
    *roll=r;return true;
}
bool pd_condition_add(pd_state *s,const char *name,unsigned level) {
    if(!s||!name||s->status.count>=PD_CONDITIONS||level>6)return false;
    size_t n=0;while(n<61&&name[n]){if((unsigned char)name[n]<32||(unsigned char)name[n]>126)return false;n++;}
    if(!n||n>60)return false;
    pd_condition *v=&s->status.conditions[s->status.count++];memset(v,0,sizeof *v);memcpy(v->name,name,n);v->level=level;return true;
}
bool pd_condition_level(pd_state *s,unsigned index,int delta) {
    if(!s||index>=s->status.count)return false;
    int64_t n=(int64_t)s->status.conditions[index].level+delta;s->status.conditions[index].level=n<0?0:n>6?6:n;return true;
}
bool pd_condition_remove(pd_state *s,unsigned index) {
    if(!s||index>=s->status.count)return false;
    memmove(s->status.conditions+index,s->status.conditions+index+1,(s->status.count-index-1)*sizeof(pd_condition));
    memset(&s->status.conditions[--s->status.count],0,sizeof(pd_condition));return true;
}
