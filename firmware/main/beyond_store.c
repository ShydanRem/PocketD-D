#include "beyond_store.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdlib.h>
#include <string.h>
/* Dedicated namespace: never part of a PG, SD snapshot, diagnostics or PG USB
backup. This firmware does NOT enable flash/NVS encryption or burn eFuses. */
static bool open_store(nvs_handle_t *h,nvs_open_mode_t mode){return nvs_flash_init_partition("pdd_nvs")==ESP_OK&&nvs_open_from_partition("pdd_nvs","pdd_beyond",mode,h)==ESP_OK;
}
bool pd_account_store_load(pd_account_access *out){
    nvs_handle_t h;
    if(!out||!open_store(&h,NVS_READONLY))return false;
    pd_account_access *a=calloc(1,sizeof *a);
    size_t size=sizeof *a;
    bool ok=a&&nvs_get_blob(h,"access",a,&size)==ESP_OK&&size==sizeof *a&&pd_account_valid(a);
    nvs_close(h);
    if(ok)*out=*a;
    if(a){pd_account_wipe(a,sizeof *a);
        free(a);
    }return ok;
}
bool pd_account_store_save(const pd_account_access *access){
    if(!pd_account_valid(access))return false;
    nvs_handle_t h;
    if(!open_store(&h,NVS_READWRITE))return false;
    esp_err_t e=nvs_set_blob(h,"access",access,sizeof *access);
    if(e==ESP_OK)e=nvs_commit(h);
    nvs_close(h);
    pd_account_access *check=calloc(1,sizeof *check);
    bool ok=e==ESP_OK&&check&&pd_account_store_load(check)&&!memcmp(check,access,sizeof *check);
    if(check){pd_account_wipe(check,sizeof *check);
        free(check);
    }return ok;
}
bool pd_account_store_forget(void){
    nvs_handle_t h;
    if(!open_store(&h,NVS_READWRITE))return false;
    esp_err_t e=nvs_erase_key(h,"access");
    if(e==ESP_ERR_NVS_NOT_FOUND)e=ESP_OK;
    if(e==ESP_OK)e=nvs_commit(h);
    nvs_close(h);
    if(e!=ESP_OK||!open_store(&h,NVS_READONLY))return false;
    size_t size=0;
    e=nvs_get_blob(h,"access",NULL,&size);
    nvs_close(h);
    return e==ESP_ERR_NVS_NOT_FOUND;
}
