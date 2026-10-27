#include "pocketdd.h"
#include "magic.h"
#include "backup.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pd_state s,before,expected,out;
static uint8_t record[PD_RECORD_SIZE];
static pd_backup restored;
static void setup(void){pd_demo(&s);s.hp=32;s.magic.count=1;s.magic.powers[0]=(pd_power){.name="Fonte sintetica",.flags=PD_PREPARED|PD_CONCENTRATES};s.magic.concentration=1;s.reaction_spent=true;}
static void crc(size_t n){uint32_t c=pd_crc32(record,n-4);for(unsigned i=0;i<4;i++)record[n-4+i]=c>>(8*i);}
int main(int argc,char **argv){
    assert(argc==1||argc==2);
    unsigned cases=0;pd_round_summary r;
    for(unsigned index=0;index<50;index++)for(unsigned owner=0;owner<4;owner++)for(unsigned event=0;event<4;event++)
    for(unsigned edge=0;edge<2;edge++)for(unsigned active=0;active<2;active++)for(unsigned count=1;count<=3;count++)
    for(unsigned spent=0;spent<2;spent++)for(unsigned linked=0;linked<2;linked++){
        setup();s.reaction_spent=spent;unsigned tick=edge?PD_TICK_END:PD_TICK_START;uint8_t *remaining;bool *enabled;uint64_t bit=0;unsigned numeric=0;
        if(index<8){s.effect_count=index+1;assert(pd_effect_set(&s,index,(pd_effect){.target=index%2,.value=4,.active=active,.timed=true,.rounds=count,.concentration=linked,.tick=tick,.owner=owner}));remaining=&s.effects[index].rounds;enabled=&s.effects[index].active;numeric=1u<<index;}
        else if(index<47){unsigned at=index-8,type=at%13,slot=at/13;assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=1,.active=active,.timed=true,.rounds=count,.concentration=linked,.tick=tick,.owner=owner}));pd_defence_temporary *d=slot?&s.defences.additional[type][slot-1]:&s.defences.temporary[type];remaining=&d->rounds;enabled=&d->active;bit=UINT64_C(1)<<at;}
        else{unsigned at=index-47;assert(pd_reduction_profile_set(&s,at,(pd_reduction_profile){.value=3,.types=8191,.active=active,.timed=true,.rounds=count,.concentration=linked,.tick=tick,.owner=owner}));remaining=&s.reductions[at].rounds;enabled=&s.reductions[at].active;bit=UINT64_C(1)<<(39+at);}
        if(pd_max_hp(&s)>32){s.hp=35;}before=s;bool match=active&&owner==event;
        if(match){--*remaining;if(!*remaining)*enabled=false;}if(s.hp>pd_max_hp(&s))s.hp=pd_max_hp(&s);if(!event&&!edge)s.reaction_spent=false;expected=s;s=before;
        assert(pd_turn_advance_subject(&s,event,!edge,&r)&&!memcmp(&s,&expected,sizeof s)&&pd_valid(&s));
        assert(r.effects_advanced==(match?numeric:0)&&r.effects_expired==(match&&count==1?numeric:0)&&r.defences_advanced==(match?bit:0)&&r.defences_expired==(match&&count==1?bit:0));
        if(!event){out=before;pd_round_summary legacy;assert(pd_turn_advance(&out,!edge,&legacy)&&!memcmp(&out,&s,sizeof s)&&!memcmp(&legacy,&r,sizeof r));}
        cases++;
    }
    /* The opposite edge and other clocks never tick these timers. */
    setup();assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_END,.owner=3}));s.hp=42;before=s;
    assert(pd_turn_advance_subject(&s,3,true,&r)&&!memcmp(&s,&before,sizeof s));
    assert(pd_round_advance(&s,&r)&&!memcmp(&s,&before,sizeof s));
    assert(pd_time_advance(&s,5940,&r)&&!memcmp(&s,&before,sizeof s));
    assert(pd_turn_advance_subject(&s,3,false,&r)&&s.hp==32&&pd_max_hp(&s)==32&&s.reaction_spent);
    setup();assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_END,.owner=3,.concentration=1}));s.hp=42;
    assert(pd_defence_grant_set(&s,0,0,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START,.owner=2,.concentration=1}));
    assert(pd_reduction_profile_set(&s,0,(pd_reduction_profile){.value=1,.types=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START,.owner=1,.concentration=1}));
    pd_concentration_end(&s);assert(pd_valid(&s)&&s.hp==32&&!s.effects[0].active&&!s.defences.temporary[0].active&&!s.reductions[0].active&&s.effects[0].owner==3&&s.defences.temporary[0].owner==2&&s.reductions[0].owner==1);
    /* Invalid expiry rolls back every numeric/defence/profile timer and reaction. */
    setup();s.armor=20;assert(pd_effect_set(&s,0,(pd_effect){.value=20,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START,.owner=1}));assert(pd_effect_set(&s,1,(pd_effect){.value=-20,.active=true,.timed=true,.rounds=2,.tick=PD_TICK_START,.owner=2}));
    assert(pd_defence_grant_set(&s,12,2,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START,.owner=1}));
    assert(pd_reduction_profile_set(&s,2,(pd_reduction_profile){.value=1,.types=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START,.owner=1}));
    before=s;pd_round_summary sentinel={.effects_advanced=123,.defences_expired=123456};r=sentinel;
    assert(!pd_turn_advance_subject(&s,1,true,&r)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&sentinel,sizeof r));
    assert(!pd_turn_advance_subject(&s,4,true,&r)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&sentinel,sizeof r));
    assert(!pd_turn_advance_subject(&s,1,true,NULL)&&!memcmp(&s,&before,sizeof s));assert(!pd_turn_advance_subject(NULL,1,true,&r));
    /* Ownership is valid only for timed start/end clocks, including paused timers. */
    setup();before=s;
    for(unsigned tick=0;tick<PD_TICK_COUNT;tick++)if(tick!=PD_TICK_START&&tick!=PD_TICK_END){
        pd_effect e={.value=1,.timed=true,.rounds=1,.tick=tick,.owner=1};assert(!pd_effect_set(&s,0,e)&&!memcmp(&s,&before,sizeof s));
        pd_defence_temporary d={.flags=1,.timed=true,.rounds=1,.tick=tick,.owner=1};assert(!pd_defence_grant_set(&s,0,0,d));
        pd_reduction_profile p={.value=1,.types=1,.timed=true,.rounds=1,.tick=tick,.owner=1};assert(!pd_reduction_profile_set(&s,0,p));
    }
    /* Every packed timer position roundtrips; PDD25 defaults to the own turn. */
    setup();s.effect_count=8;
    for(unsigned i=0;i<8;i++)s.effects[i]=(pd_effect){.value=1,.timed=true,.rounds=99,.tick=PD_TICK_START,.owner=i%4};
    for(unsigned slot=0;slot<3;slot++)for(unsigned type=0;type<13;type++)assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=1,.timed=true,.rounds=99,.tick=PD_TICK_END,.owner=(slot*13+type)%4}));
    for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&s,i,(pd_reduction_profile){.value=1,.types=1,.timed=true,.rounds=99,.tick=PD_TICK_START,.owner=i+1}));
    unsigned timer_at=8375+552*s.magic.count;uint32_t generation=0;size_t n=pd_encode(&s,98,record);assert(n&&!memcmp(record,"PDDV",4)&&pd_decode(record,n,&out,&generation)&&generation==98&&!memcmp(&s,&out,sizeof s));
    pd_history history;pd_history_reset(&history,s.history_epoch);size_t json_size;char *json=pd_backup_encode(&s,&history,&json_size);assert(json&&pd_backup_decode(json,json_size,&restored)&&!memcmp(&restored.state,&s,sizeof s));if(argc==2){FILE *f=fopen(argv[1],"wb");assert(f&&fwrite(json,1,json_size,f)==json_size&&!fclose(f));}free(json);
    for(unsigned i=0;i<50;i++){
        n=pd_encode(&s,98,record);record[timer_at+i]=0x21;crc(n);before=out;generation=97;
        assert(!pd_decode(record,n,&out,&generation)&&generation==97&&!memcmp(&out,&before,sizeof out));
    }
    n=pd_encode(&s,98,record);record[timer_at]=8|PD_TICK_MINUTES;crc(n);assert(!pd_decode(record,n,&out,&generation));
    /* Old P tag rejects packed owners instead of silently dropping them. */
    n=pd_encode(&s,98,record);record[3]='P';n-=66;crc(n);assert(!pd_decode(record,n,&out,&generation));
    for(unsigned i=0;i<50;i++){record[timer_at+i]&=7;}crc(n);assert(pd_decode(record,n,&out,&generation));
    assert(!out.effects[1].owner&&!out.reductions[2].owner&&!out.defences.additional[12][1].owner);
    setup();n=pd_encode(&s,98,record);record[8375+552*s.magic.count]=9;crc(n);assert(!pd_decode(record,n,&out,&generation)); /* Unused numeric position. */
    printf("External turns: %u independent timer/subject/edge/pause/count/reaction/concentration cases, opposite clocks, atomic rollback, all 50 packed selectors, legacy PDD25 and native backup passed\n",cases);
}
