#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static pd_state state,next,scratch;static char line[PD_PACKET_MAX+2];
int main(int argc,char **argv){assert(argc==2);FILE *f=fopen(argv[1],"r");assert(f);while(fgets(line,sizeof line,f)){
 assert(pd_parse_packet(line,strlen(line),&state));printf("[");
 for(unsigned kind=0;kind<2;kind++){
  pd_rest_plan plan={.long_rest=kind};pd_rest_summary summary;next=state;
  assert(pd_rest_preview(&state,&plan,&scratch,&summary)&&pd_rest_apply(&next,&plan)&&!memcmp(&scratch,&next,sizeof next));printf("%s[%u,[",kind?",":"",summary.charges);
  for(unsigned i=0;i<next.item_count;i++){pd_item *v=&next.items[i];printf("%s[%u,%u,%u,%u]",i?",":"",v->quantity,v->charges.current,v->charges.maximum,v->charges.recovery);}printf("]]");
 }puts("]");
}fclose(f);}
