#include "history_store.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
static nvs_handle_t history_handle;
static bool history_blocked=true,history_open;
static int history_bank=-1;
static uint32_t history_generation,history_epoch,history_sequence;
static uint32_t history_high_epoch;
static bool restore_ready;
static int restore_bank;
static uint32_t restore_generation,restore_epoch,restore_sequence;
static const char *history_status="Storico non disponibile";
static const char *const history_keys[]={"hist_a","hist_b"};
static bool known_crc(const uint8_t *data,size_t size){
    if(size<8)return false;
    const uint8_t *p=data+size-4;uint32_t crc=(uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
    return pd_crc32(data,size-4)==crc;
}
static size_t storage_encode(const pd_history *h,uint32_t generation,uint8_t *out){
    /* v2 keeps the payload, but requires epoch-aware recovery. Older firmware
       must retain the unknown bank rather than overwrite a prepared restore. */
    size_t size=pd_history_encode(h,generation,out);if(!size)return 0;
    out[3]='2';uint32_t crc=pd_crc32(out,size-4);for(unsigned i=0;i<4;i++)out[size-4+i]=crc>>(8*i);return size;
}
bool pd_history_store_load(uint32_t epoch,pd_history *out){
    if(history_open){nvs_close(history_handle);history_open=false;}
    restore_ready=false;history_high_epoch=epoch;history_sequence=0;pd_history_reset(out,epoch);history_epoch=epoch;history_blocked=true;history_bank=-1;history_generation=0;history_status="Storico non disponibile";
    if(nvs_flash_init_partition("pdd_state")!=ESP_OK||nvs_open_from_partition("pdd_state","pdd_history",NVS_READWRITE,&history_handle)!=ESP_OK)return false;
    history_open=true;
    uint8_t *data=malloc(2*PD_HISTORY_RECORD);pd_history *decoded=malloc(2*sizeof *decoded);
    if(!data||!decoded){free(data);free(decoded);return false;}
    bool valid[2]={false,false},missing[2]={false,false},ok=false;uint32_t generations[2]={0,0};
    for(unsigned i=0;i<2;i++){
        size_t size=PD_HISTORY_RECORD;uint8_t *record=data+i*PD_HISTORY_RECORD;
        esp_err_t error=nvs_get_blob(history_handle,history_keys[i],record,&size);
        missing[i]=error==ESP_ERR_NVS_NOT_FOUND;if(missing[i])continue;
        if(error!=ESP_OK||size>PD_HISTORY_RECORD)goto done;
        if(known_crc(record,size)&&memcmp(record,"PDH1",4)&&memcmp(record,"PDH2",4)){history_status="Versione storico futura";goto done;}
        valid[i]=pd_history_decode(record,size,decoded+i,generations+i);
        if(valid[i]&&decoded[i].epoch>history_high_epoch)history_high_epoch=decoded[i].epoch;
    }
    if(!valid[0]&&!valid[1]){
        if(!missing[0]||!missing[1])goto done;
        history_status="Storico vuoto";ok=true;goto done;
    }
    history_bank=valid[0]&&(!valid[1]||(int32_t)(generations[0]-generations[1])>0)?0:1;
    history_generation=generations[history_bank];
    /* A prepared restore belongs to a different epoch until state commits.
       Keep the newest matching bank instead of hiding the old character's log. */
    int other=1-history_bank;
    if(decoded[history_bank].epoch!=epoch&&valid[other]&&decoded[other].epoch==epoch)history_bank=other;
    if(decoded[history_bank].epoch==epoch)*out=decoded[history_bank];
    history_status=(!valid[1-history_bank]&&!missing[1-history_bank])?"Storico recuperato":out->count?"Storico salvato":"Storico vuoto";ok=true;
done:
    free(data);free(decoded);history_blocked=!ok;history_sequence=out->sequence;return ok;
}
bool pd_history_store_save(const pd_history *h){
    restore_ready=false;
    if(history_blocked||!pd_history_valid(h)||h->epoch!=history_epoch||history_sequence==UINT32_MAX||h->sequence!=history_sequence+1)return false;
    uint8_t *data=malloc(2*PD_HISTORY_RECORD);
    if(!data){history_status="Storico non salvato";return false;}
    size_t encoded=storage_encode(h,history_generation+1,data),size=encoded;int next=history_bank==0?1:0;
    bool ok=encoded&&nvs_set_blob(history_handle,history_keys[next],data,encoded)==ESP_OK&&nvs_commit(history_handle)==ESP_OK&&
        nvs_get_blob(history_handle,history_keys[next],data+PD_HISTORY_RECORD,&size)==ESP_OK&&size==encoded&&!memcmp(data,data+PD_HISTORY_RECORD,size);
    free(data);
    if(!ok){history_status="Storico non salvato";return false;}
    history_bank=next;history_generation++;history_sequence=h->sequence;history_status="Storico salvato";return true;
}
const char *pd_history_store_status(void){return history_status;}
bool pd_history_store_available(void){return !history_blocked;}
bool pd_history_store_next_epoch(uint32_t current,uint32_t *next){
    uint32_t highest=current>history_high_epoch?current:history_high_epoch;
    if(history_blocked||!next||highest==UINT32_MAX)return false;
    *next=highest+1;return true;
}
bool pd_history_store_prepare_restore(const pd_history *h){
    restore_ready=false;
    if(history_blocked||!pd_history_valid(h)||h->epoch==history_epoch)return false;
    if(h->epoch>history_high_epoch)history_high_epoch=h->epoch;
    uint8_t *data=malloc(2*PD_HISTORY_RECORD);if(!data)return false;
    size_t encoded=storage_encode(h,history_generation+1,data),size=encoded;int next=history_bank==0?1:0;
    bool ok=encoded&&nvs_set_blob(history_handle,history_keys[next],data,encoded)==ESP_OK&&nvs_commit(history_handle)==ESP_OK&&
        nvs_get_blob(history_handle,history_keys[next],data+PD_HISTORY_RECORD,&size)==ESP_OK&&size==encoded&&!memcmp(data,data+PD_HISTORY_RECORD,size);
    free(data);if(!ok)return false;
    restore_ready=true;restore_bank=next;restore_generation=history_generation+1;restore_epoch=h->epoch;restore_sequence=h->sequence;return true;
}
void pd_history_store_finish_restore(void){
    assert(restore_ready);history_bank=restore_bank;history_generation=restore_generation;history_epoch=restore_epoch;history_sequence=restore_sequence;
    history_status=history_sequence?"Storico salvato":"Storico vuoto";restore_ready=false;
}
void pd_history_store_cancel_restore(void){restore_ready=false;}
void pd_history_store_uncertain_restore(void){restore_ready=false;history_blocked=true;history_status="Ripristino incerto: riavvia";}
