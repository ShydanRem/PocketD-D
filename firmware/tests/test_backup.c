#include "backup.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char frame[PD_BACKUP_MAX+100];static size_t frame_size;static int fail_write,calls;
static bool writer(const char *p,size_t n,void *unused){(void)unused;if(++calls==fail_write)return false;assert(frame_size+n<sizeof frame);memcpy(frame+frame_size,p,n);frame_size+=n;frame[frame_size]=0;return true;}
int main(int argc,char **argv){
    static pd_state state;static pd_history history;static pd_backup restored,before;
    static uint8_t a[PD_RECORD_SIZE],b[PD_RECORD_SIZE],ha[PD_HISTORY_RECORD],hb[PD_HISTORY_RECORD];
    unsigned cases=0;const unsigned counts[]={0,1,3,60,119,120};
    for(unsigned c=0;c<6;c++)for(unsigned h=0;h<=20;h+=10)for(unsigned e=0;e<2;e++){
        pd_demo(&state);state.history_epoch=e?UINT32_MAX:0;state.hp=17;state.temp_hp=7;
        state.wallet=(pd_wallet){.available=true,.coins={1,2,3,4,PD_COIN_MAX}};
        state.effects[0]=(pd_effect){.target=PD_EFFECT_ARMOR,.value=2,.active=true};state.effects[1]=(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true};state.effect_count=2;
        state.scene=(c+1)%PD_SCENES;state.shortcuts[0]=14;state.brightness=80;state.magic.count=counts[c];for(unsigned i=0;i<state.magic.count;i++)state.magic.powers[i]=(pd_power){.name="Potere",.flags=PD_PREPARED};
        pd_history_reset(&history,state.history_epoch);
        for(unsigned i=0;i<h;i++){pd_history_entry entry={.kind=PD_H_FREE,.count=1,.sides=20,.dice={1+i%20},.total=1+i%20,.label="Tiro sintetico"};assert(pd_history_append(&history,&entry));}
        size_t size=0;char *json=pd_backup_encode(&state,&history,&size);assert(json&&size<PD_BACKUP_MAX&&pd_backup_decode(json,size,&restored));
        size_t n=pd_encode(&state,0,a);assert(pd_encode(&restored.state,0,b)==n&&!memcmp(a,b,n));
        n=pd_history_encode(&history,0,ha);assert(pd_history_encode(&restored.history,0,hb)==n&&!memcmp(ha,hb,n));
        assert(pd_max_hp(&restored.state)==42&&pd_armor(&restored.state)==18&&restored.state.wallet.coins[4]==PD_COIN_MAX&&restored.state.shortcuts[0]==14&&restored.state.scene==state.scene);
        before=restored;
        for(unsigned i=0;i<64;i++){size_t cut=i<32?i:size-(i-31);assert(!pd_backup_decode(json,cut,&restored)&&!memcmp(&before,&restored,sizeof before));}
        char *text=strstr(json,"\"state\":\"")+9;char original=*text;*text='!';assert(!pd_backup_decode(json,size,&restored));*text=original;
        text=strstr(json,"\"version\":1")+10;*text='2';assert(!pd_backup_decode(json,size,&restored));*text='1';
        if(argc>=2&&c==3&&h==20&&e==0){FILE *f=fopen(argv[1],"wb");assert(f);assert(fwrite(json,1,size,f)==size);fclose(f);}
        if(argc==3&&c==0&&h==20&&e==0){FILE *f=fopen(argv[2],"wb");assert(f);assert(fwrite(json,1,size,f)==size);fclose(f);}
        for(int failure=0;failure<=3;failure++){
            calls=0;frame_size=0;fail_write=failure;bool ok=pd_backup_transmit(&state,&history,writer,NULL);assert(ok==(failure==0));
            if(ok){char header[80];int length=snprintf(header,sizeof header,"PDD BACKUP_DATA %u %08x\n",(unsigned)size,(unsigned)pd_crc32((uint8_t *)json,size));assert(!memcmp(frame,header,length)&&!memcmp(frame+length,json,size)&&!strcmp(frame+length+size,"\nPDD BACKUP_DONE\n"));}
        }
        free(json);cases++;
    }
    history.epoch--;size_t untouched=91;assert(!pd_backup_encode(&state,&history,&untouched)&&untouched==91);
    const char *bad[]={"{}","[]","null","{\"format\":\"pocketdd-device-backup\",\"version\":1,\"state\":\"\",\"state\":\"\"}","{\"format\":\"pocketdd-device-backup\",\"version\":1.0,\"state\":\"\",\"history\":\"\"}"};
    for(unsigned i=0;i<sizeof bad/sizeof *bad;i++)assert(!pd_backup_decode(bad[i],strlen(bad[i]),&restored));
    printf("Backup: %u full native roundtrips (0..120 powers, 0..20 rolls, effects, wallet, settings), atomic malformed rejection and exact CRC/framing/write failures passed\n",cases);
}
