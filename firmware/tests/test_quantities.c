#include "pocketdd.h"
#include "backup.h"
#include "combat.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pd_state s,before,expected,decoded;
static pd_backup restored;
static uint8_t record[PD_RECORD_SIZE];
int main(void){
    const unsigned initial[]={0,1,7,999},targets[]={0,1,8,998,999,1000,UINT_MAX};unsigned cases=0;
    for(unsigned index=0;index<PD_ITEMS;index++)for(unsigned kind=0;kind<4;kind++)for(unsigned from=0;from<4;from++)for(unsigned to=0;to<7;to++)for(unsigned equipped=0;equipped<3;equipped++)for(unsigned charged=0;charged<2;charged++){
        pd_demo(&s);s.item_count=PD_ITEMS;s.equipped=PD_NO_WEAPON;
        for(unsigned i=0;i<PD_ITEMS;i++)s.items[i]=(pd_item){.name="Synthetic item",.quantity=3};
        unsigned other=(index+1)%PD_ITEMS;s.items[other]=(pd_item){.name="Other weapon",.quantity=1,.kind=PD_WEAPON,.count=1,.sides=8,.flags=3,.reach=1,.thrown_range=20};
        s.items[index]=(pd_item){.name="Target item",.quantity=initial[from],.kind=kind};
        if(kind==PD_WEAPON||kind==PD_HEALING){s.items[index].count=1;s.items[index].sides=kind==PD_WEAPON?8:4;}
        if(kind==PD_WEAPON){s.items[index].flags=3;s.items[index].reach=1;s.items[index].thrown_range=20;}
        if(charged)s.items[index].charges=(pd_charges){1,5,2};
        if(equipped)s.equipped=equipped==1&&kind==PD_WEAPON&&initial[from]?index:other;
        s.combat.thrown=s.equipped!=PD_NO_WEAPON;
        s.magic.count=1;s.magic.powers[0]=(pd_power){.name="Concentration",.flags=PD_PREPARED|PD_CONCENTRATES};s.magic.concentration=1;
        s.effect_count=1;s.effects[0]=(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=1};assert(pd_valid(&s));
        before=expected=s;bool allowed=targets[to]<=999;
        if(allowed){expected.items[index].quantity=targets[to];if(!targets[to]&&expected.equipped==index){expected.equipped=PD_NO_WEAPON;expected.combat.thrown=false;}}
        assert(pd_item_quantity_set(&s,index,targets[to])==allowed&&!memcmp(&s,&expected,sizeof s)&&pd_valid(&s));
        if(allowed){assert(pd_item_quantity_set(&s,index,targets[to])&&!memcmp(&s,&expected,sizeof s));
            if(charged){pd_state *copy=&decoded;*copy=s;assert(pd_item_charges_change(copy,index,1)==(targets[to]!=0));assert(copy->items[index].charges.current==(targets[to]?2:1));}
            if(kind==PD_WEAPON){pd_weapon_info weapon;assert(pd_weapon_stats(&s,index,&weapon)==(targets[to]!=0));}
        }
        cases++;
    }
    /* Zero-quantity rows and the independent charge balance survive both codecs. */
    assert(pd_item_quantity_set(&s,59,0));uint32_t generation=0;size_t n=pd_encode(&s,65,record);
    assert(n&&!memcmp(record,"PDDV",4)&&pd_decode(record,n,&decoded,&generation)&&generation==65&&!memcmp(&s,&decoded,sizeof s));
    pd_history history;pd_history_reset(&history,s.history_epoch);size_t length;char *raw=pd_backup_encode(&s,&history,&length);
    assert(raw&&pd_backup_decode(raw,length,&restored)&&!memcmp(&s,&restored.state,sizeof s));free(raw);
    before=s;assert(!pd_item_quantity_set(NULL,0,1)&&!pd_item_quantity_set(&s,PD_ITEMS,1)&&!memcmp(&s,&before,sizeof s));
    s.inventory_available=false;s.item_count=0;s.equipped=PD_NO_WEAPON;s.combat.thrown=false;before=s;assert(!pd_item_quantity_set(&s,0,1)&&!memcmp(&s,&before,sizeof s));
    pd_demo(&s);s.hp=1000;before=s;assert(!pd_item_quantity_set(&s,0,1)&&!memcmp(&s,&before,sizeof s));
    printf("Quantities: %u independent kind/index/balance/equipment/charges cases, exact full-state comparison, no refill/effects, idempotence, zero-weapon guards, native codecs and atomic rejection passed\n",cases);
}
