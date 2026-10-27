#include "../ui/room_cache.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t data[3][233*233*2];
static lv_image_dsc_t source[3];
static void check(const lv_image_dsc_t *image,unsigned id){
    assert(image&&image->header.w==466&&image->header.h==466&&image->header.stride==932&&image->data_size==466*466*2);
    for(unsigned y=0;y<466;y++)for(unsigned x=0;x<466;x++){
        unsigned at=(y*466+x)*2,from=((y/2)*233+x/2)*2;
        assert(image->data[at]==data[id][from]&&image->data[at+1]==data[id][from+1]);
    }
}
int main(void){
    lv_init();assert(!pd_room_image(NULL));
    for(unsigned id=0;id<3;id++){
        for(unsigned i=0;i<sizeof data[id];i++)data[id][i]=(uint8_t)(i*17+id*71);
        source[id]=(lv_image_dsc_t){.header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_RGB565,.w=233,.h=233,.stride=466},.data_size=sizeof data[id],.data=data[id]};
    }
    const lv_image_dsc_t *a=pd_room_image(&source[0]),*b=pd_room_image(&source[1]);assert(a!=b);check(a,0);check(b,1);assert(pd_room_image(&source[0])==a);check(b,1);
    check(pd_room_image(&source[2]),2);check(b,1);check(pd_room_image(&source[0]),0);check(pd_room_image(&source[1]),1);
    lv_image_dsc_t invalid=source[0];invalid.data_size--;assert(!pd_room_image(&invalid));invalid=source[0];invalid.header.stride--;assert(!pd_room_image(&invalid));invalid=source[0];invalid.header.cf=LV_COLOR_FORMAT_RGB565A8;assert(!pd_room_image(&invalid));invalid=source[0];invalid.data=NULL;assert(!pd_room_image(&invalid));
    invalid=(lv_image_dsc_t){.header={.w=300,.h=120}};assert(pd_room_image(&invalid)==&invalid);
    puts("Room cache: exact 2x2 RGB565 expansion, two simultaneous backgrounds, cache hits/eviction, bounds/invalid input and pass-through passed");
}
