/* Two NVS partitions in memory: legacy data must never be overwritten by migration. */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../main/storage.c"
#include "fixtures/state-v1.h"
#include "fixtures/state-v2.h"
#include "fixtures/state-v3.h"
#include "fixtures/state-v4.h"
#include "fixtures/state-v5.h"
#include "fixtures/state-v6.h"
#include "fixtures/state-v7.h"
#include "fixtures/state-v8.h"
#include "fixtures/state-v9.h"
#include "fixtures/state-v10.h"
#include "fixtures/state-v11.h"
#include "fixtures/state-v12.h"
#include "fixtures/state-v13.h"
#include "fixtures/state-v14.h"
#include "fixtures/state-v15.h"
#include "fixtures/state-v16.h"
#include "fixtures/state-v17.h"
#include "fixtures/state-v18.h"
#include "fixtures/state-v19.h"
#include "fixtures/state-v20.h"
#include "fixtures/state-v21.h"
#include "fixtures/state-v22.h"
#include "fixtures/state-v23.h"
#include "fixtures/state-v24.h"
static uint8_t blobs[2][2][PD_RECORD_SIZE];static size_t sizes[2][2];
static int fail_init,fail_set,fail_commit,fail_read,set_calls,missing_layout;
static int index_of(const char *key){return !strcmp(key,"state_a")?0:1;}
esp_err_t nvs_flash_init_partition(const char *name){assert(!strcmp(name,"pdd_nvs")||!strcmp(name,"pdd_state"));if(fail_init)return ESP_FAIL;return missing_layout&&!strcmp(name,"pdd_state")?ESP_ERR_NOT_FOUND:ESP_OK;}
esp_err_t nvs_open_from_partition(const char *p,const char *ns,int mode,nvs_handle_t *h){*h=!strcmp(p,"pdd_state")?1:0;assert(mode==(*h?NVS_READWRITE:NVS_READONLY));return ESP_OK;}
void nvs_close(nvs_handle_t h){(void)h;}
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *data,size_t *size){
 if(fail_read)return ESP_FAIL;
 int i=index_of(key);if(!sizes[h][i])return ESP_ERR_NVS_NOT_FOUND;
 if(*size<sizes[h][i])return ESP_ERR_NVS_INVALID_LENGTH;
 *size=sizes[h][i];memcpy(data,blobs[h][i],*size);return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *data,size_t size){
 set_calls++;assert(h==1);if(fail_set)return ESP_FAIL;int i=index_of(key);sizes[h][i]=size;memcpy(blobs[h][i],data,size);return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t h){return fail_commit?ESP_FAIL:ESP_OK;}
static void reboot(void){blocked=false;bank=-1;generation=0;status="Demo: non salvata";}
int main(void){
 const uint8_t *golden[]={legacy_v1,legacy_v2,legacy_v3,legacy_v4};const size_t lengths[]={sizeof legacy_v1,sizeof legacy_v2,sizeof legacy_v3,sizeof legacy_v4};pd_state s;
 for(unsigned k=0;k<4;k++){
  memset(sizes,0,sizeof sizes);memcpy(blobs[0][0],golden[k],lengths[k]);sizes[0][0]=lengths[k];reboot();int unchanged=set_calls;
  assert(pd_storage_load(&s)&&s.hp==17&&s.temp_hp==4&&!s.magic.available&&set_calls==unchanged);
  s.hp=16;assert(pd_storage_save(&s));assert(!memcmp(blobs[0][0],golden[k],lengths[k]));
  reboot();assert(pd_storage_load(&s)&&s.hp==16&&!s.magic.available);
 }
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v5,sizeof golden_v5);sizes[1][0]=sizeof golden_v5;reboot();int v5_calls=set_calls;
 assert(pd_storage_load(&s)&&s.magic.concentration==1&&s.shortcuts[1]==6&&!s.recovery.available&&set_calls==v5_calls);
 s.recovery=(pd_recovery){.available=true,.count=1,.pools={{.name="Classe",.die=8,.maximum=3,.used=2}}};assert(pd_storage_save(&s));
 assert(!memcmp(blobs[1][0],golden_v5,sizeof golden_v5));reboot();assert(pd_storage_load(&s)&&s.recovery.pools[0].used==2&&s.magic.concentration==1&&s.shortcuts[1]==6);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v6,sizeof golden_v6);sizes[1][0]=sizeof golden_v6;reboot();int v6_calls=set_calls;
 assert(pd_storage_load(&s)&&s.hp==9&&s.recovery.pools[0].used==2&&s.shortcuts[0]==7&&!s.status.available&&set_calls==v6_calls);
 s.status=(pd_status){.available=true,.count=1,.successes=1,.failures=2,.conditions={{.name="Nota sintetica",.level=2}}};assert(pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v6,sizeof golden_v6));
 reboot();assert(pd_storage_load(&s)&&s.status.successes==1&&s.status.failures==2&&s.status.conditions[0].level==2&&s.recovery.pools[0].used==2&&s.shortcuts[0]==7);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v7,sizeof golden_v7);sizes[1][0]=sizeof golden_v7;reboot();int v7_calls=set_calls;
 assert(pd_storage_load(&s)&&s.status.failures==2&&s.brightness==50&&!s.dim_timeout&&set_calls==v7_calls);s.brightness=80;s.dim_timeout=2;assert(pd_storage_save(&s));
 assert(!memcmp(blobs[1][0],golden_v7,sizeof golden_v7));reboot();assert(pd_storage_load(&s)&&s.brightness==80&&s.dim_timeout==2&&s.status.failures==2);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v8,sizeof golden_v8);sizes[1][0]=sizeof golden_v8;reboot();int v8_calls=set_calls;
 assert(pd_storage_load(&s)&&s.hp==29&&s.brightness==80&&s.dim_timeout==2&&!s.effect_count&&set_calls==v8_calls);
 assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true}));pd_hp(&s,99);assert(s.hp==42&&pd_storage_save(&s));
 assert(!memcmp(blobs[1][0],golden_v8,sizeof golden_v8));reboot();assert(pd_storage_load(&s)&&s.hp==42&&s.max_hp==32&&pd_max_hp(&s)==42&&s.effect_count==1&&s.brightness==80);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v9,sizeof golden_v9);sizes[1][0]=sizeof golden_v9;reboot();int v9_calls=set_calls;
 assert(pd_storage_load(&s)&&s.hp==41&&s.max_hp==32&&pd_max_hp(&s)==42&&s.effect_count==2&&!s.effects[1].timed&&set_calls==v9_calls);
 s.effects[1].timed=true;s.effects[1].rounds=1;assert(pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v9,sizeof golden_v9));
 reboot();assert(pd_storage_load(&s)&&s.effects[1].rounds==1&&s.effects[1].active);uint8_t advanced,expired;
 assert(pd_effect_advance(&s,&advanced,&expired)&&advanced==2&&expired==2&&s.hp==32&&pd_storage_save(&s));
 reboot();assert(pd_storage_load(&s)&&s.effects[1].timed&&!s.effects[1].rounds&&!s.effects[1].active&&s.hp==32&&pd_max_hp(&s)==32);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v10,sizeof golden_v10);sizes[1][0]=sizeof golden_v10;reboot();int v10_calls=set_calls;
 assert(pd_storage_load(&s)&&s.hp==41&&s.effects[0].rounds==2&&s.effects[1].rounds==1&&!s.defences.configured&&set_calls==v10_calls);
 assert(pd_defence_set(&s,5,true,PD_RESISTANCE)&&pd_defence_set(&s,12,true,7)&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v10,sizeof golden_v10));
 reboot();assert(pd_storage_load(&s)&&s.hp==41&&s.effects[1].rounds==1&&s.defences.configured==((1u<<5)|(1u<<12))&&pd_defence_flags(&s,5)==PD_RESISTANCE&&pd_defence_flags(&s,12)==7);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v11,sizeof golden_v11);sizes[1][0]=sizeof golden_v11;reboot();int v11_calls=set_calls;
 assert(pd_storage_load(&s)&&s.hp==41&&s.magic.concentration==1&&s.effects[0].rounds==3&&!s.effects[0].concentration&&!s.effects[1].concentration&&s.defences.immunity==4&&set_calls==v11_calls);
 pd_effect linked=s.effects[0];linked.concentration=1;assert(pd_effect_set(&s,0,linked)&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v11,sizeof golden_v11));
 reboot();assert(pd_storage_load(&s)&&s.effects[0].concentration==1&&s.effects[1].active&&s.defences.resistance==32);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v12,sizeof golden_v12);sizes[1][0]=sizeof golden_v12;reboot();int v12_calls=set_calls;
 assert(pd_storage_load(&s)&&s.hp==41&&s.effects[0].concentration==1&&s.magic.concentration==1&&s.defences.resistance==32&&!s.defences.temporary[5].active&&set_calls==v12_calls);
 assert(pd_defence_temporary_set(&s,5,(pd_defence_temporary){.flags=PD_IMMUNITY,.active=true,.timed=true,.rounds=2,.concentration=1})&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v12,sizeof golden_v12));
 reboot();assert(pd_storage_load(&s)&&s.defences.temporary[5].rounds==2&&s.defences.temporary[5].concentration==1&&pd_defence_base_flags(&s,5)==PD_RESISTANCE&&pd_defence_flags(&s,5)==(PD_RESISTANCE|PD_IMMUNITY));
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v13,sizeof golden_v13);sizes[1][0]=sizeof golden_v13;reboot();int v13_calls=set_calls;
 assert(pd_storage_load(&s)&&s.hp==41&&s.defences.temporary[5].rounds==3&&s.effects[0].concentration==1&&!s.defences.conditions[5][0]&&set_calls==v13_calls);
 assert(pd_defence_rule_set(&s,5,false,0,PD_RULE_NONMAGICAL)&&pd_defence_rule_set(&s,5,true,2,PD_RULE_MAGICAL)&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v13,sizeof golden_v13));
 reboot();assert(pd_storage_load(&s)&&s.defences.conditions[5][0]==PD_RULE_NONMAGICAL&&s.defences.temporary[5].conditions[2]==PD_RULE_MAGICAL&&s.defences.temporary[5].rounds==3&&s.hp==41);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v14,sizeof golden_v14);sizes[1][0]=sizeof golden_v14;reboot();int v14_calls=set_calls;
 assert(pd_storage_load(&s)&&s.temp_hp==7&&s.defences.conditions[5][0]==PD_RULE_NONMAGICAL&&s.defences.temporary[5].conditions[2]==PD_RULE_MAGICAL&&s.defences.temporary[5].rounds==3&&set_calls==v14_calls);
 for(unsigned type=0;type<13;type++)for(unsigned slot=1;slot<3;slot++)assert(!pd_defence_grant(&s,type,slot)->flags);
 assert(pd_defence_grant_set(&s,5,1,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=2})&&pd_defence_grant_set(&s,5,2,(pd_defence_temporary){.flags=2,.active=true,.concentration=1,.conditions={0,PD_RULE_MAGICAL,0}})&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v14,sizeof golden_v14));
 reboot();assert(pd_storage_load(&s)&&pd_defence_grant(&s,5,1)->rounds==2&&pd_defence_grant(&s,5,2)->conditions[1]==PD_RULE_MAGICAL&&s.defences.temporary[5].rounds==3&&s.temp_hp==7);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v15,sizeof golden_v15);sizes[1][0]=sizeof golden_v15;reboot();int v15_calls=set_calls;
 assert(pd_storage_load(&s)&&s.temp_hp==7&&pd_defence_grant(&s,5,2)->rounds==3&&!s.reductions[0].value&&set_calls==v15_calls);
 assert(pd_reduction_profile_set(&s,2,(pd_reduction_profile){.value=3,.types=7,.rule=PD_RULE_NONMAGICAL_ATTACK,.active=true,.timed=true,.rounds=2,.concentration=1})&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v15,sizeof golden_v15));
 reboot();assert(pd_storage_load(&s)&&s.reductions[2].value==3&&s.reductions[2].types==7&&s.reductions[2].rounds==2&&s.reductions[2].concentration==1&&s.reductions[2].rule==PD_RULE_NONMAGICAL_ATTACK&&pd_defence_grant(&s,5,2)->conditions[2]==PD_RULE_MAGICAL&&s.temp_hp==7);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v16,sizeof golden_v16);sizes[1][0]=sizeof golden_v16;reboot();int v16_calls=set_calls;
 assert(pd_storage_load(&s)&&s.reductions[0].value==3&&s.reductions[0].concentration==1&&!s.reaction_spent&&!s.reductions[0].cost_kind&&set_calls==v16_calls);
 pd_reduction_profile paid=s.reductions[0];paid.cost_kind=PD_COST_POWER;paid.cost_index=1;paid.cost=1;paid.reaction=true;assert(pd_reduction_profile_set(&s,0,paid)&&pd_reaction_change(&s,true)&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v16,sizeof golden_v16));
 reboot();assert(pd_storage_load(&s)&&s.reaction_spent&&s.reductions[0].cost_kind==PD_COST_POWER&&s.reductions[0].cost_index==1&&s.reductions[0].cost==1&&s.reductions[0].reaction&&s.reductions[0].rounds==2&&s.magic.powers[1].remaining==2&&s.temp_hp==7);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v17,sizeof golden_v17);sizes[1][0]=sizeof golden_v17;reboot();int v17_calls=set_calls;
 assert(pd_storage_load(&s)&&s.reaction_spent&&s.reductions[0].cost_kind==PD_COST_COUNTER&&s.reductions[1].cost==2&&s.magic.pact[4].current==1&&set_calls==v17_calls);
 pd_reduction_profile slot_paid=s.reductions[0];slot_paid.cost_kind=PD_COST_SLOT;slot_paid.cost=2;assert(pd_reduction_profile_set(&s,0,slot_paid)&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v17,sizeof golden_v17));
 reboot();assert(pd_storage_load(&s)&&s.reaction_spent&&s.reductions[0].cost_kind==PD_COST_SLOT&&s.reductions[0].cost==2&&s.reductions[1].cost_kind==PD_COST_POWER&&s.magic.slots[0].current==2&&s.magic.pact[4].current==1);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v18,sizeof golden_v18);sizes[1][0]=sizeof golden_v18;reboot();int v18_calls=set_calls;
 assert(pd_storage_load(&s)&&s.hp==41&&s.reaction_spent&&!s.effects[0].tick&&!s.reductions[0].tick&&set_calls==v18_calls);
 pd_effect turn_effect=s.effects[0];turn_effect.tick=PD_TICK_START;assert(pd_effect_set(&s,0,turn_effect)&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v18,sizeof golden_v18));
 reboot();assert(pd_storage_load(&s)&&s.effects[0].tick==PD_TICK_START&&s.effects[0].rounds==2&&s.reaction_spent&&pd_defence_grant(&s,5,2)->rounds==3&&!s.reductions[0].tick&&s.reductions[0].cost_kind==PD_COST_PACT);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v22,sizeof golden_v22);sizes[1][0]=sizeof golden_v22;reboot();int v22_calls=set_calls;
 assert(pd_storage_load(&s)&&set_calls==v22_calls&&s.training.poisoned&&!s.roll_conditions&&s.items[0].charges.current==3&&s.reductions[0].extra_costs[1].amount==2);
 assert(pd_condition_effects_set(&s,15)&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v22,sizeof golden_v22));reboot();assert(pd_storage_load(&s)&&s.roll_conditions==15&&s.training.poisoned&&s.effects[0].minutes==59&&s.reductions[0].extra_costs[1].amount==2);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v21,sizeof golden_v21);sizes[1][0]=sizeof golden_v21;reboot();int v21_calls=set_calls;
 assert(pd_storage_load(&s)&&set_calls==v21_calls&&s.items[0].charges.current==3&&s.reductions[0].cost_kind==PD_COST_ITEM&&!s.reductions[0].extra_costs[0].kind);
 pd_reduction_profile multi=s.reductions[0];pd_profile_cost_set(&multi,1,(pd_cost){PD_COST_COUNTER,0,1});pd_profile_cost_set(&multi,2,(pd_cost){PD_COST_SLOT,0,2});assert(pd_reduction_profile_set(&s,0,multi)&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v21,sizeof golden_v21));reboot();assert(pd_storage_load(&s)&&s.reductions[0].extra_costs[0].kind==PD_COST_COUNTER&&s.reductions[0].extra_costs[1].amount==2&&s.items[0].charges.current==3&&s.effects[0].minutes==59);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v20,sizeof golden_v20);sizes[1][0]=sizeof golden_v20;reboot();int v20_calls=set_calls;
 assert(pd_storage_load(&s)&&set_calls==v20_calls&&s.effects[0].minutes==59&&s.reductions[2].minutes==7&&!s.items[0].charges.maximum);
 s.items[0].charges=(pd_charges){3,7,1};assert(pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v20,sizeof golden_v20));reboot();assert(pd_storage_load(&s)&&s.items[0].charges.current==3&&s.items[0].charges.maximum==7&&s.items[0].charges.recovery==1&&s.effects[0].minutes==59);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v19,sizeof golden_v19);sizes[1][0]=sizeof golden_v19;reboot();int v19_calls=set_calls;
 assert(pd_storage_load(&s)&&s.hp==41&&s.reaction_spent&&s.effects[0].tick==PD_TICK_START&&!s.effects[0].minutes&&set_calls==v19_calls);
 pd_effect time_effect=s.effects[0];time_effect.tick=PD_TICK_HOURS;time_effect.rounds=0;time_effect.minutes=119;assert(pd_effect_set(&s,0,time_effect)&&pd_storage_save(&s));assert(!memcmp(blobs[1][0],golden_v19,sizeof golden_v19));
 reboot();assert(pd_storage_load(&s)&&s.effects[0].tick==PD_TICK_HOURS&&s.effects[0].minutes==119&&!s.effects[0].rounds&&pd_defence_grant(&s,5,2)->tick==PD_TICK_END&&s.reaction_spent);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v23,sizeof golden_v23);sizes[1][0]=sizeof golden_v23;reboot();int v23_calls=set_calls;
 assert(pd_storage_load(&s)&&s.roll_conditions==15&&!s.wallet.available&&set_calls==v23_calls);
 pd_wallet wallet={.available=true,.coins={1,2,3,4,PD_COIN_MAX}};assert(pd_wallet_set(&s,&wallet)&&pd_storage_save(&s));
 assert(!memcmp(blobs[1][0],golden_v23,sizeof golden_v23));reboot();assert(pd_storage_load(&s)&&s.wallet.available&&!memcmp(s.wallet.coins,wallet.coins,sizeof wallet.coins)&&s.roll_conditions==15);
 memset(sizes,0,sizeof sizes);memcpy(blobs[1][0],golden_v24,sizeof golden_v24);sizes[1][0]=sizeof golden_v24;reboot();int v24_calls=set_calls;
 assert(pd_storage_load(&s)&&s.history_epoch==0&&s.wallet.coins[4]==PD_COIN_MAX&&set_calls==v24_calls);
 s.history_epoch=UINT32_MAX;s.shortcuts[0]=13;assert(pd_storage_save(&s)&&!memcmp(blobs[1][0],golden_v24,sizeof golden_v24));
 reboot();assert(pd_storage_load(&s)&&s.history_epoch==UINT32_MAX&&s.shortcuts[0]==13&&s.wallet.coins[4]==PD_COIN_MAX&&s.roll_conditions==15);
 memset(sizes,0,sizeof sizes);memcpy(blobs[0][0],legacy_v4,sizeof legacy_v4);sizes[0][0]=sizeof legacy_v4;missing_layout=1;reboot();
 assert(!pd_storage_load(&s)&&s.hp==17&&blocked&&!strcmp(status,"Layout da aggiornare"));int calls=set_calls;assert(!pd_storage_save(&s)&&set_calls==calls);missing_layout=0;
 memset(sizes,0,sizeof sizes);reboot();assert(pd_storage_load(&s)&&s.hp==24&&s.magic.available);s.hp=23;assert(pd_storage_save(&s));
 static uint8_t first[PD_RECORD_SIZE];memcpy(first,blobs[1][0],sizeof first);s.hp=22;assert(pd_storage_save(&s));assert(!memcmp(first,blobs[1][0],sizeof first));
 reboot();assert(pd_storage_load(&s)&&s.hp==22);blobs[1][1][40]^=1;reboot();assert(pd_storage_load(&s)&&s.hp==23&&!strcmp(status,"Recuperata copia valida"));
 s.hp=21;assert(pd_storage_save(&s));assert(!memcmp(first,blobs[1][0],sizeof first));
 fail_set=1;s.hp=20;assert(!pd_storage_save(&s));fail_set=0;calls=set_calls;assert(!pd_storage_save(&s)&&calls==set_calls);reboot();assert(pd_storage_load(&s)&&s.hp==21);
 fail_commit=1;s.hp=19;assert(!pd_storage_save(&s)&&blocked);fail_commit=0;reboot();assert(pd_storage_load(&s)&&s.hp==19);
 sizes[1][1]=pd_encode(&s,100,blobs[1][1]);blobs[1][1][3]='X';uint32_t crc=pd_crc32(blobs[1][1],sizes[1][1]-4);for(int i=0;i<4;i++)blobs[1][1][sizes[1][1]-4+i]=crc>>(8*i);
 reboot();assert(!pd_storage_load(&s)&&blocked);blobs[1][0][0]=0;blobs[1][1][0]=0;reboot();assert(!pd_storage_load(&s));
 fail_init=1;reboot();assert(!pd_storage_load(&s));fail_init=0;fail_read=1;reboot();assert(!pd_storage_load(&s));
 puts("NVS: new/legacy partitions, golden v1..v24 migration without boot writes, missing layout, alternate banks, errors, rollback and future-version preservation passed");
}
