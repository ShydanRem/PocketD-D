#include "beyond_runtime.h"
#include "beyond_bundle.h"
#include "quickjs.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef PD_BEYOND_TEST_INIT
#include <stdio.h>
#endif
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#include "miniz.h"
#else
#include <zlib.h>
#endif

typedef union {max_align_t align;struct {size_t size;} block;} allocation;
static void *allocate(JSMallocState *s,size_t size){
    pd_beyond_control *c=s->opaque;if(!size)return NULL;
    size_t limit=s->malloc_limit<c->heap_limit?s->malloc_limit:c->heap_limit;
    if(size>SIZE_MAX-sizeof(allocation)||s->malloc_size>limit||size+sizeof(allocation)>limit-s->malloc_size){c->exhausted=true;return NULL;}
#ifdef ESP_PLATFORM
    allocation *p=heap_caps_aligned_alloc(_Alignof(max_align_t),size+sizeof *p,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
#else
    allocation *p=malloc(size+sizeof *p);
#endif
    if(!p){c->exhausted=true;return NULL;}p->block.size=size;s->malloc_count++;s->malloc_size+=size+sizeof *p;
    if(s->malloc_size>c->peak_bytes)c->peak_bytes=s->malloc_size;
    return p+1;
}
static void release(JSMallocState *s,void *ptr){if(!ptr)return;allocation *p=(allocation *)ptr-1;s->malloc_count--;s->malloc_size-=p->block.size+sizeof *p;free(p);}
static size_t usable(const void *ptr){return ptr?((const allocation *)ptr-1)->block.size:0;}
static void *resize(JSMallocState *s,void *ptr,size_t size){
    if(!ptr)return allocate(s,size);
    if(!size){release(s,ptr);return NULL;}
    size_t old=usable(ptr);if(size<=old)return ptr;
    /* Count BOTH allocations during a growing copy, not only the final size. */
    void *next=allocate(s,size);if(!next)return NULL;
    memcpy(next,ptr,old);release(s,ptr);return next;
}
static int interrupt(JSRuntime *rt,void *opaque){(void)rt;pd_beyond_control *c=opaque;return c->interrupt&&c->interrupt(c->context);}
static unsigned length(JSContext *ctx,JSValueConst value){JSValue v=JS_GetPropertyStr(ctx,value,"length");uint32_t n=0;if(JS_ToUint32(ctx,&n,v))n=0;JS_FreeValue(ctx,v);return n;}
static void lines(JSContext *ctx,JSValueConst value,pd_beyond_report *report){
    unsigned n=length(ctx,value);
    for(unsigned i=0;i<n&&report->count<PD_BEYOND_REPORT_LINES;i++){
        JSValue v=JS_GetPropertyUint32(ctx,value,i);const char *text=JS_ToCString(ctx,v);
        if(text){char *out=report->lines[report->count++];size_t j=0;for(size_t k=0;text[k];){unsigned char ch=text[k];unsigned n=ch<128?1:ch<0xe0?2:ch<0xf0?3:4;if(j+n>160)break;if(n==1)out[j++]=ch<32||ch==127?' ':ch;else{memcpy(out+j,text+k,n);j+=n;}k+=n;}out[j]=0;JS_FreeCString(ctx,text);}JS_FreeValue(ctx,v);
    }
}
unsigned pd_beyond_convert(const char *source,size_t size,const char *id,pd_state *out,pd_beyond_report *report,pd_beyond_control *control){
    if(!source||!size||size>PD_BEYOND_SOURCE_MAX||!pd_beyond_id(id)||!out||!report||!control||!pd_beyond_utf8((const unsigned char *)source,size))return PD_BEYOND_INVALID;
    control->peak_bytes=0;control->exhausted=false;control->phase=0;
    if(!control->heap_limit||control->heap_limit>PD_BEYOND_HEAP_MAX)control->heap_limit=PD_BEYOND_HEAP_MAX;
    JSMallocFunctions alloc={allocate,release,resize,usable};JSRuntime *rt=JS_NewRuntime2(&alloc,control);if(!rt)return PD_BEYOND_MEMORY;
    JS_SetMemoryLimit(rt,control->heap_limit);
#if defined(__SANITIZE_ADDRESS__) && !defined(ESP_PLATFORM)
    /* Instrumented compiler frames are much larger. The normal host suite
       exercises the device's 20 KiB bound; this host-only run checks memory
       access/UB with redzones, not firmware stack consumption. */
    JS_SetMaxStackSize(rt,512*1024);
#else
    JS_SetMaxStackSize(rt,20*1024);
#endif
    JS_SetInterruptHandler(rt,interrupt,control);
    pd_beyond_report *next=calloc(1,sizeof *next);
    JSContext *ctx=next?JS_NewContextRaw(rt):NULL;unsigned status=PD_BEYOND_ENGINE;char *program=NULL,*packet=NULL;if(!next)control->exhausted=true;
    JSValue evaluation=JS_UNDEFINED,payload=JS_UNDEFINED,global=JS_UNDEFINED,fn=JS_UNDEFINED,converted=JS_UNDEFINED,packet_value=JS_UNDEFINED;
    if(!ctx)goto done;
    if(JS_AddIntrinsicBaseObjects(ctx)||JS_AddIntrinsicEval(ctx)||JS_AddIntrinsicStringNormalize(ctx)||JS_AddIntrinsicRegExp(ctx)||JS_AddIntrinsicJSON(ctx)||JS_AddIntrinsicMapSet(ctx))goto done;
    program=malloc(PD_BEYOND_BUNDLE_SIZE+1);if(!program){control->exhausted=true;goto done;}
#ifdef ESP_PLATFORM
    if(tinfl_decompress_mem_to_mem(program,PD_BEYOND_BUNDLE_SIZE,pd_beyond_bundle,sizeof pd_beyond_bundle,TINFL_FLAG_PARSE_ZLIB_HEADER)!=PD_BEYOND_BUNDLE_SIZE)goto done;
#else
    uLongf unpacked=PD_BEYOND_BUNDLE_SIZE;if(uncompress((unsigned char *)program,&unpacked,pd_beyond_bundle,sizeof pd_beyond_bundle)!=Z_OK||unpacked!=PD_BEYOND_BUNDLE_SIZE)goto done;
#endif
    program[PD_BEYOND_BUNDLE_SIZE]=0;
    control->phase=1;evaluation=JS_Eval(ctx,program,PD_BEYOND_BUNDLE_SIZE,"pocket-rules",JS_EVAL_TYPE_GLOBAL);free(program);program=NULL;
    if(JS_IsException(evaluation))goto done;
    control->phase=2;payload=JS_ParseJSON(ctx,source,size,"beyond-source");if(JS_IsException(payload)){status=PD_BEYOND_INVALID;goto done;}
    global=JS_GetGlobalObject(ctx);fn=JS_GetPropertyStr(ctx,global,"pocketBeyondConvert");
    control->phase=3;JSValue args[]={payload,JS_NewString(ctx,id)};converted=JS_Call(ctx,fn,JS_UNDEFINED,2,args);JS_FreeValue(ctx,args[1]);
    if(JS_IsException(converted)){status=PD_BEYOND_INVALID;goto done;}
    JSValue blocked=JS_GetPropertyStr(ctx,converted,"blockers"),limits=JS_GetPropertyStr(ctx,converted,"limits");
    next->blockers=length(ctx,blocked);next->limits=length(ctx,limits);lines(ctx,blocked,next);lines(ctx,limits,next);JS_FreeValue(ctx,blocked);JS_FreeValue(ctx,limits);
    JSValue ready=JS_GetPropertyStr(ctx,converted,"ready");bool can_import=JS_ToBool(ctx,ready)>0;JS_FreeValue(ctx,ready);
    if(!can_import||next->blockers){status=PD_BEYOND_BLOCKED;goto done;}
    control->phase=4;packet_value=JS_GetPropertyStr(ctx,converted,"packet");JSValue encoded=JS_JSONStringify(ctx,packet_value,JS_UNDEFINED,JS_UNDEFINED);
    if(JS_IsException(encoded))goto done;
    size_t n;const char *json=JS_ToCStringLen(ctx,&n,encoded);
    if(json&&n<=PD_PACKET_MAX){packet=malloc(n+1);if(packet)memcpy(packet,json,n+1);else control->exhausted=true;}
    if(json)JS_FreeCString(ctx,json);
    JS_FreeValue(ctx,encoded);
    if(packet)status=PD_BEYOND_READY;
done:
#if defined(PD_BEYOND_TEST_INIT) && !defined(ESP_PLATFORM)
    /* Diagnostic only for trusted bundle initialization, before reading ANY
       source JSON. Never enable exception logging during source conversion. */
    if(ctx&&control->phase==1&&JS_IsException(evaluation)){
        JSValue error=JS_GetException(ctx);const char *text=JS_ToCString(ctx,error);
        if(text){fprintf(stderr,"Trusted bundle initialization: %.120s\n",text);JS_FreeCString(ctx,text);}JS_FreeValue(ctx,error);
    }
#endif
    free(program);
    if(ctx){JS_FreeValue(ctx,packet_value);JS_FreeValue(ctx,converted);JS_FreeValue(ctx,fn);JS_FreeValue(ctx,global);JS_FreeValue(ctx,payload);JS_FreeValue(ctx,evaluation);JS_FreeContext(ctx);}
    JS_FreeRuntime(rt);
    unsigned stopped=control->interrupt?control->interrupt(control->context):0;
    if(stopped)status=stopped;else if(control->exhausted)status=PD_BEYOND_MEMORY;
    if(status==PD_BEYOND_READY){control->phase=5;if(!pd_parse_packet(packet,strlen(packet),out))status=PD_BEYOND_ENGINE;else control->phase=6;}
    free(packet);if(status==PD_BEYOND_READY||status==PD_BEYOND_BLOCKED)*report=*next;free(next);return status;
}
