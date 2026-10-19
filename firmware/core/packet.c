#include "pocketdd.h"
#include "cJSON.h"
#include <math.h>
#include <string.h>
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
    bool ok=false; pd_state candidate={0};
    const char *const fields[]={"format","version","name","level","hp","maxHp","tempHp","armorClass","abilities","resources"};
    const char *const abilities[]={"FOR","DES","COS","INT","SAG","CAR"};
    const char *const resource_keys[]={"name","current","maximum"};
    cJSON *fmt=cJSON_GetObjectItemCaseSensitive(root,"format");
    if (end!=json+size || !keys(root,fields,10) || !cJSON_IsString(fmt) ||
        strcmp(fmt->valuestring,"pocketdd-device") || number(root,"version",1,1)!=1 ||
        !copy(root,"name",candidate.name,sizeof candidate.name)) goto done;
    int level=number(root,"level",1,20),hp=number(root,"hp",0,999),max=number(root,"maxHp",1,999),
        temp=number(root,"tempHp",0,999),armor=number(root,"armorClass",1,40);
    if (level<0 || hp<0 || max<0 || temp<0 || armor<0) goto done;
    candidate.level=level; candidate.hp=hp; candidate.max_hp=max; candidate.temp_hp=temp; candidate.armor=armor;
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
    cJSON_Delete(root); return ok;
}
