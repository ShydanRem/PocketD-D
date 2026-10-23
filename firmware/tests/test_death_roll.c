#include "status.h"
#include "history.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static unsigned calls,faces[2];
static uint32_t rng(void *context){assert(context==faces&&calls<2);return faces[calls++]-1;}
static pd_state source,next,before,expected,untouched,seed;
static void oracle(unsigned face,int bonus){
    expected=source;
    if(face==20){expected.hp=1;expected.status.successes=expected.status.failures=0;expected.status.stabilized=false;}
    else if(face!=1&&(int)face+bonus>=10){
        if(++expected.status.successes==3){expected.status.successes=expected.status.failures=0;expected.status.stabilized=true;}
    }else {unsigned failures=expected.status.failures+(face==1?2:1);expected.status.failures=failures>3?3:failures;}
}
static void rejected(pd_death_roll_options options){
    pd_roll r={.bonus=123,.total=456},old=r;before=source;next=untouched;calls=0;
    assert(!pd_death_roll_configured(&source,options,rng,faces,&next,&r));
    assert(!calls&&!memcmp(&source,&before,sizeof source)&&!memcmp(&next,&untouched,sizeof next)&&!memcmp(&r,&old,sizeof r));
}
int main(void){
    pd_demo(&source);source.hp=0;source.status.available=true;source.temp_hp=7;
    source.training.poisoned=true;source.roll_conditions=PD_ROLL_CONDITIONS;
    seed=source;before=source;unsigned cases=0;const int bonuses[]={-30,-10,-1,0,1,9,30};
    for(unsigned good=0;good<3;good++)for(unsigned bad=0;bad<3;bad++)
    for(unsigned flags=0;flags<4;flags++)for(unsigned b=0;b<sizeof bonuses/sizeof *bonuses;b++)
    for(unsigned a=1;a<=20;a++)for(unsigned d=1;d<=20;d++){
        source=seed;source.status.successes=good;source.status.failures=bad;
        pd_death_roll_options options={.advantage=(flags&1)!=0,.disadvantage=(flags&2)!=0,.bonus=bonuses[b]};
        int mode=flags==1?1:flags==2?-1:0;assert(pd_death_mode(options)==mode);
        unsigned chosen=mode>0?(a>d?a:d):mode<0?(a<d?a:d):a;
        oracle(chosen,bonuses[b]);faces[0]=a;faces[1]=d;calls=0;pd_roll r={0};next=untouched;
        assert(pd_death_roll_configured(&source,options,rng,faces,&next,&r));
        assert(calls==(mode?2u:1u)&&r.count==calls&&r.dice[0]==a&&r.dice[1]==(mode?d:0));
        assert(r.natural==chosen&&r.bonus==bonuses[b]&&r.total==(int)chosen+bonuses[b]);
        assert(!memcmp(&next,&expected,sizeof next)&&pd_valid(&next));
        expected=seed;expected.status.successes=good;expected.status.failures=bad;
        assert(!memcmp(&source,&expected,sizeof source));
        /* Direct face resolver uses the same public, bounded contract. */
        next=source;oracle(chosen,bonuses[b]);assert(pd_death_face_bonus(&next,chosen,bonuses[b]));assert(!memcmp(&next,&expected,sizeof next));
        cases++;
    }
    /* Every signed bonus, including exact ordinary 9/10 thresholds. */
    for(int bonus=-30;bonus<=30;bonus++)for(unsigned face=1;face<=20;face++){
        source=seed;oracle(face,bonus);next=source;
        assert(pd_death_face_bonus(&next,face,bonus)&&!memcmp(&next,&expected,sizeof next));cases++;
    }
    source=seed;pd_death_roll_options base={0};
    rejected((pd_death_roll_options){.bonus=31});rejected((pd_death_roll_options){.bonus=-31});
    rejected((pd_death_roll_options){.bonus=INT_MAX});rejected((pd_death_roll_options){.bonus=INT_MIN});
    for(unsigned unavailable=0;unavailable<6;unavailable++){
        source=seed;
        if(unavailable==0)source.hp=1;
        if(unavailable==1)source.status.available=false;
        if(unavailable==2)source.status.stabilized=true;
        if(unavailable==3)source.status.successes=3;
        if(unavailable==4)source.status.failures=3;
        if(unavailable==5)source.max_hp=0;
        rejected(base);
    }
    source=seed;before=source;next=untouched;pd_roll r={.total=432},old=r;calls=0;
    assert(!pd_death_roll_configured(NULL,base,rng,faces,&next,&r));
    assert(!pd_death_roll_configured(&source,base,NULL,faces,&next,&r));
    assert(!pd_death_roll_configured(&source,base,rng,faces,NULL,&r));
    assert(!pd_death_roll_configured(&source,base,rng,faces,&source,&r));
    assert(!pd_death_roll_configured(&source,base,rng,faces,&next,NULL));
    assert(!calls&&!memcmp(&source,&before,sizeof source)&&!memcmp(&next,&untouched,sizeof next)&&!memcmp(&r,&old,sizeof r));
    assert(!pd_death_face_bonus(&source,0,0)&&!pd_death_face_bonus(&source,21,0)&&!pd_death_face_bonus(&source,5,31)&&!memcmp(&source,&before,sizeof source));
    /* Legacy entry points remain a one-die zero-bonus roll. */
    faces[0]=20;faces[1]=1;calls=0;assert(pd_death_roll(&source,rng,faces,&next,&r));
    assert(calls==1&&r.count==1&&!r.bonus&&next.hp==1);
    /* PDH2 already represents both dice, signed bonus and chosen total. */
    pd_history h,decoded;pd_history_reset(&h,0);
    pd_history_entry e={.kind=PD_H_DEATH,.count=2,.sides=20,.dice={20,2},.mode=-1,.bonus=-30,.total=-28,.label="Salvezza morte",.note="PF 0; riuscite 0; fallite 1"};
    assert(pd_history_append(&h,&e));uint8_t record[PD_HISTORY_RECORD];size_t size=pd_history_encode(&h,42,record);uint32_t generation=0;
    assert(size&&pd_history_decode(record,size,&decoded,&generation)&&generation==42&&!memcmp(&h,&decoded,sizeof h));
    printf("Death roll: %u independent dice/circumstance/bonus/registry cases, natural 1/20 priority, no-RNG rejection, legacy compatibility and PDH2 roundtrip passed\n",cases);
}
