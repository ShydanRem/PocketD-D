#include "pocketdd.h"
#include "backup.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pd_state s,seed,before,expected,out;
static pd_backup restored;
static uint8_t record[PD_RECORD_SIZE];
static void crc(size_t size){uint32_t c=pd_crc32(record,size-4);for(unsigned i=0;i<4;i++)record[size-4+i]=c>>(8*i);}
static void setup(void){
    pd_demo(&s);s.effect_count=8;s.reaction_spent=true;s.history_epoch=123456;
    for(unsigned i=0;i<8;i++)s.effects[i]=(pd_effect){.value=1,.timed=true,.rounds=99,.tick=PD_TICK_START,.owner=i%4};
    for(unsigned slot=0;slot<3;slot++)for(unsigned type=0;type<13;type++)assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=1,.timed=true,.rounds=99,.tick=PD_TICK_END,.owner=(slot*13+type)%4}));
    for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&s,i,(pd_reduction_profile){.value=1,.types=1,.timed=true,.rounds=99,.tick=PD_TICK_START,.owner=i+1}));
    assert(pd_turn_name_set(&s,1,"Ally A")&&pd_turn_name_set(&s,2,"WWWWWWWWWWWWWWWWWWWW")&&pd_turn_name_set(&s,3,"Mage & Co"));
}
int main(int argc,char **argv){
    assert(argc==1||argc==2);setup();seed=s;unsigned cases=0;
    for(unsigned owner=0;owner<5;owner++)for(unsigned length=0;length<=21;length++)for(unsigned byte=0;byte<=255;byte++){
        char input[22]={0};memset(input,byte,length);s=seed;expected=seed;
        unsigned actual=byte?length:0;bool valid=owner>0&&owner<4&&actual<=20&&(!actual||(byte>=32&&byte<=126));
        if(valid){memset(expected.turn_names[owner-1],0,PD_TURN_NAME_BYTES);if(byte!=32)memset(expected.turn_names[owner-1],byte,actual);}
        assert(pd_turn_name_set(&s,owner,input)==valid&&!memcmp(&s,&expected,sizeof s)&&pd_valid(&s));cases++;
    }
    s=seed;assert(pd_turn_name_set(&s,1,"  The Mage  ")&&!strcmp(s.turn_names[0],"The Mage"));
    before=s;assert(pd_turn_name_set(&s,1,s.turn_names[0])&&!memcmp(&s,&before,sizeof s));
    assert(pd_turn_name_set(&s,1,s.turn_names[2])&&!strcmp(s.turn_names[0],"Mage & Co")&&!strcmp(s.turn_names[2],"Mage & Co"));
    before=s;assert(!pd_turn_name_set(NULL,1,"x")&&!pd_turn_name_set(&s,UINT_MAX,"x")&&!pd_turn_name_set(&s,1,NULL)&&!memcmp(&s,&before,sizeof s));
    s.turn_names[0][20]='X';before=s;assert(!pd_valid(&s)&&!pd_turn_name_set(&s,2,"New")&&!memcmp(&s,&before,sizeof s));
    setup();
    for(unsigned powers=0;powers<=120;powers+=powers?119:1){
        s.magic.count=powers;for(unsigned i=0;i<powers;i++)s.magic.powers[i]=(pd_power){.name="Synthetic spell",.flags=PD_PREPARED};
        size_t n=pd_encode(&s,56,record);uint32_t generation=0;
        assert(n==8945+552*powers&&!memcmp(record,"PDDV",4)&&pd_record_known(record,n)&&pd_decode(record,n,&out,&generation)&&generation==56&&!memcmp(&out,&s,sizeof s));
        /* Pre-name formats retain clocks/references and default names to empty. */
        record[3]='Q';size_t old=n-66;crc(old);expected=s;memset(expected.turn_names,0,sizeof expected.turn_names);
        assert(pd_decode(record,old,&out,&generation)&&!memcmp(&out,&expected,sizeof out));
        record[3]='P';for(unsigned i=0;i<50;i++)record[8375+552*powers+i]&=7;crc(old);
        for(unsigned i=0;i<8;i++)expected.effects[i].owner=0;
        for(unsigned slot=0;slot<3;slot++)for(unsigned type=0;type<13;type++){pd_defence_temporary *d=slot?&expected.defences.additional[type][slot-1]:&expected.defences.temporary[type];d->owner=0;}
        for(unsigned i=0;i<3;i++)expected.reductions[i].owner=0;
        assert(pd_decode(record,old,&out,&generation)&&!memcmp(&out,&expected,sizeof out));
        /* Reject noncanonical/truncated/control/non-ASCII name fields atomically. */
        for(unsigned field=0;field<3;field++)for(unsigned position=0;position<21;position++){
            pd_encode(&s,56,record);record[n-70+field*21+position]=31;crc(n);before=out;generation=55;
            assert(!pd_decode(record,n,&out,&generation)&&generation==55&&!memcmp(&out,&before,sizeof out));
        }
        pd_encode(&s,56,record);record[n-70+6]=0;record[n-70+7]='x';crc(n);assert(!pd_decode(record,n,&out,&generation));
        pd_encode(&s,56,record);record[n-70]=' ';crc(n);assert(!pd_decode(record,n,&out,&generation));
        pd_encode(&s,56,record);record[3]='W';crc(n);assert(!pd_record_known(record,n));
        if(powers==120)break;
    }
    pd_history h;pd_history_reset(&h,s.history_epoch);size_t size;char *json=pd_backup_encode(&s,&h,&size);
    assert(json&&pd_backup_decode(json,size,&restored)&&!memcmp(&s,&restored.state,sizeof s));
    if(argc==2){FILE *file=fopen(argv[1],"wb");assert(file&&fwrite(json,1,size,file)==size&&!fclose(file));}free(json);
    printf("Turn names: %u independent subject/length/byte cases, trimmed aliases, self-aliasing, unchanged 50 timer references, PDD27 0/1/120 powers, PDD25/26 migration, malformed names and native backup passed\n",cases);
}
