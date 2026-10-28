#pragma once
#include "wifi_test_sdk.h"
#include <openssl/sha.h>
#include <sys/time.h>
static int mock_settimeofday(const struct timeval *t,const void *zone){(void)t;(void)zone;return 0;}
#define settimeofday mock_settimeofday
static int mbedtls_sha256(const unsigned char *p,size_t n,unsigned char *out,int type){assert(type==0);return SHA256(p,n,out)?0:-1;}
typedef void *esp_http_client_handle_t;
typedef struct {int event_id;void *user_data,*data;size_t data_len;} esp_http_client_event_t;
typedef struct {const char *url,*cert_pem;unsigned timeout_ms;bool disable_auto_redirect;int(*event_handler)(esp_http_client_event_t*);void *user_data;unsigned buffer_size;} esp_http_client_config_t;
#define HTTP_EVENT_ON_DATA 1
#define HTTP_METHOD_POST 1
static esp_http_client_handle_t esp_http_client_init(const esp_http_client_config_t *c){assert(c->cert_pem&&c->disable_auto_redirect);return (void*)1;}
static void esp_http_client_set_method(void *h,int m){(void)h;(void)m;}
static void esp_http_client_set_header(void *h,const char *k,const char *v){(void)h;(void)k;(void)v;}
static void esp_http_client_set_post_field(void *h,const char *p,size_t n){(void)h;(void)p;(void)n;}
static int esp_http_client_perform(void *h){(void)h;return -1;}
static int esp_http_client_get_status_code(void *h){(void)h;return 500;}
static void esp_http_client_cleanup(void *h){(void)h;}
