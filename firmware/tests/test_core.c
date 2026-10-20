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
static void record_crc(uint8_t *record) {
    uint32_t crc=pd_crc32(record,PD_RECORD_SIZE-4);
    for(int i=0;i<4;i++)record[PD_RECORD_SIZE-4+i]=crc>>(8*i);
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
    uint8_t a[PD_RECORD_SIZE],b[PD_RECORD_SIZE];
    assert(pd_decode(legacy_v1,sizeof legacy_v1,&out,&gen)&&gen==41&&out.hp==17&&out.temp_hp==4);
    assert(!out.avatar.skin&&!out.avatar.hair&&!out.avatar.cloak&&!out.avatar.style&&out.idle_animation);
    assert(!out.training.available);
    assert(pd_decode(legacy_v2,sizeof legacy_v2,&out,&gen)&&gen==42&&out.hp==17&&!out.idle_animation);
    assert(out.avatar.skin==2&&!out.training.available);
    assert(pd_decode(legacy_v3,sizeof legacy_v3,&out,&gen)&&gen==43&&out.training.ranks[0]==2&&out.training.poisoned&&!out.inventory_available&&out.equipped==PD_NO_WEAPON);
    pd_demo(&s);s.avatar=(pd_avatar){2,1,2,1};s.idle_animation=false;
    for(int i=0;i<PD_SKILLS;i++) { s.training.ranks[i]=i%3;s.training.skill_bonus[i]=i-8; }
    for(int i=0;i<6;i++) { s.training.save_bonus[i]=i-2;s.training.check_bonus[i]=3-i; }
    s.training.saves=63;s.training.half_skills=0x3ffff;s.training.blocked=0x3fffffff;
    s.training.half_checks=true;s.training.poisoned=true;s.training.strength_advantage=true;
    pd_encode(&s,42,b);assert(pd_decode(b,sizeof b,&out,&gen));
    assert(!memcmp(&s.avatar,&out.avatar,sizeof s.avatar)&&!out.idle_animation);
    uint8_t encoded_again[PD_RECORD_SIZE];pd_encode(&out,gen,encoded_again);assert(!memcmp(b,encoded_again,sizeof b));
    assert(pd_select(legacy_v1,sizeof legacy_v1,b,sizeof b,&out,&gen)==1&&gen==42);
    b[368]=3;record_crc(b);assert(!pd_decode(b,sizeof b,&out,&gen));
    pd_encode(&s,42,b);b[372]=2;record_crc(b);assert(!pd_decode(b,sizeof b,&out,&gen));
    pd_demo(&s);pd_encode(&s,41,a);s.hp=7;pd_encode(&s,42,b);
    assert(pd_select(a,sizeof a,b,sizeof b,&out,&gen)==1&&out.hp==7&&gen==42);
    /* Every single-bit corruption in newest bank falls back to intact older bank. */
    for(size_t i=0;i<sizeof b;i++) for(int bit=0;bit<(i<8||i>=sizeof b-4?8:1);bit++) {
        b[i]^=1<<bit;assert(pd_select(a,sizeof a,b,sizeof b,&out,&gen)==0&&out.hp==24);b[i]^=1<<bit;
    }
    for(size_t len=0;len<sizeof b;len++)assert(pd_select(a,sizeof a,b,len,&out,&gen)==0);
    assert(pd_select(NULL,0,NULL,0,&out,&gen)==-1);
    pd_encode(&s,UINT32_MAX,a);pd_encode(&s,0,b);
    assert(pd_select(a,sizeof a,b,sizeof b,&out,&gen)==1&&gen==0);
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
        const char *numeric_keys[]={"version","level","hp","maxHp","tempHp","armorClass"};
        double bad[]={5,21,33,0,-1,41};
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
        v=cJSON_Duplicate(base,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),1);reject(v);
        v=cJSON_Duplicate(base,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),1);
        cJSON_DeleteItemFromObjectCaseSensitive(v,"inventory");cJSON_DeleteItemFromObjectCaseSensitive(v,"training");cJSON_DeleteItemFromObjectCaseSensitive(v,"avatar");char *old=cJSON_PrintUnformatted(v);
        assert(pd_parse_packet(old,strlen(old),&out)&&out.avatar.skin==0&&out.idle_animation&&!out.training.available);
        cJSON_free(old);cJSON_Delete(v);
        v=cJSON_Duplicate(base,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),2);
        cJSON_DeleteItemFromObjectCaseSensitive(v,"inventory");cJSON_DeleteItemFromObjectCaseSensitive(v,"training");old=cJSON_PrintUnformatted(v);
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
        v=cJSON_Duplicate(base,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"version"),3);
        cJSON_DeleteItemFromObjectCaseSensitive(v,"inventory");old=cJSON_PrintUnformatted(v);
        assert(pd_parse_packet(old,strlen(old),&out)&&out.training.available&&!out.inventory_available&&out.equipped==PD_NO_WEAPON);cJSON_free(old);cJSON_Delete(v);
        /* Full boundary values remain representable, including all eight counters. */
        v=cJSON_Duplicate(base,true);cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"maxHp"),999);
        cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"hp"),999);
        cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(v,"tempHp"),999);
        resources=cJSON_GetObjectItemCaseSensitive(v,"resources");
        for(int i=0;i<7;i++)cJSON_AddItemToArray(resources,cJSON_Duplicate(cJSON_GetArrayItem(resources,0),true));
        char *encoded=cJSON_PrintUnformatted(v);assert(pd_parse_packet(encoded,strlen(encoded),&out));
        assert(out.hp==999&&out.temp_hp==999&&out.resource_count==8);cJSON_free(encoded);cJSON_Delete(v);cJSON_Delete(base);
    }
    const char *invalid[]={"{}","[]","null","{\"format\":\"pocketdd-device\",\"version\":1}","{", "{\"x\":\"\\u0000\"}","1e999"};
    for(size_t i=0;i<sizeof invalid/sizeof invalid[0];i++) assert(!pd_parse_packet(invalid[i],strlen(invalid[i]),&out));
    puts("C core: HP/temp, counters, d20, CRC/corruption/wrap, golden v1 migration, v2 avatar persistence and strict USB v1/v2 passed");
    return 0;
}
