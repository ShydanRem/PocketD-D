#include "backup.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static const char alphabet[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static size_t b64_size(size_t n){return 4*((n+2)/3);}
static void encode64(const uint8_t *data,size_t size,char *out){
    size_t at=0;
    for(size_t i=0;i<size;i+=3){
        uint32_t value=(uint32_t)data[i]<<16;if(i+1<size)value|=(uint32_t)data[i+1]<<8;if(i+2<size)value|=data[i+2];
        out[at++]=alphabet[(value>>18)&63];out[at++]=alphabet[(value>>12)&63];out[at++]=i+1<size?alphabet[(value>>6)&63]:'=';out[at++]=i+2<size?alphabet[value&63]:'=';
    }
}
static int digit(char c){const char *p=c?strchr(alphabet,c):NULL;return p?(int)(p-alphabet):-1;}
static size_t decode64(const char *text,size_t size,uint8_t *out,size_t capacity){
    if(!size||size%4)return 0;
    size_t written=0;
    for(size_t i=0;i<size;i+=4){
        int a=digit(text[i]),b=digit(text[i+1]),c=digit(text[i+2]),d=digit(text[i+3]);
        unsigned bytes=text[i+2]=='='?1:text[i+3]=='='?2:3;
        if(a<0||b<0||(bytes>1&&c<0)||(bytes>2&&d<0)||written+bytes>capacity)return 0;
        if(bytes<3&&(i+4!=size||(bytes==1&&(text[i+3]!='='||(b&15)))||(bytes==2&&(c&3))))return 0;
        uint32_t value=((unsigned)a<<18)|((unsigned)b<<12)|((unsigned)(c<0?0:c)<<6)|(unsigned)(d<0?0:d);
        out[written++]=value>>16;if(bytes>1)out[written++]=value>>8;if(bytes>2)out[written++]=value;
    }
    return written;
}
char *pd_backup_encode(const pd_state *s,const pd_history *h,size_t *size){
    if(!s||!h||!size||!pd_valid(s)||!pd_history_valid(h)||s->history_epoch!=h->epoch)return NULL;
    uint8_t *records=malloc(PD_RECORD_SIZE+PD_HISTORY_RECORD);if(!records)return NULL;
    size_t state_size=pd_encode(s,0,records),history_size=pd_history_encode(h,0,records+PD_RECORD_SIZE);
    const char *prefix="{\"format\":\"pocketdd-device-backup\",\"version\":1,\"state\":\"",*middle="\",\"history\":\"",*suffix="\"}";
    size_t encoded=strlen(prefix)+b64_size(state_size)+strlen(middle)+b64_size(history_size)+strlen(suffix);
    char *json=encoded<=PD_BACKUP_MAX?malloc(encoded+1):NULL;
    if(json){
        char *p=json;memcpy(p,prefix,strlen(prefix));p+=strlen(prefix);encode64(records,state_size,p);p+=b64_size(state_size);
        memcpy(p,middle,strlen(middle));p+=strlen(middle);encode64(records+PD_RECORD_SIZE,history_size,p);p+=b64_size(history_size);
        memcpy(p,suffix,strlen(suffix)+1);*size=encoded;
    }
    free(records);return json;
}
typedef struct {const char *p,*end;} reader;
static void spaces(reader *r){while(r->p<r->end&&(*r->p==' '||*r->p=='\n'||*r->p=='\t'||*r->p=='\r'))r->p++;}
static bool token(reader *r,char c){spaces(r);if(r->p==r->end||*r->p!=c)return false;r->p++;return true;}
static bool string(reader *r,const char **text,size_t *size){
    if(!token(r,'"'))return false;
    const char *start=r->p;
    while(r->p<r->end&&*r->p!='"'){if((unsigned char)*r->p<32||(unsigned char)*r->p>126||*r->p=='\\')return false;r->p++;}
    if(r->p==r->end)return false;
    *text=start;*size=r->p-start;r->p++;return true;
}
static bool equal(const char *text,size_t size,const char *expected){return strlen(expected)==size&&!memcmp(text,expected,size);}
bool pd_backup_decode(const char *json,size_t size,pd_backup *out){
    if(!json||!out||!size||size>PD_BACKUP_MAX)return false;
    reader r={json,json+size};if(!token(&r,'{'))return false;
    const char *state_text=NULL,*history_text=NULL;size_t state_length=0,history_length=0;unsigned seen=0;
    for(unsigned field=0;field<4;field++){
        const char *key,*value;size_t key_size,value_size;if(field&&!token(&r,','))return false;
        if(!string(&r,&key,&key_size)||!token(&r,':'))return false;
        unsigned bit=equal(key,key_size,"format")?1:equal(key,key_size,"version")?2:equal(key,key_size,"state")?4:equal(key,key_size,"history")?8:0;
        if(!bit||(seen&bit))return false;
        seen|=bit;
        if(bit==2){if(!token(&r,'1'))return false;continue;}
        if(!string(&r,&value,&value_size))return false;
        if(bit==1){if(!equal(value,value_size,"pocketdd-device-backup"))return false;}
        else if(bit==4){state_text=value;state_length=value_size;}
        else {history_text=value;history_length=value_size;}
    }
    if(seen!=15||!token(&r,'}'))return false;
    spaces(&r);if(r.p!=r.end||state_length>b64_size(PD_RECORD_SIZE)||history_length>b64_size(PD_HISTORY_RECORD))return false;
    uint8_t *records=malloc(PD_RECORD_SIZE+PD_HISTORY_RECORD);pd_backup *candidate=calloc(1,sizeof *candidate);
    if(!records||!candidate){free(records);free(candidate);return false;}
    size_t n=decode64(state_text,state_length,records,PD_RECORD_SIZE),m=decode64(history_text,history_length,records+PD_RECORD_SIZE,PD_HISTORY_RECORD);
    uint32_t gen;bool valid=n>=8879&&m>=21&&(!memcmp(records,"PDDP",4)||(!memcmp(records,"PDDQ",4)||!memcmp(records,"PDDR",4)))&&!memcmp(records+PD_RECORD_SIZE,"PDH1",4)&&pd_decode(records,n,&candidate->state,&gen)&&
        pd_history_decode(records+PD_RECORD_SIZE,m,&candidate->history,&gen)&&candidate->state.history_epoch==candidate->history.epoch;
    if(valid)*out=*candidate;
    free(records);free(candidate);return valid;
}
bool pd_backup_transmit(const pd_state *s,const pd_history *h,pd_backup_writer writer,void *context){
    if(!writer)return false;
    size_t size=0;char *json=pd_backup_encode(s,h,&size);if(!json)return false;
    char header[80];int n=snprintf(header,sizeof header,"PDD BACKUP_DATA %u %08x\n",(unsigned)size,(unsigned)pd_crc32((const uint8_t *)json,size));
    const char *end="\nPDD BACKUP_DONE\n";
    bool ok=writer(header,(size_t)n,context)&&writer(json,size,context)&&writer(end,strlen(end),context);free(json);return ok;
}
