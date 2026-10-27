#include "pocketdd.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state base,s,out,expected;
static uint8_t record[PD_RECORD_SIZE],legacy[PD_RECORD_SIZE];
static void crc(uint8_t *b,size_t n){uint32_t c=pd_crc32(b,n-4);for(unsigned i=0;i<4;i++)b[n-4+i]=c>>(8*i);}
int main(void){
    pd_demo(&base);base.hp=17;base.temp_hp=7;base.wallet.available=true;base.wallet.coins[3]=123;uint32_t generation;
    for(unsigned lineage=0;lineage<PD_SPECIES_COUNT;lineage++)for(unsigned body=0;body<2;body++)for(unsigned scene=0;scene<PD_SCENES;scene++){
        s=expected=base;pd_avatar avatar={lineage%3,body,lineage%3,(lineage+body)%3};assert(pd_appearance_set(&s,&avatar,scene));assert(pd_identity_set(&s,lineage,body));expected.avatar=avatar;expected.scene=scene;expected.lineage=lineage;expected.body=body;
        assert(!memcmp(&s,&expected,sizeof s));size_t n=pd_encode(&s,67,record);
        assert(n==8946&&!memcmp(record,"PDDX",4)&&pd_decode(record,n,&out,&generation)&&generation==67&&!memcmp(&s,&out,sizeof s));
        for(unsigned v=0;v<3;v++){
            size_t old=n-1;memcpy(legacy,record,old-4);memcpy(legacy,(const char *const[]){"PDDU","PDDV","PDDW"}[v],4);crc(legacy,old);
            if(lineage<(const unsigned[]){12,16,21}[v])assert(pd_decode(legacy,old,&out,&generation)&&!memcmp(&out,&expected,sizeof out));
            else assert(!pd_decode(legacy,old,&out,&generation));
        }
        /* PDD29 retains its scene and defaults only the two new appearance bytes. */
        memcpy(legacy,record,n-7);memcpy(legacy,"PDDT",4);crc(legacy,n-3);expected.lineage=expected.body=0;
        assert(pd_decode(legacy,n-3,&out,&generation)&&!memcmp(&out,&expected,sizeof out));
        /* PDD28 also defaults its missing scene. */
        memcpy(legacy,record,n-8);memcpy(legacy,"PDDS",4);crc(legacy,n-4);expected.scene=0;
        assert(pd_decode(legacy,n-4,&out,&generation)&&!memcmp(&out,&expected,sizeof out));
        for(unsigned field=0;field<4;field++){pd_encode(&s,67,record);record[n-8+field]=(const unsigned[]){3,PD_SPECIES_COUNT,2,PD_OUTFITS}[field];crc(record,n);out=base;assert(!pd_decode(record,n,&out,&generation)&&!memcmp(&out,&base,sizeof out));}
    }
    s=base;const pd_avatar invalid={3,0,0,0};assert(!pd_appearance_set(&s,&invalid,0)&&!memcmp(&s,&base,sizeof s));assert(!pd_appearance_set(&s,&base.avatar,PD_SCENES));assert(!pd_appearance_set(NULL,&base.avatar,0));assert(!pd_appearance_set(&s,NULL,0));
    assert(!pd_identity_set(&s,PD_SPECIES_COUNT,0)&&!pd_identity_set(&s,0,2)&&!pd_identity_set(NULL,0,0)&&!memcmp(&s,&base,sizeof s));
    s=base;s.shortcuts[0]=14;s.scene=2;size_t n=pd_encode(&s,1,record);assert(pd_decode(record,n,&out,&generation)&&out.shortcuts[0]==14&&out.scene==2);
    memcpy(legacy,record,n-8);memcpy(legacy,"PDDS",4);crc(legacy,n-4);assert(!pd_decode(legacy,n-4,&out,&generation));
    assert(!strcmp(pd_scene_name(0),"Accampamento")&&strcmp(pd_scene_name(1),pd_scene_name(2)));
    for(unsigned outfit=0;outfit<PD_OUTFITS;outfit++){s=base;assert(pd_outfit_set(&s,outfit));expected=base;expected.outfit=outfit;assert(!memcmp(&s,&expected,sizeof s));n=pd_encode(&s,70,record);assert(pd_decode(record,n,&out,&generation)&&!memcmp(&s,&out,sizeof s));}
    assert(!pd_outfit_set(&s,PD_OUTFITS)&&!pd_outfit_set(NULL,0));
    puts("Appearance: 21 species x 2 bodies x 3 scenes, cosmetic-only changes, PDD33 class persistence and PDD32 migration, PDD30/PDD29/PDD28 migrations, invalid fields and old destination preservation passed");
}
