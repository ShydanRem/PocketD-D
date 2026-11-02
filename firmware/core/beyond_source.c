#include "beyond_runtime.h"
#include <string.h>
bool pd_beyond_id(const char *id){if(!id||id[0]<'1'||id[0]>'9')return false;size_t n=strlen(id);if(n>15)return false;for(size_t i=1;i<n;i++)if(id[i]<'0'||id[i]>'9')return false;return true;}
bool pd_beyond_utf8(const unsigned char *p,size_t n){
    if(!p&&n)return false;
    for(size_t i=0;i<n;){unsigned a=p[i++];if(!a)return false;if(a<128)continue;unsigned extra=a>=0xc2&&a<=0xdf?1:a>=0xe0&&a<=0xef?2:a>=0xf0&&a<=0xf4?3:9;if(extra==9||extra>n-i)return false;unsigned first=p[i];if((a==0xe0&&first<0xa0)||(a==0xed&&first>=0xa0)||(a==0xf0&&first<0x90)||(a==0xf4&&first>=0x90))return false;for(unsigned k=0;k<extra;k++)if((p[i++]&0xc0)!=0x80)return false;}
    return true;
}
