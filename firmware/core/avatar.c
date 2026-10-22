#include "avatar.h"
#include "../assets/avatar_atlas.h"
#include <string.h>

unsigned pd_avatar_pose(uint32_t elapsed) {
    unsigned phase=elapsed%3200;
    return phase>=800&&phase<1000?1:0;
}
unsigned pd_avatar_reaction(int hp,int temp,uint32_t age) {
    int delta=hp?hp:temp;
    if(delta>0&&age<1000)return 2;
    if(delta<0&&age<600)return 3;
    return 0;
}
bool pd_avatar_image(const pd_avatar *a,unsigned pose,uint8_t out[PD_AVATAR_IMAGE_BYTES]) {
    if(!a||!out||pose>3||a->skin>2||a->hair>2||a->cloak>2||a->style>2)return false;
    const unsigned choices[]={a->skin,a->hair,a->cloak};
    for(unsigned i=0;i<256;i++) {
        uint32_t color=i>=1&&i<=48?pd_avatar_materials[(i-1)/16][choices[(i-1)/16]][(i-1)%16]:pd_avatar_fixed[i];
        out[4*i]=color&255;out[4*i+1]=(color>>8)&255;
        out[4*i+2]=color>>16;out[4*i+3]=i?255:0;
    }
    memcpy(out+1024,pd_avatar_masks[a->style][pose],PD_AVATAR_WIDTH*PD_AVATAR_HEIGHT);return true;
}
bool pd_avatar_large(const pd_avatar *a,unsigned pose,uint8_t out[PD_AVATAR_LARGE_BYTES]) {
    if(!pd_avatar_image(a,pose,out))return false;
    /* Expand backwards in place so the packed source remains intact until read. */
    for(int y=119;y>=0;y--)for(int x=95;x>=0;x--)
        out[1024+y*96+x]=out[1024+(y/2)*48+x/2];
    return true;
}
bool pd_avatar_hero(const pd_avatar *a,unsigned pose,uint8_t out[PD_AVATAR_HERO_BYTES]) {
    if(!pd_avatar_image(a,pose,out))return false;
    for(int y=89;y>=0;y--)for(int x=71;x>=0;x--)
        out[1024+y*72+x]=out[1024+(y*2/3)*48+x*2/3];
    return true;
}
