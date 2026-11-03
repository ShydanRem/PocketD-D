#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "account_fixtures.h"
static time_t fake_time(time_t *out);
#define time fake_time
#define PD_BEYOND_SERVICE_TEST 1
#include "../main/beyond.c"
#undef time
static const char *body,*mime,*encoding;
static char *fixture;
static size_t body_size,body_at;
static uint32_t ticks,read_advance;
static time_t wall;
static bool network,complete,fail_open,fail_clock,fail_memory,duplicate_mime,cancel_read,cancel_resume,fail_task,sync_fail;
static unsigned status_code,created,opened,closed,cleaned,headers_set,pauses,resumes,sntp_calls;
static int64_t declared;
static void (*queued_worker)(void *);
static esp_http_client_config_t captured;
static char requested[160];
static bool service_locked,account_mode,fail_account_store,deny_private;
static unsigned auth_calls,list_calls,private_calls,account_saves;
static pd_account_access stored_account;
bool pd_account_store_load(pd_account_access *out){if(!pd_account_valid(&stored_account))return false;*out=stored_account;return true;}
bool pd_account_store_save(const pd_account_access *a){account_saves++;if(fail_account_store)return false;stored_account=*a;return true;}
bool pd_account_store_forget(void){if(fail_account_store)return false;pd_account_wipe(&stored_account,sizeof stored_account);return true;}
static time_t fake_time(time_t *out){if(out)*out=wall;return wall;}
SemaphoreHandle_t xSemaphoreCreateMutex(void){return (void *)1;}
int xSemaphoreTake(SemaphoreHandle_t h,uint32_t timeout){(void)timeout;assert(h&&!service_locked);service_locked=true;return 1;}
int xSemaphoreGive(SemaphoreHandle_t h){assert(h&&service_locked);service_locked=false;return 1;}
uint32_t xTaskGetTickCount(void){return ticks;}
void vTaskDelay(uint32_t n){ticks+=n;}
void vTaskDelete(void *task){assert(!task);}
int xTaskCreate(void (*entry)(void *),const char *name,uint32_t stack,void *arg,unsigned priority,void *handle){assert(!strcmp(name,"beyond")&&stack==32768&&!arg&&priority==2&&!handle);if(fail_task)return 0;queued_worker=entry;return pdPASS;}
void *heap_caps_malloc(size_t n,unsigned caps){assert(caps==(MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));return fail_memory?NULL:malloc(n);}
void *heap_caps_calloc(size_t n,size_t size,unsigned caps){assert(caps==(MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));return fail_memory?NULL:calloc(n,size);}
void pd_wifi_snapshot(pd_wifi_info *out){*out=(pd_wifi_info){.state=network?PD_WIFI_CONNECTED:PD_WIFI_OFF};}
bool pd_sync_pause(bool pause){if(pause)pauses++;else{resumes++;if(cancel_resume)pd_beyond_cancel();}return !sync_fail;}
esp_err_t esp_netif_sntp_init(const esp_sntp_config_t *c){sntp_calls++;assert(!strcmp(c->server,"pool.ntp.org"));return fail_clock?ESP_FAIL:ESP_OK;}
esp_err_t esp_netif_sntp_sync_wait(uint32_t n){ticks+=n;if(!fail_clock)wall=1800000000;return ESP_OK;}
esp_err_t esp_crt_bundle_attach(void *unused){(void)unused;return ESP_OK;}
esp_http_client_handle_t esp_http_client_init(const esp_http_client_config_t *c){
    assert((c->method==HTTP_METHOD_GET||c->method==HTTP_METHOD_POST)&&c->disable_auto_redirect&&!c->skip_cert_common_name_check&&c->crt_bundle_attach==esp_crt_bundle_attach&&c->timeout_ms==5000);
    if(account_mode){if(c->method==HTTP_METHOD_POST){assert(!strcmp(c->url,"https://auth-service.dndbeyond.com/v1/cobalt-token"));auth_calls++;body=ACCOUNT_AUTH;}else if(strstr(c->url,"characters/list")){assert(!strcmp(c->url,"https://character-service.dndbeyond.com/character/v5/characters/list?userId=1234"));list_calls++;body=ACCOUNT_LIST;}else{assert(strstr(c->url,"/character/123456?includeCustomItems=true"));private_calls++;body=fixture;}body_size=strlen(body);body_at=0;declared=body_size;}
    else assert(strstr(c->url,"https://character-service.dndbeyond.com/character/v5/character/")==c->url&&strstr(c->url,"?includeCustomItems=true"));
    assert(strlen(c->url)<sizeof requested);strcpy(requested,c->url);captured=*c;created++;return (void *)2;
}
esp_err_t esp_http_client_set_header(esp_http_client_handle_t h,const char *name,const char *value){assert(h==(void *)2);if(!strcmp(name,"Cookie")){assert(captured.method==HTTP_METHOD_POST&&!strcmp(value,"CobaltSession=synthetic-session")&&captured.buffer_size_tx>=strlen(value)+128);}else if(!strcmp(name,"Authorization")){assert(captured.method==HTTP_METHOD_GET&&!strncmp(value,"Bearer ",7)&&captured.buffer_size_tx>=strlen(value)+128);}else if(!strcmp(name,"Content-Type")){assert(captured.method==HTTP_METHOD_POST&&!strcmp(value,"application/json"));}else assert((!strcmp(name,"Accept")&&!strcmp(value,"application/json"))||(!strcmp(name,"Accept-Encoding")&&!strcmp(value,"identity")));headers_set++;return ESP_OK;}
esp_err_t esp_http_client_open(esp_http_client_handle_t h,unsigned size){assert(h==(void *)2&&size==(captured.method==HTTP_METHOD_POST?2u:0u));opened++;return fail_open?ESP_FAIL:ESP_OK;}
int esp_http_client_write(esp_http_client_handle_t h,const char *s,int n){assert(h==(void *)2&&n==2&&!memcmp(s,"{}",2));return 2;}
static void header(const char *name,const char *value){esp_http_client_event_t e={.event_id=HTTP_EVENT_ON_HEADER,.user_data=captured.user_data,.header_key=name,.header_value=value};assert(captured.event_handler(&e)==ESP_OK);}
int64_t esp_http_client_fetch_headers(esp_http_client_handle_t h){assert(h==(void *)2);if(mime)header("Content-Type",mime);if(duplicate_mime)header("Content-Type",mime);if(encoding)header("Content-Encoding",encoding);return declared;}
int esp_http_client_get_status_code(esp_http_client_handle_t h){assert(h==(void *)2);return deny_private&&captured.method==HTTP_METHOD_GET&&!strstr(captured.url,"characters/list")?403:status_code;}
int esp_http_client_read(esp_http_client_handle_t h,char *out,int capacity){assert(h==(void *)2&&capacity>0);ticks+=read_advance;if(cancel_read){cancel_read=false;pd_beyond_cancel();}size_t n=body_size-body_at;if(n>(size_t)capacity)n=capacity;if(n>137)n=137;memcpy(out,body+body_at,n);body_at+=n;return (int)n;}
bool esp_http_client_is_complete_data_received(esp_http_client_handle_t h){assert(h==(void *)2);return complete&&body_at==body_size;}
esp_err_t esp_http_client_close(esp_http_client_handle_t h){assert(h==(void *)2);closed++;return ESP_OK;}
esp_err_t esp_http_client_cleanup(esp_http_client_handle_t h){assert(h==(void *)2);cleaned++;return ESP_OK;}
static void reset(void){
    assert(!service_locked);account_mode=false;fail_account_store=deny_private=false;has_access=remembered=account_locked=reconnect=false;pd_account_wipe(&account_access,sizeof account_access);pd_account_wipe(&link_access,sizeof link_access);free(catalog);catalog=NULL;
    free(candidate);candidate=NULL;info=(pd_beyond_info){0};clock_started=false;atomic_store(&cancelled,false);
    body=fixture;body_size=strlen(fixture);body_at=0;declared=body_size;mime="application/json; charset=utf-8";encoding=NULL;
    ticks=0;wall=1800000000;network=complete=true;fail_open=fail_clock=fail_memory=duplicate_mime=cancel_read=cancel_resume=fail_task=sync_fail=false;
    status_code=200;read_advance=0;created=opened=closed=cleaned=headers_set=pauses=resumes=sntp_calls=0;queued_worker=NULL;requested[0]=0;
}
static pd_beyond_info run(unsigned expected){
    assert(pd_beyond_usb_begin());pd_beyond_usb_end();
    assert(pd_beyond_start("123456")&&queued_worker);assert(!pd_beyond_usb_begin());assert(!pd_beyond_start("123456"));queued_worker(NULL);
    pd_beyond_info result;pd_beyond_snapshot(&result);assert(!result.running&&result.error==expected&&pauses==1&&resumes==1);
    if(created)assert(opened==created&&closed==created&&cleaned==created&&headers_set==2);
    if(expected!=PD_BEYOND_READY&&expected!=PD_BEYOND_BLOCKED)assert(!pd_beyond_take());
    return result;
}
int main(int argc,char **argv){
    assert(argc==2);FILE *f=fopen(argv[1],"rb");assert(f);assert(!fseek(f,0,SEEK_END));long size=ftell(f);assert(size>0&&size<PD_BEYOND_SOURCE_MAX);rewind(f);fixture=malloc(size+1);assert(fixture&&fread(fixture,1,size,f)==(size_t)size);fixture[size]=0;fclose(f);
    pd_beyond_boot();reset();run(PD_BEYOND_READY);pd_beyond_candidate *result=pd_beyond_take();assert(result&&pd_valid(&result->state)&&result->state.item_count==3&&result->report.limits>0);free(result);
    reset();wall=0;run(PD_BEYOND_READY);assert(sntp_calls==1&&wall>1704067200);free(pd_beyond_take());
    reset();wall=0;fail_clock=true;run(PD_BEYOND_TIME);assert(!created);
    reset();network=false;run(PD_BEYOND_NO_WIFI);assert(!created);
    for(unsigned i=0;i<5;i++){reset();status_code=(unsigned[]){301,401,403,404,429}[i];run((unsigned[]){PD_BEYOND_REDIRECT,PD_BEYOND_DENIED,PD_BEYOND_DENIED,PD_BEYOND_NOT_FOUND,PD_BEYOND_RATE}[i]);assert(body_at==0);}
    reset();fail_open=true;run(PD_BEYOND_NETWORK);
    reset();mime="text/html";run(PD_BEYOND_MIME);assert(body_at==0);
    reset();duplicate_mime=true;run(PD_BEYOND_MIME);
    reset();encoding="gzip";run(PD_BEYOND_MIME);
    reset();declared=PD_BEYOND_SOURCE_MAX+1;run(PD_BEYOND_SIZE);assert(body_at==0);
    reset();declared=-1;run(PD_BEYOND_NETWORK);
    reset();declared=0;run(PD_BEYOND_READY);free(pd_beyond_take());
    reset();complete=false;run(PD_BEYOND_NETWORK);
    reset();declared=(int64_t)body_size+1;run(PD_BEYOND_NETWORK);
    reset();declared=(int64_t)body_size-1;run(PD_BEYOND_NETWORK);
    reset();body="<html>not JSON</html>";body_size=strlen(body);declared=body_size;run(PD_BEYOND_INVALID);
    reset();read_advance=21000;run(PD_BEYOND_TIMEOUT);
    reset();cancel_read=true;run(PD_BEYOND_CANCELLED);
    reset();cancel_resume=true;run(PD_BEYOND_CANCELLED);assert(!pd_beyond_take());
    reset();fail_memory=true;run(PD_BEYOND_MEMORY);
    reset();fail_task=true;assert(!pd_beyond_start("123456")&&!info.running&&info.error==PD_BEYOND_MEMORY&&resumes==1);
    reset();assert(!pd_beyond_start("https://evil.test")&&!created&&!queued_worker);
    reset();run(PD_BEYOND_READY);pd_beyond_cancel();assert(!pd_beyond_take()&&info.error==PD_BEYOND_CANCELLED);
    reset();account_mode=true;auth_calls=list_calls=private_calls=account_saves=0;
    pd_account_access own={.version=1};strcpy(own.user,"1234");strcpy(own.cookie,"synthetic-session");
    assert(pd_beyond_link(&own,true));queued_worker(NULL);pd_beyond_info account_info;pd_beyond_snapshot(&account_info);assert(!account_info.running&&account_info.error==0&&account_info.configured&&account_info.remembered&&account_info.count==2&&account_saves==1&&auth_calls==1&&list_calls==1);pd_beyond_cancel();pd_beyond_snapshot(&account_info);assert(account_info.phase==PD_BEYOND_FINISHED&&account_info.configured);
    pd_account_row row;assert(pd_beyond_row(0,&row)&&!strcmp(row.id,"123456")&&!pd_beyond_owned(2));
    assert(pd_beyond_owned(0));queued_worker(NULL);pd_beyond_snapshot(&account_info);assert(account_info.error==0&&private_calls==1&&auth_calls==2);free(pd_beyond_take());
    assert(pd_beyond_list());queued_worker(NULL);assert(list_calls==2&&auth_calls==3);
    status_code=401;assert(pd_beyond_list());queued_worker(NULL);pd_beyond_snapshot(&account_info);assert(account_info.error==PD_BEYOND_SESSION&&account_info.configured&&account_info.reconnect&&account_saves==1);unsigned rejected_calls=auth_calls;assert(!pd_beyond_list()&&!pd_beyond_owned(0)&&auth_calls==rejected_calls);status_code=200;
    assert(pd_beyond_forget());pd_beyond_snapshot(&account_info);assert(!account_info.configured&&!account_info.count&&!pd_beyond_list()&&!pd_account_valid(&stored_account));
    assert(pd_beyond_link(&own,false));queued_worker(NULL);pd_beyond_snapshot(&account_info);assert(account_info.configured&&!account_info.remembered&&!account_info.reconnect&&!pd_account_valid(&stored_account));
    strcpy(own.user,"9999");assert(pd_beyond_link(&own,true));queued_worker(NULL);pd_beyond_snapshot(&account_info);assert(account_info.error==PD_BEYOND_IDENTITY&&account_info.configured&&!account_info.reconnect&&!strcmp(account_access.user,"1234")&&account_saves==1);
    strcpy(own.user,"1234");cancel_resume=true;assert(pd_beyond_link(&own,true));queued_worker(NULL);pd_beyond_snapshot(&account_info);assert(account_info.error==PD_BEYOND_CANCELLED&&account_saves==1);cancel_resume=false;
    fail_account_store=true;assert(pd_beyond_link(&own,true));queued_worker(NULL);pd_beyond_snapshot(&account_info);assert(account_info.error==PD_BEYOND_STORAGE&&account_info.locked&&!pd_beyond_list());
    reset();account_mode=true;assert(pd_beyond_link(&own,true));queued_worker(NULL);
    pd_beyond_snapshot(&account_info);assert(account_info.listed&&!account_info.reconnect);
    /* A dropped connection keeps the valid account/catalog usable. */
    fail_open=true;assert(pd_beyond_owned_id("123456"));queued_worker(NULL);pd_beyond_snapshot(&account_info);
    assert(account_info.error==PD_BEYOND_NETWORK&&!account_info.reconnect&&account_info.count==2&&account_info.listed);
    fail_open=false;assert(!pd_beyond_owned_id("999999")&&!pd_beyond_owned_id("https://invalid.test"));
    assert(pd_beyond_owned_id("123456"));queued_worker(NULL);free(pd_beyond_take());
    deny_private=true;assert(pd_beyond_owned_id("123456"));queued_worker(NULL);pd_beyond_snapshot(&account_info);
    assert(account_info.error==PD_BEYOND_DENIED&&!account_info.reconnect&&account_info.count==2);deny_private=false;
    /* An empty, already loaded catalog must not be treated as uninitialized. */
    catalog->count=0;pd_beyond_snapshot(&account_info);assert(account_info.listed&&account_info.count==0);
    assert(!pd_beyond_owned_id("123456"));assert(pd_beyond_list());queued_worker(NULL);
    pd_beyond_snapshot(&account_info);assert(account_info.listed&&account_info.count==2);
    assert(pd_beyond_forget());pd_beyond_snapshot(&account_info);assert(!account_info.listed&&!account_info.reconnect);
    pd_account_wipe(&own,sizeof own);free(catalog);catalog=NULL;
    puts("Account worker: synthetic auth/catalog/private requests, fixed host credential separation, subject binding, remembered/RAM-only association, owned selection, fresh authentication, forget, old-account retention, late cancellation and storage quarantine passed. No real authenticated HTTP/hardware claim.");
    free(fixture);
    puts("Beyond worker: actual converter with fake ESP HTTP/clock/tasks; fixed GET URL, CA/hostname/redirect configuration, identity encoding, clock gates, content/status/size/truncation checks, timeout/cancel/late-cancel, no stale result, bridge pause and bounded failure paths passed. No real TLS/RF hardware claim.");
}
