/* Two NVS partitions in memory: legacy data must never be overwritten by migration. */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../main/storage.c"
#include "fixtures/state-v1.h"
#include "fixtures/state-v2.h"
#include "fixtures/state-v3.h"
#include "fixtures/state-v4.h"
#include "fixtures/state-v5.h"
#include "fixtures/state-v6.h"
static uint8_t blobs[2][2][PD_RECORD_SIZE];static size_t sizes[2][2];
static int fail_init,fail_set,fail_commit,fail_read,set_calls,missing_layout;
static int index_of(const char *key){return !strcmp(key,"state_a")?0:1;}
esp_err_t nvs_flash_init_partition(const char *name){assert(!strcmp(name,"pdd_nvs")||!strcmp(name,"pdd_state"));if(fail_init)return ESP_FAIL;return missing_layout&&!strcmp(name,"pdd_state")?ESP_ERR_NOT_FOUND:ESP_OK;}
esp_err_t nvs_open_from_partition(const char *p,const char *ns,int mode,nvs_handle_t *h){*h=!strcmp(p,"pdd_state")?1:0;assert(mode==(*h?NVS_READWRITE:NVS_READONLY));return ESP_OK;}
void nvs_close(nvs_handle_t h){(void)h;}
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *data,size_t *size){
 if(fail_read)return ESP_FAIL;
 int i=index_of(key);if(!sizes[h][i])return ESP_ERR_NVS_NOT_FOUND;
 if(*size<sizes[h][i])return ESP_ERR_NVS_INVALID_LENGTH;
 *size=sizes[h][i];memcpy(data,blobs[h][i],*size);return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *data,size_t size){
 set_calls++;assert(h==1);if(fail_set)return ESP_FAIL;int i=index_of(key);sizes[h][i]=size;memcpy(blobs[h][i],data,size);return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t h){return fail_commit?ESP_FAIL:ESP_OK;}
static void reboot(void){blocked=false;bank=-1;generation=0;status="Demo: non salvata";}
int main(void){
 const uint8_t *golden[]={legacy_v1,legacy_v2,legacy_v3,legacy_v4};const size_t lengths[]={sizeof legacy_v1,sizeof legacy_v2,sizeof legacy_v3,sizeof legacy_v4};pd_state s;
 for(unsigned k=0;k<4;k++){
  memset(sizes,0,sizeof sizes);memcpy(blobs[0][0],golden[k],lengths[k]);sizes[0][0]=lengths[k];reboot();int unchanged=set_calls;
  assert(pd_storage_load(&s)&&s.hp==17&&s.temp_hp==4&&!s.magic.available&&set_calls==unchanged);
  s.hp=16;assert(pd_storage_save(&s));assert(!memcmp(blobs[0][0],golden[k],lengths[k]));
  reboot();assert(pd_storage_load(&s)&&s.hp==16&&!s.magic.available);
 }
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v5,sizeof golden_v5);sizes[1][0]=sizeof golden_v5;reboot();int v5_calls=set_calls;
 assert(pd_storage_load(&s)&&s.magic.concentration==1&&s.shortcuts[1]==6&&!s.recovery.available&&set_calls==v5_calls);
 s.recovery=(pd_recovery){.available=true,.count=1,.pools={{.name="Classe",.die=8,.maximum=3,.used=2}}};assert(pd_storage_save(&s));
 assert(!memcmp(blobs[1][0],golden_v5,sizeof golden_v5));reboot();assert(pd_storage_load(&s)&&s.recovery.pools[0].used==2&&s.magic.concentration==1&&s.shortcuts[1]==6);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v6,sizeof golden_v6);sizes[1][0]=sizeof golden_v6;reboot();int v6_calls=set_calls;
 assert(pd_storage_load(&s)&&s.hp==9&&s.recovery.pools[0].used==2&&s.shortcuts[0]==7&&!s.status.available&&set_calls==v6_calls);
 s.status=(pd_status){.available=true,.count=1,.successes=1,.failures=2,.conditions={{.name="Nota sintetica",.level=2}}};assert(pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v6,sizeof golden_v6));
 reboot();assert(pd_storage_load(&s)&&s.status.successes==1&&s.status.failures==2&&s.status.conditions[0].level==2&&s.recovery.pools[0].used==2&&s.shortcuts[0]==7);
 memset(sizes,0,sizeof sizes);memcpy(blobs[0][0],legacy_v4,sizeof legacy_v4);sizes[0][0]=sizeof legacy_v4;missing_layout=1;reboot();
 assert(!pd_storage_load(&s)&&s.hp==17&&blocked&&!strcmp(status,"Layout da aggiornare"));int calls=set_calls;assert(!pd_storage_save(&s)&&set_calls==calls);missing_layout=0;
 memset(sizes,0,sizeof sizes);reboot();assert(pd_storage_load(&s)&&s.hp==24&&s.magic.available);s.hp=23;assert(pd_storage_save(&s));
 static uint8_t first[PD_RECORD_SIZE];memcpy(first,blobs[1][0],sizeof first);s.hp=22;assert(pd_storage_save(&s));assert(!memcmp(first,blobs[1][0],sizeof first));
 reboot();assert(pd_storage_load(&s)&&s.hp==22);blobs[1][1][40]^=1;reboot();assert(pd_storage_load(&s)&&s.hp==23&&!strcmp(status,"Recuperata copia valida"));
 s.hp=21;assert(pd_storage_save(&s));assert(!memcmp(first,blobs[1][0],sizeof first));
 fail_set=1;s.hp=20;assert(!pd_storage_save(&s));fail_set=0;calls=set_calls;assert(!pd_storage_save(&s)&&calls==set_calls);reboot();assert(pd_storage_load(&s)&&s.hp==21);
 fail_commit=1;s.hp=19;assert(!pd_storage_save(&s)&&blocked);fail_commit=0;reboot();assert(pd_storage_load(&s)&&s.hp==19);
 sizes[1][1]=pd_encode(&s,100,blobs[1][1]);blobs[1][1][3]='8';uint32_t crc=pd_crc32(blobs[1][1],sizes[1][1]-4);for(int i=0;i<4;i++)blobs[1][1][sizes[1][1]-4+i]=crc>>(8*i);
 reboot();assert(!pd_storage_load(&s)&&blocked);blobs[1][0][0]=0;blobs[1][1][0]=0;reboot();assert(!pd_storage_load(&s));
 fail_init=1;reboot();assert(!pd_storage_load(&s));fail_init=0;fail_read=1;reboot();assert(!pd_storage_load(&s));
 puts("NVS: new/legacy partitions, golden v1..v6 migration without boot writes, missing layout, alternate banks, errors, rollback and future-version preservation passed");
}
