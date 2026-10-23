#include "power.h"
bool pd_power_sample(pd_power_info *out,uint32_t now,pd_power_reader read,void *context){
    if(!out||!read)return false;
    pd_power_info p={.sampled_at=now};uint8_t id,status[2],gauge,adc,soc,voltage[2];
    if(!read(0x03,&id,1,context)||id!=0x4a||!read(0x00,status,2,context)){*out=p;return false;}
    p.available=true;p.battery=(status[0]&8)!=0;p.vbus=(status[0]&0x20)!=0;
    p.charging=p.battery&&((status[1]>>5)&3)==1;
    if(p.battery){
        if(read(0x18,&gauge,1,context)&&(gauge&8)&&read(0xa4,&soc,1,context)&&soc<=100){p.percent=soc;p.percent_valid=true;}
        if(read(0x30,&adc,1,context)&&(adc&1)&&read(0x34,voltage,2,context)){
            unsigned mv=((voltage[0]&0x3f)<<8)|voltage[1];
            if(mv>=2000&&mv<=5000){p.millivolts=mv;p.voltage_valid=true;}
        }
    }
    *out=p;return true;
}
bool pd_power_fresh(const pd_power_info *p,uint32_t now){return p&&p->available&&(uint32_t)(now-p->sampled_at)<=10000;}
bool pd_power_battery_only(const pd_power_info *p,uint32_t now){return pd_power_fresh(p,now)&&p->battery&&!p->vbus;}
unsigned pd_power_warning(const pd_power_info *p,uint32_t now){
    return pd_power_battery_only(p,now)&&p->percent_valid&&!p->charging?(p->percent<=5?2:p->percent<=20?1:0):0;
}
bool pd_power_auto_pause(const pd_power_info *p,uint32_t now,uint32_t activity,bool enabled,bool blocked){
    return enabled&&!blocked&&pd_power_battery_only(p,now)&&(uint32_t)(now-activity)>=300000;
}
bool pd_power_off(pd_power_reader read,pd_power_writer write,void *context){
    uint8_t id,status,config;
    if(!read||!write||!read(0x03,&id,1,context)||id!=0x4a||!read(0x00,&status,1,context)||(status&0x28)!=8||!read(0x10,&config,1,context))return false;
    return write(0x10,(config&0xfc)|1,context);
}
