#define _POSIX_C_SOURCE 200809L
#include "beyond_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char **argv){
    char *source=malloc(PD_BEYOND_SOURCE_MAX+1);pd_state *s=malloc(sizeof *s);pd_beyond_report *report=calloc(1,sizeof *report);
    if(argc<2||!source||!s||!report)return 2;
    size_t n=fread(source,1,PD_BEYOND_SOURCE_MAX+1,stdin);pd_demo(s);
    pd_beyond_control control={.heap_limit=argc>2?(size_t)strtoul(argv[2],NULL,10):PD_BEYOND_HEAP_MAX};
    unsigned status=!strcmp(argv[1],"--packet")?(pd_parse_packet(source,n,s)?PD_BEYOND_READY:PD_BEYOND_INVALID):pd_beyond_convert(source,n,argv[1],s,report,&control);
    printf("{\"status\":%u,\"phase\":%u,\"peak\":%zu,\"blockers\":%u,\"limits\":%u",status,control.phase,control.peak_bytes,report->blockers,report->limits);
    if(status==PD_BEYOND_READY){printf(",\"hp\":%u,\"maxHp\":%u,\"armor\":%u,\"items\":%u,\"powers\":%u,\"abilities\":[%u,%u,%u,%u,%u,%u]",s->hp,pd_max_hp(s),pd_armor(s),s->item_count,s->magic.count,s->abilities[0],s->abilities[1],s->abilities[2],s->abilities[3],s->abilities[4],s->abilities[5]);
        if(argc>3&&!strcmp(argv[3],"--record")){unsigned char *record=malloc(PD_RECORD_SIZE);if(!record)return 2;size_t count=pd_encode(s,0,record);printf(",\"record\":\"");for(size_t i=0;i<count;i++)printf("%02x",record[i]);printf("\"");free(record);}
    }
    puts("}");free(source);free(s);free(report);return 0;
}
