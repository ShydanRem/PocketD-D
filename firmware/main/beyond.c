#include "beyond.h"
#include "wifi.h"
#include "sync.h"
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
static bool clock_started;
static uint32_t runtime_started;
static unsigned runtime_checks;
static void phase(unsigned value){xSemaphoreTake(lock,portMAX_DELAY);info.phase=value;xSemaphoreGive(lock);}
void pd_beyond_boot(void){if(!lock)lock=xSemaphoreCreateMutex();}
void pd_beyond_snapshot(pd_beyond_info *out){if(!lock){memset(out,0,sizeof *out);return;}xSemaphoreTake(lock,portMAX_DELAY);*out=info;xSemaphoreGive(lock);}
bool pd_beyond_usb_begin(void){if(!lock)return false;xSemaphoreTake(lock,portMAX_DELAY);if(info.running){xSemaphoreGive(lock);return false;}return true;}
void pd_beyond_usb_end(void){xSemaphoreGive(lock);}
static bool connected(void){pd_wifi_info wifi;pd_wifi_snapshot(&wifi);return wifi.state==PD_WIFI_CONNECTED&&!wifi.suspended&&!wifi.setup&&!wifi.joining;}
static bool stop(void){return atomic_load_explicit(&cancelled,memory_order_relaxed);}
static unsigned runtime_interrupt(void *unused){
    (void)unused;if(stop())return PD_BEYOND_CANCELLED;
    if((uint32_t)(xTaskGetTickCount()-runtime_started)>pdMS_TO_TICKS(20000))return PD_BEYOND_TIMEOUT;
    if((++runtime_checks&7u)==0)vTaskDelay(1);
    return 0;
}
static unsigned clock_ready(void){
    if(time(NULL)>=1704067200)return 0;
    phase(PD_BEYOND_CLOCK);
    if(!clock_started){esp_sntp_config_t config=ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");esp_err_t r=esp_netif_sntp_init(&config);if(r!=ESP_OK&&r!=ESP_ERR_INVALID_STATE)return PD_BEYOND_TIME;clock_started=true;}
    uint32_t start=xTaskGetTickCount();
    while(time(NULL)<1704067200){
        if(stop())return PD_BEYOND_CANCELLED;
        if(!connected())return PD_BEYOND_NO_WIFI;
        if((uint32_t)(xTaskGetTickCount()-start)>pdMS_TO_TICKS(15000))return PD_BEYOND_TIME;
        esp_netif_sntp_sync_wait(pdMS_TO_TICKS(200));
    }
    return 0;
}
typedef struct {bool seen,json,bad;} response_headers;
static esp_err_t headers(esp_http_client_event_t *event){
    if(event->event_id!=HTTP_EVENT_ON_HEADER)return ESP_OK;
    response_headers *h=event->user_data;
    if(!event->header_key||!event->header_value)return ESP_OK;
    if(!strcasecmp(event->header_key,"Content-Type")){
        if(h->seen){h->bad=true;return ESP_OK;}h->seen=true;
        const char *v=event->header_value;while(*v==' '||*v=='\t')v++;
        size_t n=strcspn(v,"; \t");
        h->json=(n==16&&!strncasecmp(v,"application/json",16))||(n>17&&n<96&&!strncasecmp(v,"application/",12)&&!strncasecmp(v+n-5,"+json",5));
    }else if(!strcasecmp(event->header_key,"Content-Encoding")&&strcasecmp(event->header_value,"identity"))h->bad=true;
    return ESP_OK;
}
static unsigned fetch_source(const char *id,char **source,size_t *length){
    char url[160];snprintf(url,sizeof url,"https://character-service.dndbeyond.com/character/v5/character/%s?includeCustomItems=true",id);
    response_headers h={0};
    esp_http_client_config_t config={.url=url,.method=HTTP_METHOD_GET,.timeout_ms=5000,.disable_auto_redirect=true,
        .crt_bundle_attach=esp_crt_bundle_attach,.event_handler=headers,.user_data=&h,.buffer_size=1024,.buffer_size_tx=512};
    esp_http_client_handle_t client=esp_http_client_init(&config);if(!client)return PD_BEYOND_MEMORY;
    unsigned result=PD_BEYOND_NETWORK;char *buffer=NULL;size_t bytes=0;uint32_t start=xTaskGetTickCount();
    esp_http_client_set_header(client,"Accept","application/json");esp_http_client_set_header(client,"Accept-Encoding","identity");
    if(stop()){result=PD_BEYOND_CANCELLED;goto done;}
    if(esp_http_client_open(client,0)!=ESP_OK)goto done;
    int64_t announced=esp_http_client_fetch_headers(client);if(announced<0)goto done;
    int status=esp_http_client_get_status_code(client);xSemaphoreTake(lock,portMAX_DELAY);info.http=status;xSemaphoreGive(lock);
    if(status>=300&&status<400){result=PD_BEYOND_REDIRECT;goto done;}
    if(status==401||status==403){result=PD_BEYOND_DENIED;goto done;}
    if(status==404){result=PD_BEYOND_NOT_FOUND;goto done;}
    if(status==429){result=PD_BEYOND_RATE;goto done;}
    if(status!=200)goto done;
    if(!h.seen||!h.json||h.bad){result=PD_BEYOND_MIME;goto done;}
    if(announced>PD_BEYOND_SOURCE_MAX){result=PD_BEYOND_SIZE;goto done;}
    buffer=heap_caps_malloc(PD_BEYOND_SOURCE_MAX+1,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);if(!buffer){result=PD_BEYOND_MEMORY;goto done;}
    for(;;){
        if(stop()){result=PD_BEYOND_CANCELLED;goto done;}
        if((uint32_t)(xTaskGetTickCount()-start)>pdMS_TO_TICKS(20000)){result=PD_BEYOND_TIMEOUT;goto done;}
        if(!connected()){result=PD_BEYOND_NO_WIFI;goto done;}
        size_t room=PD_BEYOND_SOURCE_MAX+1-bytes;if(room>4096)room=4096;
        int n=esp_http_client_read(client,buffer+bytes,(int)room);if(n<0)goto done;
        if(!n){if(!esp_http_client_is_complete_data_received(client))goto done;break;}
        bytes+=(size_t)n;xSemaphoreTake(lock,portMAX_DELAY);info.bytes=bytes;xSemaphoreGive(lock);
        if(bytes>PD_BEYOND_SOURCE_MAX){result=PD_BEYOND_SIZE;goto done;}
        vTaskDelay(1);
    }
    if(stop()){result=PD_BEYOND_CANCELLED;goto done;}
    if((uint32_t)(xTaskGetTickCount()-start)>pdMS_TO_TICKS(20000)){result=PD_BEYOND_TIMEOUT;goto done;}
    if(announced>0&&(uint64_t)announced!=bytes)goto done;
    if(!bytes){result=PD_BEYOND_INVALID;goto done;}
    buffer[bytes]=0;*source=buffer;*length=bytes;buffer=NULL;result=0;
done:
    free(buffer);esp_http_client_close(client);esp_http_client_cleanup(client);return result;
}
static void worker(void *unused){
    (void)unused;char id[16];xSemaphoreTake(lock,portMAX_DELAY);memcpy(id,info.id,sizeof id);xSemaphoreGive(lock);
    char *source=NULL;size_t length=0;pd_beyond_candidate *next=NULL;size_t peak=0;
    unsigned error=stop()?PD_BEYOND_CANCELLED:!connected()?PD_BEYOND_NO_WIFI:clock_ready();
    if(!error){phase(PD_BEYOND_HTTP);error=fetch_source(id,&source,&length);}
    if(!error){
        next=heap_caps_calloc(1,sizeof *next,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
        if(!next)error=PD_BEYOND_MEMORY;
        else{
            phase(PD_BEYOND_CONVERTING);runtime_started=xTaskGetTickCount();runtime_checks=0;
            pd_beyond_control control={.heap_limit=PD_BEYOND_HEAP_MAX,.interrupt=runtime_interrupt};
            error=pd_beyond_convert(source,length,id,&next->state,&next->report,&control);peak=control.peak_bytes;
        }
    }
    free(source);if(stop())error=PD_BEYOND_CANCELLED;
    /* No VM or JSON parser remains active when the existing PC bridge resumes. */
    pd_sync_pause(false);
    xSemaphoreTake(lock,portMAX_DELAY);
    if(stop())error=PD_BEYOND_CANCELLED;
    info.error=error;info.peak=peak;info.running=false;
    if(error==PD_BEYOND_READY||error==PD_BEYOND_BLOCKED){candidate=next;next=NULL;info.phase=PD_BEYOND_FINISHED;}
    else info.phase=PD_BEYOND_FAILED;
    xSemaphoreGive(lock);free(next);vTaskDelete(NULL);
}
bool pd_beyond_start(const char *id){
    if(!pd_beyond_id(id))return false;
    pd_beyond_boot();if(!lock)return false;
    xSemaphoreTake(lock,portMAX_DELAY);bool busy=info.running;xSemaphoreGive(lock);if(busy)return false;
    if(!pd_sync_pause(true)){pd_sync_pause(false);return false;}
    xSemaphoreTake(lock,portMAX_DELAY);free(candidate);candidate=NULL;info=(pd_beyond_info){.phase=PD_BEYOND_CLOCK,.running=true};strcpy(info.id,id);atomic_store(&cancelled,false);xSemaphoreGive(lock);
    if(xTaskCreate(worker,"beyond",32768,NULL,2,NULL)!=pdPASS){xSemaphoreTake(lock,portMAX_DELAY);info.running=false;info.phase=PD_BEYOND_FAILED;info.error=PD_BEYOND_MEMORY;xSemaphoreGive(lock);pd_sync_pause(false);return false;}
    return true;
}
void pd_beyond_cancel(void){if(!lock)return;xSemaphoreTake(lock,portMAX_DELAY);if(info.running){atomic_store(&cancelled,true);info.phase=PD_BEYOND_CANCELLING;}else{free(candidate);candidate=NULL;info.phase=PD_BEYOND_FAILED;info.error=PD_BEYOND_CANCELLED;}xSemaphoreGive(lock);}
pd_beyond_candidate *pd_beyond_take(void){if(!lock)return NULL;xSemaphoreTake(lock,portMAX_DELAY);pd_beyond_candidate *out=NULL;if(!info.running){out=candidate;candidate=NULL;}xSemaphoreGive(lock);return out;}
