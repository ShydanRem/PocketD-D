/* Run the real storage adapter against an in-memory NVS, including failures. */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../main/storage.c"
#include "fixtures/state-v1.h"
#include "fixtures/state-v2.h"
static uint8_t blobs[2][PD_RECORD_SIZE];static size_t sizes[2];
static int fail_init,fail_set,fail_commit,fail_read,set_calls;
static int index_of(const char *key) { return !strcmp(key,"state_a")?0:1; }
esp_err_t nvs_flash_init_partition(const char *name) { assert(!strcmp(name,"pdd_nvs"));return fail_init?ESP_FAIL:ESP_OK; }
esp_err_t nvs_open_from_partition(const char *p,const char *ns,int mode,nvs_handle_t *h) { *h=1;return ESP_OK; }
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *data,size_t *size) {
    if (fail_read) return ESP_FAIL;
    int i=index_of(key);if(!sizes[i])return ESP_ERR_NVS_NOT_FOUND;
    if(*size<sizes[i])return ESP_ERR_NVS_INVALID_LENGTH;
    *size=sizes[i];memcpy(data,blobs[i],*size);return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *data,size_t size) {
    set_calls++;if(fail_set)return ESP_FAIL;
    int i=index_of(key);sizes[i]=size;memcpy(blobs[i],data,size);return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t h) { return fail_commit?ESP_FAIL:ESP_OK; }
static void reboot(void) { blocked=false;bank=-1;generation=0;status="Demo: non salvata"; }
int main(void) {
    pd_state s;
    memcpy(blobs[0],legacy_v1,sizeof legacy_v1);sizes[0]=sizeof legacy_v1;
    assert(pd_storage_load(&s)&&s.hp==17&&s.temp_hp==4&&s.idle_animation&&!set_calls);
    s.hp=16;s.avatar=(pd_avatar){1,2,1,2};s.idle_animation=false;
    assert(pd_storage_save(&s));assert(!memcmp(blobs[0],legacy_v1,sizeof legacy_v1));
    reboot();assert(pd_storage_load(&s)&&s.hp==16&&s.avatar.skin==1&&s.avatar.style==2&&!s.idle_animation);
    blobs[1][40]^=1;reboot();assert(pd_storage_load(&s)&&s.hp==17&&s.idle_animation);
    memcpy(blobs[1],legacy_v2,sizeof legacy_v2);sizes[1]=sizeof legacy_v2;
    reboot();int unchanged=set_calls;assert(pd_storage_load(&s)&&!s.idle_animation&&s.avatar.skin==2&&!s.training.available&&set_calls==unchanged);
    s.training.available=true;s.training.ranks[0]=2;s.training.skill_bonus[0]=-3;
    assert(pd_storage_save(&s));assert(!memcmp(blobs[1],legacy_v2,sizeof legacy_v2));
    reboot();assert(pd_storage_load(&s)&&s.training.available&&s.training.ranks[0]==2&&s.training.skill_bonus[0]==-3);
    memset(sizes,0,sizeof sizes);set_calls=0;reboot();
    assert(pd_storage_load(&s)&&s.hp==24);
    s.hp=23;assert(pd_storage_save(&s));uint8_t first[PD_RECORD_SIZE];memcpy(first,blobs[0],sizeof first);
    s.hp=22;assert(pd_storage_save(&s));assert(!memcmp(first,blobs[0],sizeof first));
    reboot();assert(pd_storage_load(&s)&&s.hp==22);
    blobs[1][40]^=1;reboot();assert(pd_storage_load(&s)&&s.hp==23);
    assert(!strcmp(pd_storage_status(),"Recuperata copia valida"));
    s.hp=21;assert(pd_storage_save(&s));assert(!memcmp(first,blobs[0],sizeof first));
    fail_set=1;s.hp=20;assert(!pd_storage_save(&s));fail_set=0;
    int calls=set_calls;assert(!pd_storage_save(&s)&&set_calls==calls);
    reboot();assert(pd_storage_load(&s)&&s.hp==21);
    fail_commit=1;s.hp=19;assert(!pd_storage_save(&s));assert(blocked);fail_commit=0;
    /* A reported I/O error can be ambiguous. Block further writes until reboot;
       a fully valid newer record may already exist, but old bank is preserved. */
    reboot();assert(pd_storage_load(&s)&&s.hp==19);
    /* A valid record from a future format must never be overwritten. */
    pd_encode(&s,100,blobs[1]);blobs[1][3]='4';
    uint32_t crc=pd_crc32(blobs[1],PD_RECORD_SIZE-4);
    for(int i=0;i<4;i++)blobs[1][PD_RECORD_SIZE-4+i]=crc>>(8*i);
    reboot();assert(!pd_storage_load(&s)&&blocked);
    blobs[0][0]=0;blobs[1][0]=0;reboot();assert(!pd_storage_load(&s));
    calls=set_calls;assert(!pd_storage_save(&s)&&set_calls==calls);
    fail_init=1;reboot();assert(!pd_storage_load(&s));fail_init=0;
    fail_read=1;reboot();assert(!pd_storage_load(&s));
    puts("NVS adapter: reboot, rollback, corrupt banks and I/O errors passed; no auto erase");
    return 0;
}
