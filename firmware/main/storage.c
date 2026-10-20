#include "storage.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <string.h>
static nvs_handle_t handle;
static bool blocked;
static int bank=-1;
static uint32_t generation;
static const char *status="Demo: non salvata";
static const char *keys[]={"state_a","state_b"};
static bool failure(void) { blocked=true; status="Errore memoria: riavvia"; return false; }
bool pd_storage_load(pd_state *s) {
    /* No erase/reformat fallback: preserve even data from an unknown newer version. */
    if (nvs_flash_init_partition("pdd_nvs")!=ESP_OK ||
        nvs_open_from_partition("pdd_nvs","pocketdd",NVS_READWRITE,&handle)!=ESP_OK) return failure();
    uint8_t data[2][PD_RECORD_SIZE]; size_t size[2]={PD_RECORD_SIZE,PD_RECORD_SIZE};
    esp_err_t errors[2];
    for (int i=0;i<2;i++) {
        errors[i]=nvs_get_blob(handle,keys[i],data[i],&size[i]);
        if (errors[i]!=ESP_OK) size[i]=0;
    }
    for (int i=0;i<2;i++) if(size[i]>=8) {
        const uint8_t *tail=data[i]+size[i]-4;
        uint32_t crc=(uint32_t)tail[0]|((uint32_t)tail[1]<<8)|((uint32_t)tail[2]<<16)|((uint32_t)tail[3]<<24);
        if(pd_crc32(data[i],size[i]-4)==crc&&!pd_record_known(data[i],size[i]))return failure();
    }
    bank=pd_select(data[0],size[0],data[1],size[1],s,&generation);
    bool absent=errors[0]==ESP_ERR_NVS_NOT_FOUND && errors[1]==ESP_ERR_NVS_NOT_FOUND;
    if (bank<0) {
        if (!absent) return failure();
        pd_demo(s); return true;
    }
    for (int i=0;i<2;i++) if (errors[i]!=ESP_OK && errors[i]!=ESP_ERR_NVS_NOT_FOUND) return failure();
    pd_state spare; uint32_t seq;
    status=pd_decode(data[1-bank],size[1-bank],&spare,&seq)||errors[1-bank]==ESP_ERR_NVS_NOT_FOUND ?
        "Salvato offline" : "Recuperata copia valida";
    return true;
}
bool pd_storage_save(const pd_state *s) {
    if (blocked || !pd_valid(s)) return false;
    int next=bank==0?1:0;
    uint8_t data[PD_RECORD_SIZE],verify[PD_RECORD_SIZE]; size_t size=sizeof verify;
    pd_encode(s,generation+1,data);
    if (nvs_set_blob(handle,keys[next],data,sizeof data)!=ESP_OK || nvs_commit(handle)!=ESP_OK ||
        nvs_get_blob(handle,keys[next],verify,&size)!=ESP_OK || size!=sizeof data || memcmp(data,verify,size)) return failure();
    bank=next; generation++; status="Salvato offline"; return true;
}
const char *pd_storage_status(void) { return status; }
