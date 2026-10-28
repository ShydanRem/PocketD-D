#include "pocketdd.h"
#include "cJSON.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
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
static void strip_charges(cJSON *root){cJSON *items=cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(root,"inventory"),"items");cJSON *item; cJSON_ArrayForEach(item,items)cJSON_DeleteItemFromObjectCaseSensitive(item,"charges");}
static void record_crc(uint8_t *record) {
    size_t size=8964; /* No powers: PDD27 prefix, recovery, status and CRC. */
    uint32_t crc=pd_crc32(record,size-4);
    for(int i=0;i<4;i++)record[size-4+i]=crc>>(8*i);
}
static void reject(cJSON *root) {
    char *encoded=cJSON_PrintUnformatted(root);pd_state untouched,before;
    pd_demo(&untouched);before=untouched;
    assert(!pd_parse_packet(encoded,strlen(encoded),&untouched));
    assert(!memcmp(&untouched,&before,sizeof before));
    cJSON_free(encoded);cJSON_Delete(root);
}
static uint32_t values[8]; static int at;
static uint32_t rng(void *ctx) { (void)ctx; return values[at++]; }
int main(int argc,char **argv) {
    pd_state s,out;uint32_t gen;pd_demo(&s);assert(pd_valid(&s));
    for(int i=1;i<=30;i++) assert(pd_modifier(i)==(i-10)/2-((i<10&&(i&1))?1:0));
    assert(pd_decode(golden_v23,sizeof golden_v23,&out,&gen)&&gen==231&&out.roll_conditions==15&&!out.wallet.available);
    const uint8_t *const old_records[]={legacy_v1,legacy_v2,legacy_v3,legacy_v4,golden_v5,golden_v6,golden_v7,golden_v8,golden_v9,golden_v10,golden_v11,golden_v12,golden_v13,golden_v14,golden_v15,golden_v16,golden_v17,golden_v18,golden_v19,golden_v20,golden_v21,golden_v22,golden_v23,golden_v24};
    const size_t old_lengths[]={sizeof legacy_v1,sizeof legacy_v2,sizeof legacy_v3,sizeof legacy_v4,sizeof golden_v5,sizeof golden_v6,sizeof golden_v7,sizeof golden_v8,sizeof golden_v9,sizeof golden_v10,sizeof golden_v11,sizeof golden_v12,sizeof golden_v13,sizeof golden_v14,sizeof golden_v15,sizeof golden_v16,sizeof golden_v17,sizeof golden_v18,sizeof golden_v19,sizeof golden_v20,sizeof golden_v21,sizeof golden_v22,sizeof golden_v23,sizeof golden_v24};
    for(unsigned i=0;i<24;i++){assert(pd_decode(old_records[i],old_lengths[i],&out,&gen)&&out.history_epoch==0);if(i<23){assert(!out.wallet.available);for(unsigned j=0;j<PD_COINS;j++)assert(!out.wallet.coins[j]);}}
    assert(pd_decode(golden_v24,sizeof golden_v24,&out,&gen)&&gen==241&&out.roll_conditions==15&&out.wallet.coins[4]==PD_COIN_MAX);
    static uint8_t previous_record[sizeof golden_v24];memcpy(previous_record,golden_v24,sizeof previous_record);previous_record[5161]=13;
    uint32_t previous_crc=pd_crc32(previous_record,sizeof previous_record-4);for(unsigned i=0;i<4;i++)previous_record[sizeof previous_record-4+i]=previous_crc>>(8*i);
    assert(!pd_decode(previous_record,sizeof previous_record,&out,&gen));
    s.temp_hp=3;pd_hp(&s,-2);assert(s.hp==24&&s.temp_hp==1);
    pd_hp(&s,-8);assert(s.hp==17&&s.temp_hp==0);
    pd_hp(&s,INT_MAX);assert(s.hp==32);pd_hp(&s,INT_MIN);assert(!s.hp);
    assert(pd_resource_change(&s,0,INT_MAX));assert(s.resources[0].current==1);
    assert(pd_resource_change(&s,0,INT_MIN));assert(!s.resources[0].current);
    assert(!pd_resource_change(&s,8,1));
    values[0]=UINT32_MAX;values[1]=19;at=0;
    pd_roll r=pd_d20(-5,0,rng,NULL);assert(at==2&&r.natural==20&&r.total==15&&r.count==1);
    values[0]=0;values[1]=19;at=0;r=pd_d20(2,1,rng,NULL);assert(r.natural==20&&r.total==22);
    at=0;r=pd_d20(-1,-1,rng,NULL);assert(r.natural==1&&r.total==0);
    uint8_t a[PD_RECORD_SIZE],b[PD_RECORD_SIZE];size_t record_size=pd_encoded_size(&s);
    assert(pd_decode(legacy_v1,sizeof legacy_v1,&out,&gen)&&gen==41&&out.hp==17&&out.temp_hp==4);
    assert(!out.avatar.skin&&!out.avatar.hair&&!out.avatar.cloak&&!out.avatar.style&&out.idle_animation);
    assert(!out.training.available);
    assert(pd_decode(legacy_v2,sizeof legacy_v2,&out,&gen)&&gen==42&&out.hp==17&&!out.idle_animation);
    assert(out.avatar.skin==2&&!out.training.available);
    assert(pd_decode(legacy_v3,sizeof legacy_v3,&out,&gen)&&gen==43&&out.training.ranks[0]==2&&out.training.poisoned&&!out.inventory_available&&out.equipped==PD_NO_WEAPON);
    assert(pd_decode(legacy_v4,sizeof legacy_v4,&out,&gen)&&gen==44&&out.inventory_available&&out.item_count==3&&!out.magic.available&&out.shortcuts[0]==1);
    assert(pd_decode(golden_v5,sizeof golden_v5,&out,&gen)&&gen==123&&out.magic.count==2&&out.magic.concentration==1&&out.magic.powers[1].remaining==1&&!out.recovery.available&&out.shortcuts[0]==4&&out.shortcuts[1]==6&&!out.shortcuts[2]);
    assert(pd_decode(golden_v6,sizeof golden_v6,&out,&gen)&&gen==124&&out.hp==9&&out.recovery.count==2&&out.recovery.pools[0].used==2&&out.shortcuts[0]==7&&!out.status.available);
    assert(pd_decode(golden_v7,sizeof golden_v7,&out,&gen)&&gen==125&&out.status.failures==2&&out.status.conditions[0].level==2&&out.shortcuts[0]==8&&out.brightness==50&&!out.dim_timeout);
    assert(pd_decode(golden_v8,sizeof golden_v8,&out,&gen)&&gen==126&&out.hp==29&&out.brightness==80&&out.dim_timeout==2&&out.shortcuts[0]==9&&!out.effect_count);
    assert(pd_decode(golden_v9,sizeof golden_v9,&out,&gen)&&gen==128&&out.hp==41&&out.max_hp==32&&pd_max_hp(&out)==42&&pd_armor(&out)==18&&out.effect_count==2&&!out.effects[0].timed&&!out.effects[1].rounds);
    assert(pd_decode(golden_v10,sizeof golden_v10,&out,&gen)&&gen==130&&out.hp==41&&out.effects[0].rounds==2&&out.effects[1].rounds==1&&out.shortcuts[0]==11&&!out.defences.configured);
    assert(pd_decode(golden_v11,sizeof golden_v11,&out,&gen)&&gen==132&&out.hp==41&&out.magic.concentration==1&&out.effects[0].rounds==3&&out.effects[1].active&&!out.effects[0].concentration&&!out.effects[1].concentration&&out.training.save_bonus[2]==3&&out.defences.resistance==32&&out.defences.immunity==4);
    assert(pd_decode(golden_v12,sizeof golden_v12,&out,&gen)&&gen==134&&out.hp==41&&out.magic.concentration==1&&out.effects[0].concentration==1&&out.effects[1].concentration==1&&out.effects[0].rounds==3&&out.defences.immunity==4);
    for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)assert(!out.defences.temporary[i].flags&&!out.defences.temporary[i].active&&!out.defences.temporary[i].timed&&!out.defences.temporary[i].rounds&&!out.defences.temporary[i].concentration);
    assert(pd_decode(golden_v13,sizeof golden_v13,&out,&gen)&&gen==137&&out.hp==41&&out.effects[0].concentration==1&&out.defences.temporary[5].concentration==1&&out.defences.temporary[5].rounds==3&&pd_defence_flags(&out,5)==5);
    for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)for(unsigned j=0;j<3;j++)assert(!out.defences.conditions[i][j]&&!out.defences.temporary[i].conditions[j]);
    assert(pd_decode(golden_v14,sizeof golden_v14,&out,&gen)&&gen==141&&out.defences.conditions[5][0]==PD_RULE_NONMAGICAL&&out.defences.temporary[5].conditions[2]==PD_RULE_MAGICAL&&out.defences.temporary[5].rounds==3);
    for(unsigned type=0;type<13;type++)for(unsigned slot=1;slot<3;slot++){const pd_defence_temporary empty={0};assert(!memcmp(pd_defence_grant(&out,type,slot),&empty,sizeof empty));}
    assert(pd_decode(golden_v15,sizeof golden_v15,&out,&gen)&&gen==151&&out.temp_hp==7&&out.defences.conditions[5][0]==PD_RULE_NONMAGICAL&&pd_defence_grant(&out,5,2)->rounds==3&&pd_defence_grant(&out,5,2)->conditions[2]==PD_RULE_MAGICAL);
    for(unsigned i=0;i<3;i++){const pd_reduction_profile empty={0};assert(!memcmp(&out.reductions[i],&empty,sizeof empty));}
    assert(pd_decode(golden_v16,sizeof golden_v16,&out,&gen)&&gen==161&&out.temp_hp==7&&out.reductions[0].value==3&&out.reductions[0].concentration==1&&out.reductions[0].rounds==2&&out.reductions[1].value==5&&out.reductions[1].stage==1&&!out.reaction_spent&&out.magic.powers[1].remaining==2);
    for(unsigned i=0;i<3;i++)assert(!out.reductions[i].cost_kind&&!out.reductions[i].cost_index&&!out.reductions[i].cost&&!out.reductions[i].reaction);
    assert(pd_decode(golden_v17,sizeof golden_v17,&out,&gen)&&gen==171&&out.reaction_spent&&out.reductions[0].cost_kind==PD_COST_COUNTER&&out.reductions[0].cost==1&&out.reductions[0].reaction&&out.reductions[1].cost_kind==PD_COST_POWER&&out.reductions[1].cost_index==1&&out.reductions[1].cost==2&&out.magic.slots[0].current==2&&out.magic.pact[4].current==1);
    assert(pd_decode(golden_v18,sizeof golden_v18,&out,&gen)&&gen==181&&out.hp==41&&out.reaction_spent&&out.effects[0].rounds==2&&!out.effects[0].tick&&pd_defence_grant(&out,5,2)->rounds==3&&!pd_defence_grant(&out,5,2)->tick&&out.reductions[0].cost_kind==PD_COST_PACT&&!out.reductions[0].tick);
    assert(pd_decode(golden_v19,sizeof golden_v19,&out,&gen)&&gen==191&&out.effects[0].tick==PD_TICK_START&&out.effects[0].rounds==2&&!out.effects[0].minutes&&pd_defence_grant(&out,5,2)->tick==PD_TICK_END&&!pd_defence_grant(&out,5,2)->minutes&&out.reductions[0].cost_kind==PD_COST_PACT&&!out.reductions[0].minutes&&out.reaction_spent&&out.hp==41);
    assert(pd_decode(golden_v20,sizeof golden_v20,&out,&gen)&&gen==201&&out.effects[0].minutes==59&&out.effects[0].tick==PD_TICK_HOURS&&out.reductions[2].minutes==7&&out.reductions[2].cost_kind==PD_COST_COUNTER&&pd_defence_grant(&out,12,2)->tick==PD_TICK_END&&out.reaction_spent);
    for(unsigned i=0;i<PD_ITEMS;i++)assert(!out.items[i].charges.maximum&&!out.items[i].charges.current&&!out.items[i].charges.recovery);
    assert(pd_decode(golden_v21,sizeof golden_v21,&out,&gen)&&gen==211&&out.items[0].charges.current==3&&out.items[0].charges.maximum==7&&out.reductions[0].cost_kind==PD_COST_ITEM&&out.reductions[1].cost_kind==PD_COST_COUNTER&&out.reductions[2].cost_kind==PD_COST_SLOT&&out.effects[0].minutes==59);
    for(unsigned i=0;i<3;i++)for(unsigned row=1;row<3;row++){pd_cost c=pd_profile_cost(&out.reductions[i],row);assert(!c.kind&&!c.index&&!c.amount);}
    assert(pd_decode(golden_v22,sizeof golden_v22,&out,&gen)&&gen==221&&out.training.poisoned&&!out.roll_conditions&&out.items[0].charges.current==3&&out.reductions[0].extra_costs[0].kind==PD_COST_COUNTER&&out.reductions[0].extra_costs[1].amount==2&&out.effects[0].minutes==59);
    pd_demo(&s);s.avatar=(pd_avatar){2,1,2,1};s.idle_animation=false;s.brightness=90;s.dim_timeout=2;
    for(int i=0;i<PD_SKILLS;i++) { s.training.ranks[i]=i%3;s.training.skill_bonus[i]=i-8; }
    for(int i=0;i<6;i++) { s.training.save_bonus[i]=i-2;s.training.check_bonus[i]=3-i; }
    s.training.saves=63;s.training.half_skills=0x3ffff;s.training.blocked=0x3fffffff;
    s.training.half_checks=true;s.training.poisoned=true;s.training.strength_advantage=true;
    pd_encode(&s,42,b);assert(pd_decode(b,record_size,&out,&gen));
    assert(!memcmp(&s.avatar,&out.avatar,sizeof s.avatar)&&!out.idle_animation&&out.brightness==90&&out.dim_timeout==2);
    uint8_t encoded_again[PD_RECORD_SIZE];pd_encode(&out,gen,encoded_again);assert(!memcmp(b,encoded_again,record_size));
    assert(pd_select(legacy_v1,sizeof legacy_v1,b,record_size,&out,&gen)==1&&gen==42);
    b[368]=3;record_crc(b);assert(!pd_decode(b,record_size,&out,&gen));
    pd_encode(&s,42,b);b[372]=2;record_crc(b);assert(!pd_decode(b,record_size,&out,&gen));
    pd_encode(&s,42,b);b[record_size-1053]=9;record_crc(b);assert(!pd_decode(b,record_size,&out,&gen));
    pd_encode(&s,42,b);b[record_size-1053]=101;record_crc(b);assert(!pd_decode(b,record_size,&out,&gen));
    pd_encode(&s,42,b);b[record_size-1052]=4;record_crc(b);assert(!pd_decode(b,record_size,&out,&gen));
    pd_demo(&s);pd_encode(&s,41,a);s.hp=7;pd_encode(&s,42,b);
    assert(pd_select(a,record_size,b,record_size,&out,&gen)==1&&out.hp==7&&gen==42);
    /* Every header/CRC bit and distributed payload corruption falls back to older bank. */
    for(size_t i=0;i<record_size;i++) for(int bit=0;bit<((i<8||i>=record_size-4)?8:i%128==0?1:0);bit++) {
        b[i]^=1<<bit;assert(pd_select(a,record_size,b,record_size,&out,&gen)==0&&out.hp==24);b[i]^=1<<bit;
    }
    for(size_t len=0;len<record_size;len++)assert(!pd_decode(b,len,&out,&gen));
    assert(pd_select(a,record_size,b,record_size/2,&out,&gen)==0);
    assert(pd_select(NULL,0,NULL,0,&out,&gen)==-1);
    pd_encode(&s,UINT32_MAX,a);pd_encode(&s,0,b);
    assert(pd_select(a,record_size,b,record_size,&out,&gen)==1&&gen==0);
    assert(pd_crc32((const uint8_t*)"123456789",9)==0xcbf43926);
    if(argc==2) {
        FILE *f=fopen(argv[1],"rb");assert(f);char json[PD_PACKET_MAX+1];
        size_t n=fread(json,1,PD_PACKET_MAX+1,f);fclose(f);
        assert(pd_parse_packet(json,n,&out));assert(pd_valid(&out));
        assert(!strcmp(out.name,"Elian"));assert(out.hp==24&&out.armor==16&&out.abilities[1]==16);
        assert(out.resource_count==1&&out.resources[0].current==1);
        assert(out.avatar.skin==2&&out.avatar.hair==1&&out.avatar.cloak==2&&out.avatar.style==1);
        assert(!pd_parse_packet(json,n-1,&out));
        cJSON *base=cJSON_ParseWithLength(json,n),*v;assert(base);
        for(unsigned lineage=0;lineage<PD_SPECIES_COUNT;lineage++)for(unsigned body=0;body<2;body++){
            v=cJSON_Duplicate(base,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),lineage>=16?12:lineage>=12?11:10);
            cJSON *a=cJSON_GetObjectItemCaseSensitive(v,"avatar");cJSON_AddNumberToObject(a,"lineage",lineage);cJSON_AddNumberToObject(a,"body",body);
            char *encoded=cJSON_PrintUnformatted(v);assert(pd_parse_packet(encoded,strlen(encoded),&out)&&out.lineage==lineage&&out.body==body);cJSON_free(encoded);
            if(lineage>=16){cJSON *old=cJSON_Duplicate(v,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(old,"version"),11);reject(old);}
            if(lineage>=12){cJSON *old=cJSON_Duplicate(v,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(old,"version"),10);reject(old);}
            cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(a,"lineage"),PD_SPECIES_COUNT);reject(v);
        }
        v=cJSON_Duplicate(base,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),10);reject(v);
        const char *numeric_keys[]={"version","level","hp","maxHp","tempHp","armorClass"};
        double bad[]={13,21,33,0,-1,41};
        for (int i=0;i<6;i++) {
            v=cJSON_Duplicate(base,true);cJSON_ReplaceItemInObjectCaseSensitive(v,numeric_keys[i],cJSON_CreateNumber(bad[i]));reject(v);
        }
        v=cJSON_Duplicate(base,true);cJSON_ReplaceItemInObjectCaseSensitive(v,"hp",cJSON_CreateNumber(1.5));reject(v);
        v=cJSON_Duplicate(base,true);cJSON_AddNumberToObject(v,"hp",12);reject(v);
        v=cJSON_Duplicate(base,true);cJSON_AddStringToObject(v,"rawBeyond","not allowed");reject(v);
        const char *avatar_keys[]={"skin","hair","cloak","style"};
        for(int i=0;i<4;i++) {
            v=cJSON_Duplicate(base,true);cJSON_ReplaceItemInObjectCaseSensitive(cJSON_GetObjectItemCaseSensitive(v,"avatar"),avatar_keys[i],cJSON_CreateNumber(i%2?1.5:3));reject(v);
        }
        v=cJSON_Duplicate(base,true);cJSON_AddNumberToObject(cJSON_GetObjectItemCaseSensitive(v,"avatar"),"skin",0);reject(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"avatar");reject(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"wallet");cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),1);cJSON_DeleteItemFromObjectCaseSensitive(v,"status");reject(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"wallet");cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),1);cJSON_DeleteItemFromObjectCaseSensitive(v,"status");
        cJSON_DeleteItemFromObjectCaseSensitive(v,"recovery");cJSON_DeleteItemFromObjectCaseSensitive(v,"magic");cJSON_DeleteItemFromObjectCaseSensitive(v,"inventory");cJSON_DeleteItemFromObjectCaseSensitive(v,"training");cJSON_DeleteItemFromObjectCaseSensitive(v,"avatar");char *old=cJSON_PrintUnformatted(v);
        assert(pd_parse_packet(old,strlen(old),&out)&&out.avatar.skin==0&&out.idle_animation&&!out.training.available);
        cJSON_free(old);cJSON_Delete(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"wallet");cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),2);cJSON_DeleteItemFromObjectCaseSensitive(v,"status");cJSON_DeleteItemFromObjectCaseSensitive(v,"recovery");
        cJSON_DeleteItemFromObjectCaseSensitive(v,"magic");cJSON_DeleteItemFromObjectCaseSensitive(v,"inventory");cJSON_DeleteItemFromObjectCaseSensitive(v,"training");old=cJSON_PrintUnformatted(v);
        assert(pd_parse_packet(old,strlen(old),&out)&&out.avatar.skin==2&&!out.training.available);cJSON_free(old);cJSON_Delete(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"training");reject(v);
        v=cJSON_Duplicate(base,true);cJSON *t=cJSON_GetObjectItemCaseSensitive(v,"training");
        cJSON_ReplaceItemInObjectCaseSensitive(cJSON_GetObjectItemCaseSensitive(t,"skillRanks"),"perception",cJSON_CreateNumber(3));reject(v);
        v=cJSON_Duplicate(base,true);t=cJSON_GetObjectItemCaseSensitive(v,"training");
        cJSON_ReplaceItemInObjectCaseSensitive(cJSON_GetObjectItemCaseSensitive(t,"skillBonuses"),"perception",cJSON_CreateNumber(-31));reject(v);
        v=cJSON_Duplicate(base,true);t=cJSON_GetObjectItemCaseSensitive(v,"training");cJSON_ReplaceItemInObjectCaseSensitive(t,"poisoned",cJSON_CreateNumber(1));reject(v);
        v=cJSON_Duplicate(base,true);t=cJSON_GetObjectItemCaseSensitive(v,"training");
        cJSON_AddItemToArray(cJSON_GetObjectItemCaseSensitive(t,"halfSkills"),cJSON_CreateString("perception"));
        cJSON_AddItemToArray(cJSON_GetObjectItemCaseSensitive(t,"halfSkills"),cJSON_CreateString("perception"));reject(v);
        v=cJSON_Duplicate(base,true);t=cJSON_GetObjectItemCaseSensitive(v,"training");
        cJSON_AddItemToArray(cJSON_GetObjectItemCaseSensitive(t,"blockedChecks"),cJSON_CreateString("save:unknown"));reject(v);
        v=cJSON_Duplicate(base,true);cJSON_ReplaceItemInObjectCaseSensitive(v,"name",cJSON_CreateString(""));reject(v);
        v=cJSON_Duplicate(base,true);cJSON_ReplaceItemInObjectCaseSensitive(v,"name",cJSON_CreateString("1234567890123456789012345678901234567890123456789"));reject(v);
        v=cJSON_Duplicate(base,true);cJSON_ReplaceItemInObjectCaseSensitive(cJSON_GetObjectItemCaseSensitive(v,"abilities"),"DES",cJSON_CreateNumber(31));reject(v);
        v=cJSON_Duplicate(base,true);cJSON *resources=cJSON_GetObjectItemCaseSensitive(v,"resources");
        for(int i=0;i<8;i++) { cJSON_AddItemToArray(resources,cJSON_Duplicate(cJSON_GetArrayItem(resources,0),true)); }
        reject(v);
        v=cJSON_Duplicate(base,true);cJSON_ReplaceItemInObjectCaseSensitive(cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(v,"resources"),0),"current",cJSON_CreateNumber(2));reject(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"inventory");reject(v);
        const char *bad_item_keys[]={"quantity","kind","ability","count","sides","flags","reach","thrownRange","attackModifier","damageModifier","healingModifier"};
        int bad_item_values[]={1000,4,6,7,20,16,121,121,-11,11,31};
        for(int i=0;i<11;i++) {
            v=cJSON_Duplicate(base,true);cJSON *items=cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(v,"inventory"),"items");
            cJSON_ReplaceItemInObjectCaseSensitive(cJSON_GetArrayItem(items,0),bad_item_keys[i],cJSON_CreateNumber(bad_item_values[i]));reject(v);
        }
        v=cJSON_Duplicate(base,true);cJSON *inv=cJSON_GetObjectItemCaseSensitive(v,"inventory");
        cJSON_ReplaceItemInObjectCaseSensitive(inv,"equipped",cJSON_CreateNumber(2));reject(v);
        v=cJSON_Duplicate(base,true);inv=cJSON_GetObjectItemCaseSensitive(v,"inventory");
        cJSON_AddNumberToObject(inv,"equipped",1);reject(v);
        v=cJSON_Duplicate(base,true);inv=cJSON_GetObjectItemCaseSensitive(v,"inventory");
        cJSON_ReplaceItemInObjectCaseSensitive(cJSON_GetObjectItemCaseSensitive(inv,"combat"),"raging",cJSON_CreateNumber(1));reject(v);
        /* v3 USB keeps training but must never fabricate an inventory. */
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"wallet");cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),3);cJSON_DeleteItemFromObjectCaseSensitive(v,"status");cJSON_DeleteItemFromObjectCaseSensitive(v,"recovery");
        cJSON_DeleteItemFromObjectCaseSensitive(v,"magic");cJSON_DeleteItemFromObjectCaseSensitive(v,"inventory");old=cJSON_PrintUnformatted(v);
        assert(pd_parse_packet(old,strlen(old),&out)&&out.training.available&&!out.inventory_available&&out.equipped==PD_NO_WEAPON);cJSON_free(old);cJSON_Delete(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"magic");reject(v);
        v=cJSON_Duplicate(base,true);cJSON *magic=cJSON_GetObjectItemCaseSensitive(v,"magic");
        cJSON_ReplaceItemInObjectCaseSensitive(magic,"concentration",cJSON_CreateNumber(1));reject(v);
        v=cJSON_Duplicate(base,true);magic=cJSON_GetObjectItemCaseSensitive(v,"magic");cJSON_AddNumberToObject(magic,"concentration",0);reject(v);
        v=cJSON_Duplicate(base,true);magic=cJSON_GetObjectItemCaseSensitive(v,"magic");
        cJSON_ReplaceItemInObjectCaseSensitive(cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(magic,"slots"),0),"current",cJSON_CreateNumber(10));reject(v);
        v=cJSON_Duplicate(base,true);magic=cJSON_GetObjectItemCaseSensitive(v,"magic");
        cJSON_AddItemToArray(cJSON_GetObjectItemCaseSensitive(magic,"powers"),cJSON_CreateObject());reject(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"wallet");cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),4);strip_charges(v);cJSON_DeleteItemFromObjectCaseSensitive(v,"status");cJSON_DeleteItemFromObjectCaseSensitive(v,"recovery");cJSON_DeleteItemFromObjectCaseSensitive(v,"magic");old=cJSON_PrintUnformatted(v);
        assert(pd_parse_packet(old,strlen(old),&out)&&out.inventory_available&&!out.magic.available&&out.shortcuts[1]==3);cJSON_free(old);cJSON_Delete(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"wallet");cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),5);strip_charges(v);cJSON_DeleteItemFromObjectCaseSensitive(v,"status");cJSON_DeleteItemFromObjectCaseSensitive(v,"recovery");old=cJSON_PrintUnformatted(v);
        assert(pd_parse_packet(old,strlen(old),&out)&&out.magic.available&&!out.recovery.available);cJSON_free(old);cJSON_Delete(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"recovery");reject(v);
        v=cJSON_Duplicate(base,true);cJSON_AddBoolToObject(cJSON_GetObjectItemCaseSensitive(v,"recovery"),"available",true);reject(v);
        v=cJSON_Duplicate(base,true);cJSON *rec=cJSON_GetObjectItemCaseSensitive(v,"recovery");
        cJSON_AddItemToArray(cJSON_GetObjectItemCaseSensitive(rec,"pools"),cJSON_CreateObject());reject(v);
        const char *hit_keys[]={"die","maximum","used","used"};double hit_bad[]={7,21,4,1.5};
        for(unsigned i=0;i<4;i++) {
            v=cJSON_Duplicate(base,true);rec=cJSON_GetObjectItemCaseSensitive(v,"recovery");cJSON_ReplaceItemInObjectCaseSensitive(rec,"available",cJSON_CreateBool(true));
            cJSON *h=cJSON_Parse("{\"name\":\"Classe\",\"die\":8,\"maximum\":3,\"used\":1}");assert(h);
            cJSON_ReplaceItemInObjectCaseSensitive(h,hit_keys[i],cJSON_CreateNumber(hit_bad[i]));cJSON_AddItemToArray(cJSON_GetObjectItemCaseSensitive(rec,"pools"),h);reject(v);
        }
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"wallet");cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),6);strip_charges(v);cJSON_DeleteItemFromObjectCaseSensitive(v,"status");old=cJSON_PrintUnformatted(v);
        assert(pd_parse_packet(old,strlen(old),&out)&&!out.status.available);cJSON_free(old);cJSON_Delete(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"status");reject(v);
        const char *status_keys[]={"successes","failures","stabilized","available"};
        for(unsigned i=0;i<4;i++) {v=cJSON_Duplicate(base,true);cJSON_ReplaceItemInObjectCaseSensitive(cJSON_GetObjectItemCaseSensitive(v,"status"),status_keys[i],cJSON_CreateNumber(i<2?4:1));reject(v);}
        v=cJSON_Duplicate(base,true);cJSON_AddBoolToObject(cJSON_GetObjectItemCaseSensitive(v,"status"),"available",true);reject(v);
        v=cJSON_Duplicate(base,true);cJSON *stat=cJSON_GetObjectItemCaseSensitive(v,"status");
        cJSON_ReplaceItemInObjectCaseSensitive(stat,"successes",cJSON_CreateNumber(1));reject(v); /* unavailable with data */
        v=cJSON_Duplicate(base,true);stat=cJSON_GetObjectItemCaseSensitive(v,"status");cJSON_ReplaceItemInObjectCaseSensitive(stat,"available",cJSON_CreateBool(true));
        cJSON *condition=cJSON_Parse("{\"name\":\"Nota\",\"level\":7}");cJSON_AddItemToArray(cJSON_GetObjectItemCaseSensitive(stat,"conditions"),condition);reject(v);
        /* Full boundary values remain representable, including all eight counters. */
        v=cJSON_Duplicate(base,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"maxHp"),999);
        cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"hp"),999);
        cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"tempHp"),999);
        resources=cJSON_GetObjectItemCaseSensitive(v,"resources");
        for(int i=0;i<7;i++)cJSON_AddItemToArray(resources,cJSON_Duplicate(cJSON_GetArrayItem(resources,0),true));
        char *encoded=cJSON_PrintUnformatted(v);assert(pd_parse_packet(encoded,strlen(encoded),&out));
        assert(out.hp==999&&out.temp_hp==999&&out.resource_count==8);cJSON_free(encoded);cJSON_Delete(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"wallet");cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),7);strip_charges(v);old=cJSON_PrintUnformatted(v);
        assert(pd_parse_packet(old,strlen(old),&out)&&!out.items[0].charges.maximum);cJSON_free(old);cJSON_Delete(v);
        v=cJSON_Duplicate(base,true);cJSON_DeleteItemFromObjectCaseSensitive(v,"wallet");cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),7);reject(v);
        const char *charge_fields[]={"current","maximum","recovery"};
        for(unsigned i=0;i<3;i++){
            v=cJSON_Duplicate(base,true);cJSON *item=cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(v,"inventory"),"items"),0);
            cJSON_ReplaceItemInObjectCaseSensitive(cJSON_GetObjectItemCaseSensitive(item,"charges"),charge_fields[i],cJSON_CreateNumber(i==2?3:1000));reject(v);
        }
        v=cJSON_Duplicate(base,true);cJSON *item=cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(v,"inventory"),"items"),0);
        cJSON_AddNumberToObject(cJSON_GetObjectItemCaseSensitive(item,"charges"),"current",0);reject(v);
        /* USB13 class look preserves the complete projected game state. */
        for(unsigned lineage=0;lineage<21;lineage++)for(unsigned body=0;body<2;body++)for(unsigned outfit=1;outfit<13;outfit++){
            v=cJSON_Duplicate(base,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),13);cJSON *a=cJSON_GetObjectItemCaseSensitive(v,"avatar");
            cJSON_AddNumberToObject(a,"lineage",lineage);cJSON_AddNumberToObject(a,"body",body);cJSON_AddNumberToObject(a,"outfit",outfit);
            char *wire=cJSON_PrintUnformatted(v);assert(pd_parse_packet(wire,strlen(wire),&out)&&out.lineage==lineage&&out.body==body&&out.outfit==outfit);cJSON_free(wire);
            pd_state expected=out;expected.lineage=expected.body=expected.outfit=0;char *original=cJSON_PrintUnformatted(base);pd_state original_state;assert(pd_parse_packet(original,strlen(original),&original_state)&&!memcmp(&expected,&original_state,sizeof expected));cJSON_free(original);
            cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),12);reject(v);
        }
        for(unsigned invalid=0;invalid<4;invalid++){
            v=cJSON_Duplicate(base,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),13);cJSON *a=cJSON_GetObjectItemCaseSensitive(v,"avatar");cJSON_AddNumberToObject(a,"lineage",0);cJSON_AddNumberToObject(a,"body",0);
            if(invalid==1)cJSON_AddNumberToObject(a,"outfit",13);
            if(invalid==2)cJSON_AddStringToObject(a,"outfit","1");
            if(invalid==3){cJSON_AddNumberToObject(a,"outfit",1);cJSON_AddNumberToObject(a,"outfit",2);}reject(v);
        }
        cJSON_Delete(base);
    }
    const char *invalid[]={"{}","[]","null","{\"format\":\"pocketdd-device\",\"version\":1}","{", "{\"x\":\"\\u0000\"}","1e999"};
    for(size_t i=0;i<sizeof invalid/sizeof invalid[0];i++) assert(!pd_parse_packet(invalid[i],strlen(invalid[i]),&out));
    puts("C core: HP/temp, counters, d20, CRC/corruption/wrap, golden v1..v24 migration, PDD33 persistence, strict legacy USB compatibility and 504 USB13 class combinations passed");
    return 0;
}
