#include "scenes.h"
#include "../room_cache.h"
#include "scene_assets.h"
#include "../companion/companion_assets.h"
#include "src/misc/cache/instance/lv_image_cache.h"
#include <string.h>
#ifdef ESP_PLATFORM
#include "esp_attr.h"
#define SCENE_RAM EXT_RAM_BSS_ATTR
#else
#define SCENE_RAM
#endif
SCENE_RAM _Alignas(4) static uint8_t full[1024+466*466],preview[1024+300*120];
static unsigned cached;
static lv_image_dsc_t image={.header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_I8,.w=466,.h=466,.stride=466},.data_size=sizeof full,.data=full};
static lv_image_dsc_t thumbnail={.header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_I8,.w=300,.h=120,.stride=300},.data_size=sizeof preview,.data=preview};
static void prepare(unsigned scene){
    if(!scene||scene>2||cached==scene)return;
    lv_image_cache_drop(&image);lv_image_cache_drop(&thumbnail);
    const uint8_t *source=pd_scene_sources[scene-1];memcpy(full,source,1024);memcpy(preview,source,1024);source+=1024;
    for(unsigned y=0;y<466;y++)for(unsigned x=0;x<466;x++)full[1024+y*466+x]=source[(y/2)*233+x/2];
    for(unsigned y=0;y<120;y++)for(unsigned x=0;x<300;x++)preview[1024+y*300+x]=source[(95+y/2)*233+41+x/2];
    cached=scene;
}
const lv_image_dsc_t *pd_scene_image(unsigned scene){if(!scene||scene>2)return pd_room_image(&companion_camp);prepare(scene);return &image;}
const lv_image_dsc_t *pd_scene_preview(unsigned scene){if(!scene||scene>2)return NULL;prepare(scene);return &thumbnail;}
