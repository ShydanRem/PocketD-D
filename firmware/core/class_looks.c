#include "class_looks.h"
#include "pocketdd.h"
#include <stdio.h>
#include <string.h>
static char source[192];
static uint8_t frame[PD_LOOK_BYTES];
static unsigned cached=UINT32_MAX,failed=UINT32_MAX,reads;
static const char *status="SD non disponibile";
static uint32_t le32(const uint8_t *b){return (uint32_t)b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;}
void pd_looks_set_file(const char *path){
    source[0]=0;cached=UINT32_MAX;failed=UINT32_MAX;
    if(path&&strlen(path)<sizeof source){strcpy(source,path);status="SD da leggere";}else status="SD non disponibile";
}
const char *pd_looks_status(void){return status;}
unsigned pd_looks_reads(void){return reads;}
static bool load(unsigned index){
    if(cached==index)return true;
    if(failed==index)return false;
    failed=index;
    cached=UINT32_MAX;
    if(!source[0])return false;
    reads++;
    FILE *f=fopen(source,"rb");if(!f){status="Pacchetto assente";return false;}
    bool ok=false;uint8_t h[32],table[PD_SPECIES_COUNT*2*12*4];
    const unsigned count=PD_SPECIES_COUNT*2*12,total=32+sizeof table+count*PD_LOOK_BYTES;
    if(fread(h,1,32,f)!=32||memcmp(h,"PDCL",4)||h[4]!=1||h[5]||h[6]!=64||h[7]!=64||h[8]!=PD_SPECIES_COUNT||h[9]||h[10]!=2||h[11]!=12||le32(h+12)!=PD_LOOK_BYTES||le32(h+16)!=count||le32(h+20)!=total||le32(h+28)!=pd_crc32(h,28))goto done;
    if(fread(table,1,sizeof table,f)!=sizeof table||pd_crc32(table,sizeof table)!=le32(h+24))goto done;
    if(fseek(f,0,SEEK_END)||ftell(f)!=(long)total||fseek(f,32+sizeof table+index*PD_LOOK_BYTES,SEEK_SET))goto done;
    if(fread(frame,1,sizeof frame,f)!=sizeof frame||pd_crc32(frame,sizeof frame)!=le32(table+index*4))goto done;
    cached=index;failed=UINT32_MAX;ok=true;
done:
    fclose(f);status=ok?"Look da SD":"Pacchetto non valido";return ok;
}
bool pd_look_image(unsigned lineage,unsigned body,unsigned outfit,unsigned scale,uint8_t *out,size_t capacity){
    if(lineage>=PD_SPECIES_COUNT||body>1||!outfit||outfit>=PD_OUTFITS||scale<2||scale>4||!out)return false;
    unsigned side=32*scale,size=side*side;if(capacity<size*3||!load((lineage*2+body)*12+outfit-1))return false;
    for(unsigned y=0;y<side;y++)for(unsigned x=0;x<side;x++){
        unsigned src=(y*64/side)*64+x*64/side,dst=y*side+x;
        out[dst*2]=frame[src*2];out[dst*2+1]=frame[src*2+1];out[size*2+dst]=frame[8192+src];
    }
    return true;
}
