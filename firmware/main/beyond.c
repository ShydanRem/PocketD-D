#include "beyond.h"
#include "wifi.h"
#include "sync.h"
#include "beyond_store.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdatomic.h>
#include <time.h>
#include <stdio.h>
#ifndef PD_BEYOND_SERVICE_TEST
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_crt_bundle.h"
#include "esp_heap_caps.h"
#include "esp_netif_sntp.h"
#else
#include "beyond_service_fake.h"
#endif

static SemaphoreHandle_t lock;
static pd_beyond_info info;
static pd_beyond_candidate *candidate;
static atomic_bool cancelled;
static bool clock_started,has_access,remembered,account_locked,link_remember,reconnect;
static pd_account_access account_access,link_access;
static pd_account_catalog *catalog;
static uint32_t runtime_started;
static unsigned runtime_checks;
static void phase(unsigned value){xSemaphoreTake(lock,portMAX_DELAY);
    info.phase=value;
    xSemaphoreGive(lock);
}
void pd_beyond_boot(void){if(!lock){
#ifndef PD_BEYOND_SERVICE_TEST
        esp_log_level_set("HTTP_CLIENT",ESP_LOG_WARN);esp_log_level_set("HTTP_HEADER",ESP_LOG_WARN);
#endif
        lock=xSemaphoreCreateMutex();
        if(lock){has_access=pd_account_store_load(&account_access);
            remembered=has_access;
}}}
void pd_beyond_snapshot(pd_beyond_info *out){if(!lock){memset(out,0,sizeof *out);
        return;
    }xSemaphoreTake(lock,portMAX_DELAY);
    *out=info;
    out->configured=has_access;
    out->remembered=remembered;
    out->locked=account_locked;
    out->listed=catalog!=NULL;
    out->reconnect=reconnect;
    out->count=catalog?catalog->count:0;
    out->partial=catalog&&catalog->partial;
    xSemaphoreGive(lock);
}
bool pd_beyond_usb_begin(void){if(!lock)return false;
    xSemaphoreTake(lock,portMAX_DELAY);
    if(info.running){xSemaphoreGive(lock);
        return false;
    }return true;
}
void pd_beyond_usb_end(void){xSemaphoreGive(lock);
}
static bool connected(void){pd_wifi_info wifi;
    pd_wifi_snapshot(&wifi);
    return wifi.state==PD_WIFI_CONNECTED&&!wifi.suspended&&!wifi.setup&&!wifi.joining;
}
static bool stop(void){return atomic_load_explicit(&cancelled,memory_order_relaxed);
}
static unsigned runtime_interrupt(void *unused){
    (void)unused;
    if(stop())return PD_BEYOND_CANCELLED;
    if((uint32_t)(xTaskGetTickCount()-runtime_started)>pdMS_TO_TICKS(20000))return PD_BEYOND_TIMEOUT;
    if((++runtime_checks&7u)==0)vTaskDelay(1);
    return 0;
}
static unsigned clock_ready(void){
    if(time(NULL)>=1704067200)return 0;
    phase(PD_BEYOND_CLOCK);
    if(!clock_started){esp_sntp_config_t config=ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
        esp_err_t r=esp_netif_sntp_init(&config);
        if(r!=ESP_OK&&r!=ESP_ERR_INVALID_STATE)return PD_BEYOND_TIME;
        clock_started=true;
    }
    uint32_t start=xTaskGetTickCount();
    while(time(NULL)<1704067200){
        if(stop())return PD_BEYOND_CANCELLED;
        if(!connected())return PD_BEYOND_NO_WIFI;
        if((uint32_t)(xTaskGetTickCount()-start)>pdMS_TO_TICKS(15000))return PD_BEYOND_TIME;
        esp_netif_sntp_sync_wait(pdMS_TO_TICKS(200));
    }
    return 0;
}
typedef struct {bool seen,json,bad;
} response_headers;
static esp_err_t headers(esp_http_client_event_t *event){
    if(event->event_id!=HTTP_EVENT_ON_HEADER)return ESP_OK;
    response_headers *h=event->user_data;
    if(!event->header_key||!event->header_value)return ESP_OK;
    if(!strcasecmp(event->header_key,"Content-Type")){
        if(h->seen){h->bad=true;
            return ESP_OK;
        }h->seen=true;
        const char *v=event->header_value;
        while(*v==' '||*v=='\t')v++;
        size_t n=strcspn(v,"; \t");
        h->json=(n==16&&!strncasecmp(v,"application/json",16))||(n>17&&n<96&&!strncasecmp(v,"application/",12)&&!strncasecmp(v+n-5,"+json",5));
    }else if(!strcasecmp(event->header_key,"Content-Encoding")&&strcasecmp(event->header_value,"identity"))h->bad=true;
    return ESP_OK;
}
static unsigned fetch_json(const char *url,const char *cookie,const char *token,size_t cap,char **source,size_t *length){
    response_headers h={0};
    esp_http_client_config_t config={.url=url,.method=cookie?HTTP_METHOD_POST:HTTP_METHOD_GET,.timeout_ms=5000,.disable_auto_redirect=true,
        .crt_bundle_attach=esp_crt_bundle_attach,.event_handler=headers,.user_data=&h,.buffer_size=1024,.buffer_size_tx=cookie?(int)strlen(cookie)+512:token?(int)strlen(token)+512:512};
    esp_http_client_handle_t client=esp_http_client_init(&config);
    if(!client)return PD_BEYOND_MEMORY;
    unsigned result=PD_BEYOND_NETWORK;
    char *buffer=NULL,*credential=NULL;
    size_t bytes=0;
    uint32_t start=xTaskGetTickCount();
    esp_http_client_set_header(client,"Accept","application/json");
    esp_http_client_set_header(client,"Accept-Encoding","identity");
    if(cookie||token){size_t size=strlen(cookie?cookie:token)+32;
        credential=malloc(size);
        if(!credential){result=PD_BEYOND_MEMORY;
            goto done;
        }snprintf(credential,size,"%s%s",cookie?"CobaltSession=":"Bearer ",cookie?cookie:token);
        if(esp_http_client_set_header(client,cookie?"Cookie":"Authorization",credential)!=ESP_OK)goto done;
    }
    if(cookie)esp_http_client_set_header(client,"Content-Type","application/json");
    if(stop()){result=PD_BEYOND_CANCELLED;
        goto done;
    }
    if(esp_http_client_open(client,cookie?2:0)!=ESP_OK)goto done;
    if(cookie&&esp_http_client_write(client,"{}",2)!=2)goto done;
    int64_t announced=esp_http_client_fetch_headers(client);
    if(announced<0)goto done;
    int status=esp_http_client_get_status_code(client);
    xSemaphoreTake(lock,portMAX_DELAY);
    info.http=status;
    xSemaphoreGive(lock);
    if(status>=300&&status<400){result=PD_BEYOND_REDIRECT;
        goto done;
    }
    if(status==401||status==403){result=PD_BEYOND_DENIED;
        goto done;
    }
    if(status==404){result=PD_BEYOND_NOT_FOUND;
        goto done;
    }
    if(status==429){result=PD_BEYOND_RATE;
        goto done;
    }
    if(status!=200)goto done;
    if(!h.seen||!h.json||h.bad){result=PD_BEYOND_MIME;
        goto done;
    }
    if(announced>(int64_t)cap){result=PD_BEYOND_SIZE;
        goto done;
    }
    buffer=heap_caps_malloc(cap+1,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if(!buffer){result=PD_BEYOND_MEMORY;
        goto done;
    }
    for(;;){
        if(stop()){result=PD_BEYOND_CANCELLED;
            goto done;
        }
        if((uint32_t)(xTaskGetTickCount()-start)>pdMS_TO_TICKS(20000)){result=PD_BEYOND_TIMEOUT;
            goto done;
        }
        if(!connected()){result=PD_BEYOND_NO_WIFI;
            goto done;
        }
        size_t room=cap+1-bytes;
        if(room>4096)room=4096;
        int n=esp_http_client_read(client,buffer+bytes,(int)room);
        if(n<0){goto done;}
        if(!n){if(!esp_http_client_is_complete_data_received(client))goto done;
            break;
        }
        bytes+=(size_t)n;
        xSemaphoreTake(lock,portMAX_DELAY);
        info.bytes=bytes;
        xSemaphoreGive(lock);
        if(bytes>cap){result=PD_BEYOND_SIZE;
            goto done;
        }
        vTaskDelay(1);
    }
    if(stop()){result=PD_BEYOND_CANCELLED;
        goto done;
    }
    if((uint32_t)(xTaskGetTickCount()-start)>pdMS_TO_TICKS(20000)){result=PD_BEYOND_TIMEOUT;
        goto done;
    }
    if(announced>0&&(uint64_t)announced!=bytes)goto done;
    if(!bytes){result=PD_BEYOND_INVALID;
        goto done;
    }
    buffer[bytes]=0;
    *source=buffer;
    *length=bytes;
    buffer=NULL;
    result=0;
    done:
    if(cookie&&buffer)pd_account_wipe(buffer,bytes);
    free(buffer);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    if(credential){pd_account_wipe(credential,strlen(credential));
        free(credential);
    }return result;
}
static unsigned authenticate(const pd_account_access *using,char **token){
    char *json=NULL;
    size_t size=0;
    unsigned result=fetch_json("https://auth-service.dndbeyond.com/v1/cobalt-token",using->cookie,NULL,32768,&json,&size);
    if(result==PD_BEYOND_DENIED)result=PD_BEYOND_SESSION;
    if(!result&&!pd_account_token(json,size,using->user,(int64_t)time(NULL),token))result=PD_BEYOND_IDENTITY;
    if(json){pd_account_wipe(json,size);
        free(json);
    }return result;
}
static void worker(void *unused){
    (void)unused;
    char id[16];
    unsigned operation;
    bool remember;
    pd_account_access *using=calloc(1,sizeof *using);
    xSemaphoreTake(lock,portMAX_DELAY);
    memcpy(id,info.id,sizeof id);
    operation=info.operation;
    remember=link_remember;
    if(using)*using=operation==PD_BEYOND_LINK?link_access:account_access;
    pd_account_wipe(&link_access,sizeof link_access);
    xSemaphoreGive(lock);
    char *source=NULL,*token=NULL;
    size_t length=0;
    pd_beyond_candidate *next=NULL;
    pd_account_catalog *next_catalog=NULL;
    size_t peak=0;
    bool committed=false;
    unsigned error=!using?PD_BEYOND_MEMORY:stop()?PD_BEYOND_CANCELLED:!connected()?PD_BEYOND_NO_WIFI:clock_ready();
    if(!error&&operation!=PD_BEYOND_PUBLIC){phase(PD_BEYOND_HTTP);
        error=authenticate(using,&token);
    }
    if(!error&&(operation==PD_BEYOND_LIST||operation==PD_BEYOND_LINK)){
        char url[160];
        snprintf(url,sizeof url,"https://character-service.dndbeyond.com/character/v5/characters/list?userId=%s",using->user);
        phase(PD_BEYOND_HTTP);
        error=fetch_json(url,NULL,token,512u*1024u,&source,&length);
        if(error==PD_BEYOND_DENIED)error=PD_BEYOND_SESSION;
        if(!error){next_catalog=heap_caps_calloc(1,sizeof *next_catalog,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
            if(!next_catalog)error=PD_BEYOND_MEMORY;
            else if(!pd_account_catalog_parse(source,length,next_catalog))error=PD_BEYOND_CATALOG_ERROR;
        }
    }else if(!error){
        char url[160];
        snprintf(url,sizeof url,"https://character-service.dndbeyond.com/character/v5/character/%s?includeCustomItems=true",id);
        phase(PD_BEYOND_HTTP);
        error=fetch_json(url,NULL,token,PD_BEYOND_SOURCE_MAX,&source,&length);
        if(!error){next=heap_caps_calloc(1,sizeof *next,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
            if(!next)error=PD_BEYOND_MEMORY;
            else{phase(PD_BEYOND_CONVERTING);
                runtime_started=xTaskGetTickCount();
                runtime_checks=0;
                pd_beyond_control control={.heap_limit=PD_BEYOND_HEAP_MAX,.interrupt=runtime_interrupt};
                error=pd_beyond_convert(source,length,id,&next->state,&next->report,&control);
                peak=control.peak_bytes;
        }}
    }
    free(source);
    if(token){pd_account_wipe(token,strlen(token));
        free(token);
    }
    if(stop())error=PD_BEYOND_CANCELLED;
    /* Parsers/VM are gone before resuming the existing PC bridge. */
    pd_sync_pause(false);
    xSemaphoreTake(lock,portMAX_DELAY);
    if(stop())error=PD_BEYOND_CANCELLED;
    if(!error&&operation==PD_BEYOND_LINK){
        /* Commit and publication share the cancellation mutex: once committed, a
        late cancel cannot claim that the association was discarded. */
        bool saved=remember?pd_account_store_save(using):pd_account_store_forget();
        if(!saved){account_locked=true;
            error=PD_BEYOND_STORAGE;
        }else{pd_account_wipe(&account_access,sizeof account_access);
            account_access=*using;
            has_access=true;
            remembered=remember;
            committed=true;
        }
    }
    /* Failed replacement credentials must not invalidate the previous account.
       Transport and catalog/character format errors do not require relinking.
       An unverifiable authentication token remains an identity failure. */
    if(operation!=PD_BEYOND_PUBLIC&&operation!=PD_BEYOND_LINK&&
       (error==PD_BEYOND_SESSION||error==PD_BEYOND_IDENTITY))reconnect=true;
    if(!error&&operation!=PD_BEYOND_PUBLIC)reconnect=false;
    if(!error&&next_catalog){free(catalog);
        catalog=next_catalog;
        next_catalog=NULL;
    }
    info.error=error;
    info.peak=peak;
    info.running=false;
    if(error==PD_BEYOND_READY||error==PD_BEYOND_BLOCKED){candidate=next;
        next=NULL;
        info.phase=PD_BEYOND_FINISHED;
    }else info.phase=PD_BEYOND_FAILED;
    (void)committed;
    xSemaphoreGive(lock);
    free(next);
    free(next_catalog);
    if(using){pd_account_wipe(using,sizeof *using);
        free(using);
    }vTaskDelete(NULL);
}
static bool start(unsigned operation,const char *id,const pd_account_access *offered,bool remember){
    if((operation==PD_BEYOND_PUBLIC||operation==PD_BEYOND_OWNED)&&!pd_beyond_id(id))return false;
    pd_beyond_boot();
    if(!lock)return false;
    xSemaphoreTake(lock,portMAX_DELAY);
    bool busy=info.running||(operation!=PD_BEYOND_PUBLIC&&account_locked)||(operation!=PD_BEYOND_PUBLIC&&operation!=PD_BEYOND_LINK&&(!has_access||reconnect));
    xSemaphoreGive(lock);
    if(busy)return false;
    if(!pd_sync_pause(true)){pd_sync_pause(false);
        return false;
    }
    xSemaphoreTake(lock,portMAX_DELAY);
    free(candidate);
    candidate=NULL;
    info=(pd_beyond_info){.phase=PD_BEYOND_CLOCK,.running=true,.operation=operation};
    if(id)strcpy(info.id,id);
    if(offered)link_access=*offered;
    link_remember=remember;
    atomic_store(&cancelled,false);
    xSemaphoreGive(lock);
    if(xTaskCreate(worker,"beyond",32768,NULL,2,NULL)!=pdPASS){xSemaphoreTake(lock,portMAX_DELAY);
        pd_account_wipe(&link_access,sizeof link_access);
        info.running=false;
        info.phase=PD_BEYOND_FAILED;
        info.error=PD_BEYOND_MEMORY;
        xSemaphoreGive(lock);
        pd_sync_pause(false);
        return false;
    }return true;
}
bool pd_beyond_start(const char *id){return start(PD_BEYOND_PUBLIC,id,NULL,false);
}
bool pd_beyond_list(void){return start(PD_BEYOND_LIST,NULL,NULL,false);
}
bool pd_beyond_link(const pd_account_access *offered,bool remember){return pd_account_valid(offered)&&start(PD_BEYOND_LINK,NULL,offered,remember);
}
bool pd_beyond_owned(unsigned index){char id[16]={0};
    if(!lock)return false;
    xSemaphoreTake(lock,portMAX_DELAY);
    if(catalog&&index<catalog->count)strcpy(id,catalog->rows[index].id);
    xSemaphoreGive(lock);
    return *id&&pd_beyond_owned_id(id);
}
bool pd_beyond_owned_id(const char *id){
    if(!lock||!pd_beyond_id(id))return false;
    bool found=false;
    xSemaphoreTake(lock,portMAX_DELAY);
    if(catalog)for(unsigned i=0;i<catalog->count;i++)if(!strcmp(id,catalog->rows[i].id)){found=true;break;}
    xSemaphoreGive(lock);
    return found&&start(PD_BEYOND_OWNED,id,NULL,false);
}
bool pd_beyond_row(unsigned index,pd_account_row *out){if(!out||!lock)return false;
    xSemaphoreTake(lock,portMAX_DELAY);
    bool ok=catalog&&index<catalog->count;
    if(ok)*out=catalog->rows[index];
    xSemaphoreGive(lock);
    return ok;
}
bool pd_beyond_forget(void){if(!lock)return false;
    xSemaphoreTake(lock,portMAX_DELAY);
    bool ok=!info.running&&!account_locked;
    if(ok){ok=pd_account_store_forget();
        if(ok){pd_account_wipe(&account_access,sizeof account_access);
            has_access=remembered=reconnect=false;
            free(catalog);
            catalog=NULL;
        }else account_locked=true;
    }xSemaphoreGive(lock);
    return ok;
}
void pd_beyond_cancel(void){if(!lock)return;
    xSemaphoreTake(lock,portMAX_DELAY);
    if(info.running){atomic_store(&cancelled,true);
        info.phase=PD_BEYOND_CANCELLING;
    }else if(!(info.operation==PD_BEYOND_LINK&&info.phase==PD_BEYOND_FINISHED)){free(candidate);
        candidate=NULL;
        info.phase=PD_BEYOND_FAILED;
        info.error=PD_BEYOND_CANCELLED;
    }xSemaphoreGive(lock);
}
pd_beyond_candidate *pd_beyond_take(void){if(!lock)return NULL;
    xSemaphoreTake(lock,portMAX_DELAY);
    pd_beyond_candidate *out=NULL;
    if(!info.running){out=candidate;
        candidate=NULL;
    }xSemaphoreGive(lock);
    return out;
}
