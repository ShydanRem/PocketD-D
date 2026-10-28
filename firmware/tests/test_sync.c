#include "sync_config.h"
#include "sync_snapshot.h"
#include "cJSON.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static pd_sync_sample s;
static void set(cJSON *o,const char *key,cJSON *value){cJSON_ReplaceItemInObjectCaseSensitive(o,key,value);}
int main(int argc,char **argv){
 assert(argc==2);FILE *f=fopen(argv[1],"r");assert(f);char json[4096];size_t n=fread(json,1,sizeof json-1,f);fclose(f);json[n]=0;
 pd_sync_config c,before;assert(pd_sync_config_parse(json,n,&c));before=c;
 for(unsigned i=0;i<12;i++){cJSON *o=cJSON_Parse(json);assert(o);switch(i){case 0:set(o,"host",cJSON_CreateString("127.0.0.1"));break;case 1:set(o,"host",cJSON_CreateString("example.com"));break;case 2:set(o,"host",cJSON_CreateString("192.168.01.1"));break;case 3:set(o,"port",cJSON_CreateNumber(65536));break;case 4:set(o,"time",cJSON_CreateNumber(0));break;case 5:set(o,"characterId",cJSON_CreateString("0"));break;case 6:set(o,"token",cJSON_CreateString("wrong"));break;case 7:set(o,"version",cJSON_CreateNumber(2));break;case 8:cJSON_AddStringToObject(o,"id","aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");break;case 9:set(o,"certificate",cJSON_CreateString("not a certificate"));break;case 10:set(o,"id",cJSON_CreateArray());break;case 11:cJSON_AddNumberToObject(o,"extra",1);break;}
  char *bad=cJSON_PrintUnformatted(o);assert(bad&&!pd_sync_config_parse(bad,strlen(bad),&c)&&!memcmp(&c,&before,sizeof c));free(bad);cJSON_Delete(o);
 }
 const char *nested="{\"x\":[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]]]]]]]] }";assert(!pd_sync_config_parse(nested,strlen(nested),&c));
 pd_history_reset(&s.history,5);s.epoch=5;s.hp=20;s.max=30;s.temp=7;strcpy(s.name,"Native bridge synthetic");
 for(unsigned i=0;i<32;i++){
  if(i){pd_history_entry e={.kind=i%11,.count=2,.sides=20,.dice={3,17},.mode=(int)(i%3)-1,.bonus=2};if(!e.mode)e.total=22;else e.total=e.mode>0?19:5;strcpy(e.label,"Frozen native roll");assert(pd_history_append(&s.history,&e));}
  char *encoded=pd_sync_encode(&s,i+1,"0123456789abcdef");assert(encoded);puts(encoded);free(encoded);
 }
 fprintf(stderr,"Sync native: valid private-IP/CA configuration, twelve unchanged-output failures, bounded malformed JSON and 32 persisted-history snapshots passed; no physical TLS/flash claimed\n");
}
