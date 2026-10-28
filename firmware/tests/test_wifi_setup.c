#include "wifi_setup.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static const char token[]="0123456789abcdef0123456789abcdef";
static pd_wifi_credentials c,before;
static void check(const char *form,bool okay){c=before;assert(pd_wifi_parse_form(form,strlen(form),token,&c)==okay);if(!okay)assert(!memcmp(&c,&before,sizeof c));}
int main(void){
 memset(&before,0x5a,sizeof before);
 check("ssid=Casa&password=12345678&token=0123456789abcdef0123456789abcdef",true);assert(!strcmp(c.ssid,"Casa")&&!strcmp(c.password,"12345678")&&!c.open);
 check("token=0123456789abcdef0123456789abcdef&ssid=Caff%C3%A8+%26+%2B&password=&open=1",true);assert(!strcmp(c.ssid,"Caffè & +")&&c.open&&!c.password[0]);
 const char *bad[]={"ssid=Casa&password=1234567&token=0123456789abcdef0123456789abcdef","ssid=&password=12345678&token=0123456789abcdef0123456789abcdef","ssid=Casa&password=12345678&token=1123456789abcdef0123456789abcdef","ssid=Casa&ssid=Altro&password=12345678&token=0123456789abcdef0123456789abcdef","ssid=Casa&password=&token=0123456789abcdef0123456789abcdef","ssid=Casa&password=12345678&open=1&token=0123456789abcdef0123456789abcdef","ssid=%00Casa&password=12345678&token=0123456789abcdef0123456789abcdef","ssid=%C0%AF&password=12345678&token=0123456789abcdef0123456789abcdef","ssid=%ED%A0%80&password=12345678&token=0123456789abcdef0123456789abcdef","ssid=Casa&password=12345678&token=0123456789abcdef0123456789abcdef&","ssid=%GG&password=12345678&token=0123456789abcdef0123456789abcdef","ssid=Casa&password=12345678&token=0123456789abcdef0123456789abcdef&extra=1","ssid=Casa&password=12345678&token=0123456789abcdef0123456789abcdef&open=0"};
 for(unsigned i=0;i<sizeof bad/sizeof bad[0];i++)check(bad[i],false);
 c=(pd_wifi_credentials){.ssid="Cafe",.password="12345678"};assert(pd_wifi_credentials_valid(&c));memset(c.ssid,'a',32);c.ssid[32]=0;assert(pd_wifi_credentials_valid(&c));c.ssid[32]='a';assert(!pd_wifi_credentials_valid(&c));c.ssid[32]=0;memset(c.password,'A',64);c.password[64]=0;assert(pd_wifi_credentials_valid(&c));c.password[0]='G';assert(!pd_wifi_credentials_valid(&c));c.password[0]='A';c.open=true;assert(!pd_wifi_credentials_valid(&c));memset(c.password,0,sizeof c.password);assert(pd_wifi_credentials_valid(&c));
 /* Exercise malformed percent, UTF-8 and arbitrary form boundaries, deterministically. */
 unsigned seed=12345;char noise[513];for(unsigned i=0;i<20000;i++){unsigned n=i%513;for(unsigned j=0;j<n;j++){seed=seed*1664525u+1013904223u;noise[j]=seed>>24;}c=before;if(!pd_wifi_parse_form(noise,n,token,&c))assert(!memcmp(&c,&before,sizeof c));else assert(pd_wifi_credentials_valid(&c));}
 puts("Wi-Fi form: UTF-8 SSID/byte limits, WPA2/open/hex PSK, exact token, duplicates/unknown/control rejection, unchanged failure and 20,000 malformed bodies passed; no radio simulated as tested");
}
