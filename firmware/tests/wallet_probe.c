#include "pocketdd.h"
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv){
    if(argc!=2)return 2;
    FILE *f=fopen(argv[1],"r");if(!f)return 2;
    static char line[PD_PACKET_MAX+2];static pd_state state;
    while(fgets(line,sizeof line,f)){
        size_t n=strlen(line);if(n&&line[n-1]=='\n')n--;
        bool ok=pd_parse_packet(line,n,&state);printf("{\"valid\":%s",ok?"true":"false");
        if(ok){printf(",\"available\":%s,\"coins\":[",state.wallet.available?"true":"false");for(unsigned i=0;i<5;i++)printf("%s%u",i?",":"",(unsigned)state.wallet.coins[i]);printf("],\"copper\":%llu",(unsigned long long)pd_wallet_copper(&state.wallet));}
        puts("}");
    }
    fclose(f);return 0;
}
