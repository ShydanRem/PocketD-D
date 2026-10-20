/* Host-only bridge: exercise actual wire conversion and C arithmetic against JS. */
#include "checks.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
    FILE *input=argc==2?fopen(argv[1],"rb"):stdin;if(!input)return 4;
    char wire[PD_PACKET_MAX+2];
    while(fgets(wire,sizeof wire,input)) {
        pd_state s;if(!pd_parse_packet(wire,strlen(wire),&s))return 2;
        cJSON *rows=cJSON_CreateArray();
        for(int kind=0;kind<3;kind++)for(unsigned i=0;i<(kind==PD_SKILL?PD_SKILLS:6);i++) {
            pd_check_info info;if(!pd_describe_check(&s,kind,i,&info))return 3;
            cJSON *row=cJSON_CreateObject();cJSON_AddNumberToObject(row,"kind",kind);
            cJSON_AddStringToObject(row,"key",kind==PD_SKILL?pd_skill_ids[i]:pd_abilities[i]);
            cJSON_AddNumberToObject(row,"base",info.base);cJSON_AddNumberToObject(row,"trained",info.trained);
            cJSON_AddNumberToObject(row,"extra",info.extra);cJSON_AddNumberToObject(row,"total",info.total);
            cJSON_AddBoolToObject(row,"blocked",info.blocked);
            cJSON *modes=cJSON_AddArrayToObject(row,"modes");
            for(int mode=-1;mode<=1;mode++)cJSON_AddItemToArray(modes,cJSON_CreateNumber(pd_check_mode(&s,kind,i,mode,false)));
            cJSON_AddItemToArray(rows,row);
        }
        char *encoded=cJSON_PrintUnformatted(rows);puts(encoded);cJSON_free(encoded);cJSON_Delete(rows);
    }
}
