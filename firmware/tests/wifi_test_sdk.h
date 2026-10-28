/* Deliberately fake SDK for exercising production Wi-Fi service failure paths.
   It proves neither radio behavior nor actual NVS persistence. */
#pragma once
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <arpa/inet.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_STATE 1
#define ESP_ERR_NVS_NOT_FOUND 2
#define ESP_LOG_ERROR 0
#define ESP_EVENT_ANY_ID -1
#define WIFI_STORAGE_RAM 1
#define WIFI_MODE_STA 1
#define WIFI_MODE_APSTA 3
#define WIFI_IF_STA 0
#define WIFI_IF_AP 1
#define WIFI_AUTH_OPEN 0
#define WIFI_AUTH_WPA2_PSK 3
#define ESP_MAC_WIFI_SOFTAP 1
#define WIFI_EVENT mock_wifi_event
static const char mock_wifi_event[]="wifi";
#define IP_EVENT mock_ip_event
static const char mock_ip_event[]="ip";
#define IP_EVENT_STA_GOT_IP 1
#define WIFI_EVENT_STA_DISCONNECTED 2
#define NVS_READONLY 0
#define NVS_READWRITE 1
#define pdTRUE 1
#define pdPASS 1
#define portMAX_DELAY UINT32_MAX
#define portTICK_PERIOD_MS 1
#define pdMS_TO_TICKS(v) (v)
#define HTTPD_403_FORBIDDEN 403
#define HTTPD_400_BAD_REQUEST 400
#define HTTP_GET 0
#define HTTP_POST 1
#define IPSTR "%u.%u.%u.%u"
#define IP2STR(v) 192u,168u,1u,23u
#define WIFI_INIT_CONFIG_DEFAULT() ((wifi_init_config_t){.nvs_enable=1})
#define ESP_NETIF_DEFAULT_WIFI_STA() ((esp_netif_config_t){.station=true})
#define ESP_NETIF_DEFAULT_WIFI_AP() ((esp_netif_config_t){.station=false})
typedef struct {bool station;} esp_netif_config_t;
typedef struct {int nvs_enable;} wifi_init_config_t;
typedef struct {uint8_t ssid[32],password[64];unsigned ssid_len,authmode,max_connection,channel;struct{unsigned authmode;}threshold;struct{bool capable;}pmf_cfg;} wifi_part;
typedef union {wifi_part sta,ap;} wifi_config_t;
typedef struct {uint8_t ssid[33];} wifi_ap_record_t;
typedef struct {int ip;} esp_netif_ip_info_t;
typedef int esp_netif_t;
typedef const char *esp_event_base_t;
typedef int nvs_handle_t;
typedef void *QueueHandle_t;
typedef void *SemaphoreHandle_t;
typedef void *httpd_handle_t;
typedef struct {unsigned max_open_sockets;bool lru_purge_enable;unsigned stack_size;} httpd_config_t;
#define HTTPD_DEFAULT_CONFIG() ((httpd_config_t){0})
typedef struct {int content_len;const char *host,*origin,*type,*body;unsigned at;int status;char response[8192];} httpd_req_t;
typedef struct {const char *uri;int method;esp_err_t(*handler)(httpd_req_t*);} httpd_uri_t;
static unsigned mock_clock,mock_ticks,mock_inits,mock_starts,mock_stops,mock_stores,mock_erases,mock_commits;
static bool mock_rf,mock_sar=true,mock_associated,mock_fail_init,mock_fail_start,mock_fail_stop,mock_fail_store,mock_fail_connect;
static unsigned char mock_blob[2048],mock_staged[2048],mock_queue[3][256];static size_t mock_blob_size,mock_staged_size,mock_request_size;static unsigned mock_queued;
static wifi_config_t mock_config;static jmp_buf mock_yield;static int mock_netif;static bool mock_netif_fail;
static void mock_run(unsigned ticks);
static void *xSemaphoreCreateMutex(void){return (void*)1;}
static int xSemaphoreTake(SemaphoreHandle_t l,unsigned wait){(void)l;(void)wait;return 1;}
static void xSemaphoreGive(SemaphoreHandle_t l){(void)l;}
static void *xQueueCreate(unsigned n,size_t size){assert(n==3&&size<=256);mock_request_size=size;return (void*)1;}
static int xQueueSend(void *q,const void *p,unsigned wait){(void)q;(void)wait;if(mock_queued==3)return 0;memcpy(mock_queue[mock_queued++],p,mock_request_size);return 1;}
static int xQueueReceive(void *q,void *p,unsigned wait){(void)q;if(!mock_ticks)longjmp(mock_yield,1);mock_ticks--;mock_clock+=wait;if(!mock_queued)return 0;memcpy(p,mock_queue[0],mock_request_size);mock_queued--;memmove(mock_queue,mock_queue+1,mock_queued*256);return 1;}
static void vQueueDelete(void *q){(void)q;}
static int xTaskCreate(void(*fn)(void*),const char *name,unsigned stack,void *ctx,unsigned priority,void *out){(void)fn;(void)name;(void)stack;(void)ctx;(void)priority;(void)out;return 1;}
static unsigned xTaskGetTickCount(void){return mock_clock;}
static void vTaskDelay(unsigned n){mock_clock+=n;}
static void bootloader_random_enable(void){assert(!mock_rf);mock_sar=true;}
static void bootloader_random_disable(void){mock_sar=false;}
static uint32_t esp_random(void){assert(mock_rf||mock_sar);return 12345;}
static void esp_fill_random(void *p,size_t n){assert(mock_rf||mock_sar);memset(p,0x12,n);}
static int esp_read_mac(uint8_t *p,int kind){(void)kind;memset(p,0x34,6);return 0;}
static int esp_netif_init(void){return 0;}
static int esp_event_loop_create_default(void){return 0;}
static esp_netif_t *esp_netif_new(const esp_netif_config_t *c){(void)c;return mock_netif_fail?NULL:&mock_netif;}
static int esp_netif_attach_wifi_station(esp_netif_t *n){(void)n;return 0;}
static int esp_netif_attach_wifi_ap(esp_netif_t *n){(void)n;return 0;}
static int esp_wifi_set_default_wifi_sta_handlers(void){return 0;}
static int esp_wifi_set_default_wifi_ap_handlers(void){return 0;}
static void esp_netif_destroy_default_wifi(esp_netif_t *n){(void)n;}
static int esp_wifi_init(const wifi_init_config_t *c){assert(!mock_sar&&!c->nvs_enable);mock_inits++;return mock_fail_init?-1:0;}
static int esp_wifi_set_storage(int type){assert(type==WIFI_STORAGE_RAM);return 0;}
static int esp_wifi_set_mode(unsigned mode){assert(mode==WIFI_MODE_STA||mode==WIFI_MODE_APSTA);return 0;}
static int esp_wifi_start(void){assert(!mock_sar);mock_starts++;mock_rf=!mock_fail_start;return mock_fail_start?-1:0;}
static int esp_wifi_stop(void){assert(!mock_sar);mock_stops++;if(mock_fail_stop)return -1;mock_rf=mock_associated=false;return 0;}
static int esp_wifi_disconnect(void){mock_associated=false;return 0;}
static int esp_wifi_set_config(int iface,const wifi_config_t *c){if(iface==WIFI_IF_STA)mock_config=*c;else assert(c->ap.authmode==WIFI_AUTH_WPA2_PSK&&c->ap.ssid_len&&strlen((const char*)c->ap.password)==16);return 0;}
static int esp_wifi_connect(void){return mock_fail_connect?-1:0;}
static int esp_wifi_sta_get_ap_info(wifi_ap_record_t *ap){if(!mock_associated)return -1;memset(ap,0,sizeof *ap);memcpy(ap->ssid,mock_config.sta.ssid,32);return 0;}
static int esp_event_handler_register(esp_event_base_t b,int id,void(*handler)(void*,esp_event_base_t,int32_t,void*),void *ctx){(void)b;(void)id;(void)handler;(void)ctx;return 0;}
static void esp_log_level_set(const char *tag,int level){(void)tag;(void)level;}
static esp_netif_t *esp_netif_get_handle_from_ifkey(const char *key){assert(!strcmp(key,"WIFI_STA_DEF"));return &mock_netif;}
static int esp_netif_get_ip_info(esp_netif_t *net,esp_netif_ip_info_t *ip){(void)net;ip->ip=1;return 0;}
static const char *mock_namespace="pdd_wifi";
static int nvs_flash_init_partition(const char *p){assert(!strcmp(p,"pdd_nvs"));return 0;}
static int nvs_open_from_partition(const char *p,const char *ns,int mode,nvs_handle_t *h){assert(!strcmp(p,"pdd_nvs")&&!strcmp(ns,mock_namespace));*h=1;return mode==NVS_READONLY&&!mock_blob_size?ESP_ERR_NVS_NOT_FOUND:0;}
static int nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *n){(void)h;assert(!strcmp(key,"config"));if(!mock_blob_size||*n<mock_blob_size)return -1;memcpy(out,mock_blob,mock_blob_size);*n=mock_blob_size;return 0;}
static int nvs_set_blob(nvs_handle_t h,const char *key,const void *p,size_t n){(void)h;assert(!strcmp(key,"config")&&n<=2048);mock_stores++;if(mock_fail_store)return -1;memcpy(mock_staged,p,n);mock_staged_size=n;return 0;}
static int nvs_erase_all(nvs_handle_t h){assert(h==1);mock_erases++;mock_staged_size=0;return mock_fail_store?-1:0;}
static int nvs_commit(nvs_handle_t h){(void)h;mock_commits++;if(mock_fail_store)return -1;memcpy(mock_blob,mock_staged,mock_staged_size);mock_blob_size=mock_staged_size;return 0;}
static void nvs_close(nvs_handle_t h){(void)h;}
static int httpd_start(httpd_handle_t *h,const httpd_config_t *cfg){assert(cfg->max_open_sockets==4);*h=(void*)1;return 0;}
static int httpd_stop(httpd_handle_t h){assert(h);return 0;}
static int httpd_register_uri_handler(httpd_handle_t h,const httpd_uri_t *uri){assert(h&&uri->uri&&uri->handler);return 0;}
static const char *mock_hdr(httpd_req_t *r,const char *key){return !strcmp(key,"Host")?r->host:!strcmp(key,"Origin")?r->origin:r->type;}
static size_t httpd_req_get_hdr_value_len(httpd_req_t *r,const char *key){const char *s=mock_hdr(r,key);return s?strlen(s):0;}
static int httpd_req_get_hdr_value_str(httpd_req_t *r,const char *key,char *out,size_t n){const char *s=mock_hdr(r,key);if(!s||strlen(s)>=n)return -1;strcpy(out,s);return 0;}
static int httpd_req_to_sockfd(httpd_req_t *r){(void)r;return 1;}
static int mock_getsockname(int fd,struct sockaddr *p,socklen_t *n){(void)fd;(void)n;((struct sockaddr_in*)p)->sin_addr.s_addr=inet_addr("192.168.4.1");return 0;}
#define getsockname mock_getsockname
static int httpd_req_recv(httpd_req_t *r,char *out,size_t n){assert(r->at+n<=(unsigned)r->content_len);memcpy(out,r->body+r->at,n);r->at+=n;return n;}
static int httpd_resp_set_hdr(httpd_req_t *r,const char *name,const char *value){(void)r;(void)name;(void)value;return 0;}
static int httpd_resp_set_type(httpd_req_t *r,const char *type){(void)r;(void)type;return 0;}
static int httpd_resp_set_status(httpd_req_t *r,const char *status){r->status=atoi(status);return 0;}
static int httpd_resp_sendstr(httpd_req_t *r,const char *text){if(text){assert(strlen(r->response)+strlen(text)<sizeof r->response);strcat(r->response,text);}return 0;}
static int httpd_resp_sendstr_chunk(httpd_req_t *r,const char *text){return httpd_resp_sendstr(r,text);}
static int httpd_resp_send_err(httpd_req_t *r,int status,const char *text){r->status=status;return httpd_resp_sendstr(r,text);}
