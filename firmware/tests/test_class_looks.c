#include "class_looks.h"
#include "pocketdd.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t out[PD_LOOK_LARGE_BYTES],before[PD_LOOK_LARGE_BYTES];
int main(int argc,char **argv){
    assert(argc>=2);memset(out,0x5a,sizeof out);memcpy(before,out,sizeof out);
    pd_looks_set_file(NULL);assert(!pd_look_image(0,0,1,4,out,sizeof out)&&!memcmp(out,before,sizeof out));
    pd_looks_set_file("not-a-pocketdd-catalog.pdc");unsigned reads=pd_looks_reads();assert(!pd_look_image(0,0,1,4,out,sizeof out));assert(!pd_look_image(0,0,1,4,out,sizeof out)&&pd_looks_reads()==reads+1&&!memcmp(out,before,sizeof out));
    pd_looks_set_file(argv[1]);reads=pd_looks_reads();
    for(unsigned s=0;s<21;s++)for(unsigned b=0;b<2;b++)for(unsigned c=1;c<13;c++){
        for(unsigned scale=2;scale<=4;scale++){
            assert(pd_look_image(s,b,c,scale,out,sizeof out));
            if(argc>2)printf("%u %u %u %u %08x\n",s,b,c,scale,pd_crc32(out,32*scale*32*scale*3));
        }
    }
    assert(pd_looks_reads()==reads+504);memcpy(before,out,sizeof out);reads=pd_looks_reads();
    assert(pd_look_image(20,1,12,4,out,sizeof out)&&pd_looks_reads()==reads&&!memcmp(out,before,sizeof out));
    assert(!pd_look_image(21,0,1,4,out,sizeof out)&&!pd_look_image(0,2,1,4,out,sizeof out)&&!pd_look_image(0,0,0,4,out,sizeof out)&&!pd_look_image(0,0,13,4,out,sizeof out)&&!pd_look_image(0,0,1,5,out,sizeof out)&&!pd_look_image(0,0,1,4,out,sizeof out-1)&&!pd_look_image(0,0,1,4,NULL,sizeof out));
    assert(!memcmp(out,before,sizeof out)&&pd_looks_reads()==reads);
    /* Corruption/truncation/extra bytes reject before mutating output. */
    FILE *f=fopen(argv[1],"rb");assert(f&&!fseek(f,0,SEEK_END));long size=ftell(f);rewind(f);uint8_t *data=malloc(size+1);assert(data&&fread(data,1,size,f)==(size_t)size);fclose(f);
    for(unsigned test=0;test<5;test++){
        size_t length=size;
        if(test==0)data[28]^=1;
        if(test==1)data[32]^=1;
        if(test==2)data[32+504*4]^=1;
        if(test==3)length--;
        if(test==4)data[length++]=0;
        const char *name="class-corrupt-test.pdc";f=fopen(name,"wb");assert(f&&fwrite(data,1,length,f)==length&&!fclose(f));pd_looks_set_file(name);
        assert(!pd_look_image(0,0,1,4,out,sizeof out)&&!memcmp(out,before,sizeof out));assert(!remove(name));
        if(test==0)data[28]^=1;
        if(test==1)data[32]^=1;
        if(test==2)data[32+504*4]^=1;
    }
    free(data);pd_looks_set_file(argv[1]);assert(pd_look_image(0,0,1,4,out,sizeof out));
    if(argc==2)puts("Class looks: all 504 sprites x 3 scales, per-frame CRC, cached/no-repeat reads, absence/corruption/truncation/length/argument guards and untouched output on failure passed (host files, no SD hardware)");
}
