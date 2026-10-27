#include "pocketdd.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static pd_state s,before,expected,decoded;
static uint8_t record[PD_RECORD_SIZE];
int main(void){
    const unsigned maxima[]={1,32,42,999},temps[]={0,1,7,999},amounts[]={0,1,7,32,999,1000,UINT_MAX};
    unsigned cases=0;
    for(unsigned m=0;m<4;m++)for(unsigned h=0;h<3;h++)for(unsigned t=0;t<4;t++)
    for(unsigned a=0;a<7;a++)for(unsigned kind=0;kind<=PD_VITAL_COUNT;kind++)for(unsigned registry=0;registry<5;registry++){
        pd_demo(&s);s.max_hp=maxima[m];
        /* The 42-HP case has an active local bonus; healing must use that cap. */
        if(m==2){s.max_hp=32;s.effect_count=1;s.effects[0]=(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true};}
        s.hp=h==0?0:h==1?maxima[m]/2:maxima[m];s.temp_hp=temps[t];
        s.status.available=registry!=0;s.status.failures=registry==2?2:registry==3?3:0;
        s.status.successes=registry==4?3:0;s.status.stabilized=registry==4;
        assert(pd_valid(&s));before=expected=s;
        unsigned amount=amounts[a];bool allowed=kind<PD_VITAL_COUNT&&amount<=999&&(amount||kind==PD_VITAL_TEMP_SET)&&
            !(kind==PD_VITAL_HEAL&&!s.hp&&registry==3);
        if(allowed){
            if(kind==PD_VITAL_HEAL){
                unsigned sum=s.hp+amount;expected.hp=sum>maxima[m]?maxima[m]:sum;
                if(!s.hp&&s.status.available){expected.status.successes=expected.status.failures=0;expected.status.stabilized=false;}
            }else if(kind==PD_VITAL_TEMP_SET)expected.temp_hp=amount;
            else if(amount>temps[t])expected.temp_hp=amount;
        }
        assert(pd_vitals_apply(&s,kind,amount)==allowed&&!memcmp(&s,&expected,sizeof s));
        assert(pd_valid(&s));cases++;
    }
    assert(!pd_vitals_apply(NULL,PD_VITAL_HEAL,1));pd_demo(&s);s.temp_hp=1000;before=s;
    assert(!pd_vitals_apply(&s,PD_VITAL_TEMP_SET,0)&&!memcmp(&s,&before,sizeof s));
    pd_demo(&s);s.hp=0;s.status.available=true;s.status.failures=2;
    assert(pd_vitals_apply(&s,PD_VITAL_TEMP_GRANT,12)&&!s.hp&&s.temp_hp==12&&s.status.failures==2);
    assert(pd_vitals_apply(&s,PD_VITAL_HEAL,7)&&s.hp==7&&s.temp_hp==12&&!s.status.failures);
    size_t n=pd_encode(&s,61,record);uint32_t generation=0;
    assert(!memcmp(record,"PDDW",4)&&pd_decode(record,n,&decoded,&generation)&&generation==61&&!memcmp(&s,&decoded,sizeof s));
    printf("Vitals: %u independent HP/cap/temp/amount/operation/registry cases, no temp stacking/healing, dead-heal rejection, exact unchanged state and PDD28 persistence passed\n",cases);
}
