/* One continuous synthetic table session through production rules and storage.
 * NVS commits are modeled as durable; this is not an electrical power-cut test. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "combat.h"
#include "damage.h"
#include "rest.h"
#include "status.h"
#include "magic.h"
#include "../main/storage.c"

static uint8_t durable[2][PD_RECORD_SIZE],staged[PD_RECORD_SIZE];
static size_t lengths[2],staged_size;
static int staged_bank,cut;
static unsigned writes,draws;
static uint32_t face;
esp_err_t nvs_flash_init_partition(const char *name){(void)name;return ESP_OK;}
esp_err_t nvs_open_from_partition(const char *p,const char *ns,int mode,nvs_handle_t *h){(void)ns;(void)mode;*h=!strcmp(p,"pdd_state");return ESP_OK;}
void nvs_close(nvs_handle_t h){(void)h;}
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *size){
    unsigned b=strcmp(key,"state_a")!=0;
    if(!h||!lengths[b])return ESP_ERR_NVS_NOT_FOUND;
    if(*size<lengths[b])return ESP_ERR_NVS_INVALID_LENGTH;
    *size=lengths[b];memcpy(out,durable[b],*size);return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *data,size_t size){
    assert(h&&size<=PD_RECORD_SIZE);writes++;
    if(cut==1)return ESP_FAIL;
    staged_bank=strcmp(key,"state_a")!=0;staged_size=size;memcpy(staged,data,size);return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t h){
    assert(h);if(cut==2)return ESP_FAIL;
    memcpy(durable[staged_bank],staged,staged_size);lengths[staged_bank]=staged_size;return ESP_OK;
}
static uint32_t random_face(void *unused){(void)unused;draws++;return face;}
static void restart(pd_state *s){
    blocked=false;bank=-1;generation=0;status="Demo: non salvata";staged_size=0;
    unsigned count=writes;assert(pd_storage_load(s)&&writes==count);
}
static void equal_state(const pd_state *a,const pd_state *b){
    static uint8_t x[PD_RECORD_SIZE],y[PD_RECORD_SIZE];
    size_t n=pd_encode(a,1,x);assert(n&&pd_encode(b,1,y)==n&&!memcmp(x,y,n));
}
static void save_restart(pd_state *live,const pd_state *candidate){
    static pd_state expected;expected=*candidate;assert(pd_storage_save(&expected));
    memset(live,0,sizeof *live);restart(live);equal_state(live,&expected);
}
int main(void){
    static pd_state live,next,before;
    restart(&live);strcpy(live.name,"Sessione sintetica");
    live.hp=30;live.temp_hp=3;live.defences.configured=(1u<<PD_DAMAGE_TYPES)-1;
    live.magic.count=2;live.magic.slots[0]=(pd_slot){2,3};live.magic.pact[0]=(pd_slot){1,1};
    live.magic.powers[0]=(pd_power){.name="Luce concentrata",.level=1,.flags=PD_PREPARED|PD_USES_SLOT|PD_CONCENTRATES};
    live.magic.powers[1]=(pd_power){.name="Passo",.kind=1,.flags=PD_PREPARED,.maximum=2,.remaining=1,.recovery=1};
    live.recovery=(pd_recovery){.available=true,.count=1,.pools={{.name="Classe di prova",.die=8,.maximum=3}}};
    live.status.available=true;live.shortcuts[1]=7;live.scene=2;live.avatar.cloak=1;
    assert(pd_valid(&live));save_restart(&live,&live);before=live;

    /* Attack and damage do not spend the PG's HP or consume inventory. */
    pd_strike hit;pd_pool pool;face=9;
    assert(pd_attack(&live,12,0,random_face,NULL,&hit)&&hit.hits&&hit.roll.total==14);
    face=2;assert(pd_weapon_damage(&hit,random_face,NULL,&pool)&&pool.total==5);
    equal_state(&live,&before);
    next=live;assert(pd_magic_use(&next,0,1,false));save_restart(&live,&next);
    assert(live.magic.slots[0].current==1&&live.magic.concentration==1);

    /* Damage, temporary HP and one concentration save are one saved candidate. */
    pd_damage_plan damage={.type=0,.amount=11};pd_damage_summary summary;pd_roll roll;
    before=live;assert(pd_damage_preview(&live,&damage,&summary)&&summary.needs_concentration&&!summary.allowed);
    equal_state(&live,&before);face=9;
    assert(pd_concentration_roll(&live,&damage,0,0,random_face,NULL,&next,&roll,&summary,false));
    assert(next.hp==22&&!next.temp_hp&&next.magic.concentration==1&&roll.total==12);
    save_restart(&live,&next);

    /* An interrupted potion commit retains the old durable state. The frozen
       candidate can be saved after restart without drawing again. */
    unsigned gained;face=0;before=live;
    assert(pd_use_item(&live,1,random_face,NULL,&next,&pool,&gained)&&gained==4);
    unsigned frozen_draws=draws;
    for(cut=1;cut<=2;cut++){
        assert(!pd_storage_save(&next));restart(&live);equal_state(&live,&before);
    }
    cut=0;save_restart(&live,&next);
    assert(draws==frozen_draws&&live.hp==26&&live.items[1].quantity==1);
    next=live;assert(pd_magic_use(&next,1,0,false)&&!next.magic.powers[1].remaining);save_restart(&live,&next);
    next=live;assert(pd_use_item(&live,2,random_face,NULL,&next,&pool,&gained));save_restart(&live,&next);
    assert(live.items[2].quantity==2&&live.resources[0].current==1);

    pd_rest_plan rest={0};pd_rest_summary recovered;before=live;
    assert(pd_rest_preview(&live,&rest,&next,&recovered));equal_state(&live,&before);
    save_restart(&live,&next);
    assert(live.hp==26&&live.magic.slots[0].current==1&&live.magic.powers[1].remaining==2&&live.magic.concentration==1);
    face=3;assert(pd_spend_hit_die(&live,0,random_face,NULL,&next,&pool,&gained)&&gained==6);
    save_restart(&live,&next);assert(live.hp==32&&live.recovery.pools[0].used==1);
    rest=(pd_rest_plan){.long_rest=true,.policy=PD_HD_2024};
    assert(pd_rest_preview(&live,&rest,&next,&recovered));save_restart(&live,&next);
    assert(live.hp==32&&live.magic.slots[0].current==3&&!live.magic.concentration&&!live.recovery.pools[0].used);
    assert(live.items[1].quantity==1&&live.items[2].quantity==2&&live.resources[0].current==1);
    assert(live.shortcuts[1]==7&&live.scene==2&&live.avatar.cloak==1&&draws==6);
    puts("Continuous native session: attack, damage/concentration, frozen potion, failed set/commit, consumable/feature, short rest/hit die/long rest, exact persisted fields and restart after every save passed; synthetic NVS, no physical power/USB claim");
}
