#include "avatar.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv) {
    if(argc==2&&!strcmp(argv[1],"--species-hashes")){
        struct {uint8_t before[8],image[PD_AVATAR_LARGE_BYTES],after[8];} target;
        memset(&target,0xa5,sizeof target);uint8_t native[PD_AVATAR_IMAGE_BYTES];
        for(unsigned lineage=0;lineage<12;lineage++)for(unsigned body=0;body<2;body++)for(unsigned skin=0;skin<3;skin++)for(unsigned hair=0;hair<3;hair++)for(unsigned cloak=0;cloak<3;cloak++)for(unsigned style=0;style<3;style++)for(unsigned pose=0;pose<4;pose++){
            pd_avatar a={skin,hair,cloak,style};assert(pd_avatar_variant_image(&a,lineage,body,pose,native));assert(pd_avatar_variant_large(&a,lineage,body,pose,target.image));
            for(unsigned i=0;i<8;i++)assert(target.before[i]==0xa5&&target.after[i]==0xa5);
            for(unsigned y=0;y<120;y++)for(unsigned x=0;x<96;x++)assert(target.image[1024+y*96+x]==native[1024+(y/2)*48+x/2]);
            printf("%u\n",pd_crc32(native,sizeof native));
        }
        pd_avatar a={0};assert(!pd_avatar_variant_image(&a,12,0,0,native));assert(!pd_avatar_variant_image(&a,0,2,0,native));return 0;
    }
    bool hashes_only=argc==2&&!strcmp(argv[1],"--hashes");
    struct { uint8_t before[8],image[PD_AVATAR_IMAGE_BYTES],after[8]; } buffer;
    struct { uint8_t before[8],image[PD_AVATAR_LARGE_BYTES],after[8]; } large;
    struct { uint8_t before[8],image[PD_AVATAR_HERO_BYTES],after[8]; } hero;
    memset(&hero,0xa5,sizeof hero);memset(&large,0xa5,sizeof large);
    memset(&buffer,0xa5,sizeof buffer);uint32_t hashes[324];unsigned n=0;
    for(unsigned skin=0;skin<3;skin++)for(unsigned hair=0;hair<3;hair++)
    for(unsigned cloak=0;cloak<3;cloak++)for(unsigned style=0;style<3;style++) {
        pd_avatar a={skin,hair,cloak,style};
        for(unsigned pose=0;pose<4;pose++) {
            assert(pd_avatar_image(&a,pose,buffer.image));
            assert(pd_avatar_large(&a,pose,large.image));
            assert(pd_avatar_hero(&a,pose,hero.image));
            assert(!memcmp(hero.image,buffer.image,1024));
            assert(!memcmp(large.image,buffer.image,1024));
            for(unsigned i=0;i<8;i++) {
                assert(hero.before[i]==0xa5&&hero.after[i]==0xa5);
                assert(buffer.before[i]==0xa5&&buffer.after[i]==0xa5);
                assert(large.before[i]==0xa5&&large.after[i]==0xa5);
            }
            for(unsigned y=0;y<90;y++)for(unsigned x=0;x<72;x++)
                assert(hero.image[1024+y*72+x]==buffer.image[1024+(y*2/3)*48+x*2/3]);
            for(unsigned y=0;y<120;y++)for(unsigned x=0;x<96;x++)
                assert(large.image[1024+y*96+x]==buffer.image[1024+(y/2)*48+x/2]);
            assert(buffer.image[3]==0);for(unsigned i=1;i<256;i++)assert(buffer.image[4*i+3]==255);
            unsigned visible=0;for(unsigned i=1024;i<PD_AVATAR_IMAGE_BYTES;i++)visible+=buffer.image[i]!=0;
            assert(visible>800&&visible<PD_AVATAR_WIDTH*PD_AVATAR_HEIGHT);
            /* Every frame has clear outer rows/columns: no sliced hair or boots. */
            for(unsigned x=0;x<48;x++)assert(!buffer.image[1024+x]&&!buffer.image[1024+59*48+x]);
            for(unsigned y=0;y<60;y++)assert(!buffer.image[1024+y*48]&&!buffer.image[1024+y*48+47]);
            uint32_t hash=pd_crc32(buffer.image,sizeof buffer.image);
            for(unsigned i=0;i<n;i++)assert(hashes[i]!=hash);
            hashes[n++]=hash;if(hashes_only)printf("%u\n",hash);
        }
    }
    uint8_t previous[PD_AVATAR_IMAGE_BYTES];memcpy(previous,buffer.image,sizeof previous);
    pd_avatar a={0};assert(!pd_avatar_image(&a,4,buffer.image));
    a.style=3;assert(!pd_avatar_image(&a,0,buffer.image));
    assert(!pd_avatar_image(NULL,0,buffer.image));assert(!pd_avatar_image(&a,0,NULL));
    assert(!memcmp(previous,buffer.image,sizeof previous));
    uint8_t hero_previous[PD_AVATAR_HERO_BYTES];memcpy(hero_previous,hero.image,sizeof hero_previous);
    assert(!pd_avatar_hero(&a,0,hero.image)&&!pd_avatar_hero(NULL,0,hero.image)&&!pd_avatar_hero(&a,0,NULL));
    a.style=0;assert(!pd_avatar_hero(&a,4,hero.image)&&!memcmp(hero_previous,hero.image,sizeof hero_previous));
    assert(pd_avatar_pose(0)==0&&pd_avatar_pose(799)==0&&pd_avatar_pose(800)==1&&pd_avatar_pose(999)==1);
    assert(pd_avatar_pose(1000)==0&&pd_avatar_pose(2400)==0&&pd_avatar_pose(3200)==0&&pd_avatar_pose(UINT32_MAX)<2);
    assert(pd_avatar_reaction(1,0,999)==2&&pd_avatar_reaction(1,0,1000)==0);
    assert(pd_avatar_reaction(-1,0,599)==3&&pd_avatar_reaction(-1,0,600)==0);
    assert(pd_avatar_reaction(0,-1,0)==3&&pd_avatar_reaction(0,1,0)==2);
    assert(pd_avatar_reaction(0,0,0)==0&&pd_avatar_reaction(-1,2,0)==3);
    assert(pd_avatar_reaction(1,0,UINT32_MAX)==0);
    if(!hashes_only)puts("Avatar: 81 appearances x 4 poses, I8 palettes, exact scaling, clear edges, guards, invalid input and cosmetic timing passed");
}
