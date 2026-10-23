#include "power.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t registers[256];
static int failed_reg=-1;
static bool failed_write;
static unsigned reads,writes,written_reg,written_value;
static bool read_reg(uint8_t reg,uint8_t *out,size_t n,void *ctx){
    assert(ctx==registers);reads++;if(reg==failed_reg)return false;memcpy(out,registers+reg,n);return true;
}
static bool write_reg(uint8_t reg,uint8_t value,void *ctx){
    assert(ctx==registers);writes++;written_reg=reg;written_value=value;return !failed_write;
}
static void setup(void){
    memset(registers,0,sizeof registers);registers[3]=0x4a;registers[0]=8;registers[0x18]=8;registers[0x30]=1;
    registers[0xa4]=64;registers[0x34]=3940>>8;registers[0x35]=3940&255;failed_reg=-1;failed_write=false;reads=writes=0;
}
int main(void){
    unsigned cases=0;pd_power_info p;
    for(unsigned raw=0;raw<256;raw++)for(unsigned direction=0;direction<4;direction++)for(unsigned soc=0;soc<256;soc++){
        setup();registers[0]=raw;registers[1]=0x80|(direction<<5);registers[0xa4]=soc;
        assert(pd_power_sample(&p,123,read_reg,registers)&&p.available&&p.battery==((raw&8)!=0)&&p.vbus==((raw&32)!=0));
        assert(p.charging==((raw&8)!=0&&direction==1));assert(p.percent_valid==((raw&8)!=0&&soc<=100));
        assert(p.voltage_valid==((raw&8)!=0)&&(!p.voltage_valid||p.millivolts==3940));
        unsigned expected=(raw&8)&&!(raw&32)&&direction!=1&&soc<=100?(soc<=5?2:soc<=20?1:0):0;
        assert(pd_power_warning(&p,123)==expected&&!writes);cases++;
    }
    setup();assert(pd_power_sample(&p,UINT32_MAX-5000,read_reg,registers));
    assert(pd_power_fresh(&p,4999)&&!pd_power_fresh(&p,5000));
    p.sampled_at=300000;assert(!pd_power_auto_pause(&p,300000,1,true,false));p.sampled_at=300001;
    assert(pd_power_auto_pause(&p,300001,1,true,false));assert(!pd_power_auto_pause(&p,300001,1,false,false)&&!pd_power_auto_pause(&p,300001,1,true,true));
    p.vbus=true;assert(!pd_power_auto_pause(&p,300001,1,true,false));p.vbus=false;p.battery=false;assert(!pd_power_auto_pause(&p,300001,1,true,false));
    p.battery=true;p.sampled_at=UINT32_MAX-1000;assert(pd_power_auto_pause(&p,499,UINT32_MAX-300000,true,false));
    for(unsigned adc=0;adc<2;adc++)for(unsigned gauge=0;gauge<2;gauge++){
        setup();registers[0x30]=adc;registers[0x18]=gauge<<3;assert(pd_power_sample(&p,1,read_reg,registers));assert(p.voltage_valid==!!adc&&p.percent_valid==!!gauge&&!writes);
    }
    for(unsigned mv=0;mv<=16383;mv++){
        setup();registers[0x34]=mv>>8;registers[0x35]=mv&255;assert(pd_power_sample(&p,1,read_reg,registers));assert(p.voltage_valid==(mv>=2000&&mv<=5000)&&!writes);
    }
    for(unsigned i=0;i<7;i++){
        setup();failed_reg=(int[]){3,0,0x18,0xa4,0x30,0x34,0x10}[i];bool required=i<2;
        assert(pd_power_sample(&p,100,read_reg,registers)==!required&&p.available==!required&&!writes);
        if(required)assert(!p.battery&&!p.percent_valid&&!p.voltage_valid);
        if(i==2||i==3)assert(!p.percent_valid&&p.voltage_valid);
        if(i==4||i==5)assert(!p.voltage_valid&&p.percent_valid);
    }
    setup();registers[3]=0;assert(!pd_power_sample(&p,1,read_reg,registers)&&!p.available&&!writes);
    assert(!pd_power_sample(NULL,1,read_reg,registers)&&!pd_power_sample(&p,1,NULL,registers));
    /* Exactly one permitted register write, every reserved/configuration bit,
       no accidental RESET command, and all failure paths. */
    for(unsigned config=0;config<256;config++){
        setup();registers[0x10]=config;assert(pd_power_off(read_reg,write_reg,registers)&&writes==1&&written_reg==0x10&&written_value==((config&0xfc)|1));
    }
    for(unsigned status=0;status<256;status++){
        setup();registers[0]=status;bool allowed=(status&0x28)==8;assert(pd_power_off(read_reg,write_reg,registers)==allowed&&writes==(unsigned)allowed);
    }
    for(unsigned i=0;i<3;i++){setup();failed_reg=(int[]){3,0,0x10}[i];assert(!pd_power_off(read_reg,write_reg,registers)&&!writes);}
    setup();registers[3]=0;assert(!pd_power_off(read_reg,write_reg,registers)&&!writes);
    setup();failed_write=true;assert(!pd_power_off(read_reg,write_reg,registers)&&writes==1);
    assert(!pd_power_off(NULL,write_reg,registers)&&!pd_power_off(read_reg,NULL,registers));
    printf("Power: %u independent status/direction/SOC cases, 16384 voltage values, unknown/stale/failing reads, wrap/idle guards and exact single-register shutdown passed (synthetic)\n",cases);
}
