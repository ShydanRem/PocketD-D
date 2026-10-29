#include "catalog.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static pd_state s,before;
static void expect(unsigned catalog,const char *query,const unsigned *indices,unsigned count){
    assert(pd_catalog_count(&s,catalog,query)==count);
    for(unsigned i=0;i<count;i++)assert(pd_catalog_index(&s,catalog,query,i)==(int)indices[i]);
    assert(pd_catalog_index(&s,catalog,query,count)==-1&&pd_catalog_index(&s,catalog,query,UINT_MAX)==-1);
}
int main(void){
    pd_demo(&s);s.equipped=PD_NO_WEAPON;s.item_count=60;s.magic.available=true;s.magic.count=120;
    for(unsigned i=0;i<60;i++)s.items[i]=(pd_item){.name="Ordinary item",.quantity=1};
    for(unsigned i=0;i<120;i++)s.magic.powers[i]=(pd_power){.name="Ordinary spell",.flags=PD_PREPARED,.kind=i%2,.maximum=i%2?3:0,.remaining=i%2?3:0};
    strcpy(s.items[0].name,"Wand of Light");strcpy(s.items[31].name,"LIGHT shield +1");strcpy(s.items[59].name,"wand OF light");s.items[59].quantity=0;
    strcpy(s.magic.powers[0].name,"Light");strcpy(s.magic.powers[61].name,"Light step");strcpy(s.magic.powers[118].name,"Daylight");strcpy(s.magic.powers[119].name,"Light step");
    assert(pd_valid(&s));before=s;expect(PD_CATALOG_ITEMS,"light",(unsigned[]){0,31,59},3);expect(PD_CATALOG_ITEMS,"WAND",(unsigned[]){0,59},2);
    expect(PD_CATALOG_ITEMS,"+1",(unsigned[]){31},1);expect(PD_CATALOG_ITEMS,"wand light",NULL,0);
    expect(PD_CATALOG_POWERS," LiGhT ",(unsigned[]){0,61,118,119},4);expect(PD_CATALOG_SPELLS,"light",(unsigned[]){0,118},2);expect(PD_CATALOG_FEATURES,"LIGHT",(unsigned[]){61,119},2);
    expect(PD_CATALOG_POWERS,"Not present",NULL,0);assert(!memcmp(&s,&before,sizeof s));
    for(unsigned i=0;i<60;i++)assert(pd_catalog_index(&s,PD_CATALOG_ITEMS,"",i)==(int)i);
    for(unsigned i=0;i<120;i++)assert(pd_catalog_index(&s,PD_CATALOG_POWERS,"",i)==(int)i);
    assert(pd_catalog_count(&s,PD_CATALOG_ITEMS,"")==60&&pd_catalog_count(&s,PD_CATALOG_POWERS,"")==120);
    /* Every position, literal substring and upper/lower spelling maps to the
       same original row, including depleted objects and duplicate names. */
    unsigned cases=0;
    for(unsigned catalog=0;catalog<4;catalog++)for(unsigned row=0;row<(catalog?120:60);row++){
        const char *name=catalog?s.magic.powers[row].name:s.items[row].name;
        for(unsigned start=0;start<strlen(name);start++)for(unsigned mode=0;mode<2;mode++){
            char query[PD_SEARCH_BYTES]={0};unsigned n=0;
            while(name[start+n]&&n<20){char c=name[start+n];query[n]=mode&&c>='A'&&c<='Z'?c-'A'+'a':!mode&&c>='a'&&c<='z'?c-'a'+'A':c;n++;}
            unsigned found=pd_catalog_count(&s,catalog,query);bool present=false;
            for(unsigned i=0;i<found;i++)if(pd_catalog_index(&s,catalog,query,i)==(int)row)present=true;
            bool belongs=!catalog||catalog==PD_CATALOG_POWERS||s.magic.powers[row].kind==(catalog==PD_CATALOG_FEATURES);
            assert(present==belongs);cases++;
        }
    }
    char out[PD_SEARCH_BYTES]="UNCHANGED",saved[PD_SEARCH_BYTES];memcpy(saved,out,sizeof out);
    assert(pd_search_query(out,"  wand +1  ")&&!strcmp(out,"WAND +1"));assert(pd_search_query(out,out)&&!strcmp(out,"WAND +1"));
    assert(pd_search_query(out,"   ")&&!out[0]);
    for(unsigned length=0;length<=20;length++){char query[22]={0};memset(query,'w',length);assert(pd_search_query(out,query)&&strlen(out)==length);}
    memcpy(out,saved,sizeof out);assert(!pd_search_query(out,"123456789012345678901")&&!memcmp(out,saved,sizeof out));
    for(unsigned value=0;value<256;value++)if(value<32||value>126){char query[]={'A',(char)value,'B',0};if(!value)continue;assert(!pd_search_query(out,query)&&!memcmp(out,saved,sizeof out));}
    assert(!pd_search_query(NULL,"x")&&!pd_search_query(out,NULL));
    assert(!pd_catalog_count(NULL,0,"")&&!pd_catalog_count(&s,4,"")&&!pd_catalog_count(&s,0,NULL));
    s.inventory_available=false;expect(PD_CATALOG_ITEMS,"",NULL,0);s.inventory_available=true;s.item_count=61;expect(PD_CATALOG_ITEMS,"",NULL,0);
    s.magic.available=false;expect(PD_CATALOG_POWERS,"",NULL,0);s.magic.available=true;s.magic.count=121;expect(PD_CATALOG_POWERS,"",NULL,0);
    s=before;memset(s.items[59].name,'W',40);s.items[59].name[40]=0;expect(PD_CATALOG_ITEMS,"WWWWWWWWWWWWWWWWWWWW",(unsigned[]){59},1);
    /* Representative play inventory: no auto-consumption or index remapping. */
    pd_demo(&s);s.item_count=6;s.equipped=0;
    s.items[0]=(pd_item){.name="Sword",.kind=PD_WEAPON,.quantity=1};
    s.items[1]=(pd_item){.name="Healing potion",.kind=PD_HEALING,.quantity=2};
    s.items[2]=(pd_item){.name="Spent potion",.kind=PD_HEALING,.quantity=0};
    s.items[3]=(pd_item){.name="Wand",.quantity=1,.charges={.current=2,.maximum=3}};
    s.items[4]=(pd_item){.name="Empty wand",.quantity=1,.charges={.maximum=3}};
    s.items[5]=(pd_item){.name="Ration",.kind=PD_CONSUMABLE,.quantity=1};before=s;
    assert(pd_catalog_filtered_count(&s,PD_CATALOG_ITEMS,"",PD_CATALOG_POUCH)==3);
    const unsigned pouch_indices[]={1,3,5};for(unsigned i=0;i<3;i++)assert(pd_catalog_filtered_index(&s,PD_CATALOG_ITEMS,"",PD_CATALOG_POUCH,i)==(int)pouch_indices[i]);
    assert(pd_catalog_filtered_index(&s,PD_CATALOG_ITEMS,"wand",PD_CATALOG_POUCH,0)==3);
    assert(pd_catalog_filtered_index(&s,PD_CATALOG_ITEMS,"",PD_CATALOG_POUCH,3)==-1&&!memcmp(&s,&before,sizeof s));
    /* Readiness is a view of preparation/pools, never execution or a promise
       that a situational class rule is satisfied. Slot levels matter. */
    s.magic.available=true;s.magic.count=7;memset(s.magic.slots,0,sizeof s.magic.slots);memset(s.magic.pact,0,sizeof s.magic.pact);
    s.magic.powers[0]=(pd_power){.name="Cantrip",.flags=PD_PREPARED};
    s.magic.powers[1]=(pd_power){.name="Unprepared",.flags=PD_USES_SLOT,.level=1};
    s.magic.powers[2]=(pd_power){.name="Prepared level two",.flags=PD_PREPARED|PD_USES_SLOT,.level=2};
    s.magic.powers[3]=(pd_power){.name="Spent innate",.flags=PD_PREPARED|PD_LIMITED,.maximum=1};
    s.magic.powers[4]=(pd_power){.name="Feature",.kind=1,.remaining=1,.maximum=1};
    s.magic.powers[5]=(pd_power){.name="Passive",.kind=1,.flags=PD_PASSIVE};
    s.magic.powers[6]=(pd_power){.name="Cannon",.kind=1,.effect=3,.maximum=1};
    before=s;assert(pd_catalog_filtered_count(&s,PD_CATALOG_POWERS,"",PD_CATALOG_READY)==2);
    assert(pd_catalog_filtered_index(&s,PD_CATALOG_POWERS,"",PD_CATALOG_READY,0)==0&&pd_catalog_filtered_index(&s,PD_CATALOG_POWERS,"",PD_CATALOG_READY,1)==4&&!memcmp(&s,&before,sizeof s));
    s.magic.slots[0].current=1;assert(pd_catalog_filtered_count(&s,PD_CATALOG_POWERS,"",PD_CATALOG_READY)==3);
    s.magic.pact[1].current=1;assert(pd_catalog_filtered_count(&s,PD_CATALOG_POWERS,"",PD_CATALOG_READY)==4);
    assert(pd_catalog_filtered_index(&s,PD_CATALOG_SPELLS,"two",PD_CATALOG_READY,0)==2);
    s.combat.raging=true;assert(pd_catalog_filtered_count(&s,PD_CATALOG_POWERS,"",PD_CATALOG_READY)==2);
    assert(!pd_catalog_filtered_count(&s,PD_CATALOG_ITEMS,"",PD_CATALOG_READY)&&!pd_catalog_filtered_count(&s,PD_CATALOG_POWERS,"",PD_CATALOG_POUCH)&&pd_catalog_filtered_index(&s,PD_CATALOG_POWERS,"",99,0)==-1);
    printf("Catalog: %u case/substring/original-index checks, 60 items/120 powers, type intersections, duplicate/depleted rows, literal spaces/punctuation and bounded query normalization passed\n",cases);
}
