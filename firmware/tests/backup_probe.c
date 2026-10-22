#include "backup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char **argv){
    if(argc!=2)return 2;
    FILE *file=fopen(argv[1],"r");if(!file)return 2;
    static char line[PD_BACKUP_MAX+2];static pd_backup snapshot;
    while(fgets(line,sizeof line,file)){
        size_t size=strlen(line);if(size&&line[size-1]=='\n')size--;
        bool valid=pd_backup_decode(line,size,&snapshot);
        if(valid)printf("{\"valid\":true,\"hp\":%u,\"maxHp\":%u,\"armor\":%u,\"powers\":%u,\"rolls\":%u}\n",snapshot.state.hp,pd_max_hp(&snapshot.state),pd_armor(&snapshot.state),snapshot.state.magic.count,snapshot.history.count);
        else puts("{\"valid\":false}");
    }
    fclose(file);return 0;
}
