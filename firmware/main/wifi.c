#include "wifi.h"
#include "wifi_setup.h"
#ifdef PD_WIFI_TEST
#include "wifi_test_sdk.h"
#else
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_random.h"
#include "bootloader_random.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "lwip/sockets.h"
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#define SETUP_MS 300000u
#define CONNECT_MS 30000u
static struct { char magic[4],ssid[33],password[65];uint8_t open,enabled; } saved;
typedef struct { unsigned kind;uint32_t generation;pd_wifi_credentials credentials; } request;
static QueueHandle_t requests;static SemaphoreHandle_t lock,entropy_lock;
static pd_wifi_info current,view;
static uint32_t generation,published_generation;static char published_csrf[33];
static esp_netif_t *sta_net,*ap_net;static bool driver_ready,wifi_registered,ip_registered;static pd_wifi_credentials trial;
static bool initialized,running,wanted,paused,resume_wanted,pending;
static atomic_bool got_ip,disconnected,provision_busy;
static uint32_t started,setup_started,retry_at;static unsigned retries;
static httpd_handle_t http;static char csrf[33];
static uint32_t now_ms(void){return (uint32_t)(xTaskGetTickCount()*portTICK_PERIOD_MS);}
static void publish(void){current.configured=saved.ssid[0]!=0;current.suspended=paused;current.radio_active=running;xSemaphoreTake(lock,portMAX_DELAY);view=current;published_generation=generation;memcpy(published_csrf,csrf,sizeof csrf);xSemaphoreGive(lock);}
static bool storage(bool erase){
 nvs_handle_t h;if(nvs_flash_init_partition("pdd_nvs")!=ESP_OK||nvs_open_from_partition("pdd_nvs","pdd_wifi",NVS_READWRITE,&h)!=ESP_OK)return false;
 esp_err_t e=erase?nvs_erase_all(h):nvs_set_blob(h,"config",&saved,sizeof saved);if(e==ESP_OK)e=nvs_commit(h);nvs_close(h);return e==ESP_OK;
}
static void load(void){
 nvs_handle_t h;if(nvs_flash_init_partition("pdd_nvs")!=ESP_OK){current.error=PD_WIFI_ERROR_STORAGE;return;}
 esp_err_t e=nvs_open_from_partition("pdd_nvs","pdd_wifi",NVS_READONLY,&h);if(e==ESP_ERR_NVS_NOT_FOUND)return;if(e!=ESP_OK){current.error=PD_WIFI_ERROR_STORAGE;return;}
 size_t n=sizeof saved;e=nvs_get_blob(h,"config",&saved,&n);nvs_close(h);
 pd_wifi_credentials c={0};memcpy(c.ssid,saved.ssid,33);memcpy(c.password,saved.password,65);c.open=saved.open==1;
 if(e!=ESP_OK||n!=sizeof saved||memcmp(saved.magic,"PWF1",4)||saved.open>1||saved.enabled>1||!pd_wifi_credentials_valid(&c)){memset(&saved,0,sizeof saved);current.error=PD_WIFI_ERROR_STORAGE;}memset(&c,0,sizeof c);publish();
}
static void event(void *arg,esp_event_base_t base,int32_t id,void *data){
 (void)arg;(void)data;
 if(base==IP_EVENT&&id==IP_EVENT_STA_GOT_IP)got_ip=true;
 if(base==WIFI_EVENT&&id==WIFI_EVENT_STA_DISCONNECTED)disconnected=true;

}
static esp_netif_t *create_netif(bool station){
 esp_netif_config_t config=station?(esp_netif_config_t)ESP_NETIF_DEFAULT_WIFI_STA():(esp_netif_config_t)ESP_NETIF_DEFAULT_WIFI_AP();
 esp_netif_t *net=esp_netif_new(&config);if(!net)return NULL;
 esp_err_t e=station?esp_netif_attach_wifi_station(net):esp_netif_attach_wifi_ap(net);
 if(e==ESP_OK)e=station?esp_wifi_set_default_wifi_sta_handlers():esp_wifi_set_default_wifi_ap_handlers();
 if(e!=ESP_OK){esp_netif_destroy_default_wifi(net);return NULL;}return net;
}
static bool init_radio(void){
 if(initialized)return true;
 esp_err_t e=esp_netif_init();if(e!=ESP_OK&&e!=ESP_ERR_INVALID_STATE)return false;
 e=esp_event_loop_create_default();if(e!=ESP_OK&&e!=ESP_ERR_INVALID_STATE)return false;
 if(!sta_net)sta_net=create_netif(true);
 if(!ap_net)ap_net=create_netif(false);
 if(!sta_net||!ap_net)return false;
 wifi_init_config_t config=WIFI_INIT_CONFIG_DEFAULT();config.nvs_enable=0;if(!driver_ready){xSemaphoreTake(entropy_lock,portMAX_DELAY);bootloader_random_disable();esp_err_t result=esp_wifi_init(&config);bootloader_random_enable();xSemaphoreGive(entropy_lock);if(result!=ESP_OK)return false;driver_ready=true;}
 esp_log_level_set("wifi",ESP_LOG_ERROR);
 if(esp_wifi_set_storage(WIFI_STORAGE_RAM)!=ESP_OK)return false;
 if(!wifi_registered){if(esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,event,NULL)!=ESP_OK)return false;wifi_registered=true;}
 if(!ip_registered){if(esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,event,NULL)!=ESP_OK)return false;ip_registered=true;}
 initialized=true;return true;
}
static bool start_radio(void){
 xSemaphoreTake(entropy_lock,portMAX_DELAY);bootloader_random_disable();esp_err_t e=esp_wifi_start();if(e!=ESP_OK)bootloader_random_enable();xSemaphoreGive(entropy_lock);return e==ESP_OK;
}
static bool stop_radio(void){
 if(!running)return true;
 xSemaphoreTake(entropy_lock,portMAX_DELAY);esp_err_t e=esp_wifi_stop();if(e==ESP_OK){running=false;bootloader_random_enable();}xSemaphoreGive(entropy_lock);return e==ESP_OK;
}
uint32_t pd_wifi_random(void){if(!entropy_lock)return esp_random();xSemaphoreTake(entropy_lock,portMAX_DELAY);uint32_t value=esp_random();xSemaphoreGive(entropy_lock);return value;}
static void stop_portal(void){
 generation++;memset(csrf,0,sizeof csrf);current.setup=false;current.setup_seconds=0;memset(current.setup_ssid,0,sizeof current.setup_ssid);memset(current.setup_password,0,sizeof current.setup_password);publish();
 if(http){httpd_stop(http);http=NULL;}if(running)esp_wifi_set_mode(WIFI_MODE_STA);
}
static bool host_ok(httpd_req_t *r){
 char host[48],origin[64];size_t n=httpd_req_get_hdr_value_len(r,"Host");if(!n||n>=sizeof host||httpd_req_get_hdr_value_str(r,"Host",host,sizeof host)!=ESP_OK)return false;
 if(strcmp(host,"192.168.4.1")&&strcmp(host,"192.168.4.1:80"))return false;
 n=httpd_req_get_hdr_value_len(r,"Origin");if(n&&(n>=sizeof origin||httpd_req_get_hdr_value_str(r,"Origin",origin,sizeof origin)!=ESP_OK||strcmp(origin,"http://192.168.4.1")))return false;
 struct sockaddr_in local;socklen_t size=sizeof local;if(getsockname(httpd_req_to_sockfd(r),(struct sockaddr*)&local,&size)||local.sin_addr.s_addr!=inet_addr("192.168.4.1"))return false;
 return true;
}
static void headers(httpd_req_t *r){httpd_resp_set_hdr(r,"Cache-Control","no-store");httpd_resp_set_hdr(r,"X-Content-Type-Options","nosniff");httpd_resp_set_hdr(r,"Content-Security-Policy","default-src 'self'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; connect-src 'self'; form-action 'self'; frame-ancestors 'none'; base-uri 'none'");}
static esp_err_t page(httpd_req_t *r){
 if(!host_ok(r)){httpd_resp_send_err(r,HTTPD_403_FORBIDDEN,"Setup non disponibile");return ESP_OK;}
 char token[33];xSemaphoreTake(lock,portMAX_DELAY);bool active=view.setup;memcpy(token,published_csrf,sizeof token);xSemaphoreGive(lock);if(!active){httpd_resp_send_err(r,HTTPD_403_FORBIDDEN,"Setup terminato");return ESP_OK;}
 headers(r);httpd_resp_set_type(r,"text/html; charset=utf-8");
 httpd_resp_sendstr_chunk(r,"<!doctype html><html lang=it><meta name=viewport content='width=device-width,initial-scale=1'><title>PocketD-D Wi-Fi</title><style>body{font:18px system-ui;background:#10242b;color:#efdbac;max-width:32rem;margin:auto;padding:24px}label,input,button{display:block;box-sizing:border-box;width:100%;margin:12px 0}input,button{font:inherit;padding:12px}input[type=checkbox]{width:auto;display:inline}button{background:#a2dec9}p{line-height:1.5}</style><h1>Collega PocketD-D</h1><p>Rete a 2,4 GHz. Le schede restano sul dispositivo anche senza Wi-Fi.</p><form><input type=hidden name=token value='");
 httpd_resp_sendstr_chunk(r,token);httpd_resp_sendstr_chunk(r,"'><label>Nome della rete (SSID)<input name=ssid required maxlength=32 autocomplete=off></label><label>Password<input name=password type=password required maxlength=64 autocomplete=new-password></label><label><input type=checkbox name=open value=1>Rete aperta, senza password</label><button>Prova e salva la rete</button></form><p id=result role=status>Nessuna password viene inviata a Beyond o inclusa nei backup del PG.</p><script>const f=document.querySelector('form'),o=document.querySelector('#result');let waiting=false,attempt=0;f.open.onchange=()=>{f.password.required=!f.open.checked;if(f.open.checked)f.password.value='';};f.onsubmit=async e=>{e.preventDefault();const id=++attempt;waiting=false;try{const r=await fetch('/connect',{method:'POST',body:new URLSearchParams(new FormData(f))});if(id!==attempt)return;waiting=r.ok;o.textContent=r.ok?'Connessione in corso. Attendi la conferma sul dispositivo.':'Dati non validi o configurazione occupata.';if(r.ok)f.password.value='';}catch{if(id!==attempt)return;o.textContent='Collegamento interrotto. Controlla il display e, se necessario, ricollegati alla rete PocketDD.'}};setInterval(async()=>{if(!waiting)return;const id=attempt;try{const r=await fetch('/state'),s=await r.json();if(id!==attempt||!waiting)return;if(s.storage)o.textContent='Salvataggio della rete non confermato. Controlla il dispositivo prima di riavviare.';else if(s.saved)o.textContent='Rete collegata e salvata. Puoi tornare al Wi-Fi abituale e chiudere la configurazione sul dispositivo.';else if(s.failed)o.textContent='Connessione non riuscita. La rete salvata in precedenza e conservata. Controlla nome e password.';if(s.saved||s.failed||s.storage)waiting=false;}catch{}},2000)</script></html>");httpd_resp_sendstr_chunk(r,NULL);return ESP_OK;
}
static esp_err_t connect_handler(httpd_req_t *r){
 if(!host_ok(r)||r->content_len<=0||r->content_len>512){httpd_resp_send_err(r,HTTPD_400_BAD_REQUEST,"Dati non validi");return ESP_OK;}
 char type[96],body[513],token[33];if(httpd_req_get_hdr_value_str(r,"Content-Type",type,sizeof type)!=ESP_OK||(strncmp(type,"application/x-www-form-urlencoded",33)||(type[33]&&type[33]!=';'))){httpd_resp_send_err(r,HTTPD_400_BAD_REQUEST,"Formato non valido");return ESP_OK;}
 size_t at=0;while(at<(size_t)r->content_len){int n=httpd_req_recv(r,body+at,r->content_len-at);if(n<=0){memset(body,0,sizeof body);return ESP_FAIL;}at+=n;}
 xSemaphoreTake(lock,portMAX_DELAY);bool active=view.setup;uint32_t gen=published_generation;memcpy(token,published_csrf,sizeof token);xSemaphoreGive(lock);
 request q={.kind=10,.generation=gen};bool okay=active&&pd_wifi_parse_form(body,at,token,&q.credentials);memset(body,0,sizeof body);memset(token,0,sizeof token);
 if(!okay){httpd_resp_send_err(r,HTTPD_400_BAD_REQUEST,"Nome o password non validi");return ESP_OK;}
 if(atomic_exchange(&provision_busy,true)){memset(&q,0,sizeof q);(httpd_resp_set_status(r,"503 Service Unavailable"),httpd_resp_sendstr(r,"Connessione gia in corso"));return ESP_OK;}
 bool queued=xQueueSend(requests,&q,0)==pdTRUE;if(!queued)provision_busy=false;memset(&q,0,sizeof q);headers(r);if(queued){httpd_resp_set_status(r,"202 Accepted");httpd_resp_sendstr(r,"Connessione richiesta");}else (httpd_resp_set_status(r,"503 Service Unavailable"),httpd_resp_sendstr(r,"Riprova tra poco"));return ESP_OK;
}
static esp_err_t state_handler(httpd_req_t *r){
 if(!host_ok(r)){httpd_resp_send_err(r,HTTPD_403_FORBIDDEN,"Setup non disponibile");return ESP_OK;}
 pd_wifi_info s;pd_wifi_snapshot(&s);bool busy=atomic_load(&provision_busy);headers(r);httpd_resp_set_type(r,"application/json");char text[128];snprintf(text,sizeof text,"{\"saved\":%s,\"failed\":%s,\"storage\":%s}",s.setup_saved&&!busy?"true":"false",s.setup_failed&&!busy?"true":"false",s.error==PD_WIFI_ERROR_STORAGE&&!busy?"true":"false");httpd_resp_sendstr(r,text);return ESP_OK;
}
static bool start_portal(void){
 if(!init_radio())return false;
 if(!running){if(esp_wifi_set_mode(WIFI_MODE_STA)!=ESP_OK||!start_radio())return false;running=true;}
 uint8_t random[32],mac[6];esp_fill_random(random,sizeof random);esp_read_mac(mac,ESP_MAC_WIFI_SOFTAP);
 for(unsigned i=0;i<16;i++){csrf[i*2]="0123456789abcdef"[random[i]>>4];csrf[i*2+1]="0123456789abcdef"[random[i]&15];}
 snprintf(current.setup_ssid,sizeof current.setup_ssid,"PocketDD-%02X%02X%02X",mac[3],mac[4],mac[5]);for(unsigned i=0;i<16;i++)current.setup_password[i]="ABCDEFGHJKLMNPQRSTUVWXYZ23456789"[random[i+16]%32];
 wifi_config_t config={0};memcpy(config.ap.ssid,current.setup_ssid,strlen(current.setup_ssid));config.ap.ssid_len=strlen(current.setup_ssid);memcpy(config.ap.password,current.setup_password,16);config.ap.authmode=WIFI_AUTH_WPA2_PSK;config.ap.max_connection=1;config.ap.channel=1;
 if(esp_wifi_set_mode(WIFI_MODE_APSTA)!=ESP_OK||esp_wifi_set_config(WIFI_IF_AP,&config)!=ESP_OK){memset(&config,0,sizeof config);return false;}memset(&config,0,sizeof config);
 httpd_config_t cfg=HTTPD_DEFAULT_CONFIG();cfg.max_open_sockets=4;cfg.lru_purge_enable=true;cfg.stack_size=6144;
 if(httpd_start(&http,&cfg)!=ESP_OK)return false;
 httpd_uri_t root={.uri="/",.method=HTTP_GET,.handler=page},post={.uri="/connect",.method=HTTP_POST,.handler=connect_handler},status={.uri="/state",.method=HTTP_GET,.handler=state_handler};
 if(httpd_register_uri_handler(http,&root)!=ESP_OK||httpd_register_uri_handler(http,&post)!=ESP_OK||httpd_register_uri_handler(http,&status)!=ESP_OK){httpd_stop(http);http=NULL;return false;}
 current.setup=true;current.setup_saved=current.setup_failed=false;generation++;setup_started=now_ms();current.setup_seconds=300;publish();return true;
}
static bool station(const pd_wifi_credentials *c){
 if(!init_radio())return false;
 retry_at=0;got_ip=disconnected=false;
 if(running){esp_wifi_disconnect();wifi_ap_record_t ap;for(unsigned i=0;i<20&&esp_wifi_sta_get_ap_info(&ap)==ESP_OK;i++)vTaskDelay(pdMS_TO_TICKS(20));if(esp_wifi_sta_get_ap_info(&ap)==ESP_OK)return false;}
 wifi_config_t config={0};memcpy(config.sta.ssid,c->ssid,strlen(c->ssid));memcpy(config.sta.password,c->password,strlen(c->password));config.sta.threshold.authmode=c->open?WIFI_AUTH_OPEN:WIFI_AUTH_WPA2_PSK;config.sta.pmf_cfg.capable=true;
 if(esp_wifi_set_mode(current.setup?WIFI_MODE_APSTA:WIFI_MODE_STA)!=ESP_OK||esp_wifi_set_config(WIFI_IF_STA,&config)!=ESP_OK){memset(&config,0,sizeof config);return false;}memset(&config,0,sizeof config);
 if(!running){if(!start_radio())return false;running=true;}
 got_ip=disconnected=false;started=now_ms();retries=0;current.state=PD_WIFI_CONNECTING;snprintf(current.ssid,sizeof current.ssid,"%s",c->ssid);current.ip[0]=0;return esp_wifi_connect()==ESP_OK;
}
static void saved_station(void){pd_wifi_credentials c={0};memcpy(c.ssid,saved.ssid,33);memcpy(c.password,saved.password,65);c.open=saved.open==1;if(saved.ssid[0]&&station(&c))wanted=true;else{current.state=PD_WIFI_FAILED;current.error=PD_WIFI_ERROR_NETWORK;}memset(&c,0,sizeof c);}
static void radio_off(void){retry_at=0;stop_portal();bool stopped=stop_radio();pending=false;provision_busy=false;wanted=false;current.state=stopped?PD_WIFI_OFF:PD_WIFI_FAILED;if(!stopped)current.error=PD_WIFI_ERROR_UNAVAILABLE;current.ip[0]=0;memset(&trial,0,sizeof trial);}
static void service(void *unused){
 (void)unused;request q;for(;;){bool received=xQueueReceive(requests,&q,pdMS_TO_TICKS(200))==pdTRUE;
  if(received){
   if(q.kind==10&&current.setup&&!pending&&q.generation==generation){trial=q.credentials;pending=true;provision_busy=true;current.setup_saved=current.setup_failed=false;current.error=0;if(!station(&trial)){pending=false;provision_busy=false;memset(&trial,0,sizeof trial);current.setup_failed=true;current.error=PD_WIFI_ERROR_NETWORK;if(saved.enabled&&saved.ssid[0])saved_station();}}
   else if(q.kind==PD_WIFI_SETUP&&!paused){pending=false;provision_busy=false;memset(&trial,0,sizeof trial);stop_portal();current.error=0;if(!start_portal()){stop_portal();if(!wanted)stop_radio();current.error=PD_WIFI_ERROR_UNAVAILABLE;}}
   else if(q.kind==PD_WIFI_ENABLE&&!paused){current.error=0;if(saved.ssid[0]){if(!saved.enabled){saved.enabled=1;if(!storage(false))current.error=PD_WIFI_ERROR_STORAGE;}}saved_station();}
   else if(q.kind==PD_WIFI_DISABLE){radio_off();saved.enabled=0;if(saved.ssid[0]&&!storage(false))current.error=PD_WIFI_ERROR_STORAGE;}
   else if(q.kind==PD_WIFI_FORGET){radio_off();if(storage(true)){memset(&saved,0,sizeof saved);if(current.state==PD_WIFI_OFF)current.error=0;}else current.error=PD_WIFI_ERROR_STORAGE;}
   else if(q.kind==20){resume_wanted=wanted;radio_off();paused=true;}
   else if(q.kind==21){paused=false;if(resume_wanted)saved_station();resume_wanted=false;}
   if(q.kind==10&&!pending)provision_busy=false;
   memset(&q,0,sizeof q);
  }
  uint32_t now=now_ms();
  if(atomic_exchange(&got_ip,false)&&running&&(wanted||pending)){wifi_ap_record_t ap;if(esp_wifi_sta_get_ap_info(&ap)==ESP_OK&&(!pending||(strnlen((const char*)ap.ssid,32)==strlen(trial.ssid)&&!memcmp(ap.ssid,trial.ssid,strlen(trial.ssid))))){
    current.state=PD_WIFI_CONNECTED;current.error=0;retry_at=0;retries=0;started=now;esp_netif_ip_info_t ip;esp_netif_t *net=esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");if(net&&esp_netif_get_ip_info(net,&ip)==ESP_OK)snprintf(current.ip,sizeof current.ip,IPSTR,IP2STR(&ip.ip));
    if(pending){__typeof__(saved) old=saved;memset(&saved,0,sizeof saved);memcpy(saved.magic,"PWF1",4);memcpy(saved.ssid,trial.ssid,33);memcpy(saved.password,trial.password,65);saved.open=trial.open;saved.enabled=1;if(!storage(false)){saved=old;current.error=PD_WIFI_ERROR_STORAGE;current.setup_failed=true;}else current.setup_saved=true;memset(&old,0,sizeof old);memset(&trial,0,sizeof trial);pending=false;provision_busy=false;wanted=true;}
   }}
  if(atomic_exchange(&disconnected,false)&&running&&(wanted||pending)&&current.state!=PD_WIFI_OFF){if(current.state==PD_WIFI_CONNECTED){started=now;retries=0;}if(retries<3&&(uint32_t)(now-started)<CONNECT_MS){retries++;retry_at=now+1000;}else started=now-CONNECT_MS;current.state=PD_WIFI_CONNECTING;}
  if(retry_at&&(int32_t)(now-retry_at)>=0&&running){retry_at=0;esp_wifi_connect();}
  if(current.state==PD_WIFI_CONNECTING&&(uint32_t)(now-started)>=CONNECT_MS){current.state=PD_WIFI_FAILED;current.error=PD_WIFI_ERROR_NETWORK;if(pending){pending=false;provision_busy=false;current.setup_failed=true;memset(&trial,0,sizeof trial);if(saved.enabled&&saved.ssid[0])saved_station();}}
  if(current.setup){uint32_t elapsed=now-setup_started;if(elapsed>=SETUP_MS){stop_portal();if(!wanted&&!pending)stop_radio();}else current.setup_seconds=(SETUP_MS-elapsed+999)/1000;}
  publish();
 }
}
void pd_wifi_boot(void){
 if(requests)return;
 lock=xSemaphoreCreateMutex();entropy_lock=xSemaphoreCreateMutex();requests=xQueueCreate(3,sizeof(request));if(!lock||!entropy_lock||!requests){if(requests)vQueueDelete(requests);requests=NULL;return;}load();publish();if(xTaskCreate(service,"pdd_wifi",6144,NULL,4,NULL)!=pdPASS){vQueueDelete(requests);requests=NULL;return;}if(saved.enabled&&saved.ssid[0])pd_wifi_command(PD_WIFI_ENABLE);
}
void pd_wifi_snapshot(pd_wifi_info *out){if(!out)return;memset(out,0,sizeof *out);if(!lock||!requests){out->error=PD_WIFI_ERROR_UNAVAILABLE;return;}if(xSemaphoreTake(lock,pdMS_TO_TICKS(20))==pdTRUE){*out=view;xSemaphoreGive(lock);}else out->error=PD_WIFI_ERROR_UNAVAILABLE;}
bool pd_wifi_command(unsigned command){if(!requests||command<1||command>4)return false;request q={.kind=command};return xQueueSend(requests,&q,0)==pdTRUE;}
bool pd_wifi_pause(bool value){
 if(!requests)return true;
 request q={.kind=value?20:21};if(xQueueSend(requests,&q,pdMS_TO_TICKS(50))!=pdTRUE)return false;
 if(value){uint32_t begin=now_ms();for(;;){pd_wifi_info info;pd_wifi_snapshot(&info);if(info.error!=PD_WIFI_ERROR_UNAVAILABLE&&info.suspended&&!info.radio_active&&info.state==PD_WIFI_OFF&&!info.setup)return true;if((uint32_t)(now_ms()-begin)>=3000)return false;vTaskDelay(pdMS_TO_TICKS(20));}}return true;
}
