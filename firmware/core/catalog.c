#include "catalog.h"
#include <string.h>
static unsigned upper(unsigned c){return c>='a'&&c<='z'?c-'a'+'A':c;}
bool pd_search_query(char out[PD_SEARCH_BYTES],const char *text){
    if(!out||!text)return false;
    size_t start=0,end=strlen(text);while(start<end&&text[start]==' ')start++;while(end>start&&text[end-1]==' ')end--;
    if(end-start>=PD_SEARCH_BYTES)return false;
    char query[PD_SEARCH_BYTES]={0};
    for(size_t i=start;i<end;i++){unsigned c=(unsigned char)text[i];if(c<32||c>126)return false;query[i-start]=(char)upper(c);}
    memcpy(out,query,sizeof query);return true;
}
static bool contains(const char *name,const char *query){
    if(!query[0])return true;
    for(unsigned i=0;i<PD_ITEM_NAME_BYTES&&name[i];i++){
        unsigned j=0;while(query[j]&&i+j<PD_ITEM_NAME_BYTES&&name[i+j]&&upper((unsigned char)name[i+j])==(unsigned char)query[j])j++;
        if(!query[j])return true;
    }
    return false;
}
static bool slot_available(const pd_state *s,unsigned minimum){
    for(unsigned i=minimum?minimum-1:0;i<9;i++)if(s->magic.slots[i].current||s->magic.pact[i].current)return true;
    return false;
}
static bool matches_filter(const pd_state *s,bool items,unsigned i,unsigned filter){
    if(filter==PD_CATALOG_ALL)return true;
    if(items){const pd_item *v=&s->items[i];return v->quantity&&(v->kind==PD_HEALING||v->kind==PD_CONSUMABLE||v->charges.current);}
    const pd_power *p=&s->magic.powers[i];
    if(p->kind){if(p->flags&PD_PASSIVE)return false;return p->remaining||(p->effect==3&&slot_available(s,1));}
    if(!(p->flags&PD_PREPARED)||s->combat.raging||((p->flags&PD_LIMITED)&&!p->remaining))return false;
    return !(p->flags&PD_USES_SLOT)||!p->level||slot_available(s,p->level);
}
static unsigned scan(const pd_state *s,unsigned catalog,const char *text,unsigned filter,unsigned position,int *index){
    char query[PD_SEARCH_BYTES];if(!s||catalog>PD_CATALOG_POWERS||!pd_search_query(query,text))return 0;
    bool items=catalog==PD_CATALOG_ITEMS;unsigned count=items?s->item_count:s->magic.count;
    if(filter>PD_CATALOG_POUCH||(filter==PD_CATALOG_READY&&items)||(filter==PD_CATALOG_POUCH&&!items))return 0;
    if((items?(!s->inventory_available||count>PD_ITEMS):(!s->magic.available||count>PD_POWERS)))return 0;
    unsigned found=0;
    for(unsigned i=0;i<count;i++){
        if(!items&&s->magic.powers[i].effect>=4)continue;
        if(!items&&catalog!=PD_CATALOG_POWERS&&s->magic.powers[i].kind!=(catalog==PD_CATALOG_FEATURES))continue;
        if(!matches_filter(s,items,i,filter))continue;
        if(!contains(items?s->items[i].name:s->magic.powers[i].name,query))continue;
        if(index&&found==position)*index=(int)i;
        found++;
    }
    return found;
}
unsigned pd_catalog_filtered_count(const pd_state *s,unsigned catalog,const char *query,unsigned filter){return scan(s,catalog,query,filter,0,NULL);}
int pd_catalog_filtered_index(const pd_state *s,unsigned catalog,const char *query,unsigned filter,unsigned position){int index=-1;scan(s,catalog,query,filter,position,&index);return index;}
unsigned pd_catalog_count(const pd_state *s,unsigned catalog,const char *query){return pd_catalog_filtered_count(s,catalog,query,PD_CATALOG_ALL);}
int pd_catalog_index(const pd_state *s,unsigned catalog,const char *query,unsigned position){return pd_catalog_filtered_index(s,catalog,query,PD_CATALOG_ALL,position);}
