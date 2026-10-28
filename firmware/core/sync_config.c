#include "sync_config.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
static bool copy(cJSON *o,const char *k,char *v,size_t cap){cJSON *x=cJSON_GetObjectItemCaseSensitive(o,k);if(!cJSON_IsString(x)||!x->valuestring[0]||strlen(x->valuestring)>=cap)return false;strcpy(v,x->valuestring);return true;}
static bool hex(const char *s,size_t n){if(strlen(s)!=n)return false;for(size_t i=0;i<n;i++)if(!((s[i]>='0'&&s[i]<='9')||(s[i]>='a'&&s[i]<='f')))return false;return true;}
bool pd_sync_config_parse(const char *json,size_t size,pd_sync_config *out){
 if(!json||!out||!size||size>3000||memchr(json,0,size))return false;
 for(size_t i=0;i+1<size;i++)if(json[i]=='\\'){if(json[i+1]=='u')return false;i++;}
 unsigned depth=0;bool quoted=false;for(size_t i=0;i<size;i++){char c=json[i];if(quoted&&c=='\\'){i++;continue;}if(c=='"')quoted=!quoted;else if(!quoted&&(c=='{'||c=='[')){if(++depth>1)return false;}else if(!quoted&&(c=='}'||c==']')){if(!depth)return false;depth--;}}
 const char *end; cJSON *root=cJSON_ParseWithLengthOpts(json,size,&end,false);if(!root)return false;
 while(end<json+size&&(*end==' '||*end=='\n'||*end=='\r'||*end=='\t'))end++;
 const char *keys[]={"version","id","host","port","characterId","token","certificate","time"};bool okay=cJSON_IsObject(root)&&cJSON_GetArraySize(root)==8&&end==json+size;
 for(unsigned i=0;i<8&&okay;i++){unsigned n=0;cJSON *x;cJSON_ArrayForEach(x,root)if(x->string&&!strcmp(x->string,keys[i]))n++;if(n!=1)okay=false;}
 pd_sync_config c={0};cJSON *v=cJSON_GetObjectItemCaseSensitive(root,"version"),*port=cJSON_GetObjectItemCaseSensitive(root,"port"),*time=cJSON_GetObjectItemCaseSensitive(root,"time");
 if(!okay||!cJSON_IsNumber(v)||v->valuedouble!=1||!copy(root,"id",c.id,sizeof c.id)||!hex(c.id,32)||!copy(root,"token",c.token,sizeof c.token)||!hex(c.token,64)||!copy(root,"host",c.host,sizeof c.host)||!copy(root,"characterId",c.character_id,sizeof c.character_id)||!copy(root,"certificate",c.certificate,sizeof c.certificate)||!cJSON_IsNumber(port)||port->valuedouble<1024||port->valuedouble>65535||floor(port->valuedouble)!=port->valuedouble||!cJSON_IsNumber(time)||time->valuedouble<1735689600||time->valuedouble>4102444800||floor(time->valuedouble)!=time->valuedouble)okay=false;
 if(okay){unsigned a=0,b=0,d=0,e=0;char tail;if(sscanf(c.host,"%u.%u.%u.%u%c",&a,&b,&d,&e,&tail)!=4||a>255||b>255||d>255||e>255||!(a==10||(a==172&&b>=16&&b<=31)||(a==192&&b==168)))okay=false;
  char canonical[16];snprintf(canonical,sizeof canonical,"%u.%u.%u.%u",a,b,d,e);if(strcmp(canonical,c.host))okay=false;
  if(c.character_id[0]<'1'||c.character_id[0]>'9')okay=false;
  for(size_t i=0;i<strlen(c.character_id);i++)if(c.character_id[i]<'0'||c.character_id[i]>'9')okay=false;
  if(strncmp(c.certificate,"-----BEGIN CERTIFICATE-----\n",28)||!strstr(c.certificate,"\n-----END CERTIFICATE-----\n"))okay=false;
  c.format=1;c.port=port->valueint;c.time=(uint32_t)time->valuedouble;
 }
 cJSON_Delete(root);if(okay)*out=c;memset(&c,0,sizeof c);return okay;
}
