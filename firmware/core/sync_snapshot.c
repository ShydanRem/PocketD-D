#include "sync_snapshot.h"
#include "cJSON.h"
#include <string.h>
typedef pd_sync_sample sample;
char *pd_sync_encode(const sample *s,uint32_t serial,const char *session){
 cJSON *o=cJSON_CreateObject(),*v=cJSON_CreateObject(),*rolls=cJSON_CreateArray();if(!o||!v||!rolls){cJSON_Delete(o);cJSON_Delete(v);cJSON_Delete(rolls);return NULL;}
 cJSON_AddNumberToObject(o,"version",1);cJSON_AddStringToObject(o,"boot",session);cJSON_AddNumberToObject(o,"counter",serial);cJSON_AddNumberToObject(o,"epoch",s->epoch);cJSON_AddStringToObject(o,"name",s->name);cJSON_AddNumberToObject(v,"hp",s->hp);cJSON_AddNumberToObject(v,"max",s->max);cJSON_AddNumberToObject(v,"temp",s->temp);cJSON_AddItemToObject(o,"vitals",v);cJSON_AddNumberToObject(o,"sequence",s->history.sequence);cJSON_AddItemToObject(o,"rolls",rolls);
 for(unsigned i=0;i<s->history.count;i++){const pd_history_entry *e=&s->history.entries[i];cJSON *r=cJSON_CreateObject(),*dice=cJSON_CreateArray();if(!r||!dice){cJSON_Delete(r);cJSON_Delete(dice);cJSON_Delete(o);return NULL;}cJSON_AddItemToArray(rolls,r);cJSON_AddNumberToObject(r,"sequence",e->sequence);cJSON_AddNumberToObject(r,"kind",e->kind);cJSON_AddStringToObject(r,"label",e->label);cJSON_AddNumberToObject(r,"sides",e->sides);cJSON_AddNumberToObject(r,"mode",e->mode);cJSON_AddNumberToObject(r,"bonus",e->bonus);cJSON_AddNumberToObject(r,"total",e->total);cJSON_AddBoolToObject(r,"floor",e->floor_zero);cJSON_AddItemToObject(r,"dice",dice);for(unsigned j=0;j<e->count;j++)cJSON_AddItemToArray(dice,cJSON_CreateNumber(e->dice[j]));}
 char *json=cJSON_PrintUnformatted(o);cJSON_Delete(o);return json;
}
