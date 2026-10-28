#include "sync.h"
#include "sync_config.h"
#include "sync_snapshot.h"
#include "wifi.h"
#ifdef PD_SYNC_TEST
#include "sync_test_sdk.h"
#else
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_http_client.h"
#include "mbedtls/sha256.h"
#include "nvs_flash.h"
#include "nvs.h"
#endif
#include "cJSON.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
static SemaphoreHandle_t lock;static bool worker,offered,configured,ready,paused,active;
static pd_sync_config config,pending;
static pd_sync_info info;
static char boot[17];static uint32_t counter;
typedef pd_sync_sample sample;
static sample latest;
static uint32_t offered_at;
static uint32_t ticks(void){return xTaskGetTickCount()*portTICK_PERIOD_MS;}
static void fingerprint(const char *cert,char out[33]){unsigned char hash[32];mbedtls_sha256((const unsigned char*)cert,strlen(cert),hash,0);for(unsigned i=0;i<16;i++)snprintf(out+i*2,3,"%02x",hash[i]);}
static bool store_config(bool erase){nvs_handle_t h;if(nvs_flash_init_partition("pdd_nvs")!=ESP_OK||nvs_open_from_partition("pdd_nvs","pdd_sync",NVS_READWRITE,&h)!=ESP_OK)return false;esp_err_t e=erase?nvs_erase_all(h):nvs_set_blob(h,"config",&config,sizeof config);if(e==ESP_OK)e=nvs_commit(h);nvs_close(h);return e==ESP_OK;}
void pd_sync_snapshot(pd_sync_info *out){memset(out,0,sizeof *out);if(!lock)return;xSemaphoreTake(lock,portMAX_DELAY);*out=info;out->configured=configured;out->offer=offered&&(uint32_t)(ticks()-offered_at)<300000;if(out->offer){strcpy(out->character_id,pending.character_id);strcpy(out->host,pending.host);fingerprint(pending.certificate,out->fingerprint);}else if(configured){strcpy(out->character_id,config.character_id);strcpy(out->host,config.host);fingerprint(config.certificate,out->fingerprint);}xSemaphoreGive(lock);}
bool pd_sync_stage_json(const char *json,size_t size){if(!lock)return false;pd_sync_config c;if(!pd_sync_config_parse(json,size,&c))return false;xSemaphoreTake(lock,portMAX_DELAY);if(offered&&(uint32_t)(ticks()-offered_at)>=300000){offered=false;memset(&pending,0,sizeof pending);}bool okay=!offered;if(okay){pending=c;offered=true;offered_at=ticks();}xSemaphoreGive(lock);memset(&c,0,sizeof c);return okay;}
typedef struct {char data[512];size_t size;bool overflow;} response;
static esp_err_t on_http(esp_http_client_event_t *e){response *r=e->user_data;if(e->event_id==HTTP_EVENT_ON_DATA){if(r->size+e->data_len>=sizeof r->data){r->overflow=true;return ESP_FAIL;}memcpy(r->data+r->size,e->data,e->data_len);r->size+=e->data_len;r->data[r->size]=0;}return ESP_OK;}
static void task(void *unused){(void)unused;for(;;){pd_wifi_info wifi;pd_wifi_snapshot(&wifi);sample s;pd_sync_config cfg;uint32_t serial;char session[17];
 xSemaphoreTake(lock,portMAX_DELAY);bool go=configured&&ready&&!paused&&info.state!=PD_SYNC_REBIND&&wifi.state==PD_WIFI_CONNECTED&&!wifi.suspended;if(configured&&ready&&!go&&info.state!=PD_SYNC_REBIND)info.state=PD_SYNC_WAIT_NETWORK;cfg=config;s=latest;active=go;serial=counter;memcpy(session,boot,sizeof session);xSemaphoreGive(lock);
 if(go){char *json=pd_sync_encode(&s,serial,session);if(json){char url[96],auth[72];snprintf(url,sizeof url,"https://%s:%u/device/snapshot",cfg.host,cfg.port);snprintf(auth,sizeof auth,"Bearer %s",cfg.token);response r={0};esp_http_client_config_t options={.url=url,.cert_pem=cfg.certificate,.timeout_ms=3000,.disable_auto_redirect=true,.event_handler=on_http,.user_data=&r,.buffer_size=1024};esp_http_client_handle_t client=esp_http_client_init(&options);
 unsigned status=PD_SYNC_ERROR,queued=0,observed=0;
 if(client){esp_http_client_set_method(client,HTTP_METHOD_POST);esp_http_client_set_header(client,"Content-Type","application/json");esp_http_client_set_header(client,"Authorization",auth);esp_http_client_set_post_field(client,json,strlen(json));esp_err_t e=esp_http_client_perform(client);if(e==ESP_OK&&esp_http_client_get_status_code(client)==200&&!r.overflow){cJSON *root=cJSON_Parse(r.data),*state=cJSON_GetObjectItemCaseSensitive(root,"state"),*q=cJSON_GetObjectItemCaseSensitive(root,"queued"),*done=cJSON_GetObjectItemCaseSensitive(root,"observed");if(cJSON_IsString(state)){status=!strcmp(state->valuestring,"ready")?PD_SYNC_READY:!strcmp(state->valuestring,"baseline")||!strcmp(state->valuestring,"paused")?PD_SYNC_BASELINE:!strcmp(state->valuestring,"conflict")?PD_SYNC_CONFLICT:!strcmp(state->valuestring,"uncertain")?PD_SYNC_UNCERTAIN:PD_SYNC_ERROR;if(cJSON_IsNumber(q)&&q->valueint>=0&&q->valueint<=256)queued=q->valueint;if(cJSON_IsNumber(done)&&done->valueint>=0&&done->valueint<=256)observed=done->valueint;}cJSON_Delete(root);}esp_http_client_cleanup(client);}
 xSemaphoreTake(lock,portMAX_DELAY);if(configured&&!strcmp(config.id,cfg.id)&&info.state!=PD_SYNC_REBIND){info.state=status;info.queued=queued;info.observed=observed;}xSemaphoreGive(lock);free(json);memset(auth,0,sizeof auth);}
 }
 xSemaphoreTake(lock,portMAX_DELAY);active=false;xSemaphoreGive(lock);memset(&cfg,0,sizeof cfg);vTaskDelay(pdMS_TO_TICKS(1000));}}
static void start(void){if(!worker&&xTaskCreate(task,"pdd_sync",12288,NULL,1,NULL)==pdPASS)worker=true;else if(!worker)info.state=PD_SYNC_ERROR;}
void pd_sync_boot(void){if(lock)return;lock=xSemaphoreCreateMutex();if(!lock)return;snprintf(boot,sizeof boot,"%08lx%08lx",(unsigned long)pd_wifi_random(),(unsigned long)pd_wifi_random());
 nvs_handle_t h;if(nvs_flash_init_partition("pdd_nvs")==ESP_OK&&nvs_open_from_partition("pdd_nvs","pdd_sync",NVS_READONLY,&h)==ESP_OK){size_t n=sizeof config;esp_err_t e=nvs_get_blob(h,"config",&config,&n);nvs_close(h);if(e==ESP_OK&&n==sizeof config&&config.format==1&&memchr(config.certificate,0,sizeof config.certificate)&&memchr(config.host,0,sizeof config.host)&&memchr(config.token,0,sizeof config.token)&&memchr(config.id,0,sizeof config.id)&&memchr(config.character_id,0,sizeof config.character_id)){/* Revalidate persisted fields through the same bounded parser. */
 cJSON *r=cJSON_CreateObject();if(r){cJSON_AddNumberToObject(r,"version",1);cJSON_AddStringToObject(r,"id",config.id);cJSON_AddStringToObject(r,"host",config.host);cJSON_AddStringToObject(r,"characterId",config.character_id);cJSON_AddStringToObject(r,"token",config.token);cJSON_AddStringToObject(r,"certificate",config.certificate);cJSON_AddNumberToObject(r,"port",config.port);cJSON_AddNumberToObject(r,"time",config.time);char *json=cJSON_PrintUnformatted(r);pd_sync_config checked;if(json&&pd_sync_config_parse(json,strlen(json),&checked)){configured=true;info.state=PD_SYNC_BASELINE;}free(json);cJSON_Delete(r);}}}
 if(configured){struct timeval now;gettimeofday(&now,NULL);if(now.tv_sec<config.time){now.tv_sec=config.time;now.tv_usec=0;settimeofday(&now,NULL);}start();}}
bool pd_sync_decide(bool accept,uint32_t epoch){if(!lock)return false;xSemaphoreTake(lock,portMAX_DELAY);if(!accept){offered=false;memset(&pending,0,sizeof pending);xSemaphoreGive(lock);return true;}if(!offered||(uint32_t)(ticks()-offered_at)>=300000){offered=false;xSemaphoreGive(lock);return false;}pd_sync_config old=config;config=pending;config.epoch=epoch;bool okay=store_config(false);if(okay){configured=true;offered=false;ready=false;counter=0;snprintf(boot,sizeof boot,"%08lx%08lx",(unsigned long)pd_wifi_random(),(unsigned long)pd_wifi_random());info=(pd_sync_info){.state=PD_SYNC_BASELINE};struct timeval now;gettimeofday(&now,NULL);if(now.tv_sec<config.time){now.tv_sec=config.time;now.tv_usec=0;settimeofday(&now,NULL);}memset(&pending,0,sizeof pending);}else config=old;memset(&old,0,sizeof old);xSemaphoreGive(lock);if(okay)start();return okay;}
bool pd_sync_forget(void){if(!lock)return false;xSemaphoreTake(lock,portMAX_DELAY);bool okay=store_config(true);if(okay){configured=ready=offered=false;memset(&config,0,sizeof config);memset(&pending,0,sizeof pending);info=(pd_sync_info){0};}xSemaphoreGive(lock);return okay;}
void pd_sync_observe(const pd_state *s,const pd_history *h,bool durable){if(!lock||!durable||h->epoch!=s->history_epoch||!pd_history_valid(h))return;xSemaphoreTake(lock,portMAX_DELAY);if(configured){if(config.epoch!=s->history_epoch){info.state=PD_SYNC_REBIND;ready=false;}else{sample next={.epoch=s->history_epoch,.hp=s->hp,.max=pd_max_hp(s),.temp=s->temp_hp,.history=*h};memcpy(next.name,s->name,sizeof next.name);if(!ready||next.epoch!=latest.epoch||next.hp!=latest.hp||next.max!=latest.max||next.temp!=latest.temp||strcmp(next.name,latest.name)||next.history.sequence!=latest.history.sequence){if(counter==UINT32_MAX){info.state=PD_SYNC_REBIND;ready=false;}else{latest=next;counter++;ready=true;}}}}xSemaphoreGive(lock);}

bool pd_sync_pause(bool value){if(!lock)return true;xSemaphoreTake(lock,portMAX_DELAY);paused=value;xSemaphoreGive(lock);if(!value)return true;uint32_t start=ticks();for(;;){xSemaphoreTake(lock,portMAX_DELAY);bool done=!active;xSemaphoreGive(lock);if(done)return true;if((uint32_t)(ticks()-start)>4000)return false;vTaskDelay(pdMS_TO_TICKS(20));}}
