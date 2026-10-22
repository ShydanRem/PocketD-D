/* Real history NVS implementation; isolated namespace, synthetic fault injection. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../main/history_store.c"
static uint8_t blobs[2][PD_HISTORY_RECORD+16];static size_t lengths[2];
static unsigned writes,commits,opens;static int fail_init,fail_open,fail_set,fail_commit,fail_read;
static unsigned key_index(const char *key){assert(!strcmp(key,"hist_a")||!strcmp(key,"hist_b"));return !strcmp(key,"hist_b");}
esp_err_t nvs_flash_init_partition(const char *partition){assert(!strcmp(partition,"pdd_state"));return fail_init?ESP_FAIL:ESP_OK;}
esp_err_t nvs_open_from_partition(const char *partition,const char *ns,int mode,nvs_handle_t *handle){assert(!strcmp(partition,"pdd_state")&&!strcmp(ns,"pdd_history")&&mode==NVS_READWRITE);if(fail_open)return ESP_FAIL;assert(!opens);opens++;*handle=7;return ESP_OK;}
void nvs_close(nvs_handle_t handle){assert(handle==7&&opens==1);opens--;}
esp_err_t nvs_get_blob(nvs_handle_t handle,const char *key,void *data,size_t *size){
    assert(handle==7);if(fail_read)return ESP_FAIL;unsigned bank=key_index(key);
    if(!lengths[bank])return ESP_ERR_NVS_NOT_FOUND;
    if(*size<lengths[bank])return ESP_ERR_NVS_INVALID_LENGTH;
    *size=lengths[bank];memcpy(data,blobs[bank],*size);return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t handle,const char *key,const void *data,size_t size){
    assert(handle==7&&size<=PD_HISTORY_RECORD);writes++;if(fail_set)return ESP_FAIL;
    unsigned bank=key_index(key);lengths[bank]=size;memcpy(blobs[bank],data,size);return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t handle){assert(handle==7);commits++;return fail_commit?ESP_FAIL:ESP_OK;}
static pd_history_entry entry(unsigned face){pd_history_entry e={.kind=PD_H_FREE,.count=1,.sides=20,.dice={face},.total=face};strcpy(e.label,"d20 libero");return e;}
static void fix_crc(uint8_t *p,size_t n){uint32_t crc=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=crc>>(8*i);}
int main(void){
    pd_history h,next,out;assert(pd_history_store_load(5,&h)&&!h.count&&h.epoch==5&&!writes);
    pd_history_entry e=entry(20);assert(pd_history_append(&h,&e)&&pd_history_store_save(&h)&&writes==1&&commits==1);
    unsigned first=history_bank;uint8_t original[PD_HISTORY_RECORD];size_t original_size=lengths[first];memcpy(original,blobs[first],original_size);
    assert(pd_history_store_load(5,&out)&&out.count==1&&out.entries[0].total==20&&writes==1);
    next=out;e=entry(1);assert(pd_history_append(&next,&e));
    fail_set=1;assert(!pd_history_store_save(&next));fail_set=0;assert(history_sequence==1&&!memcmp(original,blobs[first],original_size));
    fail_commit=1;assert(!pd_history_store_save(&next));fail_commit=0;assert(history_sequence==1&&!memcmp(original,blobs[first],original_size));
    fail_read=1;assert(!pd_history_store_save(&next));fail_read=0;assert(history_sequence==1);
    assert(pd_history_store_save(&next)&&history_sequence==2);assert(pd_history_store_load(5,&out)&&out.count==2&&out.entries[0].total==1&&out.entries[1].total==20);
    assert(!pd_history_store_save(&h));next=out;next.epoch++;assert(pd_history_append(&next,&e)&&!pd_history_store_save(&next));
    /* CRC damage recovers the prior committed bank, without a boot write. */
    unsigned saved_writes=writes;blobs[history_bank][20]^=1;
    assert(pd_history_store_load(5,&out)&&out.sequence==1&&writes==saved_writes&&!strcmp(pd_history_store_status(),"Storico recuperato"));
    /* Future valid formats are retained, even if an older bank is readable. */
    unsigned future=1-history_bank;lengths[future]=pd_history_encode(&h,99,blobs[future]);blobs[future][3]='2';fix_crc(blobs[future],lengths[future]);
    assert(!pd_history_store_load(5,&out)&&!out.count&&!pd_history_store_save(&h)&&writes==saved_writes);
    memset(lengths,0,sizeof lengths);assert(pd_history_store_load(5,&out));h=out;assert(pd_history_append(&h,&e)&&pd_history_store_save(&h));
    /* An import epoch starts empty; its first write leaves the previous bank. */
    first=history_bank;original_size=lengths[first];memcpy(original,blobs[first],original_size);
    saved_writes=writes;assert(pd_history_store_load(6,&out)&&!out.count&&out.epoch==6&&writes==saved_writes);
    next=out;e=entry(17);assert(pd_history_append(&next,&e)&&pd_history_store_save(&next));assert(!memcmp(original,blobs[first],original_size));
    assert(pd_history_store_load(6,&out)&&out.sequence==1&&out.entries[0].total==17);
    /* Wrap of the storage generation preserves the newest bank. */
    lengths[0]=pd_history_encode(&h,UINT32_MAX,blobs[0]);lengths[1]=pd_history_encode(&next,0,blobs[1]);
    assert(pd_history_store_load(6,&out)&&out.entries[0].total==17&&history_bank==1);
    /* No fallback to a different character's history on load errors. */
    fail_read=1;saved_writes=writes;assert(!pd_history_store_load(6,&out)&&!out.count&&!pd_history_store_save(&next));fail_read=0;
    fail_init=1;assert(!pd_history_store_load(6,&out));fail_init=0;fail_open=1;assert(!pd_history_store_load(6,&out));fail_open=0;
    lengths[0]=PD_HISTORY_RECORD+1;assert(!pd_history_store_load(6,&out)&&writes==saved_writes);
    memset(lengths,0,sizeof lengths);lengths[0]=8;memset(blobs[0],0,8);assert(!pd_history_store_load(6,&out)&&writes==saved_writes);
    puts("History NVS: namespace isolation, reboot, no boot writes, exact dice, bank recovery, frozen/idempotent retries, stale rejection, import epochs, future formats, generation wrap and I/O failures passed");
}
