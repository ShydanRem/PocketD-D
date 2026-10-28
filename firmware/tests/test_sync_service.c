#define _POSIX_C_SOURCE 200809L
#define PD_SYNC_TEST 1
#include "../main/sync.c"
uint32_t pd_wifi_random(void){static uint32_t n=100;return n++;}
void pd_wifi_snapshot(pd_wifi_info *v){*v=(pd_wifi_info){.state=PD_WIFI_CONNECTED};}
int main(int argc,char **argv){
 assert(argc==2);mock_namespace="pdd_sync";FILE *f=fopen(argv[1],"r");assert(f);char json[4096];size_t n=fread(json,1,sizeof json-1,f);fclose(f);json[n]=0;pd_sync_boot();assert(!configured&&!worker);
 assert(pd_sync_stage_json(json,n));pd_sync_info visible;pd_sync_snapshot(&visible);assert(visible.offer&&!strcmp(visible.character_id,"123456")&&strlen(visible.fingerprint)==32);assert(!pd_sync_stage_json(json,n));
 mock_fail_store=true;assert(!pd_sync_decide(true,1)&&!configured&&offered);mock_fail_store=false;assert(pd_sync_decide(true,1)&&configured&&!offered&&worker&&config.epoch==1);pd_sync_config first=config;
 pd_state state;pd_demo(&state);state.history_epoch=1;pd_history history;pd_history_reset(&history,1);pd_sync_observe(&state,&history,false);assert(!ready&&!counter);pd_sync_observe(&state,&history,true);assert(ready&&counter==1);for(unsigned i=0;i<100;i++)pd_sync_observe(&state,&history,true);assert(counter==1);
 state.hp--;pd_sync_observe(&state,&history,false);assert(counter==1);pd_sync_observe(&state,&history,true);assert(counter==2&&latest.hp==state.hp);assert(pd_sync_pause(true)&&paused);assert(pd_sync_pause(false)&&!paused);active=true;assert(!pd_sync_pause(true));active=false;assert(pd_sync_pause(false));
 state.history_epoch=2;pd_history_reset(&history,2);pd_sync_observe(&state,&history,true);assert(!ready&&info.state==PD_SYNC_REBIND);
 lock=NULL;configured=worker=offered=ready=false;memset(&config,0,sizeof config);pd_sync_boot();assert(configured&&worker&&config.epoch==1&&!memcmp(&config,&first,sizeof first));pd_sync_observe(&state,&history,true);assert(info.state==PD_SYNC_REBIND&&!ready);
 assert(pd_sync_stage_json(json,n));mock_clock+=300001;pd_sync_snapshot(&visible);assert(!visible.offer);assert(pd_sync_stage_json(json,n));assert(pd_sync_decide(false,2)&&!offered&&!memcmp(&config,&first,sizeof first));
 mock_fail_store=true;assert(!pd_sync_forget()&&configured&&!memcmp(&config,&first,sizeof first));mock_fail_store=false;assert(pd_sync_forget()&&!configured&&!mock_blob_size&&mock_erases==2);
 puts("Production sync control with fake SDK/NVS: staged fingerprint, no save before confirmation, failed-save retry, saved-state observations, stable counters, boot reload, epoch/restore freeze, offer expiry, pause wait/failure and namespace-only forget passed; no physical flash/TLS claimed.");
}
