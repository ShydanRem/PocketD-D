#include "spell_roll.h"
#include "backup.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pd_state s,decoded;
static pd_backup restored;
static unsigned char record[PD_RECORD_SIZE];
int main(int argc,char **argv){
    static char line[PD_PACKET_MAX+2];bool written=false;FILE *input=argc>1?fopen(argv[1],"rb"):stdin;assert(input);
    while(fgets(line,sizeof line,input)){
        if(!pd_parse_packet(line,strlen(line),&s)){puts("{\"valid\":false}");continue;}
        uint32_t generation;size_t n=pd_encode(&s,43,record);
        assert(pd_decode(record,n,&decoded,&generation)&&generation==43&&!memcmp(&s,&decoded,sizeof s));
        pd_history h;pd_history_reset(&h,s.history_epoch);size_t size;char *raw=pd_backup_encode(&s,&h,&size);
        assert(raw&&pd_backup_decode(raw,size,&restored)&&!memcmp(&s,&restored.state,sizeof s));
        if(argc==3&&!written){FILE *f=fopen(argv[2],"wb");assert(f&&fwrite(raw,1,size,f)==size);fclose(f);written=true;}free(raw);
        printf("{\"valid\":true,\"formulas\":[");
        for(unsigned i=0;i<s.magic.count;i++){pd_spell_formula f=s.formulas[i];printf("%s[%u,%u,%u,%u,%u,%u,%u,%d]",i?",":"",f.kind,f.count,f.sides,f.ability,f.scaling,f.step,f.source,f.bonus);}
        printf("],\"resolved\":[");bool comma=false;
        for(unsigned i=0;i<s.magic.count;i++)if(s.formulas[i].kind){
            unsigned minimum=s.magic.powers[i].level&&(s.magic.powers[i].flags&PD_USES_SLOT)?s.magic.powers[i].level:0,maximum=minimum?9:0;
            for(unsigned level=minimum;level<=maximum;level++){
                pd_spell_roll_plan p;printf("%s[%u,%u,",comma?",":"",i,level);comma=true;
                if(pd_spell_formula_resolve(&s,i,level,false,&p))printf("%u,%u,%u,%d]",p.kind==PD_SPELL_DAMAGE?1u:2u,p.count,p.sides,p.bonus);else printf("null]");
            }
        }
        puts("]}");
    }
    if(input!=stdin)fclose(input);
}
