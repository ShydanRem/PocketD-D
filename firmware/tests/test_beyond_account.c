#include "beyond_account.h"
#include "beyond_store.h"
#include "nvs.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "account_fixtures.h"
static pd_account_access stored;
static bool exists,fail_commit,bad_read;
static unsigned writes;
esp_err_t nvs_flash_init_partition(const char *p){assert(!strcmp(p,"pdd_nvs"));return ESP_OK;}
esp_err_t nvs_open_from_partition(const char *p,const char *ns,int mode,nvs_handle_t *h){assert(!strcmp(p,"pdd_nvs")&&!strcmp(ns,"pdd_beyond")&&(mode==NVS_READONLY||mode==NVS_READWRITE));*h=1;return ESP_OK;}
void nvs_close(nvs_handle_t h){assert(h==1);}
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *size){assert(h==1&&!strcmp(key,"access"));if(!exists)return ESP_ERR_NVS_NOT_FOUND;if(!out){*size=sizeof stored;return ESP_OK;}assert(*size>=sizeof stored);memcpy(out,&stored,sizeof stored);if(bad_read)((pd_account_access*)out)->version=99;*size=sizeof stored;return ESP_OK;}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *data,size_t size){assert(h==1&&!strcmp(key,"access")&&size==sizeof stored);memcpy(&stored,data,size);exists=true;writes++;return ESP_OK;}
esp_err_t nvs_commit(nvs_handle_t h){assert(h==1);return fail_commit?ESP_FAIL:ESP_OK;}
esp_err_t nvs_erase_key(nvs_handle_t h,const char *key){assert(h==1&&!strcmp(key,"access"));if(!exists)return ESP_ERR_NVS_NOT_FOUND;exists=false;memset(&stored,0,sizeof stored);return ESP_OK;}
int main(void){
 pd_account_access a={.version=1},b={0};strcpy(a.user,"1234");strcpy(a.cookie,"synthetic-session");assert(pd_account_valid(&a));
 const char *bad[]={"","bad\nheader","bad;header","bad\\header","bad header","bad\"header","bad,header"};for(unsigned i=0;i<7;i++)assert(!pd_account_cookie(bad[i]));
 assert(!pd_account_store_load(&b)&&pd_account_store_save(&a)&&writes==1&&pd_account_store_load(&b)&&!memcmp(&a,&b,sizeof a));
 bad_read=true;assert(!pd_account_store_save(&a));bad_read=false;fail_commit=true;assert(!pd_account_store_save(&a)&&!pd_account_store_forget());fail_commit=false;assert(pd_account_store_forget()&&!pd_account_store_load(&b)&&pd_account_store_forget());
 char *token=NULL;assert(pd_account_token(ACCOUNT_AUTH,strlen(ACCOUNT_AUTH),"1234",1800000000,&token)&&token);pd_account_wipe(token,strlen(token));free(token);token=NULL;
 assert(!pd_account_token(ACCOUNT_AUTH,strlen(ACCOUNT_AUTH),"9999",1800000000,&token)&&!token);assert(!pd_account_token(ACCOUNT_AUTH,strlen(ACCOUNT_AUTH),"1234",2000000000,&token)&&!token);
 assert(!pd_account_token(ACCOUNT_NUL_AUTH,strlen(ACCOUNT_NUL_AUTH),"1234",1800000000,&token)&&!token);
 pd_account_catalog *catalog=calloc(1,sizeof *catalog),*before=malloc(sizeof *before);assert(catalog&&before&&pd_account_catalog_parse(ACCOUNT_LIST,strlen(ACCOUNT_LIST),catalog)&&catalog->count==2&&!catalog->partial);*before=*catalog;
 const char *invalid[]={"{}","[]","{\"success\":true,\"data\":{\"characters\":[{\"id\":1,\"name\":\"x\"},{\"id\":1,\"name\":\"y\"}]}}","{\"success\":true,\"data\":{\"characters\":[{\"id\":1,\"name\":\"x\",\"level\":21}]}}"};
 for(unsigned i=0;i<4;i++)assert(!pd_account_catalog_parse(invalid[i],strlen(invalid[i]),catalog)&&!memcmp(catalog,before,sizeof *catalog));
 pd_account_request *r=calloc(1,sizeof *r),*old=malloc(sizeof *old);assert(r&&old);const char *hello="PDD ACCOUNT HELLO 1";assert(pd_account_command(hello,strlen(hello),r)&&r->kind==PD_ACCOUNT_HELLO);*old=*r;
 char offer[256];snprintf(offer,sizeof offer,"PDD ACCOUNT OFFER 1 0123456789abcdef0123456789abcdef 1234 synthetic-session");assert(pd_account_command(offer,strlen(offer),r)&&r->kind==PD_ACCOUNT_OFFER&&pd_account_valid(&r->access));
 *r=*old;uint32_t rng=1;for(unsigned i=0;i<20000;i++){size_t n=1+i%100;char raw[101];for(size_t j=0;j<n;j++){rng=rng*1664525u+1013904223u;raw[j]=(char)(rng>>24);}assert(!pd_account_command(raw,n,r)&&!memcmp(r,old,sizeof *r));}
 pd_account_wipe(&a,sizeof a);pd_account_wipe(&b,sizeof b);pd_account_wipe(r,sizeof *r);free(r);free(old);free(catalog);free(before);
 puts("Beyond account: synthetic subject/expiry/JWT validation, catalog/duplicate refusal, strict USB/cookie grammar and 20,000 malformed requests, namespace-only NVS save/readback/failure/forget passed. No real credentials/account access.");
}
