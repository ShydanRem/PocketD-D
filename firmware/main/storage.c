#include "storage.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <string.h>
#include <stdlib.h>
static nvs_handle_t handle;
static bool blocked;
static int bank=-1;
static uint32_t generation;
static const char *status="Demo: non salvata";
static const char *keys[]={"state_a","state_b"};
static bool failure(void) {blocked=true;status="Errore memoria: riavvia";return false;}
/* Return 0 for an empty partition, 1 for a valid state, -1 for unsafe data/I/O. */
static int load_banks(nvs_handle_t source,pd_state *s) {
    uint8_t *data=malloc(2*PD_RECORD_SIZE);pd_state *spare=malloc(sizeof *spare);
    if(!data||!spare){free(data);free(spare);return -1;}
    size_t size[2]={PD_RECORD_SIZE,PD_RECORD_SIZE};esp_err_t errors[2];int result=-1;
    for(int i=0;i<2;i++) {
        errors[i]=nvs_get_blob(source,keys[i],data+i*PD_RECORD_SIZE,&size[i]);
        if(errors[i]!=ESP_OK)size[i]=0;
        if(errors[i]!=ESP_OK&&errors[i]!=ESP_ERR_NVS_NOT_FOUND)goto done;
        if(size[i]>=8) {
            const uint8_t *tail=data+i*PD_RECORD_SIZE+size[i]-4;
            uint32_t crc=(uint32_t)tail[0]|((uint32_t)tail[1]<<8)|((uint32_t)tail[2]<<16)|((uint32_t)tail[3]<<24);
            if(pd_crc32(data+i*PD_RECORD_SIZE,size[i]-4)==crc&&!pd_record_known(data+i*PD_RECORD_SIZE,size[i]))goto done;
        }
    }
    if(errors[0]==ESP_ERR_NVS_NOT_FOUND&&errors[1]==ESP_ERR_NVS_NOT_FOUND){result=0;goto done;}
    bank=pd_select(data,size[0],data+PD_RECORD_SIZE,size[1],s,&generation);
    if(bank<0)goto done;
    uint32_t seq;
    status=pd_decode(data+(1-bank)*PD_RECORD_SIZE,size[1-bank],spare,&seq)||errors[1-bank]==ESP_ERR_NVS_NOT_FOUND?"Salvato offline":"Recuperata copia valida";
    result=1;
done:
    free(data);free(spare);return result;
}
bool pd_storage_load(pd_state *s) {
    /* New partition leaves the v0.15-v0.21 layout and legacy records untouched. */
    esp_err_t initialized=nvs_flash_init_partition("pdd_state");
    bool current=initialized==ESP_OK;
    if(!current&&initialized!=ESP_ERR_NOT_FOUND)return failure();
    if(current){
        if(nvs_open_from_partition("pdd_state","pocketdd",NVS_READWRITE,&handle)!=ESP_OK)return failure();
        int found=load_banks(handle,s);if(found<0)return failure();if(found)return true;
    }
    nvs_handle_t legacy;int old=0;
    if(nvs_flash_init_partition("pdd_nvs")!=ESP_OK)return failure();
    esp_err_t opened=nvs_open_from_partition("pdd_nvs","pocketdd",NVS_READONLY,&legacy);
    if(opened==ESP_OK){old=load_banks(legacy,s);nvs_close(legacy);}
    else if(opened!=ESP_ERR_NVS_NOT_FOUND)return failure();
    if(old<0)return failure();
    if(!old)pd_demo(s);
    bank=-1;
    if(!current){blocked=true;status="Layout da aggiornare";return false;}
    status=old?"Copia precedente conservata":"Demo: non salvata";return true;
}
bool pd_storage_save(const pd_state *s) {
    if(blocked||!pd_valid(s))return false;
    size_t encoded=pd_encoded_size(s);uint8_t *data=malloc(2*encoded);if(!data)return failure();
    uint8_t *verify=data+encoded;size_t size=encoded;int next=bank==0?1:0;
    pd_encode(s,generation+1,data);
    bool ok=nvs_set_blob(handle,keys[next],data,encoded)==ESP_OK&&nvs_commit(handle)==ESP_OK&&
        nvs_get_blob(handle,keys[next],verify,&size)==ESP_OK&&size==encoded&&!memcmp(data,verify,size);
    free(data);if(!ok)return failure();
    bank=next;generation++;status="Salvato offline";return true;
}
const char *pd_storage_status(void){return status;}
