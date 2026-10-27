#include "pocketdd.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state base,s,out,expected;
static uint8_t record[PD_RECORD_SIZE],legacy[PD_RECORD_SIZE];
static void crc(uint8_t *b,size_t n){uint32_t c=pd_crc32(b,n-4);for(unsigned i=0;i<4;i++)b[n-4+i]=c>>(8*i);}
int main(void){
    pd_demo(&base);base.hp=17;base.temp_hp=7;base.wallet.available=true;base.wallet.coins[3]=123;uint32_t generation;
    for(unsigned skin=0;skin<3;skin++)for(unsigned hair=0;hair<3;hair++)for(unsigned cloak=0;cloak<3;cloak++)for(unsigned style=0;style<3;style++)for(unsigned scene=0;scene<PD_SCENES;scene++){
        s=expected=base;pd_avatar avatar={skin,hair,cloak,style};assert(pd_appearance_set(&s,&avatar,scene));expected.avatar=avatar;expected.scene=scene;
        assert(!memcmp(&s,&expected,sizeof s));size_t n=pd_encode(&s,66,record);
        assert(n==8943&&!memcmp(record,"PDDT",4)&&pd_decode(record,n,&out,&generation)&&generation==66&&!memcmp(&s,&out,sizeof s));
        /* The previous record has no scene byte; reading it does not write or infer a scene. */
        memcpy(legacy,record,n-5);memcpy(legacy,"PDDS",4);crc(legacy,n-1);expected.scene=0;
        assert(pd_decode(legacy,n-1,&out,&generation)&&!memcmp(&out,&expected,sizeof out));
        record[n-5]=PD_SCENES;crc(record,n);out=base;assert(!pd_decode(record,n,&out,&generation)&&!memcmp(&out,&base,sizeof out));
    }
    s=base;const pd_avatar invalid={3,0,0,0};assert(!pd_appearance_set(&s,&invalid,0)&&!memcmp(&s,&base,sizeof s));assert(!pd_appearance_set(&s,&base.avatar,PD_SCENES));assert(!pd_appearance_set(NULL,&base.avatar,0));assert(!pd_appearance_set(&s,NULL,0));
    s=base;s.shortcuts[0]=14;s.scene=2;size_t n=pd_encode(&s,1,record);assert(pd_decode(record,n,&out,&generation)&&out.shortcuts[0]==14&&out.scene==2);
    memcpy(legacy,record,n-5);memcpy(legacy,"PDDS",4);crc(legacy,n-1);assert(!pd_decode(legacy,n-1,&out,&generation));
    assert(!strcmp(pd_scene_name(0),"Accampamento")&&strcmp(pd_scene_name(1),pd_scene_name(2)));
    puts("Appearance: all 243 independent avatar/scene combinations, cosmetic-only changes, PDD29 persistence, PDD28 migration/defaults, invalid scene/shortcut rejection and old destination preservation passed");
}
