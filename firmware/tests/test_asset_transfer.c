#define _POSIX_C_SOURCE 200809L
#include "asset_transfer.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <openssl/sha.h>
static uint8_t *original,*changed;
static bool no_space,fail_write,fail_sync;
static unsigned rename_calls,fail_rename;
size_t __real_fwrite(const void *,size_t,size_t,FILE *);
size_t __wrap_fwrite(const void *b,size_t n,size_t count,FILE *f){if(fail_write){errno=ENOSPC;return 0;}return __real_fwrite(b,n,count,f);}
int __real_fsync(int);
int __wrap_fsync(int fd){if(fail_sync){errno=EIO;return -1;}return __real_fsync(fd);}
int __real_rename(const char *,const char *);
int __wrap_rename(const char *a,const char *b){rename_calls++;if(fail_rename&&rename_calls>=fail_rename){errno=EIO;return -1;}return __real_rename(a,b);}
static void digest(const uint8_t *b,char out[65]){unsigned char hash[32];SHA256(b,PD_ASSET_BYTES,hash);for(unsigned i=0;i<32;i++)snprintf(out+2*i,3,"%02x",hash[i]);}
static bool space(uint64_t *out){*out=no_space?0:50*1024*1024;return true;}
static bool verify(const char *path,const char *sha){
    if(!pd_looks_verify_file(path,NULL,NULL))return false;
    if(!sha)return true;
    FILE *f=fopen(path,"rb");assert(f);uint8_t *bytes=malloc(PD_ASSET_BYTES);assert(bytes);assert(fread(bytes,1,PD_ASSET_BYTES,f)==PD_ASSET_BYTES);fclose(f);char actual[65];digest(bytes,actual);free(bytes);return !strcmp(actual,sha);
}
static void put(const char *path,const uint8_t *bytes){FILE *f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,PD_ASSET_BYTES,f)==PD_ASSET_BYTES);assert(!fclose(f));}
static void le32(uint8_t *p,uint32_t value){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(value>>(8*i));}
static void transfer(pd_asset_writer *w,const uint8_t *bytes){
    pd_asset_request *r=calloc(1,sizeof *r);assert(r);r->kind=PD_ASSET_WRITE;
    for(unsigned offset=0;offset<PD_ASSET_BYTES;offset+=PD_ASSET_CHUNK){r->offset=offset;r->count=PD_ASSET_BYTES-offset<PD_ASSET_CHUNK?PD_ASSET_BYTES-offset:PD_ASSET_CHUNK;memcpy(r->data,bytes+offset,r->count);r->crc=pd_crc32(r->data,r->count);assert(pd_asset_write(w,r));}
    free(r);
}
int main(int argc,char **argv){
    assert(argc==2);original=malloc(PD_ASSET_BYTES);changed=malloc(PD_ASSET_BYTES);assert(original&&changed);FILE *f=fopen(argv[1],"rb");assert(f);assert(fread(original,1,PD_ASSET_BYTES,f)==PD_ASSET_BYTES);fclose(f);
    memcpy(changed,original,PD_ASSET_BYTES);changed[2048]^=1;le32(changed+32,pd_crc32(changed+2048,PD_LOOK_BYTES));le32(changed+24,pd_crc32(changed+32,2016));le32(changed+28,pd_crc32(changed,28));
    char old_sha[65],new_sha[65];digest(original,old_sha);digest(changed,new_sha);
    char dir[]="/tmp/pdd-assets-XXXXXX";assert(mkdtemp(dir));pd_asset_writer w;pd_asset_writer_init(&w,dir,verify,space);
    put(w.main,original);char other[192];snprintf(other,sizeof other,"%s/unrelated.dat",dir);FILE *keep=fopen(other,"wb");assert(keep&&fputs("keep",keep)>=0&&!fclose(keep));
    assert(pd_asset_begin(&w,PD_ASSET_BYTES,old_sha)==2&&!w.file);
    no_space=true;assert(!pd_asset_begin(&w,PD_ASSET_BYTES,new_sha)&&!strcmp(w.error,"space"));no_space=false;assert(verify(w.main,old_sha));
    assert(!pd_asset_begin(&w,PD_ASSET_BYTES-1,new_sha));assert(!pd_asset_begin(&w,PD_ASSET_BYTES,"../escape"));
    // Cancel, corrupt chunk and short disk write never replace the old package.
    pd_asset_request r={.kind=PD_ASSET_WRITE,.count=1};r.data[0]='P';r.crc=pd_crc32(r.data,1);
    assert(pd_asset_begin(&w,PD_ASSET_BYTES,new_sha)==1);assert(pd_asset_write(&w,&r));pd_asset_abort(&w);assert(verify(w.main,old_sha)&&access(w.part,F_OK));
    assert(pd_asset_begin(&w,PD_ASSET_BYTES,new_sha)==1);r.crc++;assert(!pd_asset_write(&w,&r)&&!strcmp(w.error,"chunk"));assert(verify(w.main,old_sha));r.crc--;
    assert(pd_asset_begin(&w,PD_ASSET_BYTES,new_sha)==1);fail_write=true;assert(!pd_asset_write(&w,&r));fail_write=false;assert(verify(w.main,old_sha));
    assert(pd_asset_begin(&w,PD_ASSET_BYTES,new_sha)==1);assert(!pd_asset_finish(&w,PD_ASSET_BYTES)&&!strcmp(w.error,"incomplete"));assert(verify(w.main,old_sha));
    // Full bytes with wrong digest, sync failure, then a successful replacement.
    assert(pd_asset_begin(&w,PD_ASSET_BYTES,new_sha)==1);transfer(&w,original);assert(!pd_asset_finish(&w,PD_ASSET_BYTES)&&!strcmp(w.error,"integrity"));assert(verify(w.main,old_sha));
    assert(pd_asset_begin(&w,PD_ASSET_BYTES,new_sha)==1);transfer(&w,changed);fail_sync=true;assert(!pd_asset_finish(&w,PD_ASSET_BYTES));fail_sync=false;assert(verify(w.main,old_sha));
    assert(pd_asset_begin(&w,PD_ASSET_BYTES,new_sha)==1);transfer(&w,changed);assert(pd_asset_finish(&w,PD_ASSET_BYTES)&&verify(w.main,new_sha)&&verify(w.backup,old_sha));
    no_space=true;assert(pd_asset_begin(&w,PD_ASSET_BYTES,new_sha)==2);no_space=false;
    // Power cut equivalent between renames: loader recovers the valid .bak.
    assert(pd_asset_begin(&w,PD_ASSET_BYTES,old_sha)==1);transfer(&w,original);rename_calls=0;fail_rename=2;assert(!pd_asset_finish(&w,PD_ASSET_BYTES));fail_rename=0;
    assert(access(w.main,F_OK)&&verify(w.backup,new_sha));pd_looks_set_file(w.main);pd_looks_set_backup(w.backup);uint8_t image[PD_LOOK_BYTES];assert(pd_look_image(0,0,1,2,image,sizeof image));assert(strstr(pd_looks_status(),"recuperato"));
    assert(pd_asset_begin(&w,PD_ASSET_BYTES,old_sha)==1);transfer(&w,original);assert(pd_asset_finish(&w,PD_ASSET_BYTES)&&verify(w.main,old_sha)&&verify(w.backup,new_sha));
    keep=fopen(other,"rb");assert(keep&&fgetc(keep)=='k');fclose(keep);
    // Strict path-free command framing and untouched output on malformed input.
    pd_asset_request parsed={0},before=parsed;const char *bad[]={"PDD ASSET BEGIN 2 10 abc","PDD ASSET BEGIN 1 ../path","PDD ASSET WRITE 1 -1 00000000 ff","PDD ASSET WRITE 1 0 00000000 f","PDD ASSET WRITE 1 0 00000000 zz","PDD ASSET ABORT 1 extra","PDD ASSET FINISH 1 99999999999999999999"};
    for(unsigned i=0;i<sizeof bad/sizeof bad[0];i++){assert(!pd_asset_parse(bad[i],strlen(bad[i]),&parsed));assert(!memcmp(&parsed,&before,sizeof parsed));}
    assert(pd_asset_parse("PDD ASSET HELLO 1",17,&parsed)&&parsed.kind==PD_ASSET_HELLO);
    char command[160];snprintf(command,sizeof command,"PDD ASSET BEGIN 1 %u %s",PD_ASSET_BYTES,new_sha);assert(pd_asset_parse(command,strlen(command),&parsed)&&parsed.size==PD_ASSET_BYTES&&!strcmp(parsed.sha,new_sha));
    assert(pd_asset_parse("PDD ASSET WRITE 1 0 00000000 ff",31,&parsed)&&parsed.count==1&&parsed.data[0]==255);
    unlink(w.main);unlink(w.backup);unlink(w.part);unlink(other);rmdir(dir);free(original);free(changed);
    puts("SD writer: strict commands, full catalog/SHA, exact chunks, no space, cancel/corrupt/short write/sync failure, rename interruption/recovery, already-installed and unrelated file retention passed; host filesystem only");
}
