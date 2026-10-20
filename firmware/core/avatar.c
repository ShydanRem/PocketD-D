#include "avatar.h"
#include "../assets/avatar_atlas.h"
#include <string.h>

unsigned pd_avatar_pose(uint32_t elapsed) {
    static const uint8_t poses[]={0,0,0,0,1,1,0,0,2,2,0,0,3,0,0,0};
    return poses[(elapsed/200)%16];
}
static uint32_t shade(uint32_t color) {
    return ((((color>>16)&255)*3/4)<<16)|((((color>>8)&255)*3/4)<<8)|((color&255)*3/4);
}
bool pd_avatar_image(const pd_avatar *a,unsigned pose,uint8_t out[PD_AVATAR_IMAGE_BYTES]) {
    if(!a||!out||pose>3||a->skin>2||a->hair>2||a->cloak>2||a->style>2)return false;
    uint32_t skin=pd_avatar_skin[a->skin],hair=pd_avatar_hair[a->hair];
    const uint32_t *cloak=pd_avatar_cloak[a->cloak];
    const uint32_t colors[]={0,0x081711,shade(skin),skin,hair,shade(hair),
        cloak[2],cloak[0],cloak[1],0xb8c2a3,0x647967,0xd0ae68,0xf1d590,0x73543b,0x352e28,0x172621};
    for(unsigned i=0;i<16;i++) {
        out[4*i]=colors[i]&255;out[4*i+1]=(colors[i]>>8)&255;
        out[4*i+2]=colors[i]>>16;out[4*i+3]=i?255:0;
    }
    memcpy(out+64,pd_avatar_masks[a->style][pose],PD_AVATAR_IMAGE_BYTES-64);return true;
}
bool pd_avatar_large(const pd_avatar *a,unsigned pose,uint8_t out[PD_AVATAR_LARGE_BYTES]) {
    if(!pd_avatar_image(a,pose,out))return false;
    /* Expand backwards in place so the packed source remains intact until read. */
    for(int y=PD_AVATAR_HEIGHT*2-1;y>=0;y--)for(int x=PD_AVATAR_WIDTH-1;x>=0;x--) {
        uint8_t source=out[64+(y/2)*(PD_AVATAR_WIDTH/2)+x/2];
        unsigned pixel=(x&1)?source&15:source>>4;
        out[64+y*PD_AVATAR_WIDTH+x]=pixel*17;
    }
    return true;
}
