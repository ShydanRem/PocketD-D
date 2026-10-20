#include "pocketdd.h"
#include "checks.h"
#include "cJSON.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
/* Reject duplicates, unknown members, fractional numbers and nested source data. */
static bool keys(cJSON *o, const char *const *allowed, int count) {
    if (!cJSON_IsObject(o) || cJSON_GetArraySize(o)!=count) return false;
    for (int i=0;i<count;i++) {
        int found=0; cJSON *p;
        cJSON_ArrayForEach(p,o) if (p->string && !strcmp(p->string,allowed[i])) found++;
        if (found!=1) return false;
    }
    return true;
}
static int number(cJSON *o,const char *key,int lo,int hi) {
    cJSON *n=cJSON_GetObjectItemCaseSensitive(o,key);
    if (!cJSON_IsNumber(n) || !isfinite(n->valuedouble) || n->valuedouble<lo || n->valuedouble>hi ||
        floor(n->valuedouble)!=n->valuedouble) return -1;
    return n->valueint;
}
static bool copy(cJSON *o,const char *key,char *out,size_t cap) {
    cJSON *n=cJSON_GetObjectItemCaseSensitive(o,key);
    if (!cJSON_IsString(n) || !n->valuestring[0] || strlen(n->valuestring)>=cap) return false;
    strcpy(out,n->valuestring); return true;
}
static bool bonus_map(cJSON *o,const char *const *names,int count,int8_t *out) {
    if(!keys(o,names,count))return false;
    for(int i=0;i<count;i++) {
        cJSON *v=cJSON_GetObjectItemCaseSensitive(o,names[i]);
        if(!cJSON_IsNumber(v)||!isfinite(v->valuedouble)||v->valuedouble<-30||v->valuedouble>30||floor(v->valuedouble)!=v->valuedouble)return false;
        out[i]=v->valueint;
    }
    return true;
}
static int named(const char *name,const char *const *names,int count) {
    for(int i=0;i<count;i++)if(!strcmp(name,names[i]))return i;
    return -1;
}
static bool training(cJSON *o,pd_training *t) {
    const char *const fields[]={"skillRanks","saveProficiencies","skillBonuses","saveBonuses","checkBonuses","halfAbilityChecks","halfSkills","blockedChecks","poisoned","strengthAdvantage"};
    if(!keys(o,fields,10))return false;
    cJSON *ranks=cJSON_GetObjectItemCaseSensitive(o,"skillRanks"),*saves=cJSON_GetObjectItemCaseSensitive(o,"saveProficiencies");
    if(!keys(ranks,pd_skill_ids,PD_SKILLS)||!keys(saves,pd_abilities,6))return false;
    for(int i=0;i<PD_SKILLS;i++) { int n=number(ranks,pd_skill_ids[i],0,2);if(n<0)return false;t->ranks[i]=n; }
    for(int i=0;i<6;i++) {
        cJSON *v=cJSON_GetObjectItemCaseSensitive(saves,pd_abilities[i]);if(!cJSON_IsBool(v))return false;
        if(cJSON_IsTrue(v))t->saves|=1u<<i;
    }
    if(!bonus_map(cJSON_GetObjectItemCaseSensitive(o,"skillBonuses"),pd_skill_ids,PD_SKILLS,t->skill_bonus)||
       !bonus_map(cJSON_GetObjectItemCaseSensitive(o,"saveBonuses"),pd_abilities,6,t->save_bonus)||
       !bonus_map(cJSON_GetObjectItemCaseSensitive(o,"checkBonuses"),pd_abilities,6,t->check_bonus))return false;
    const char *const booleans[]={"halfAbilityChecks","poisoned","strengthAdvantage"};
    bool *targets[]={&t->half_checks,&t->poisoned,&t->strength_advantage};
    for(int i=0;i<3;i++) {
        cJSON *v=cJSON_GetObjectItemCaseSensitive(o,booleans[i]);if(!cJSON_IsBool(v))return false;*targets[i]=cJSON_IsTrue(v);
    }
    cJSON *half=cJSON_GetObjectItemCaseSensitive(o,"halfSkills");
    if(!cJSON_IsArray(half)||cJSON_GetArraySize(half)>PD_SKILLS)return false;
    cJSON *v;cJSON_ArrayForEach(v,half) {
        if(!cJSON_IsString(v))return false;
        int i=named(v->valuestring,pd_skill_ids,PD_SKILLS);
        if(i<0||(t->half_skills&(1u<<i)))return false;
        t->half_skills|=1u<<i;
    }
    cJSON *blocked=cJSON_GetObjectItemCaseSensitive(o,"blockedChecks");
    if(!cJSON_IsArray(blocked)||cJSON_GetArraySize(blocked)>30)return false;
    cJSON_ArrayForEach(v,blocked) {
        if(!cJSON_IsString(v))return false;
        const char *name=v->valuestring;int bit=-1,index;
        if(!strncmp(name,"check:",6)&&(index=named(name+6,pd_abilities,6))>=0)bit=index;
        else if(!strncmp(name,"skill:",6)&&(index=named(name+6,pd_skill_ids,PD_SKILLS))>=0)bit=6+index;
        else if(!strncmp(name,"save:",5)&&(index=named(name+5,pd_abilities,6))>=0)bit=24+index;
        if(bit<0||(t->blocked&(1u<<bit)))return false;
        t->blocked|=1u<<bit;
    }
    t->available=true;return true;
}
static bool inventory(cJSON *o,pd_state *s) {
    const char *const fields[]={"items","equipped","combat"};
    const char *const combat_fields[]={"rageBonus","criticalDice","raging","powerAttack","thrown"};
    const char *const item_fields[]={"name","damageType","quantity","kind","ability","count","sides","flags","reach","thrownRange","attackModifier","damageModifier","healingModifier"};
    if(!keys(o,fields,3))return false;
    cJSON *items=cJSON_GetObjectItemCaseSensitive(o,"items"),*c=cJSON_GetObjectItemCaseSensitive(o,"combat");
    if(!cJSON_IsArray(items)||cJSON_GetArraySize(items)>PD_ITEMS||!keys(c,combat_fields,5))return false;
    int equipped=number(o,"equipped",0,PD_ITEMS),rage=number(c,"rageBonus",0,10),critical=number(c,"criticalDice",0,6);
    if(equipped<0||rage<0||critical<0)return false;
    s->inventory_available=true;s->item_count=cJSON_GetArraySize(items);s->equipped=equipped?equipped-1:PD_NO_WEAPON;
    s->combat.rage_bonus=rage;s->combat.critical_dice=critical;
    bool *targets[]={&s->combat.raging,&s->combat.power_attack,&s->combat.thrown};
    for(int i=0;i<3;i++) { cJSON *v=cJSON_GetObjectItemCaseSensitive(c,combat_fields[i+2]);if(!cJSON_IsBool(v))return false;*targets[i]=cJSON_IsTrue(v); }
    for(int i=0;i<s->item_count;i++) {
        cJSON *v=cJSON_GetArrayItem(items,i);pd_item *item=&s->items[i];
        if(!keys(v,item_fields,13)||!copy(v,"name",item->name,sizeof item->name))return false;
        cJSON *type=cJSON_GetObjectItemCaseSensitive(v,"damageType");
        if(!cJSON_IsString(type)||strlen(type->valuestring)>=sizeof item->damage_type)return false;
        strcpy(item->damage_type,type->valuestring);
        const int maxima[]={999,3,5,6,12,15,120,120,10,10,30};
        int values[11];
        for(int j=0;j<11;j++) {
            cJSON *n=cJSON_GetObjectItemCaseSensitive(v,item_fields[j+2]);int lo=(j==8||j==9)?-10:0;
            if(!cJSON_IsNumber(n)||!isfinite(n->valuedouble)||n->valuedouble<lo||n->valuedouble>maxima[j]||floor(n->valuedouble)!=n->valuedouble)return false;
            values[j]=n->valueint;
        }
        item->quantity=values[0];item->kind=values[1];item->ability=values[2];item->count=values[3];item->sides=values[4];item->flags=values[5];item->reach=values[6];item->thrown_range=values[7];
        item->attack_modifier=values[8];item->damage_modifier=values[9];item->healing_modifier=values[10];
    }
    return true;
}
static bool magic(cJSON *o,pd_magic *m) {
    const char *const fields[]={"powers","slots","pact","concentration"};
    const char *const power_keys[]={"name","description","kind","level","flags","ability","maximum","remaining","recovery","effect","attackModifier","saveModifier"};
    const char *const slot_keys[]={"current","maximum"};
    if(!keys(o,fields,4))return false;
    cJSON *powers=cJSON_GetObjectItemCaseSensitive(o,"powers");
    int concentration=number(o,"concentration",0,PD_POWERS);
    if(concentration<0||!cJSON_IsArray(powers)||cJSON_GetArraySize(powers)>PD_POWERS)return false;
    m->available=true;m->count=cJSON_GetArraySize(powers);m->concentration=concentration;
    for(int k=0;k<2;k++) {
        cJSON *slots=cJSON_GetObjectItemCaseSensitive(o,k?"pact":"slots");if(!cJSON_IsArray(slots)||cJSON_GetArraySize(slots)!=9)return false;
        for(int i=0;i<9;i++) {
            cJSON *v=cJSON_GetArrayItem(slots,i);int cur=number(v,"current",0,9),max=number(v,"maximum",0,9);
            if(!keys(v,slot_keys,2)||cur<0||max<0||cur>max)return false;
            (k?m->pact:m->slots)[i]=(pd_slot){cur,max};
        }
    }
    for(int i=0;i<m->count;i++) {
        cJSON *o=cJSON_GetArrayItem(powers,i);pd_power *v=&m->powers[i];
        if(!keys(o,power_keys,12)||!copy(o,"name",v->name,sizeof v->name))return false;
        cJSON *description=cJSON_GetObjectItemCaseSensitive(o,"description");
        if(!cJSON_IsString(description)||strlen(description->valuestring)>=sizeof v->description)return false;
        strcpy(v->description,description->valuestring);
        const int maxima[]={1,9,63,5,99,99,2,2,30,30};int values[10];
        for(int j=0;j<10;j++) {
            cJSON *n=cJSON_GetObjectItemCaseSensitive(o,power_keys[j+2]);int lo=j>=8?-15:0;
            if(!cJSON_IsNumber(n)||!isfinite(n->valuedouble)||n->valuedouble<lo||n->valuedouble>maxima[j]||floor(n->valuedouble)!=n->valuedouble)return false;
            values[j]=n->valueint;
        }
        v->kind=values[0];v->level=values[1];v->flags=values[2];v->ability=values[3];v->maximum=values[4];v->remaining=values[5];v->recovery=values[6];v->effect=values[7];v->attack_modifier=values[8];v->save_modifier=values[9];
    }
    return true;
}
static bool recovery(cJSON *o,pd_recovery *r) {
    const char *const fields[]={"available","pools"},*const pool_fields[]={"name","die","maximum","used"};
    if(!keys(o,fields,2))return false;
    cJSON *available=cJSON_GetObjectItemCaseSensitive(o,"available"),*pools=cJSON_GetObjectItemCaseSensitive(o,"pools");
    if(!cJSON_IsBool(available)||!cJSON_IsArray(pools)||cJSON_GetArraySize(pools)>PD_HIT_POOLS)return false;
    r->available=cJSON_IsTrue(available);r->count=cJSON_GetArraySize(pools);
    for(unsigned i=0;i<r->count;i++) {
        cJSON *o=cJSON_GetArrayItem(pools,i);pd_hit_die *v=&r->pools[i];
        int die=number(o,"die",6,12),maximum=number(o,"maximum",1,20),used=number(o,"used",0,20);
        if(!keys(o,pool_fields,4)||!copy(o,"name",v->name,sizeof v->name)||die<0||maximum<0||used<0)return false;
        v->die=die;v->maximum=maximum;v->used=used;
    }
    return true;
}
static bool status(cJSON *o,pd_status *s) {
    const char *const fields[]={"available","successes","failures","stabilized","conditions"},*const condition_fields[]={"name","level"};
    if(!keys(o,fields,5))return false;
    cJSON *available=cJSON_GetObjectItemCaseSensitive(o,"available"),*stable=cJSON_GetObjectItemCaseSensitive(o,"stabilized"),*conditions=cJSON_GetObjectItemCaseSensitive(o,"conditions");
    int successes=number(o,"successes",0,3),failures=number(o,"failures",0,3);
    if(!cJSON_IsBool(available)||!cJSON_IsBool(stable)||successes<0||failures<0||!cJSON_IsArray(conditions)||cJSON_GetArraySize(conditions)>PD_CONDITIONS)return false;
    s->available=cJSON_IsTrue(available);s->stabilized=cJSON_IsTrue(stable);s->successes=successes;s->failures=failures;s->count=cJSON_GetArraySize(conditions);
    for(unsigned i=0;i<s->count;i++) {
        cJSON *v=cJSON_GetArrayItem(conditions,i);int level=number(v,"level",0,6);
        if(!keys(v,condition_fields,2)||level<0||!copy(v,"name",s->conditions[i].name,61))return false;
        s->conditions[i].level=level;
    }
    return true;
}
bool pd_parse_packet(const char *json,size_t size,pd_state *s) {
    if (!json || !size || size>PD_PACKET_MAX || memchr(json,0,size)) return false;
    /* cJSON's string model cannot represent embedded NUL. Refuse all JSON Unicode
       escapes: wire strings are deliberately printable ASCII; accents projected on PC. */
    for (size_t i=0;i+1<size;i++) if (json[i]=='\\') { if (json[i+1]=='u') return false; ++i; }
    /* Bound recursive parser work before calling cJSON (USB task has finite stack). */
    int depth=0; bool quoted=false;
    for (size_t i=0;i<size;i++) {
        if (quoted && json[i]=='\\') { ++i; continue; }
        if (json[i]=='"') { quoted=!quoted; continue; }
        if (!quoted && (json[i]=='{' || json[i]=='[') && ++depth>4) return false;
        if (!quoted && (json[i]=='}' || json[i]==']') && --depth<0) return false;
    }
    const char *end=NULL;
    cJSON *root=cJSON_ParseWithLengthOpts(json,size,&end,false);
    if (!root) return false;
    while (end<json+size && (*end==' ' || *end=='\n' || *end=='\r' || *end=='\t')) end++;
    bool ok=false;pd_state *allocated=calloc(1,sizeof *allocated);if(!allocated){cJSON_Delete(root);return false;}
    #define candidate (*allocated)
    candidate.brightness=50;candidate.idle_animation=true;candidate.equipped=PD_NO_WEAPON;
    candidate.shortcuts[0]=1;candidate.shortcuts[1]=3;candidate.shortcuts[2]=5;
    const char *const fields[]={"format","version","name","level","hp","maxHp","tempHp","armorClass","abilities","resources","avatar","training","inventory","magic","recovery","status"};
    const char *const abilities[]={"FOR","DES","COS","INT","SAG","CAR"};
    const char *const resource_keys[]={"name","current","maximum"};
    const char *const avatar_keys[]={"skin","hair","cloak","style"};
    int version=number(root,"version",1,7);
    cJSON *fmt=cJSON_GetObjectItemCaseSensitive(root,"format");
    if (version<0 || end!=json+size || !keys(root,fields,version==1?10:version==2?11:version==3?12:version==4?13:version==5?14:version==6?15:16) || !cJSON_IsString(fmt) ||
        strcmp(fmt->valuestring,"pocketdd-device") ||
        !copy(root,"name",candidate.name,sizeof candidate.name)) goto done;
    int level=number(root,"level",1,20),hp=number(root,"hp",0,999),max=number(root,"maxHp",1,999),
        temp=number(root,"tempHp",0,999),armor=number(root,"armorClass",1,40);
    if (level<0 || hp<0 || max<0 || temp<0 || armor<0) goto done;
    candidate.level=level; candidate.hp=hp; candidate.max_hp=max; candidate.temp_hp=temp; candidate.armor=armor;
    if(version>=2) {
        cJSON *avatar=cJSON_GetObjectItemCaseSensitive(root,"avatar");
        if(!keys(avatar,avatar_keys,4))goto done;
        int skin=number(avatar,"skin",0,2),hair=number(avatar,"hair",0,2),
            cloak=number(avatar,"cloak",0,2),style=number(avatar,"style",0,2);
        if(skin<0||hair<0||cloak<0||style<0)goto done;
        candidate.avatar=(pd_avatar){skin,hair,cloak,style};
    }
    if(version>=3&&!training(cJSON_GetObjectItemCaseSensitive(root,"training"),&candidate.training))goto done;
    if(version>=4&&!inventory(cJSON_GetObjectItemCaseSensitive(root,"inventory"),&candidate))goto done;
    if(version>=5&&!magic(cJSON_GetObjectItemCaseSensitive(root,"magic"),&candidate.magic))goto done;
    if(version>=6&&!recovery(cJSON_GetObjectItemCaseSensitive(root,"recovery"),&candidate.recovery))goto done;
    if(version==7&&!status(cJSON_GetObjectItemCaseSensitive(root,"status"),&candidate.status))goto done;
    cJSON *a=cJSON_GetObjectItemCaseSensitive(root,"abilities");
    if (!keys(a,abilities,6)) goto done;
    for (int i=0;i<6;i++) { int n=number(a,abilities[i],1,30); if (n<0) goto done; candidate.abilities[i]=n; }
    cJSON *res=cJSON_GetObjectItemCaseSensitive(root,"resources");
    if (!cJSON_IsArray(res) || cJSON_GetArraySize(res)>PD_RESOURCES) goto done;
    candidate.resource_count=cJSON_GetArraySize(res);
    for (int i=0;i<candidate.resource_count;i++) {
        cJSON *r=cJSON_GetArrayItem(res,i);
        int cur=number(r,"current",0,999),maximum=number(r,"maximum",1,999);
        if (!keys(r,resource_keys,3) || cur<0 || maximum<0 ||
            !copy(r,"name",candidate.resources[i].name,PD_RESOURCE_BYTES)) goto done;
        candidate.resources[i].current=cur; candidate.resources[i].maximum=maximum;
    }
    if (pd_valid(&candidate)) { *s=candidate; ok=true; }
done:
    free(allocated);cJSON_Delete(root);return ok;
    #undef candidate
}
