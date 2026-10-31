/* Production install identity; independent function-local caches model reboots. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define pd_library_owner owner_first
#include "../main/library_owner.c"
#undef pd_library_owner
#define pd_library_owner owner_reboot
#include "../main/library_owner.c"
#undef pd_library_owner
#define pd_library_owner owner_reset
#include "../main/library_owner.c"
#undef pd_library_owner
#define pd_library_owner owner_uncertain
#include "../main/library_owner.c"
#undef pd_library_owner
#define pd_library_owner owner_uncertain_reboot
#include "../main/library_owner.c"
#undef pd_library_owner
static uint64_t stored;
static bool present,fail_commit;
static unsigned writes,randoms;
uint32_t esp_random(void){return 0x12340000+ ++randoms;}
esp_err_t nvs_flash_init_partition(const char *p){assert(!strcmp(p,"pdd_state"));return ESP_OK;}
esp_err_t nvs_open_from_partition(const char *p,const char *ns,int mode,nvs_handle_t *h){assert(!strcmp(p,"pdd_state")&&!strcmp(ns,"pdd_library")&&mode==NVS_READWRITE);*h=1;return ESP_OK;}
void nvs_close(nvs_handle_t h){assert(h==1);}
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *size){assert(h==1&&!strcmp(key,"owner")&&*size>=sizeof stored);if(!present)return ESP_ERR_NVS_NOT_FOUND;memcpy(out,&stored,sizeof stored);*size=sizeof stored;return ESP_OK;}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *data,size_t size){assert(h==1&&!strcmp(key,"owner")&&size==sizeof stored);memcpy(&stored,data,size);present=true;writes++;return ESP_OK;}
esp_err_t nvs_commit(nvs_handle_t h){assert(h==1);return fail_commit?ESP_FAIL:ESP_OK;}
int main(void){
    uint64_t first=owner_first();assert(first&&writes==1&&randoms==2&&owner_first()==first&&writes==1);
    assert(owner_reboot()==first&&writes==1&&randoms==2);
    present=false;uint64_t reset=owner_reset();assert(reset&&reset!=first&&writes==2&&randoms==4);
    present=false;fail_commit=true;assert(!owner_uncertain()&&writes==3&&randoms==6);fail_commit=false;
    assert(!owner_uncertain()&&writes==3); /* No new identity until reboot. */
    assert(owner_uncertain_reboot()==stored&&stored&&writes==3&&randoms==6);
    puts("Library install identity: verified durable creation, reboot reuse/no RNG, factory-reset separation, uncertain commit blocks until reboot, existing value retained passed with NVS doubles.");
}
