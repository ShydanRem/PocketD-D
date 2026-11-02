#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_STATE 2
typedef void *SemaphoreHandle_t;
#define portMAX_DELAY 0xffffffffu
#define pdMS_TO_TICKS(x) (x)
#define pdPASS 1
#define MALLOC_CAP_SPIRAM 1
#define MALLOC_CAP_8BIT 2
SemaphoreHandle_t xSemaphoreCreateMutex(void);
int xSemaphoreTake(SemaphoreHandle_t,uint32_t);
int xSemaphoreGive(SemaphoreHandle_t);
uint32_t xTaskGetTickCount(void);
void vTaskDelay(uint32_t);
void vTaskDelete(void *);
int xTaskCreate(void (*)(void *),const char *,uint32_t,void *,unsigned,void *);
void *heap_caps_malloc(size_t,unsigned);
void *heap_caps_calloc(size_t,size_t,unsigned);
typedef struct {const char *server;} esp_sntp_config_t;
#define ESP_NETIF_SNTP_DEFAULT_CONFIG(value) {.server=value}
esp_err_t esp_netif_sntp_init(const esp_sntp_config_t *);
esp_err_t esp_netif_sntp_sync_wait(uint32_t);
typedef struct {unsigned event_id;void *user_data;const char *header_key,*header_value;} esp_http_client_event_t;
typedef struct {const char *url;unsigned method,timeout_ms;bool disable_auto_redirect,skip_cert_common_name_check;esp_err_t (*crt_bundle_attach)(void *);esp_err_t (*event_handler)(esp_http_client_event_t *);void *user_data;unsigned buffer_size,buffer_size_tx;} esp_http_client_config_t;
typedef void *esp_http_client_handle_t;
#define HTTP_METHOD_GET 0
#define HTTP_METHOD_POST 1
int esp_http_client_write(esp_http_client_handle_t,const char *,int);
#define HTTP_EVENT_ON_HEADER 1
esp_err_t esp_crt_bundle_attach(void *);
esp_http_client_handle_t esp_http_client_init(const esp_http_client_config_t *);
esp_err_t esp_http_client_set_header(esp_http_client_handle_t,const char *,const char *);
esp_err_t esp_http_client_open(esp_http_client_handle_t,unsigned);
int64_t esp_http_client_fetch_headers(esp_http_client_handle_t);
int esp_http_client_get_status_code(esp_http_client_handle_t);
int esp_http_client_read(esp_http_client_handle_t,char *,int);
bool esp_http_client_is_complete_data_received(esp_http_client_handle_t);
esp_err_t esp_http_client_close(esp_http_client_handle_t);
esp_err_t esp_http_client_cleanup(esp_http_client_handle_t);
