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
static unsigned scan(const pd_state *s,unsigned catalog,const char *text,unsigned position,int *index){
    char query[PD_SEARCH_BYTES];if(!s||catalog>PD_CATALOG_POWERS||!pd_search_query(query,text))return 0;
    bool items=catalog==PD_CATALOG_ITEMS;unsigned count=items?s->item_count:s->magic.count;
    if((items?(!s->inventory_available||count>PD_ITEMS):(!s->magic.available||count>PD_POWERS)))return 0;
    unsigned found=0;
    for(unsigned i=0;i<count;i++){
        if(!items&&catalog!=PD_CATALOG_POWERS&&s->magic.powers[i].kind!=(catalog==PD_CATALOG_FEATURES))continue;
        if(!contains(items?s->items[i].name:s->magic.powers[i].name,query))continue;
        if(index&&found==position)*index=(int)i;
        found++;
    }
    return found;
}
unsigned pd_catalog_count(const pd_state *s,unsigned catalog,const char *query){return scan(s,catalog,query,0,NULL);}
int pd_catalog_index(const pd_state *s,unsigned catalog,const char *query,unsigned position){int index=-1;scan(s,catalog,query,position,&index);return index;}
