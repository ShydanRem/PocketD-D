#define _POSIX_C_SOURCE 200809L
#include "asset_transfer.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
static bool take(const char **p,const char *end,const char *s){size_t n=strlen(s);if((size_t)(end-*p)<n||memcmp(*p,s,n))return false;*p+=n;return true;}
static bool number(const char **p,const char *end,unsigned *out){
    unsigned n=0;const char *start=*p;while(*p<end&&**p>='0'&&**p<='9'){unsigned digit=(unsigned)(**p-'0');if(n>(PD_ASSET_BYTES-digit)/10)return false;n=n*10+digit;(*p)++;}
    if(*p==start||(*p-start>1&&*start=='0'))return false;
    *out=n;return true;
}
static int hex(char c){return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:-1;}
bool pd_asset_parse(const char *line,size_t size,pd_asset_request *out){
    if(!line||!out||size<10||size>PD_ASSET_CHUNK*2+80)return false;
    for(size_t i=0;i<size;i++)if((unsigned char)line[i]<32||(unsigned char)line[i]>126)return false;
    pd_asset_request r={0};const char *p=line,*end=line+size;
    if(!take(&p,end,"PDD ASSET "))return false;
    if(take(&p,end,"HELLO 1")&&p==end){r.kind=PD_ASSET_HELLO;*out=r;return true;}
    p=line+10;
    if(take(&p,end,"ABORT 1")&&p==end){r.kind=PD_ASSET_ABORT;*out=r;return true;}
    p=line+10;
    if(take(&p,end,"BEGIN 1 ")){
        if(!number(&p,end,&r.size)||!take(&p,end," ")||end-p!=64)return false;
        for(unsigned i=0;i<64;i++)if(hex(p[i])<0)return false;
        memcpy(r.sha,p,64);r.kind=PD_ASSET_BEGIN;*out=r;return true;
    }
    p=line+10;
    if(take(&p,end,"FINISH 1 ")){if(!number(&p,end,&r.size)||p!=end)return false;r.kind=PD_ASSET_FINISH;*out=r;return true;}
    p=line+10;
    if(!take(&p,end,"WRITE 1 ")||!number(&p,end,&r.offset)||!take(&p,end," ")||end-p<10)return false;
    for(unsigned i=0;i<8;i++){int value=hex(*p++);if(value<0)return false;r.crc=(r.crc<<4)|(unsigned)value;}
    if(!take(&p,end," ")||p==end||(end-p)%2||(size_t)(end-p)>PD_ASSET_CHUNK*2)return false;
    r.count=(unsigned)(end-p)/2;
    for(unsigned i=0;i<r.count;i++){int hi=hex(*p++),lo=hex(*p++);if(hi<0||lo<0)return false;r.data[i]=(uint8_t)((hi<<4)|lo);}
    r.kind=PD_ASSET_WRITE;*out=r;return true;
}
void pd_asset_writer_init(pd_asset_writer *w,const char *dir,bool (*verify)(const char *,const char *),bool (*space)(uint64_t *)){
    memset(w,0,sizeof *w);w->verify=verify;w->space=space;w->error="idle";
    if(!dir||strlen(dir)>=sizeof w->directory)return;
    strcpy(w->directory,dir);snprintf(w->main,sizeof w->main,"%s/classes.pdc",dir);snprintf(w->part,sizeof w->part,"%s/classes.new",dir);snprintf(w->backup,sizeof w->backup,"%s/classes.bak",dir);
}
void pd_asset_abort(pd_asset_writer *w){if(w->file){fclose(w->file);w->file=NULL;}if(w->part[0])unlink(w->part);w->offset=0;}
static bool fail(pd_asset_writer *w,const char *error){w->error=error;pd_asset_abort(w);return false;}
unsigned pd_asset_begin(pd_asset_writer *w,unsigned size,const char *sha){
    if(w->file){w->error="busy";return 0;}
    if(!w->directory[0]||!w->verify||!w->space||size!=PD_ASSET_BYTES||!sha||strlen(sha)!=64){w->error="metadata";return 0;}
    for(unsigned i=0;i<64;i++)if(hex(sha[i])<0){w->error="metadata";return 0;}
    if(w->verify(w->main,sha)){w->error="";return 2;}
    if(mkdir(w->directory,0777)&&errno!=EEXIST){w->error="storage";return 0;}
    if(unlink(w->part)&&errno!=ENOENT){w->error="storage";return 0;}
    uint64_t available=0;if(!w->space(&available)){w->error="storage";return 0;}
    if(available<(uint64_t)size+4096){w->error="space";return 0;}
    w->file=fopen(w->part,"wb");if(!w->file){w->error="storage";return 0;}
    strcpy(w->sha,sha);w->offset=0;w->error="";return 1;
}
bool pd_asset_write(pd_asset_writer *w,const pd_asset_request *r){
    if(!w->file){w->error="idle";return false;}
    if(r->kind!=PD_ASSET_WRITE||!r->count||r->count>PD_ASSET_CHUNK||r->offset!=w->offset||r->count>PD_ASSET_BYTES-w->offset||pd_crc32(r->data,r->count)!=r->crc)return fail(w,"chunk");
    if(fwrite(r->data,1,r->count,w->file)!=r->count)return fail(w,"storage");
    w->offset+=r->count;return true;
}
bool pd_asset_finish(pd_asset_writer *w,unsigned size){
    if(!w->file){w->error="idle";return false;}
    if(size!=PD_ASSET_BYTES||w->offset!=size)return fail(w,"incomplete");
    bool ok=fflush(w->file)==0;if(ok)ok=fsync(fileno(w->file))==0;if(fclose(w->file))ok=false;w->file=NULL;
    if(!ok)return fail(w,"storage");
    if(!w->verify(w->part,w->sha))return fail(w,"integrity");
    // Retain a previous valid catalog. Loader can read .bak if power stops
    // between the two renames; no unrelated SD file is opened or removed.
    bool old=w->verify(w->main,NULL);
    if(old){if(unlink(w->backup)&&errno!=ENOENT)return fail(w,"storage");if(rename(w->main,w->backup))return fail(w,"storage");}
    else if(unlink(w->main)&&errno!=ENOENT)return fail(w,"storage");
    if(rename(w->part,w->main)){if(old)rename(w->backup,w->main);return fail(w,"storage");}
    if(!w->verify(w->main,w->sha)){unlink(w->main);if(old)rename(w->backup,w->main);return fail(w,"integrity");}
    w->error="";return true;
}
