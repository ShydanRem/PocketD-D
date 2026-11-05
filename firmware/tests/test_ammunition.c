#include "combat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state s,before,next,expected,decoded;
static uint8_t record[PD_RECORD_SIZE];
static void setup(unsigned index,unsigned quantity,unsigned kind,unsigned gear,unsigned charges,unsigned ranged){
    pd_demo(&s);s.item_count=PD_ITEMS;s.equipped=(index+1)%PD_ITEMS;s.combat.thrown=false;
    memset(s.gear,0,sizeof s.gear);
    for(unsigned i=0;i<PD_ITEMS;i++)s.items[i]=(pd_item){.name="Other stack",.quantity=5};
    s.items[index]=(pd_item){.name="Explicit source",.kind=kind,.quantity=quantity};
    if(kind>=PD_HEALING){s.items[index].count=1;s.items[index].sides=6;}
    if(kind==PD_WEAPON){s.items[index].flags=3;s.items[index].reach=1;}
    if(gear)s.gear[index]=(pd_gear){.kind=PD_WEAR_ITEM};
    if(charges)s.items[index].charges=(pd_charges){.maximum=3,.current=2};
    s.items[s.equipped]=(pd_item){.name="Weapon",.kind=PD_WEAPON,.quantity=1,.count=1,.sides=8,.flags=3|(ranged?4:0),.reach=16};
    assert(pd_valid(&s));
}
int main(void){
    unsigned cases=0,quantities[]={0,1,17,999};
    for(unsigned index=0;index<PD_ITEMS;index++)for(unsigned q=0;q<4;q++)for(unsigned kind=0;kind<4;kind++)for(unsigned gear=0;gear<2;gear++)for(unsigned charges=0;charges<2;charges++)for(unsigned ranged=0;ranged<2;ranged++){
        if(gear&&kind!=PD_GEAR)continue;
        setup(index,quantities[q],kind,gear,charges,ranged);before=next=expected=s;
        bool eligible=kind<=PD_CONSUMABLE&&!gear&&!charges,allowed=eligible&&quantities[q]&&ranged;
        assert(pd_ammunition_eligible(&s,index)==eligible);
        if(allowed)expected.items[index].quantity--;
        assert(pd_ammunition_prepare(&s,index,&next)==allowed&&!memcmp(&s,&before,sizeof s)&&!memcmp(&next,&expected,sizeof next)&&pd_valid(&next));
        assert(!pd_ammunition_prepare(&s,index,&s)&&!memcmp(&s,&before,sizeof s));cases++;
    }
    setup(59,1,PD_GEAR,0,0,1);assert(pd_ammunition_prepare(&s,59,&next)&&!next.items[59].quantity&&next.equipped==s.equipped);
    uint32_t generation;size_t n=pd_encode(&next,7,record);assert(n&&pd_decode(record,n,&decoded,&generation)&&generation==7&&!memcmp(&decoded,&next,sizeof next));
    before=decoded;assert(!pd_ammunition_prepare(&decoded,59,&next)&&!memcmp(&decoded,&before,sizeof before));
    assert(!pd_ammunition_eligible(NULL,0)&&!pd_ammunition_eligible(&s,PD_ITEMS)&&!pd_ammunition_eligible(&s,s.equipped));
    assert(!pd_ammunition_prepare(NULL,0,&next)&&!pd_ammunition_prepare(&s,59,NULL)&&!pd_ammunition_prepare(&s,PD_ITEMS,&next));
    s.combat.thrown=true;before=s;next=s;assert(!pd_ammunition_prepare(&s,59,&next)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&next,&before,sizeof next));
    s.hp=1000;before=next=s;assert(!pd_ammunition_prepare(&s,59,&next)&&!memcmp(&next,&before,sizeof next));
    printf("Ammunition: %u index/kind/balance/gear/charge/range cases, separate source only, untouched input/atomic output, no charge or effect changes, last shot keeps weapon and zero-stack native roundtrip passed; synthetic.\n",cases);
}
