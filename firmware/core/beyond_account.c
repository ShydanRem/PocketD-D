#include "beyond_account.h"
#include "beyond_runtime.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
void pd_account_wipe(void *p,size_t n){volatile unsigned char *v=p;
    while(n--)*v++=0;
}
bool pd_account_cookie(const char *p){if(!p)return false;
    size_t n=strlen(p);
    if(!n||n>PD_ACCOUNT_COOKIE)return false;
    for(size_t i=0;i<n;i++){unsigned c=(unsigned char)p[i];
        if(c<33||c>126||c=='"'||c==','||c==';'||c=='\\')return false;
    }return true;
}
bool pd_account_valid(const pd_account_access *a){return a&&a->version==1&&memchr(a->user,0,sizeof a->user)&&memchr(a->cookie,0,sizeof a->cookie)&&pd_beyond_id(a->user)&&pd_account_cookie(a->cookie);
}
static bool nonce(const char *p){if(strlen(p)!=32)return false;
    for(unsigned i=0;i<32;i++)if(!((p[i]>='0'&&p[i]<='9')||(p[i]>='a'&&p[i]<='f')))return false;
    return true;
}
bool pd_account_command(const char *line,size_t size,pd_account_request *out){
    if(!line||!out||!size||size>PD_ACCOUNT_LINE||memchr(line,0,size))return false;
    char *copy=malloc(size+1);
    if(!copy)return false;
    memcpy(copy,line,size);
    copy[size]=0;
    pd_account_request *r=calloc(1,sizeof *r);
    bool ok=false;
    if(!r)goto done;
    if(!strcmp(copy,"PDD ACCOUNT HELLO 1")){r->kind=PD_ACCOUNT_HELLO;
        ok=true;
    }
    else if(!strncmp(copy,"PDD ACCOUNT CANCEL 1 ",21)&&nonce(copy+21)){r->kind=PD_ACCOUNT_CANCEL;
        strcpy(r->nonce,copy+21);
        ok=true;
    }
    else if(!strncmp(copy,"PDD ACCOUNT OFFER 1 ",20)){
        char *p=copy+20,*space=strchr(p,' ');
        if(!space)goto done;
        *space++=0;
        if(!nonce(p))goto done;
        strcpy(r->nonce,p);
        p=space;
        space=strchr(p,' ');
        if(!space)goto done;
        *space++=0;
        if(!pd_beyond_id(p)||!pd_account_cookie(space))goto done;
        strcpy(r->access.user,p);
        strcpy(r->access.cookie,space);
        r->access.version=1;
        r->kind=PD_ACCOUNT_OFFER;
        ok=true;
    }
    if(ok)*out=*r;
    done:if(r){pd_account_wipe(r,sizeof *r);
        free(r);
    }pd_account_wipe(copy,size+1);
    free(copy);
    return ok;
}
static cJSON *parse(const char *json,size_t size){if(!json||!size||!pd_beyond_utf8((const unsigned char *)json,size))return NULL;
    /* cJSON strings are NUL-terminated: never accept an escaped NUL that
       could hide a suffix of an identity or a critical object key. */
    for(size_t i=0;i+6<=size;i++)if(!memcmp(json+i,"\\u0000",6))return NULL;
    unsigned depth=0;bool quoted=false,escaped=false;
    for(size_t i=0;i<size;i++){unsigned char c=json[i];if(quoted){if(escaped)escaped=false;else if(c=='\\')escaped=true;else if(c=='"')quoted=false;}else if(c=='"')quoted=true;else if(c=='{'||c=='['){if(++depth>32)return NULL;}else if(c=='}'||c==']'){if(!depth)return NULL;depth--;}}
    if(depth||quoted)return NULL;
    const char *end=NULL;
    cJSON *p=cJSON_ParseWithLengthOpts(json,size,&end,false);
    if(!p)return NULL;
    while(end<json+size&&(*end==' '||*end=='\r'||*end=='\n'||*end=='\t'))end++;
    if(end!=json+size){cJSON_Delete(p);
        return NULL;
    }return p;
}
static const cJSON *field(const cJSON *o,const char *key){const cJSON *found=NULL;
    for(const cJSON *c=o?o->child:NULL;c;c=c->next)if(c->string&&!strcmp(c->string,key)){if(found)return NULL;
        found=c;
    }return found;
}
static bool number(const cJSON *v,double min,double max){return cJSON_IsNumber(v)&&isfinite(v->valuedouble)&&floor(v->valuedouble)==v->valuedouble&&v->valuedouble>=min&&v->valuedouble<=max;
}
static int b64(unsigned c){if(c>='A'&&c<='Z')return c-'A';
    if(c>='a'&&c<='z')return c-'a'+26;
    if(c>='0'&&c<='9')return c-'0'+52;
    return c=='-'?62:c=='_'?63:-1;
}
bool pd_account_token(const char *json,size_t size,const char *expected,int64_t now,char **out){
    if(size>32768||!pd_beyond_id(expected)||!out)return false;
    bool ok=false;
    cJSON *root=parse(json,size),*claims=NULL;
    char *decoded=NULL,*token=NULL;size_t decoded_size=0;
    const cJSON *v=field(root,"token");
    if(!cJSON_IsString(v))goto done;
    const char *s=v->valuestring;
    size_t n=strlen(s);
    if(!n||n>PD_ACCOUNT_TOKEN)goto done;
    const char *first=strchr(s,'.'),*last=strrchr(s,'.');
    if(!first||first==last||first==s||last==first+1||!last[1])goto done;
    for(size_t i=0;i<n;i++)if(s[i]!='.'&&b64((unsigned char)s[i])<0)goto done;
    if(strchr(first+1,'.')!=last)goto done;
    size_t encoded=last-first-1;
    if(encoded%4==1)goto done;
    decoded_size=encoded+1;decoded=malloc(decoded_size);
    if(!decoded)goto done;
    uint32_t bits=0;
    unsigned remain=0;
    size_t at=0;
    for(const char *p=first+1;p<last;p++){bits=(bits<<6)|(unsigned)b64((unsigned char)*p);
        remain+=6;
        if(remain>=8){remain-=8;
            decoded[at++]=(char)(bits>>remain);
    }}
    if(remain&&(bits&((1u<<remain)-1)))goto done;
    decoded[at]=0;
    claims=parse(decoded,at);
    const cJSON *id=field(claims,"http://schemas.xmlsoap.org/ws/2005/05/identity/claims/nameidentifier"),*exp=field(claims,"exp");
    if(!cJSON_IsString(id)||strcmp(id->valuestring,expected)||!number(exp,1,9007199254740991.0)||exp->valuedouble<=(double)now+15)goto done;
    token=malloc(n+1);
    if(!token)goto done;
    memcpy(token,s,n+1);
    *out=token;
    token=NULL;
    ok=true;
    done:if(token){pd_account_wipe(token,strlen(token));
        free(token);
    }if(decoded){pd_account_wipe(decoded,decoded_size);
        free(decoded);
    }cJSON_Delete(claims);
    if(root){cJSON *t=cJSON_GetObjectItemCaseSensitive(root,"token");
        if(cJSON_IsString(t))pd_account_wipe(t->valuestring,strlen(t->valuestring));
    }cJSON_Delete(root);
    return ok;
}
static bool text(const cJSON *v,char *out,size_t capacity,bool required){
    if(!v||cJSON_IsNull(v)){out[0]=0;
        return !required;
    }if(!cJSON_IsString(v))return false;
    const char *s=v->valuestring;
    size_t n=strlen(s);
    if(n>640||!pd_beyond_utf8((const unsigned char *)s,n))return false;
    bool visible=false;
    for(size_t i=0;i<n;i++){unsigned c=(unsigned char)s[i];
        if(c<32||c==127)return false;
        if(c!=' ')visible=true;
    }if(required&&!visible)return false;
    size_t take=n<capacity?n:capacity-1;
    while(take<n&&take&&((unsigned char)s[take]&0xc0)==0x80)take--;
    memcpy(out,s,take);
    out[take]=0;
    return true;
}
bool pd_account_catalog_parse(const char *json,size_t size,pd_account_catalog *out){
    if(size>512u*1024u||!out)return false;
    cJSON *root=parse(json,size);
    pd_account_catalog *next=calloc(1,sizeof *next);
    bool ok=false;
    if(!root||!next)goto done;
    const cJSON *data=field(root,"data"),*rows=field(data,"characters");
    if(!cJSON_IsTrue(field(root,"success"))||!cJSON_IsObject(data)||!cJSON_IsArray(rows)||cJSON_GetArraySize(rows)>PD_ACCOUNT_ROWS)goto done;
    for(const cJSON *row=rows->child;row;row=row->next){pd_account_row *r=&next->rows[next->count];
        const cJSON *id=field(row,"id"),*level=field(row,"level");
        if(!cJSON_IsObject(row)||!number(id,1,999999999999999.0)||!text(field(row,"name"),r->name,sizeof r->name,true))goto done;
        snprintf(r->id,sizeof r->id,"%.0f",id->valuedouble);
        for(unsigned i=0;i<next->count;i++)if(!strcmp(r->id,next->rows[i].id))goto done;
        if(level&&!cJSON_IsNull(level)&&!number(level,0,20))goto done;
        r->level=cJSON_IsNumber(level)?(unsigned)level->valuedouble:0;
        char race[65],classes[65];
        if(!text(field(row,"raceName"),race,sizeof race,false)||!text(field(row,"classDescription"),classes,sizeof classes,false))goto done;
        snprintf(r->detail,sizeof r->detail,"%s",race);
        size_t used=strlen(r->detail);const char *parts[]={*race&&*classes?" | ":"",classes};for(unsigned j=0;j<2;j++){size_t add=strlen(parts[j]);if(add>sizeof r->detail-1-used)add=sizeof r->detail-1-used;memcpy(r->detail+used,parts[j],add);used+=add;r->detail[used]=0;}
        size_t len=strlen(r->detail);
        while(len&&!pd_beyond_utf8((unsigned char *)r->detail,len))r->detail[--len]=0;
        next->count++;
    }
    const cJSON *page=field(root,"pagination"),*total=field(page,"total"),*pages=field(page,"pages");
    next->partial=(number(total,0,9007199254740991.0)&&total->valuedouble>next->count)||(number(pages,1,9007199254740991.0)&&pages->valuedouble>1);
    *out=*next;
    ok=true;
    done:free(next);
    cJSON_Delete(root);
    return ok;
}
