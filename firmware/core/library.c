#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

/* PDL1: magic, slot u32le, revision u32le, install owner u64le, native backup JSON, CRC32 over
   everything before the final u32le. Two independently verified banks per PG.
   No index or active pointer can become inconsistent with the NVS epoch. */
#define OVERHEAD 24u
static uint32_t u32(const unsigned char *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void put(unsigned char *p,uint32_t n){for(unsigned i=0;i<4;i++)p[i]=n>>(8*i);}
static bool good(pd_library_result r){return r==PD_LIBRARY_OK||r==PD_LIBRARY_RECOVERED;}
static void cooperate(const pd_library *l){if(l->yield)l->yield(l->context);}
static bool path(const pd_library *l,unsigned slot,unsigned bank,char *out,size_t size){
    return l&&l->root[0]&&slot>=1&&slot<=PD_LIBRARY_LIMIT&&bank<2&&snprintf(out,size,"%s/pg%02u%c.pdl",l->root,slot,bank?'b':'a')<(int)size;
}
static bool directory(const char *name){struct stat s;if(!stat(name,&s))return S_ISDIR(s.st_mode);return errno==ENOENT&&mkdir(name,0700)==0;}
bool pd_library_init(pd_library *l,const char *root,uint64_t owner,void (*yield)(void *),void *context){
    if(!l||!owner||!root||!root[0]||strlen(root)>=sizeof l->root)return false;
    char parent[192];strcpy(parent,root);char *slash=strrchr(parent,'/');
    if(slash&&slash!=parent){*slash=0;if(!directory(parent))return false;}
    if(!directory(root))return false;
    memset(l,0,sizeof *l);strcpy(l->root,root);l->owner=owner;l->yield=yield;l->context=context;return true;
}
static pd_library_result bank_read(const pd_library *l,unsigned slot,unsigned bank,pd_backup *out,uint32_t *revision,uint64_t *owner){
    char name[224];if(!path(l,slot,bank,name,sizeof name))return PD_LIBRARY_IO;
    FILE *f=fopen(name,"rb");if(!f)return errno==ENOENT?PD_LIBRARY_EMPTY:PD_LIBRARY_IO;
    pd_library_result result=PD_LIBRARY_CORRUPT;unsigned char *bytes=NULL;
    if(fseek(f,0,SEEK_END)){result=PD_LIBRARY_IO;goto done;}
    long size=ftell(f);if(size<0){result=PD_LIBRARY_IO;goto done;}
    if(size>PD_BACKUP_MAX+(long)OVERHEAD){result=PD_LIBRARY_FUTURE;goto done;}
    if(size<(long)OVERHEAD)goto done;
    if(fseek(f,0,SEEK_SET)){result=PD_LIBRARY_IO;goto done;}
    bytes=malloc((size_t)size);if(!bytes){result=PD_LIBRARY_MEMORY;goto done;}
    for(size_t at=0;at<(size_t)size;){size_t n=(size_t)size-at;if(n>4096)n=4096;if(fread(bytes+at,1,n,f)!=n){if(ferror(f))result=PD_LIBRARY_IO;goto done;}at+=n;cooperate(l);}
    if(fgetc(f)!=EOF||ferror(f)){result=PD_LIBRARY_IO;goto done;}
    if(pd_crc32(bytes,(size_t)size-4)!=u32(bytes+size-4))goto done;
    if(memcmp(bytes,"PDL1",4)){result=PD_LIBRARY_FUTURE;goto done;}
    if(u32(bytes+4)!=slot||!u32(bytes+8)||(!u32(bytes+12)&&!u32(bytes+16)))goto done;
    /* A valid container with an undecodable native payload may be a newer
       native format. Preserve it; never turn a downgrade into a lossy save. */
    if(!pd_backup_decode((const char *)bytes+20,(size_t)size-OVERHEAD,out)){result=PD_LIBRARY_FUTURE;goto done;}
    *revision=u32(bytes+8);*owner=(uint64_t)u32(bytes+12)|(uint64_t)u32(bytes+16)<<32;result=PD_LIBRARY_OK;
done:
    free(bytes);if(fclose(f)&&result==PD_LIBRARY_OK)result=PD_LIBRARY_IO;return result;
}
static pd_library_result load(const pd_library *l,unsigned slot,pd_backup *out,unsigned *bank,uint32_t *revision,uint64_t *owner){
    pd_backup *best=malloc(sizeof *best),*candidate=malloc(sizeof *candidate);
    if(!best||!candidate){free(best);free(candidate);return PD_LIBRARY_MEMORY;}
    pd_library_result results[2];uint32_t revisions[2]={0,0};uint64_t owners[2]={0,0};unsigned selected=0;bool found=false;
    for(unsigned i=0;i<2;i++){
        results[i]=bank_read(l,slot,i,candidate,revisions+i,owners+i);
        if(results[i]==PD_LIBRARY_IO||results[i]==PD_LIBRARY_MEMORY||results[i]==PD_LIBRARY_FUTURE){free(best);free(candidate);return results[i];}
        if(results[i]==PD_LIBRARY_OK){
            if(found&&revisions[i]==revisions[selected]){free(best);free(candidate);return PD_LIBRARY_CONFLICT;}
            if(!found||revisions[i]>revisions[selected]){*best=*candidate;selected=i;found=true;}
        }
    }
    pd_library_result result=found?(results[1-selected]==PD_LIBRARY_CORRUPT?PD_LIBRARY_RECOVERED:PD_LIBRARY_OK):results[0]==PD_LIBRARY_EMPTY&&results[1]==PD_LIBRARY_EMPTY?PD_LIBRARY_EMPTY:PD_LIBRARY_CORRUPT;
    if(found){if(out)*out=*best;if(bank)*bank=selected;if(revision)*revision=revisions[selected];if(owner)*owner=owners[selected];}
    free(best);free(candidate);return result;
}
pd_library_result pd_library_read(const pd_library *l,unsigned slot,pd_backup *out){if(!out)return PD_LIBRARY_IO;return load(l,slot,out,NULL,NULL,NULL);}
bool pd_library_scan(const pd_library *l,pd_library_catalog *out){
    if(!l||!out)return false;
    pd_library_catalog *next=calloc(1,sizeof *next);pd_backup *snapshot=malloc(sizeof *snapshot);
    if(!next||!snapshot){free(next);free(snapshot);return false;}
    bool ok=true;
    for(unsigned slot=1;slot<=PD_LIBRARY_LIMIT;slot++){
        uint64_t owner=0;pd_library_result r=load(l,slot,snapshot,NULL,NULL,&owner);if(r==PD_LIBRARY_EMPTY)continue;
        if(r==PD_LIBRARY_IO||r==PD_LIBRARY_MEMORY){ok=false;break;}
        pd_library_entry *entry=next->entries+next->count++;entry->slot=slot;entry->status=r;
        if(good(r)){memcpy(entry->name,snapshot->state.name,PD_NAME_BYTES);entry->level=snapshot->state.level;entry->epoch=snapshot->state.history_epoch;entry->owner=owner;}
        cooperate(l);
    }
    if(ok)*out=*next;
    free(next);free(snapshot);return ok;
}
pd_library_result pd_library_save(const pd_library *l,unsigned slot,const pd_state *s,const pd_history *h,unsigned *saved){
    if(!l||!s||!h||!pd_valid(s)||!pd_history_valid(h)||s->history_epoch!=h->epoch||slot>PD_LIBRARY_LIMIT)return PD_LIBRARY_CONFLICT;
    if(!slot){
        pd_library_catalog catalog;if(!pd_library_scan(l,&catalog))return PD_LIBRARY_IO;
        unsigned empty=1,matches=0;
        for(unsigned i=0;i<catalog.count;i++){const pd_library_entry *e=catalog.entries+i;if(e->slot==empty)empty++;if(good(e->status)&&e->owner==l->owner&&e->epoch==s->history_epoch){slot=e->slot;matches++;}}
        if(matches>1)return PD_LIBRARY_CONFLICT;
        if(!slot){if(empty>PD_LIBRARY_LIMIT)return PD_LIBRARY_FULL;slot=empty;}
    }
    unsigned active=1;uint32_t revision=0;
    pd_library_result before=load(l,slot,NULL,&active,&revision,NULL);
    if(before!=PD_LIBRARY_EMPTY&&!good(before))return before;
    if(revision==UINT32_MAX)return PD_LIBRARY_FULL;
    size_t n;char *json=pd_backup_encode(s,h,&n);if(!json)return PD_LIBRARY_MEMORY;
    unsigned char *bytes=malloc(n+OVERHEAD);if(!bytes){free(json);return PD_LIBRARY_MEMORY;}
    memcpy(bytes,"PDL1",4);put(bytes+4,slot);put(bytes+8,revision+1);put(bytes+12,(uint32_t)l->owner);put(bytes+16,(uint32_t)(l->owner>>32));memcpy(bytes+20,json,n);free(json);put(bytes+20+n,pd_crc32(bytes,n+20));
    char name[224];pd_library_result result=PD_LIBRARY_IO;
    if(!path(l,slot,1-active,name,sizeof name)){free(bytes);return result;}
    FILE *f=fopen(name,"wb");if(!f){free(bytes);return result;}
    bool ok=true;
    for(size_t at=0;at<n+OVERHEAD;){size_t chunk=n+OVERHEAD-at;if(chunk>4096)chunk=4096;if(fwrite(bytes+at,1,chunk,f)!=chunk){ok=false;break;}at+=chunk;cooperate(l);}
    if(ok)ok=fflush(f)==0&&fsync(fileno(f))==0;
    if(fclose(f))ok=false;
    if(ok){
        f=fopen(name,"rb");if(!f)ok=false;
        else{unsigned char verify[512];for(size_t at=0;ok&&at<n+OVERHEAD;){size_t chunk=n+OVERHEAD-at;if(chunk>sizeof verify)chunk=sizeof verify;if(fread(verify,1,chunk,f)!=chunk||memcmp(verify,bytes+at,chunk))ok=false;at+=chunk;cooperate(l);}if(fgetc(f)!=EOF||ferror(f))ok=false;if(fclose(f))ok=false;}
    }
    free(bytes);if(ok){if(saved)*saved=slot;result=PD_LIBRARY_OK;}return result;
}
