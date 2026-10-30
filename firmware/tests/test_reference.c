#include "magic.h"
#include "catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state s,before,decoded;
static uint8_t record[PD_RECORD_SIZE];
static void crc(size_t n){uint32_t c=pd_crc32(record,n-4);for(unsigned i=0;i<4;i++)record[n-4+i]=c>>(i*8);}
int main(void){
 unsigned cases=0;
 for(unsigned key=0;key<7;key++)for(int bonus=-15;bonus<=30;bonus++){
  pd_demo(&s);memset(&s.magic,0,sizeof s.magic);s.magic.available=true;s.magic.count=1;
  s.magic.powers[0]=(pd_power){.name="Reference synthetic",.description="Snapshot only.",.kind=1,.flags=PD_PASSIVE|PD_PREPARED,.level=key,.effect=key==6?5:4,.attack_modifier=key==6?bonus:0};assert(pd_valid(&s));before=s;assert(!pd_magic_use(&s,0,1,false)&&!memcmp(&s,&before,sizeof s));assert(pd_catalog_count(&s,PD_CATALOG_POWERS,"")==0);
  size_t n=pd_encode(&s,78,record);uint32_t gen=0;assert(n&&pd_decode(record,n,&decoded,&gen)&&gen==78&&!memcmp(&s,&decoded,sizeof s));
  for(unsigned tag='Y';tag<='Z';tag++){record[3]=tag;crc(n-PD_EQUIPMENT_BYTES);before=decoded;gen=5;assert(!pd_decode(record,n-PD_EQUIPMENT_BYTES,&decoded,&gen)&&gen==5&&!memcmp(&before,&decoded,sizeof before));}cases++;
 }
 pd_demo(&s);s.magic.available=true;s.magic.count=120;for(unsigned i=0;i<120;i++)s.magic.powers[i]=(pd_power){.name="Reference",.kind=1,.flags=PD_PASSIVE|PD_PREPARED,.effect=4,.level=i%6};assert(pd_valid(&s));size_t n=pd_encode(&s,78,record);uint32_t gen;assert(n==PD_RECORD_SIZE&&pd_decode(record,n,&decoded,&gen)&&!memcmp(&s,&decoded,sizeof s));
 for(unsigned field=0;field<8;field++){before=s;pd_power *p=&s.magic.powers[119];switch(field){case 0:p->kind=0;break;case 1:p->flags|=PD_USES_SLOT;break;case 2:p->flags&=~PD_PASSIVE;break;case 3:p->maximum=1;break;case 4:p->remaining=1;break;case 5:p->recovery=1;break;case 6:p->level=6;break;case 7:p->attack_modifier=1;break;}assert(!pd_valid(&s));s=before;}
 puts("Reference: 322 key/bonus states, passive-only/excluded catalog, no spending, max120, same-size PDD36 persistence and no older-tag smuggling passed");printf("Reference cases: %u\n",cases);
}
