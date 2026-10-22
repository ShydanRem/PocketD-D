#include "history.h"
#include <string.h>
static bool text(const char *s,size_t capacity,bool empty){
    size_t i=0;for(;i<capacity&&s[i];i++)if((unsigned char)s[i]<32||(unsigned char)s[i]>126)return false;
    return i<capacity&&(i||empty);
}
void pd_history_reset(pd_history *h,uint32_t epoch){memset(h,0,sizeof *h);h->epoch=epoch;}
bool pd_history_entry_valid(const pd_history_entry *e){
    if(!e||e->kind>=PD_H_KINDS||!text(e->label,sizeof e->label,false)||!text(e->note,sizeof e->note,true)||
       !e->count||e->count>PD_HISTORY_DICE||e->mode< -1||e->mode>1||e->bonus< -200||e->bonus>200||
       (e->sides!=4&&e->sides!=6&&e->sides!=8&&e->sides!=10&&e->sides!=12&&e->sides!=20))return false;
    if(e->mode&&(e->sides!=20||e->count!=2||e->floor_zero))return false;
    int sum=0;
    for(unsigned i=0;i<PD_HISTORY_DICE;i++){
        if(i<e->count){if(!e->dice[i]||e->dice[i]>e->sides)return false;sum+=e->dice[i];}
        else if(e->dice[i])return false;
    }
    if(e->mode)sum=e->mode>0?(e->dice[0]>e->dice[1]?e->dice[0]:e->dice[1]):(e->dice[0]<e->dice[1]?e->dice[0]:e->dice[1]);
    int total=sum+e->bonus;if(e->floor_zero&&total<0)total=0;
    return total==e->total;
}
bool pd_history_valid(const pd_history *h){
    if(!h||h->count>PD_HISTORY_LIMIT||h->sequence<h->count||(!h->count&&h->sequence))return false;
    for(unsigned i=0;i<h->count;i++)if(!pd_history_entry_valid(&h->entries[i])||h->entries[i].sequence!=h->sequence-i)return false;
    return true;
}
bool pd_history_append(pd_history *h,const pd_history_entry *entry){
    if(!pd_history_valid(h)||!pd_history_entry_valid(entry)||h->sequence==UINT32_MAX)return false;
    pd_history_entry copy=*entry;copy.sequence=h->sequence+1;
    unsigned retained=h->count<PD_HISTORY_LIMIT?h->count:PD_HISTORY_LIMIT-1;
    memmove(h->entries+1,h->entries,retained*sizeof *h->entries);h->entries[0]=copy;
    h->sequence++;if(h->count<PD_HISTORY_LIMIT)h->count++;return true;
}
static void put32(uint8_t **p,uint32_t n){for(unsigned i=0;i<4;i++)*(*p)++=n>>(8*i);}
static uint32_t get32(const uint8_t **p){uint32_t n=0;for(unsigned i=0;i<4;i++)n|=(uint32_t)*(*p)++<<(8*i);return n;}
size_t pd_history_encode(const pd_history *h,uint32_t generation,uint8_t out[PD_HISTORY_RECORD]){
    if(!out||!pd_history_valid(h))return 0;
    uint8_t *p=out;memcpy(p,"PDH1",4);p+=4;put32(&p,generation);put32(&p,h->epoch);put32(&p,h->sequence);*p++=h->count;
    for(unsigned i=0;i<h->count;i++){
        const pd_history_entry *e=&h->entries[i];put32(&p,e->sequence);
        *p++=e->kind;*p++=e->mode+1;*p++=e->count;*p++=e->sides;*p++=e->floor_zero;
        unsigned bonus=e->bonus+200;*p++=bonus;*p++=bonus>>8;put32(&p,(uint32_t)(e->total+200));
        memset(p,0,PD_HISTORY_LABEL);memcpy(p,e->label,strlen(e->label));p+=PD_HISTORY_LABEL;
        memset(p,0,PD_HISTORY_NOTE);memcpy(p,e->note,strlen(e->note));p+=PD_HISTORY_NOTE;
        memcpy(p,e->dice,PD_HISTORY_DICE);p+=PD_HISTORY_DICE;
    }
    size_t payload=p-out;put32(&p,pd_crc32(out,payload));return p-out;
}
bool pd_history_decode(const uint8_t *data,size_t size,pd_history *h,uint32_t *generation){
    if(!data||!h||!generation||size<21||size>PD_HISTORY_RECORD||(memcmp(data,"PDH1",4)&&memcmp(data,"PDH2",4)))return false;
    unsigned count=data[16];if(count>PD_HISTORY_LIMIT||size!=21u+123u*count)return false;
    const uint8_t *crc=data+size-4;if(pd_crc32(data,size-4)!=get32(&crc))return false;
    pd_history candidate;const uint8_t *p=data+4;uint32_t gen=get32(&p);pd_history_reset(&candidate,get32(&p));candidate.sequence=get32(&p);candidate.count=*p++;
    for(unsigned i=0;i<candidate.count;i++){
        pd_history_entry *e=&candidate.entries[i];e->sequence=get32(&p);e->kind=*p++;
        if(*p>2)return false;
        e->mode=(int)*p++-1;e->count=*p++;e->sides=*p++;
        if(*p>1)return false;
        e->floor_zero=*p++;
        unsigned bonus=p[0]|((unsigned)p[1]<<8);p+=2;if(bonus>400)return false;e->bonus=(int)bonus-200;
        uint32_t total=get32(&p);if(total>1000)return false;e->total=(int)total-200;
        memcpy(e->label,p,PD_HISTORY_LABEL);p+=PD_HISTORY_LABEL;memcpy(e->note,p,PD_HISTORY_NOTE);p+=PD_HISTORY_NOTE;
        memcpy(e->dice,p,PD_HISTORY_DICE);p+=PD_HISTORY_DICE;
    }
    if(!pd_history_valid(&candidate))return false;
    *h=candidate;*generation=gen;return true;
}
