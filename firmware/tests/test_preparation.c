#include "magic.h"
#include "backup.h"
#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pd_state s,before,expected,used,decoded;
static pd_backup restored;
static uint8_t record[PD_RECORD_SIZE];
int main(void){
    unsigned cases=0;
    for(unsigned index=0;index<PD_POWERS;index++)for(unsigned style=0;style<4;style++)
    for(unsigned initial=0;initial<2;initial++)for(unsigned target=0;target<2;target++)for(unsigned bound=0;bound<2;bound++){
        pd_demo(&s);s.magic.available=true;s.magic.count=PD_POWERS;
        for(unsigned i=0;i<PD_POWERS;i++)s.magic.powers[i]=(pd_power){.name="Synthetic spell",.flags=PD_PREPARED|PD_CONCENTRATES};
        pd_power *p=&s.magic.powers[index];p->flags=PD_CONCENTRATES|(initial?PD_PREPARED:0)|((style&1)?PD_USES_SLOT:0)|((style&2)?PD_LIMITED:0);
        p->level=(style&1)?1:0;p->maximum=(style&2)?2:0;p->remaining=(style&2)?1:0;
        s.magic.slots[0]=(pd_slot){2,3};s.magic.pact[0]=(pd_slot){1,1};s.magic.concentration=bound?index+1:0;
        s.effect_count=1;s.effects[0]=(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=bound?index+1:0};
        s.defences.temporary[0]=(pd_defence_temporary){.flags=1,.active=true,.concentration=bound?index+1:0};
        s.reductions[0]=(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.concentration=bound?index+1:0};
        assert(pd_valid(&s));before=expected=s;
        if(target)expected.magic.powers[index].flags|=PD_PREPARED;else expected.magic.powers[index].flags&=~PD_PREPARED;
        assert(pd_magic_prepare(&s,index,target)&&!memcmp(&s,&expected,sizeof s)&&pd_valid(&s));
        assert(pd_magic_prepare(&s,index,target)&&!memcmp(&s,&expected,sizeof s));
        used=s;assert(pd_magic_use(&used,index,(style&1)?1:0,false)==!!target);
        if(!target)assert(!memcmp(&used,&s,sizeof s));
        else {assert(used.magic.slots[0].current==2-(unsigned)((style&1)!=0));assert(!used.magic.powers[index].remaining);}
        cases++;
    }
    /* Last-row maximum-sized record and native snapshot retain the flag. */
    assert(pd_magic_prepare(&s,119,false));size_t n=pd_encode(&s,64,record);uint32_t generation=0;
    assert(n==PD_RECORD_SIZE&&!memcmp(record,"PDDW",4)&&pd_decode(record,n,&decoded,&generation)&&generation==64&&!memcmp(&s,&decoded,sizeof s));
    pd_history h;pd_history_reset(&h,s.history_epoch);size_t json_size;char *json=pd_backup_encode(&s,&h,&json_size);
    assert(json&&pd_backup_decode(json,json_size,&restored)&&!memcmp(&s,&restored.state,sizeof s));free(json);
    before=s;assert(!pd_magic_prepare(NULL,0,true)&&!pd_magic_prepare(&s,PD_POWERS,true)&&!memcmp(&s,&before,sizeof s));
    s.magic.powers[119]=(pd_power){.name="Feature",.kind=1,.maximum=2,.remaining=1};s.magic.concentration=0;
    s.effects[0].concentration=s.defences.temporary[0].concentration=s.reductions[0].concentration=0;assert(pd_valid(&s));before=s;
    assert(!pd_magic_prepare(&s,119,true)&&!pd_magic_prepare(&s,119,false)&&!memcmp(&s,&before,sizeof s));
    s.magic.available=false;s.magic.count=0;before=s;assert(!pd_magic_prepare(&s,0,true)&&!memcmp(&s,&before,sizeof s));
    pd_demo(&s);s.magic.count=1;s.magic.powers[0]=(pd_power){.name="Unprepared spell"};assert(pd_valid(&s));
    assert(pd_rest_apply(&s,&(pd_rest_plan){0})&&!(s.magic.powers[0].flags&PD_PREPARED));
    assert(pd_rest_apply(&s,&(pd_rest_plan){.long_rest=true})&&!(s.magic.powers[0].flags&PD_PREPARED));
    s.hp=1000;before=s;assert(!pd_magic_prepare(&s,0,true)&&!memcmp(&s,&before,sizeof s));
    printf("Preparation: %u full-state/flag/style/binding/index cases, idempotence, unchanged concentration/pools, cast gating, native maximum backup, rest preservation and atomic rejection passed\n",cases);
}
