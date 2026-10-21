#include "avatar.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    struct { uint8_t before[8],image[PD_AVATAR_IMAGE_BYTES],after[8]; } buffer;
    struct { uint8_t before[8],image[PD_AVATAR_LARGE_BYTES],after[8]; } large;
    struct { uint8_t before[8],image[PD_AVATAR_HERO_BYTES],after[8]; } hero;
    memset(&hero,0xa5,sizeof hero);
    memset(&large,0xa5,sizeof large);
    memset(&buffer,0xa5,sizeof buffer);uint32_t hashes[324];unsigned n=0;
    for(unsigned skin=0;skin<3;skin++)for(unsigned hair=0;hair<3;hair++)
    for(unsigned cloak=0;cloak<3;cloak++)for(unsigned style=0;style<3;style++) {
        pd_avatar a={skin,hair,cloak,style};
        for(unsigned pose=0;pose<4;pose++) {
            assert(pd_avatar_image(&a,pose,buffer.image));
            assert(pd_avatar_large(&a,pose,large.image));
            assert(pd_avatar_hero(&a,pose,hero.image));
            assert(!memcmp(hero.image,buffer.image,64));
            for(unsigned i=0;i<8;i++)assert(hero.before[i]==0xa5&&hero.after[i]==0xa5);
            for(unsigned y=0;y<90;y++)for(unsigned x=0;x<72;x++) {
                unsigned sx=x*2/3;uint8_t src=buffer.image[64+(y*2/3)*24+sx/2];
                unsigned expected=(sx&1)?src&15:src>>4;
                uint8_t dst=hero.image[64+y*36+x/2];
                assert(((x&1)?dst&15:dst>>4)==expected);
            }
            for(unsigned i=0;i<8;i++)assert(buffer.before[i]==0xa5&&buffer.after[i]==0xa5);
            for(unsigned i=0;i<8;i++)assert(large.before[i]==0xa5&&large.after[i]==0xa5);
            assert(!memcmp(buffer.image,large.image,64));
            for(unsigned y=0;y<PD_AVATAR_HEIGHT*2;y++)for(unsigned x=0;x<PD_AVATAR_WIDTH*2;x++) {
                unsigned sx=x/2;uint8_t src=buffer.image[64+(y/2)*(PD_AVATAR_WIDTH/2)+sx/2];
                unsigned expected=(sx&1)?src&15:src>>4;
                uint8_t dst=large.image[64+y*PD_AVATAR_WIDTH+x/2];
                assert(((x&1)?dst&15:dst>>4)==expected);
            }
            assert(buffer.image[3]==0);for(unsigned i=1;i<16;i++)assert(buffer.image[4*i+3]==255);
            unsigned visible=0;
            for(unsigned i=64;i<PD_AVATAR_IMAGE_BYTES;i++) {
                visible+=(buffer.image[i]>>4)!=0;visible+=(buffer.image[i]&15)!=0;
            }
            assert(visible>800&&visible<PD_AVATAR_WIDTH*PD_AVATAR_HEIGHT);
            uint32_t hash=pd_crc32(buffer.image,sizeof buffer.image);
            for(unsigned i=0;i<n;i++)assert(hashes[i]!=hash);
            hashes[n++]=hash;
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
    assert(pd_avatar_pose(0)==0&&pd_avatar_pose(800)==1&&pd_avatar_pose(1600)==2&&pd_avatar_pose(2400)==3&&pd_avatar_pose(2600)==0);
    assert(pd_avatar_pose(3200)==0&&pd_avatar_pose(UINT32_MAX)<4);
    puts("Avatar: 81 appearances x 4 unique poses, exact 2x/3x/4x scaling, transparency, guards, invalid input and timing passed");
}
