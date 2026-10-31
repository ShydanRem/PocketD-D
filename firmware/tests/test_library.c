#define _POSIX_C_SOURCE 200809L
#include "library.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
static bool short_write,fail_sync;
size_t __real_fwrite(const void *,size_t,size_t,FILE *);
size_t __wrap_fwrite(const void *p,size_t size,size_t count,FILE *f){if(short_write){short_write=false;return count?__real_fwrite(p,size,count/2,f):0;}return __real_fwrite(p,size,count,f);}
int __real_fsync(int);
int __wrap_fsync(int fd){if(fail_sync){fail_sync=false;errno=EIO;return -1;}return __real_fsync(fd);}
static void same(const pd_backup *actual,const pd_state *s,const pd_history *h){size_t n,m;char *a=pd_backup_encode(&actual->state,&actual->history,&n),*b=pd_backup_encode(s,h,&m);assert(a&&b&&n==m&&!memcmp(a,b,n));free(a);free(b);}
static void reset(pd_state *s,pd_history *h,uint32_t epoch){pd_demo(s);s->history_epoch=epoch;pd_history_reset(h,epoch);}
static void corrupt(const char *root,unsigned slot,char bank){char name[256];snprintf(name,sizeof name,"%s/pg%02u%c.pdl",root,slot,bank);FILE *f=fopen(name,"r+b");assert(f);assert(!fseek(f,20,SEEK_SET));int old=fgetc(f);assert(old!=EOF);assert(!fseek(f,20,SEEK_SET));assert(fputc(old^1,f)!=EOF);assert(!fclose(f));}
static void future(const char *root,unsigned slot,char bank){char name[256];snprintf(name,sizeof name,"%s/pg%02u%c.pdl",root,slot,bank);FILE *f=fopen(name,"r+b");assert(f);assert(!fseek(f,0,SEEK_END));long n=ftell(f);assert(n>16);rewind(f);unsigned char *data=malloc(n);assert(data&&fread(data,1,n,f)==(size_t)n);data[3]='2';uint32_t crc=pd_crc32(data,n-4);for(unsigned i=0;i<4;i++)data[n-4+i]=crc>>(8*i);rewind(f);assert(fwrite(data,1,n,f)==(size_t)n);assert(!fclose(f));free(data);}
static void clear(const char *root){for(unsigned id=1;id<=PD_LIBRARY_LIMIT;id++)for(unsigned b=0;b<2;b++){char name[256];snprintf(name,sizeof name,"%s/pg%02u%c.pdl",root,id,b?'b':'a');if(unlink(name))assert(errno==ENOENT);}}
int main(void){
    char tmp[]="/tmp/pdd-library-XXXXXX";assert(mkdtemp(tmp));char root[256];snprintf(root,sizeof root,"%s/characters",tmp);pd_library l;assert(pd_library_init(&l,root,42,NULL,NULL));
    pd_state *s=malloc(sizeof *s);pd_history *h=malloc(sizeof *h);pd_backup *out=malloc(sizeof *out),*before=malloc(sizeof *before);assert(s&&h&&out&&before);
    pd_library_catalog list;assert(pd_library_scan(&l,&list)&&!list.count);reset(s,h,10);unsigned slot=99;
    assert(pd_library_save(&l,0,s,h,&slot)==PD_LIBRARY_OK&&slot==1);assert(pd_library_read(&l,1,out)==PD_LIBRARY_OK);same(out,s,h);
    s->hp--;s->avatar.skin=2;s->scene=2;s->brightness=70;s->wallet.available=true;s->wallet.coins[3]=40;
    pd_history_entry e={.kind=PD_H_ATTACK,.count=1,.sides=20,.dice={17},.bonus=3,.total=20};strcpy(e.label,"Tiro sintetico");assert(pd_history_append(h,&e));
    assert(pd_library_save(&l,0,s,h,&slot)==PD_LIBRARY_OK&&slot==1);assert(pd_library_read(&l,1,out)==PD_LIBRARY_OK);same(out,s,h);
    /* Same name is a different PG when the local identity differs. */
    reset(s,h,20);assert(pd_library_save(&l,0,s,h,&slot)==PD_LIBRARY_OK&&slot==2);assert(pd_library_scan(&l,&list)&&list.count==2&&list.entries[0].epoch==10&&list.entries[1].epoch==20);
    /* A short/torn inactive-bank write retains the previous snapshot. */
    short_write=true;s->hp=2;assert(pd_library_save(&l,0,s,h,NULL)==PD_LIBRARY_IO);assert(pd_library_read(&l,2,out)==PD_LIBRARY_RECOVERED&&out->state.hp!=2);
    assert(pd_library_save(&l,0,s,h,NULL)==PD_LIBRARY_OK);assert(pd_library_read(&l,2,out)==PD_LIBRARY_OK);same(out,s,h);
    /* fsync failure may have a complete new bank: report failure, but leave
       a reboot able to read a complete OLD or NEW snapshot, never a mixture. */
    s->hp=3;fail_sync=true;assert(pd_library_save(&l,0,s,h,NULL)==PD_LIBRARY_IO);assert(pd_library_read(&l,2,out)==PD_LIBRARY_OK);assert(out->state.hp==2||out->state.hp==3);assert(out->state.history_epoch==out->history.epoch);
    /* Latest bank corruption recovers the complete previous bank. */
    corrupt(root,2,'a');assert(pd_library_read(&l,2,out)==PD_LIBRARY_RECOVERED&&out->state.hp==2);
    *before=*out;future(root,2,'b');assert(pd_library_read(&l,2,out)==PD_LIBRARY_FUTURE&&!memcmp(before,out,sizeof *out));assert(pd_library_save(&l,2,s,h,NULL)==PD_LIBRARY_FUTURE);
    /* A corrupt/unknown slot remains occupied; automatic allocation uses a
       different free slot rather than erasing the unreadable file. */
    reset(s,h,30);assert(pd_library_save(&l,0,s,h,&slot)==PD_LIBRARY_OK&&slot==3);assert(pd_library_scan(&l,&list)&&list.count==3&&list.entries[1].status==PD_LIBRARY_FUTURE);
    /* Epoch follows a selected PG across restored NVS identities, no pointer. */
    reset(s,h,40);strcpy(s->name,"Altro eroe");assert(pd_library_save(&l,1,s,h,NULL)==PD_LIBRARY_OK);assert(pd_library_save(&l,0,s,h,&slot)==PD_LIBRARY_OK&&slot==1);
    reset(s,h,50);assert(pd_library_save(&l,4,s,h,NULL)==PD_LIBRARY_OK);assert(pd_library_save(&l,5,s,h,NULL)==PD_LIBRARY_OK);assert(pd_library_save(&l,0,s,h,NULL)==PD_LIBRARY_CONFLICT);
    clear(root);
    /* Moving an SD or resetting NVS never merges identical local epoch/name. */
    reset(s,h,1);assert(pd_library_save(&l,0,s,h,&slot)==PD_LIBRARY_OK&&slot==1);
    pd_library another;assert(pd_library_init(&another,root,99,NULL,NULL));s->hp=1;
    assert(pd_library_save(&another,0,s,h,&slot)==PD_LIBRARY_OK&&slot==2);
    assert(pd_library_read(&l,1,out)==PD_LIBRARY_OK&&out->state.hp!=1);
    assert(pd_library_read(&another,2,out)==PD_LIBRARY_OK&&out->state.hp==1);
    clear(root);
    /* Capacity is explicit and refuses the 33rd PG without touching any copy. */
    for(unsigned i=1;i<=PD_LIBRARY_LIMIT;i++){reset(s,h,i);assert(pd_library_save(&l,i,s,h,NULL)==PD_LIBRARY_OK);}
    reset(s,h,100);assert(pd_library_save(&l,0,s,h,NULL)==PD_LIBRARY_FULL);assert(pd_library_scan(&l,&list)&&list.count==PD_LIBRARY_LIMIT);
    for(unsigned i=1;i<=PD_LIBRARY_LIMIT;i++){assert(pd_library_read(&l,i,out)==PD_LIBRARY_OK);assert(out->state.history_epoch==i&&out->history.epoch==i);}
    reset(s,h,1);s->magic.available=true;s->magic.count=PD_POWERS;s->magic.slots[0]=(pd_slot){1,4};s->magic.pact[2]=(pd_slot){1,2};
    for(unsigned i=0;i<PD_POWERS;i++){s->magic.powers[i]=(pd_power){.name="Potere sintetico",.flags=PD_PREPARED|PD_CONCENTRATES};memset(s->magic.powers[i].description,'x',PD_POWER_TEXT_BYTES-1);}
    s->magic.concentration=1;s->effects[0]=(pd_effect){.target=PD_EFFECT_ARMOR,.value=2,.active=true};s->effect_count=1;
    for(unsigned i=0;i<PD_HISTORY_LIMIT;i++){e.dice[0]=1+i%20;e.total=e.dice[0]+3;assert(pd_history_append(h,&e));}
    assert(pd_library_save(&l,0,s,h,&slot)==PD_LIBRARY_OK&&slot==1);assert(pd_library_read(&l,1,out)==PD_LIBRARY_OK);same(out,s,h);
    assert(out->state.magic.concentration==1&&out->state.magic.slots[0].current==1&&out->state.magic.pact[2].current==1&&out->history.count==20);
    clear(root);assert(!rmdir(root));assert(!rmdir(tmp));free(s);free(h);free(out);free(before);
    puts("Library: native state/history/progress roundtrip; same-name distinct epochs; inactive-bank short write/fsync uncertainty; CRC recovery; future preservation; output atomicity; slot remap; duplicate identity refusal; full-capacity preservation passed. Host filesystem and injected I/O only, no SD/power-loss hardware claim.");
}
