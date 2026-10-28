#include "wifi_setup.h"
#include <string.h>
static bool utf8(const unsigned char *s){
 while(*s){unsigned c=*s++;if(c<32||c==127)return false;if(c<128)continue;
  unsigned n=c>=0xc2&&c<=0xdf?1:c>=0xe0&&c<=0xef?2:c>=0xf0&&c<=0xf4?3:0;if(!n)return false;
  unsigned first=*s;if((c==0xe0&&first<0xa0)||(c==0xed&&first>=0xa0)||(c==0xf0&&first<0x90)||(c==0xf4&&first>=0x90))return false;
  while(n--){if((*s&0xc0)!=0x80)return false;s++;}
 }return true;
}
static size_t bounded(const char *s,size_t n){size_t i=0;while(i<n&&s[i])i++;return i;}
static int hex(char c){return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:-1;}
bool pd_wifi_credentials_valid(const pd_wifi_credentials *c){
 if(!c)return false;
 size_t n=bounded(c->ssid,sizeof c->ssid),p=bounded(c->password,sizeof c->password);
 if(!n||n>32||p>64||!utf8((const unsigned char*)c->ssid))return false;
 if(c->open)return p==0;
 if(p<8)return false;
 for(size_t i=0;i<p;i++)if((unsigned char)c->password[i]<32||(unsigned char)c->password[i]>126||(p==64&&hex(c->password[i])<0))return false;
 return true;
}
static bool decode(const char *s,size_t n,char *out,size_t cap){
 size_t at=0;for(size_t i=0;i<n;i++){unsigned char c=s[i];if(c=='%'){if(i+2>=n)return false;int a=hex(s[i+1]),b=hex(s[i+2]);if(a<0||b<0)return false;c=(a<<4)|b;i+=2;}else if(c=='+')c=' ';if(!c||c<32||c==127||at+1>=cap)return false;out[at++]=c;}out[at]=0;return true;
}
bool pd_wifi_parse_form(const char *body,size_t size,const char *token,pd_wifi_credentials *out){
 if(!body||!token||!out||!size||size>512||bounded(token,33)!=32||memchr(body,0,size))return false;
 pd_wifi_credentials c={0};unsigned seen=0;char received[33]={0};
 for(size_t start=0;start<size;){size_t end=start;while(end<size&&body[end]!='&')end++;size_t eq=start;while(eq<end&&body[eq]!='=')eq++;if(eq==end)return false;
  char key[16],value[129];if(!decode(body+start,eq-start,key,sizeof key)||!decode(body+eq+1,end-eq-1,value,sizeof value))return false;
  unsigned bit=!strcmp(key,"ssid")?1:!strcmp(key,"password")?2:!strcmp(key,"token")?4:!strcmp(key,"open")?8:0;if(!bit||(seen&bit))return false;seen|=bit;
  size_t n=strlen(value);if(bit==1){if(n>32)return false;memcpy(c.ssid,value,n+1);}else if(bit==2){if(n>64)return false;memcpy(c.password,value,n+1);}else if(bit==4){if(n!=32)return false;memcpy(received,value,33);}else{if(strcmp(value,"1"))return false;c.open=true;}
  memset(value,0,sizeof value);if(end==size)break;start=end+1;if(start==size)return false;
 }
 unsigned diff=0;for(unsigned i=0;i<32;i++)diff|=(unsigned char)received[i]^(unsigned char)token[i];
 if((seen&7)!=7||diff||!pd_wifi_credentials_valid(&c))return false;
 *out=c;memset(&c,0,sizeof c);return true;
}
