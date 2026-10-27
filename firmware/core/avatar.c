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

#include "../assets/species_atlas.h"
const char *pd_species_name(unsigned lineage){return lineage<PD_SPECIES_COUNT?pd_species_names[lineage]:"Specie non valida";}
const char *pd_body_name(unsigned body){return body==0?"Maschile":body==1?"Femminile":"Non valido";}
bool pd_species_hair(unsigned lineage){return lineage<PD_SPECIES_COUNT&&pd_species_has_hair[lineage];}
const char *pd_species_choice(unsigned lineage,unsigned field,unsigned value){
    static const char *const cloaks[]={"Bosco","Vino","Notte"};
    if(lineage>=PD_SPECIES_COUNT||value>2||field>3)return "Non valido";
    return field==0?pd_species_skin_names[lineage][value]:field==1?pd_species_hair_names[lineage][value]:field==2?cloaks[value]:pd_species_style_names[lineage][value];
}
static bool species_decode(const pd_species_frame *frame,uint8_t *pixels){
    const uint8_t *p=pd_species_data+frame->offset,*end=p+frame->size;size_t at=0;
    while(p<end){
        unsigned token=*p++,count=token<128?token+1:token-127;
        if(at+count>PD_AVATAR_WIDTH*PD_AVATAR_HEIGHT||(size_t)(end-p)<(token<128?count:1))return false;
        for(unsigned i=0;i<count;i++){uint8_t value=token<128?*p++:*p;if(frame->delta)pixels[at++]^=value;else pixels[at++]=value;}
        if(token>=128)p++;
    }
    return at==PD_AVATAR_WIDTH*PD_AVATAR_HEIGHT;
}
bool pd_avatar_variant_image(const pd_avatar *a,unsigned lineage,unsigned body,unsigned pose,uint8_t out[PD_AVATAR_IMAGE_BYTES]){
    if(lineage>=PD_SPECIES_COUNT||body>1)return false;
    if(!lineage&&!body)return pd_avatar_image(a,pose,out);
    if(!a||!out||pose>3||a->skin>2||a->hair>2||a->cloak>2||a->style>2)return false;
    const unsigned choices[]={a->skin,a->hair,a->cloak};
    for(unsigned i=0;i<256;i++){
        uint32_t color=i>=1&&i<=48?pd_species_materials[lineage][(i-1)/16][choices[(i-1)/16]][(i-1)%16]:pd_species_fixed[i];
        out[4*i]=color&255;out[4*i+1]=(color>>8)&255;out[4*i+2]=color>>16;out[4*i+3]=i?255:0;
    }
    const pd_species_frame *frame=&pd_species_frames[lineage][body][a->style][pose];
    if(frame->delta&&!species_decode(&pd_species_frames[lineage][body][a->style][0],out+1024))return false;
    return species_decode(frame,out+1024);
}
bool pd_avatar_variant_large(const pd_avatar *a,unsigned lineage,unsigned body,unsigned pose,uint8_t out[PD_AVATAR_LARGE_BYTES]){
    if(!pd_avatar_variant_image(a,lineage,body,pose,out))return false;
    for(int y=119;y>=0;y--)for(int x=95;x>=0;x--)out[1024+y*96+x]=out[1024+(y/2)*48+x/2];
    return true;
}
bool pd_avatar_variant_hero(const pd_avatar *a,unsigned lineage,unsigned body,unsigned pose,uint8_t out[PD_AVATAR_HERO_BYTES]){
    if(!pd_avatar_variant_image(a,lineage,body,pose,out))return false;
    for(int y=89;y>=0;y--)for(int x=71;x>=0;x--)out[1024+y*72+x]=out[1024+(y*2/3)*48+x*2/3];
    return true;
}
