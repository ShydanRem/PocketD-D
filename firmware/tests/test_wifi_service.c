#define _POSIX_C_SOURCE 200809L
#define PD_WIFI_TEST 1
#include "../main/wifi.c"
bool pd_sync_pause(bool value){(void)value;return true;}
static bool mock_stage;static unsigned mock_stage_calls;
bool pd_sync_stage_json(const char *json,size_t size){assert(size==2&&!strcmp(json,"{}"));mock_stage_calls++;return mock_stage;}
static void mock_run(unsigned ticks){mock_ticks=ticks;if(!setjmp(mock_yield))service(NULL);}
static void command(unsigned cmd){assert(pd_wifi_command(cmd));mock_run(1);}
static httpd_req_t form(const char *ssid,const char *password){static char body[256];snprintf(body,sizeof body,"ssid=%s&password=%s&token=%s",ssid,password,csrf);return (httpd_req_t){.host="192.168.4.1",.origin="http://192.168.4.1",.type="application/x-www-form-urlencoded",.body=body,.content_len=(int)strlen(body)};}
static void ip(void){mock_associated=true;event(NULL,IP_EVENT,IP_EVENT_STA_GOT_IP,NULL);mock_run(1);}
int main(int argc,char **argv){
 pd_wifi_boot();pd_wifi_info info;pd_wifi_snapshot(&info);assert(!info.configured&&info.state==PD_WIFI_OFF&&!mock_inits&&!mock_stores&&!mock_erases);assert(pd_wifi_random()==12345);
 mock_netif_fail=true;command(PD_WIFI_SETUP);assert(current.error==PD_WIFI_ERROR_UNAVAILABLE&&!current.setup&&!mock_rf&&mock_sar);mock_netif_fail=false;
 mock_fail_init=true;command(PD_WIFI_SETUP);assert(!current.setup&&!mock_rf&&mock_sar);mock_fail_init=false;
 command(PD_WIFI_SETUP);assert(current.setup&&mock_rf&&!mock_sar&&current.setup_seconds==300&&strlen(csrf)==32&&strlen(current.setup_password)==16);
 if(argc==2&&!strcmp(argv[1],"--page")){httpd_req_t root={.host="192.168.4.1"};assert(page(&root)==0);puts(root.response);return 0;}
 httpd_req_t r={.host="evil.example"};assert(page(&r)==0&&r.status==403);r=(httpd_req_t){.host="192.168.4.1",.origin="https://evil.example"};assert(page(&r)==0&&r.status==403);
 r=(httpd_req_t){.host="192.168.4.1"};assert(page(&r)==0&&strstr(r.response,"name=token")&&!strstr(r.response,"SYNTHETIC-PASSWORD"));
 /* Portal only stages the exact bundle after origin, MIME, CSRF and bounded JSON checks. */
 char bridge_body[256];snprintf(bridge_body,sizeof bridge_body,"{\"token\":\"%s\",\"bundle\":\"{}\"}",csrf);httpd_req_t br={.host="192.168.4.1",.origin="http://192.168.4.1",.type="application/json",.body=bridge_body,.content_len=(int)strlen(bridge_body)};mock_stage=true;assert(bridge_handler(&br)==0&&br.status==202&&mock_stage_calls==1&&!mock_stores);br.at=0;br.origin="https://evil.example";assert(bridge_handler(&br)==0&&br.status==400&&mock_stage_calls==1);br.origin="http://192.168.4.1";br.type="text/plain";assert(bridge_handler(&br)==0&&br.status==400&&mock_stage_calls==1);br.type="application/json";bridge_body[10]='X';br.at=0;assert(bridge_handler(&br)==0&&br.status==400&&mock_stage_calls==1);mock_stage=false;
 r=form("Casa_sintetica","SYNTHETIC-PASSWORD");assert(connect_handler(&r)==0&&r.status==202&&!mock_stores&&!saved.ssid[0]);httpd_req_t busy=form("Altro","12345678");assert(connect_handler(&busy)==0&&busy.status==503);mock_run(1);assert(pending&&current.state==PD_WIFI_CONNECTING&&!mock_stores);mock_config.sta.ssid[0]='X';ip();assert(pending&&!mock_stores);mock_config.sta.ssid[0]='C';ip();assert(!pending&&current.setup_saved&&!current.setup_failed&&current.state==PD_WIFI_CONNECTED&&mock_stores==1&&mock_commits==1&&current.configured);assert(!strcmp(saved.ssid,"Casa_sintetica"));r=(httpd_req_t){.host="192.168.4.1"};assert(page(&r)==0&&!strstr(r.response,"SYNTHETIC-PASSWORD"));
 mock_clock+=60000;mock_associated=false;event(NULL,WIFI_EVENT,WIFI_EVENT_STA_DISCONNECTED,NULL);mock_run(1);assert(current.state==PD_WIFI_CONNECTING);ip();assert(current.state==PD_WIFI_CONNECTED&&!retry_at);
 __typeof__(saved) original=saved;unsigned stores=mock_stores;memset(&saved,0,sizeof saved);load();assert(!memcmp(&saved,&original,sizeof saved));
 /* A failed new network never replaces the durable old credentials. */
 r=form("Rete_errata","12345678");assert(connect_handler(&r)==0&&r.status==202);mock_fail_connect=true;mock_run(1);mock_fail_connect=false;assert(current.setup_failed&&!current.setup_saved&&!memcmp(&saved,&original,sizeof saved)&&mock_stores==stores);command(PD_WIFI_ENABLE);ip();r=(httpd_req_t){.host="192.168.4.1"};assert(state_handler(&r)==0&&strstr(r.response,"\"saved\":false")&&strstr(r.response,"\"failed\":true"));
 /* A trial that times out also retains the old configuration. */
 r=form("Rete_timeout","12345678");assert(connect_handler(&r)==0&&r.status==202);mock_run(1);mock_run(151);assert(!pending&&current.setup_failed&&!current.setup_saved&&!memcmp(&saved,&original,sizeof saved)&&mock_stores==stores);ip();
 /* NVS failure is not successful provisioning, even with a DHCP address. */
 r=form("Rete_non_salvata","12345678");assert(connect_handler(&r)==0&&r.status==202);mock_run(1);mock_fail_store=true;ip();mock_fail_store=false;assert(current.error==PD_WIFI_ERROR_STORAGE&&current.setup_failed&&!current.setup_saved&&!memcmp(&saved,&original,sizeof saved));
 command(PD_WIFI_DISABLE);assert(!running&&!mock_rf&&mock_sar&&!current.setup&&!pending&&!saved.enabled);command(PD_WIFI_ENABLE);ip();
 /* Pause acknowledgement follows the completed RF shutdown, then resumes saved STA only. */
 request q={.kind=20};assert(xQueueSend(requests,&q,0));mock_run(1);pd_wifi_snapshot(&info);assert(paused&&info.suspended&&!info.radio_active&&!running&&!mock_rf&&mock_sar);command(PD_WIFI_SETUP);assert(!current.setup);q.kind=21;assert(xQueueSend(requests,&q,0));mock_run(1);assert(!paused&&running&&!current.setup);ip();
 mock_fail_stop=true;command(PD_WIFI_DISABLE);assert(running&&mock_rf&&!mock_sar&&current.state==PD_WIFI_FAILED&&current.error==PD_WIFI_ERROR_UNAVAILABLE);mock_fail_stop=false;command(PD_WIFI_DISABLE);assert(!running&&!mock_rf&&mock_sar);
 mock_fail_start=true;command(PD_WIFI_ENABLE);assert(!running&&mock_sar&&!mock_rf);mock_fail_start=false;command(PD_WIFI_DISABLE);
 command(PD_WIFI_SETUP);assert(current.setup);mock_clock+=SETUP_MS;mock_run(1);assert(!current.setup&&!running&&!mock_rf&&mock_sar);
 command(PD_WIFI_FORGET);assert(!saved.ssid[0]&&!mock_blob_size&&mock_erases==1&&!current.configured);q=(request){.kind=10,.generation=generation-1,.credentials={.ssid="Stale",.password="12345678"}};assert(xQueueSend(requests,&q,0));mock_run(1);assert(!pending&&!running&&!saved.ssid[0]);
 puts("Production Wi-Fi service with fake SDK: offline boot, netif/init/start/stop faults, no SDK NVS, SAR/RF handover, portal Host/Origin/token/busy, save only after matching DHCP, timeout/fallback, NVS failure, load/forget namespace isolation, completed pause acknowledgement and AP expiry passed. No physical RF/NVS claimed.");
}
