#include "history.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void crc(uint8_t *data,size_t n){uint32_t c=pd_crc32(data,n-4);for(unsigned i=0;i<4;i++)data[n-4+i]=c>>(8*i);}
int main(void){
    pd_history h,out,before;uint8_t record[PD_HISTORY_RECORD];uint32_t generation=99;pd_history_reset(&h,17);
    assert(pd_history_valid(&h));size_t size=pd_history_encode(&h,42,record);assert(size==21&&pd_history_decode(record,size,&out,&generation)&&generation==42&&out.epoch==17&&!out.count);
    unsigned cases=0;const unsigned sides[]={4,6,8,10,12,20};
    for(unsigned kind=0;kind<PD_H_KINDS;kind++)for(unsigned die=0;die<6;die++)for(unsigned count=1;count<=18;count++)for(int bonus=-200;bonus<=200;bonus+=200){
        pd_history_entry e={.kind=kind,.count=count,.sides=sides[die],.bonus=bonus};strcpy(e.label,"Tiro sintetico");strcpy(e.note,"Nessuna modifica al personaggio");
        for(unsigned i=0;i<count;i++)e.dice[i]=1+(i%sides[die]);
        int sum=0;for(unsigned i=0;i<count;i++)sum+=e.dice[i];e.floor_zero=(kind==PD_H_DAMAGE||kind==PD_H_HEAL);e.total=sum+bonus;if(e.floor_zero&&e.total<0)e.total=0;
        assert(pd_history_append(&h,&e)&&h.entries[0].sequence==h.sequence&&pd_history_entry_valid(&h.entries[0]));
        size=pd_history_encode(&h,42,record);assert(size==21u+123u*h.count&&pd_history_decode(record,size,&out,&generation)&&out.sequence==h.sequence&&!memcmp(out.entries[0].dice,e.dice,18));cases++;
    }
    for(int mode=-1;mode<=1;mode+=2)for(unsigned a=1;a<=20;a++)for(unsigned b=1;b<=20;b++){
        pd_history_entry e={.kind=PD_H_SKILL,.count=2,.sides=20,.mode=mode,.dice={a,b},.bonus=-35};strcpy(e.label,"Furtivita");
        e.total=(mode>0?(a>b?a:b):(a<b?a:b))-35;assert(pd_history_append(&h,&e));cases++;
    }
    assert(h.count==20&&h.entries[19].sequence==h.sequence-19);size=pd_history_encode(&h,UINT32_MAX,record);assert(size==PD_HISTORY_RECORD);
    assert(pd_history_decode(record,size,&out,&generation)&&generation==UINT32_MAX&&pd_history_valid(&out));
    /* Every byte and every truncation must reject without changing output. */
    before=out;
    for(size_t i=0;i<size;i++){record[i]^=1;generation=123;assert(!pd_history_decode(record,size,&out,&generation)&&generation==123&&!memcmp(&out,&before,sizeof out));record[i]^=1;}
    for(size_t i=0;i<size;i++)assert(!pd_history_decode(record,i,&out,&generation));
    const unsigned offsets[]={16,17+4,17+5,17+6,17+7,17+8,17+9,17+11,17+15,17+56,17+105};
    for(unsigned i=0;i<sizeof offsets/sizeof *offsets;i++){pd_history_encode(&h,4,record);record[offsets[i]]=255;crc(record,size);bool accepted=pd_history_decode(record,size,&out,&generation);if(accepted)fprintf(stderr,"Accepted corruption offset %u\n",offsets[i]);assert(!accepted);}
    pd_history_entry bad=h.entries[0];before=h;bad.total++;assert(!pd_history_append(&h,&bad)&&!memcmp(&h,&before,sizeof h));
    bad=h.entries[0];bad.dice[17]=1;assert(!pd_history_append(&h,&bad));bad=h.entries[0];memset(bad.label,'W',sizeof bad.label);assert(!pd_history_append(&h,&bad));
    h.sequence=UINT32_MAX;for(unsigned i=0;i<h.count;i++)h.entries[i].sequence=UINT32_MAX-i;before=h;
    assert(!pd_history_append(&h,&before.entries[0])&&!memcmp(&h,&before,sizeof h));
    printf("History: %u independent dice/bonus/mode cases, 20-entry rollover, maximum codec, every-byte corruption/truncation, strict fields and atomic rejection passed\n",cases);
}
