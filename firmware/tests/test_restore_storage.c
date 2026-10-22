/* Both production NVS stores, with cuts at each write/commit/reread boundary. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../main/storage.c"
#include "../main/history_store.c"
static uint8_t blobs[3][2][PD_RECORD_SIZE];static size_t lengths[3][2];
static unsigned operations,fail_at;static bool armed;
static unsigned key_index(const char *key){assert(!strcmp(key,"state_a")||!strcmp(key,"state_b")||!strcmp(key,"hist_a")||!strcmp(key,"hist_b"));return !strcmp(key,"state_b")||!strcmp(key,"hist_b");}
static bool cut(void){return armed&&++operations==fail_at;}
esp_err_t nvs_flash_init_partition(const char *p){assert(!strcmp(p,"pdd_state")||!strcmp(p,"pdd_nvs"));return ESP_OK;}
esp_err_t nvs_open_from_partition(const char *p,const char *ns,int mode,nvs_handle_t *h){
    if(!strcmp(p,"pdd_nvs")){assert(mode==NVS_READONLY);return ESP_ERR_NVS_NOT_FOUND;}
    assert(mode==NVS_READWRITE);*h=!strcmp(ns,"pdd_history")?2:1;return ESP_OK;
}
void nvs_close(nvs_handle_t h){(void)h;}
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *size){
    if(cut())return ESP_FAIL;
    unsigned i=key_index(key);if(!lengths[h][i])return ESP_ERR_NVS_NOT_FOUND;if(*size<lengths[h][i])return ESP_ERR_NVS_INVALID_LENGTH;
    *size=lengths[h][i];memcpy(out,blobs[h][i],*size);return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *data,size_t size){
    unsigned i=key_index(key);bool failed=cut();lengths[h][i]=failed?size/2:size;memcpy(blobs[h][i],data,lengths[h][i]);return failed?ESP_FAIL:ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t h){(void)h;return cut()?ESP_FAIL:ESP_OK;}
static void boot(void){blocked=false;bank=-1;generation=0;status="Demo: non salvata";history_open=false;}
static void same(const pd_state *a,const pd_state *b){static uint8_t left[PD_RECORD_SIZE],right[PD_RECORD_SIZE];size_t n=pd_encode(a,0,left);assert(n==pd_encode(b,0,right)&&!memcmp(left,right,n));}
int main(void){
    static pd_state old,next,out;static pd_history old_log,new_log,log;
    for(unsigned failure=0;failure<=6;failure++){
        armed=false;memset(lengths,0,sizeof lengths);boot();assert(pd_storage_load(&old));old.history_epoch=7;old.shortcuts[0]=4;assert(pd_storage_save(&old));
        assert(pd_history_store_load(7,&old_log));pd_history_entry e={.kind=PD_H_FREE,.count=1,.sides=20,.dice={20},.total=20,.label="Prima del ripristino"};assert(pd_history_append(&old_log,&e)&&pd_history_store_save(&old_log));
        next=old;next.hp=5;next.history_epoch=8;next.shortcuts[0]=13;next.wallet=(pd_wallet){.available=true,.coins={1,2,3,4,5}};
        new_log=old_log;new_log.epoch=8;e.dice[0]=1;e.total=1;assert(pd_history_append(&new_log,&e));
        unsigned state_bank=(unsigned)bank,log_bank=(unsigned)history_bank;static uint8_t old_state[PD_RECORD_SIZE],old_history[PD_HISTORY_RECORD];size_t old_n=lengths[1][state_bank],old_m=lengths[2][log_bank];memcpy(old_state,blobs[1][state_bank],old_n);memcpy(old_history,blobs[2][log_bank],old_m);
        operations=0;fail_at=failure;armed=true;bool prepared=pd_history_store_prepare_restore(&new_log),saved=false;
        if(prepared){saved=pd_storage_save(&next);if(saved)pd_history_store_finish_restore();else {
            pd_history_store_uncertain_restore();pd_history_store_cancel_restore();assert(!pd_history_store_available());
            pd_history attempt=old_log;assert(pd_history_append(&attempt,&e)&&!pd_history_store_save(&attempt));
        }}
        assert(saved==(failure==0));assert(!memcmp(old_state,blobs[1][state_bank],old_n)&&!memcmp(old_history,blobs[2][log_bank],old_m));
        /* An unacknowledged write may already be durable. Either complete pair is OK. */
        armed=false;boot();assert(pd_storage_load(&out)&&pd_history_store_load(out.history_epoch,&log));
        if(out.history_epoch==7){same(&out,&old);assert(log.count==1&&log.entries[0].total==20&&log.epoch==7);}
        else {assert(out.history_epoch==8);same(&out,&next);assert(log.count==2&&log.entries[0].total==1&&log.entries[1].total==20&&log.epoch==8);}
    }
    /* Cancelling a successful prepare retains the current log across reboot. */
    assert(pd_history_store_prepare_restore(&old_log));pd_history_store_cancel_restore();boot();assert(pd_storage_load(&out)&&out.history_epoch==8&&pd_history_store_load(8,&log)&&log.count==2);
    pd_history_entry e={.kind=PD_H_FREE,.count=1,.sides=20,.dice={17},.total=17,.label="Dopo annullamento"};assert(pd_history_append(&log,&e)&&pd_history_store_save(&log));
    boot();assert(pd_storage_load(&out)&&pd_history_store_load(8,&log)&&log.count==3&&log.entries[0].total==17);
    /* An abandoned prepared epoch must never be reused by a later USB import. */
    uint32_t next_epoch=0;assert(pd_history_store_next_epoch(out.history_epoch,&next_epoch)&&next_epoch>out.history_epoch);
    pd_history abandoned=log;abandoned.epoch=next_epoch;assert(pd_history_store_prepare_restore(&abandoned));pd_history_store_cancel_restore();
    boot();assert(pd_storage_load(&out)&&pd_history_store_load(out.history_epoch,&log));uint32_t fresh_epoch;
    assert(pd_history_store_next_epoch(out.history_epoch,&fresh_epoch)&&fresh_epoch>abandoned.epoch);
    out.history_epoch=fresh_epoch;assert(pd_storage_save(&out)&&pd_history_store_load(fresh_epoch,&log)&&!log.count&&log.epoch==fresh_epoch);
    boot();assert(pd_storage_load(&out)&&pd_history_store_load(out.history_epoch,&log)&&!log.count);
    puts("Native restore NVS: all six set/commit/reread cuts, coherent old/new state-history pairs, complete field equality, old banks retained, cancelled prepare, subsequent rolls and no identity reuse on later imports passed");
}
