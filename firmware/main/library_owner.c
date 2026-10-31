#include "library_owner.h"
#include <stdbool.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_random.h"
uint64_t pd_library_owner(void){
    static uint64_t owner;
    static bool failed;
    if(owner||failed)return owner;
    nvs_handle_t handle;failed=true;
    if(nvs_flash_init_partition("pdd_state")!=ESP_OK||nvs_open_from_partition("pdd_state","pdd_library",NVS_READWRITE,&handle)!=ESP_OK)return 0;
    uint64_t candidate=0;size_t size=sizeof candidate;esp_err_t r=nvs_get_blob(handle,"owner",&candidate,&size);
    bool ok=r==ESP_OK&&size==sizeof candidate&&candidate;
    if(r==ESP_ERR_NVS_NOT_FOUND){
        candidate=(uint64_t)esp_random()<<32|esp_random();
        if(candidate){uint64_t verify=0;size=sizeof verify;ok=nvs_set_blob(handle,"owner",&candidate,sizeof candidate)==ESP_OK&&nvs_commit(handle)==ESP_OK&&nvs_get_blob(handle,"owner",&verify,&size)==ESP_OK&&size==sizeof verify&&verify==candidate;}
    }
    nvs_close(handle);if(ok){owner=candidate;failed=false;}return owner;
}
