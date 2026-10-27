#include "room_cache.h"
#include "src/misc/cache/instance/lv_image_cache.h"
#include <string.h>
#ifdef ESP_PLATFORM
#include "esp_attr.h"
#define ROOM_RAM EXT_RAM_BSS_ATTR
#else
#define ROOM_RAM
#endif
ROOM_RAM _Alignas(4) static uint8_t pixels[2][466*466*2];
static const lv_image_dsc_t *keys[2];
static unsigned next_slot;
static lv_image_dsc_t images[2]={
    {.header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_RGB565,.w=466,.h=466,.stride=932},.data_size=sizeof pixels[0],.data=pixels[0]},
    {.header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_RGB565,.w=466,.h=466,.stride=932},.data_size=sizeof pixels[1],.data=pixels[1]}
};
const lv_image_dsc_t *pd_room_image(const lv_image_dsc_t *source){
    if(!source||source->header.w!=233||source->header.h!=233)return source;
    if(source->header.cf!=LV_COLOR_FORMAT_RGB565||source->header.stride!=466||source->data_size!=233*233*2||!source->data)return NULL;
    for(unsigned slot=0;slot<2;slot++)if(keys[slot]==source)return &images[slot];
    unsigned slot=next_slot;next_slot=(next_slot+1)%2;lv_image_cache_drop(&images[slot]);
    for(unsigned y=0;y<233;y++){
        uint8_t *row=pixels[slot]+y*2*932;const uint8_t *in=source->data+y*466;
        for(unsigned x=0;x<233;x++){uint8_t a=in[x*2],b=in[x*2+1];row[x*4]=row[x*4+2]=a;row[x*4+1]=row[x*4+3]=b;}
        memcpy(row+932,row,932);
    }
    keys[slot]=source;return &images[slot];
}
