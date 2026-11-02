#include "beyond_runtime.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned abort_code;
static unsigned abort_now(void *unused){(void)unused;return abort_code;}
int main(int argc,char **argv){
    assert(argc==2);FILE *f=fopen(argv[1],"rb");assert(f);assert(!fseek(f,0,SEEK_END));long size=ftell(f);assert(size>0&&size<PD_BEYOND_SOURCE_MAX);rewind(f);char *source=malloc(size+1);assert(source&&fread(source,1,size,f)==(size_t)size);source[size]=0;fclose(f);
    assert(pd_beyond_id("1")&&pd_beyond_id("999999999999999"));
    for(unsigned i=0;i<7;i++)assert(!pd_beyond_id((const char *[]){"","0","01","123/evil","https://evil.test","1000000000000000","1\n"}[i]));
    assert(pd_beyond_utf8((const unsigned char *)"citta\xc3\xa0",7));
    const unsigned char invalid[][4]={{0xc0,0xaf,0,0},{0xed,0xa0,0x80,0},{0xf4,0x90,0x80,0x80},{0xff,0x80,0x80,0x80},{0,0,0,0}};
    for(unsigned i=0;i<5;i++)assert(!pd_beyond_utf8(invalid[i],i==0?2:i==1?3:4));
    pd_state *state=malloc(sizeof *state),*before=malloc(sizeof *before);pd_beyond_report *report=malloc(sizeof *report);assert(state&&before&&report);pd_demo(state);*before=*state;
    const size_t budgets[]={4096,16384,32768,262144};
    for(unsigned i=0;i<4;i++){pd_beyond_control c={.heap_limit=budgets[i]};assert(pd_beyond_convert(source,size,"123456",state,report,&c)==PD_BEYOND_MEMORY&&!memcmp(state,before,sizeof *state)&&c.peak_bytes<=budgets[i]);}
    for(unsigned i=0;i<2;i++){abort_code=i?PD_BEYOND_TIMEOUT:PD_BEYOND_CANCELLED;pd_beyond_control c={.interrupt=abort_now};assert(pd_beyond_convert(source,size,"123456",state,report,&c)==abort_code&&!memcmp(state,before,sizeof *state));}
    const char *bad[]={"{broken","[]","null","{\"success\":true,\"data\":{\"id\":999}}","\"not a character\""};
    for(unsigned i=0;i<5;i++){pd_beyond_control c={0};unsigned status=pd_beyond_convert(bad[i],strlen(bad[i]),"123456",state,report,&c);if(status!=PD_BEYOND_INVALID)fprintf(stderr,"Malformed-case %u: status %u, phase %u, peak %zu\n",i,status,c.phase,c.peak_bytes);assert(status==PD_BEYOND_INVALID&&!memcmp(state,before,sizeof *state));}
    pd_beyond_control normal={0};assert(pd_beyond_convert(source,size,"123456",state,report,&normal)==PD_BEYOND_READY&&pd_valid(state)&&report->limits>0&&normal.peak_bytes<=PD_BEYOND_HEAP_MAX);
    free(state);free(before);free(report);free(source);
    puts("Native Beyond runtime: strict ID/UTF8, explicit OOM budgets, cancel/deadline, malformed/foreign schema refusal and unchanged output on every failure passed. Trusted source only, no storage/network/RNG.");
}
