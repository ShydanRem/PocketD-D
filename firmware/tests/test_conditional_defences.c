#include "damage.h"
#include "magic.h"
#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;
static uint32_t rng(void *unused){(void)unused;calls++;return 0;}
static bool applies(unsigned rule,unsigned traits){
 bool magic=traits&1,attack=traits&2,silver=traits&4,adamantine=traits&8;
 switch(rule){case 0:return true;case 1:return magic;case 2:return !magic;case 3:return attack&&!magic;case 4:return attack&&!magic&&!silver;case 5:return attack&&!magic&&!adamantine;default:return attack&&!magic&&!silver&&!adamantine;}
}
static unsigned adjusted(unsigned amount,unsigned flags){if(flags&4)return 0;if(flags&1)amount/=2;if(flags&2)amount*=2;return amount;}
static void setup(pd_state *s){pd_demo(s);s->hp=s->max_hp=999;s->temp_hp=9;s->defences.configured=8191;s->magic.count=1;s->magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};s->magic.concentration=1;}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
 static pd_state s,next,before,decoded;pd_damage_event_summary report,original,reference;pd_roll roll;unsigned cases=0;
 /* Independent boolean evaluation of every completion of all 81 partial sources.
    Include absent grants, inactive extras, all 7 predicates, all types/properties. */
 for(unsigned type=0;type<13;type++)for(unsigned property=0;property<3;property++)for(unsigned base=0;base<8;base++)for(unsigned extra=0;extra<8;extra++)for(unsigned active=0;active<2;active++){
  setup(&s);unsigned bit=1u<<property;assert(pd_defence_set(&s,type,true,base?bit:0));if(base)assert(pd_defence_rule_set(&s,type,false,property,base-1));
  pd_defence_temporary d={.flags=extra?bit:0,.active=extra&&active,.timed=true,.rounds=2,.concentration=1};if(extra)d.conditions[property]=extra-1;assert(pd_defence_temporary_set(&s,type,d));
  for(unsigned known=0;known<16;known++)for(unsigned values=0;values<16;values++)if(!(values&~known)){
   pd_damage_event event={.count=1,.components={{type,11}},.contexts={{known,values}}};
   bool any=false,all=true;unsigned needed=0;
   for(unsigned full=0;full<16;full++)if((full&known)==values){bool yes=(base&&applies(base-1,full))||(extra&&active&&applies(extra-1,full));any|=yes;all&=yes;
    for(unsigned trait=1;trait<=8;trait<<=1)if(!(known&trait)){unsigned changed=full^trait;bool other=(base&&applies(base-1,changed))||(extra&&active&&applies(extra-1,changed));if(yes!=other)needed|=trait;}
   }
   assert(pd_damage_event_preview(&s,&event,&report));bool unresolved=any&&!all;unsigned flags=all?bit:0;
   assert(report.groups[0].flags==flags&&report.groups[0].unresolved==(unresolved?bit:0)&&report.context_missing==(unresolved?1:0)&&report.needed[0]==needed&&report.total.configured==!unresolved);
   if(unresolved){assert(!report.total.adjusted&&!report.total.allowed&&report.total.hp_after==999&&report.total.temp_after==9);calls=0;assert(!pd_concentration_event_roll(&s,&event,0,0,rng,NULL,&next,&roll,&report,false)&&!calls);}
   else assert(report.total.adjusted==adjusted(11,flags));
   cases++;
  }
 }
 /* Known immunity makes unknown resistance irrelevant. Complementary sources
    also resolve without making up source properties. */
 setup(&s);assert(pd_defence_set(&s,0,true,5)&&pd_defence_rule_set(&s,0,false,0,PD_RULE_NONMAGICAL));
 pd_damage_event event={.count=1,.components={{0,50}}};assert(pd_damage_event_preview(&s,&event,&report)&&!report.context_missing&&!report.total.adjusted&&report.total.allowed);
 assert(pd_defence_set(&s,0,true,1)&&pd_defence_rule_set(&s,0,false,0,PD_RULE_MAGICAL));pd_defence_temporary d={.flags=1,.active=true,.conditions={PD_RULE_NONMAGICAL}};assert(pd_defence_temporary_set(&s,0,d));assert(pd_damage_event_preview(&s,&event,&report)&&!report.context_missing&&report.total.adjusted==25);
 /* Same-type portions with different applicability stay separate; equal applied
    properties combine once, including partially known but sufficient sources. */
 setup(&s);assert(pd_defence_set(&s,0,true,1)&&pd_defence_rule_set(&s,0,false,0,PD_RULE_NONMAGICAL_ATTACK));
 event=(pd_damage_event){.count=3,.components={{0,3},{0,3},{0,3}},.contexts={{3,2},{15,2},{3,3}}};
 assert(pd_damage_event_preview(&s,&event,&report)&&report.count==2&&report.total.adjusted==6&&report.groups[0].flags==0&&report.groups[0].amount==3&&report.groups[1].flags==1&&report.groups[1].amount==6&&report.groups[1].adjusted==3);
 reference=report;pd_damage_component c=event.components[0];event.components[0]=event.components[2];event.components[2]=c;pd_damage_context ctx=event.contexts[0];event.contexts[0]=event.contexts[2];event.contexts[2]=ctx;
 assert(pd_damage_event_preview(&s,&event,&report)&&!memcmp(&report,&reference,sizeof report));
 /* All flags can be conditional independently in each source, without stacking. */
 setup(&s);assert(pd_defence_set(&s,0,true,7));for(unsigned i=0;i<3;i++)assert(pd_defence_rule_set(&s,0,false,i,i+1));
 for(unsigned traits=0;traits<16;traits++){event=(pd_damage_event){.count=1,.components={{0,11}},.contexts={{15,traits}}};unsigned expected=0;for(unsigned i=0;i<3;i++)if(applies(i+1,traits))expected|=1u<<i;assert(pd_damage_event_preview(&s,&event,&report)&&!report.context_missing&&report.total.adjusted==adjusted(11,expected)&&report.groups[0].flags==expected);}
 /* Death/rage/recast/rest/expiry preserve selectors, but deactivate the extra.
    One concentration roll freezes original applied flags before the end. */
 for(unsigned end=0;end<6;end++){
  setup(&s);assert(pd_defence_set(&s,0,true,0));d=(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1,.concentration=1,.conditions={PD_RULE_NONMAGICAL_ATTACK}};assert(pd_defence_temporary_set(&s,0,d));
  event=(pd_damage_event){.count=2,.components={{0,21},{0,4}},.contexts={{3,2},{3,2}}};before=s;
  if(end==0){calls=0;assert(pd_concentration_event_roll(&s,&event,0,0,rng,NULL,&next,&roll,&report,false)&&calls==1&&report.total.adjusted==12&&report.total.concentration_dc==10&&report.groups[0].flags==1);s=next;}
  if(end==1)pd_concentration_end(&s);
  if(end==2)assert(pd_magic_use(&s,0,0,false));
  if(end==3)assert(pd_rest_apply(&s,&(pd_rest_plan){.long_rest=true}));
  if(end==4)pd_hp(&s,-9999);
  if(end==5){pd_round_summary r;assert(pd_round_advance(&s,&r)&&r.defences_expired==1);}
  assert(!s.defences.temporary[0].active&&s.defences.temporary[0].conditions[0]==PD_RULE_NONMAGICAL_ATTACK&&pd_valid(&s));
  assert(pd_damage_event_preview(&s,&event,&report)&&report.total.adjusted==25&&!report.context_missing);
 }
 /* Incomplete context blocks before entropy; unused/malformed facts or selectors reject. */
 setup(&s);assert(pd_defence_set(&s,0,true,1)&&pd_defence_rule_set(&s,0,false,0,PD_RULE_NEITHER_ATTACK));event=(pd_damage_event){.count=1,.components={{0,1}}};before=s;next=s;memset(&report,0x55,sizeof report);original=report;calls=0;
 assert(!pd_damage_event_apply(&s,&event,&report)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&report,&original,sizeof report));assert(!pd_concentration_event_roll(&s,&event,0,0,rng,NULL,&next,&roll,&report,false)&&!calls&&!memcmp(&next,&before,sizeof next));
 event.contexts[0]=(pd_damage_context){.known=16};assert(!pd_damage_event_preview(&s,&event,&report));event.contexts[0]=(pd_damage_context){.values=1};assert(!pd_damage_event_preview(&s,&event,&report));
 assert(!pd_defence_rule_set(&s,0,false,1,1)&&!pd_defence_rule_set(&s,0,false,0,7)&&!pd_defence_rule_set(&s,13,false,0,1));d=(pd_defence_temporary){.conditions={1}};assert(!pd_defence_temporary_set(&s,0,d));
 assert(pd_defence_set(&s,0,true,3)&&s.defences.conditions[0][0]==PD_RULE_NEITHER_ATTACK&&!s.defences.conditions[0][1]);assert(pd_defence_set(&s,0,true,2)&&!s.defences.conditions[0][0]);
 /* Largest PDD23 and all 78 selector bytes, both scopes, CRC-valid corruption. */
 setup(&s);for(unsigned type=0;type<13;type++){assert(pd_defence_set(&s,type,true,7));d=(pd_defence_temporary){.flags=7,.active=true,.concentration=1};for(unsigned i=0;i<3;i++)d.conditions[i]=(type+i)%7;assert(pd_defence_temporary_set(&s,type,d));for(unsigned i=0;i<3;i++)assert(pd_defence_rule_set(&s,type,false,i,(type+i+1)%7));}
 s.magic.count=PD_POWERS;for(unsigned i=1;i<PD_POWERS;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};static uint8_t data[PD_RECORD_SIZE];size_t n=pd_encode(&s,138,data);uint32_t gen;
 assert(n==PD_RECORD_SIZE&&!memcmp(data,"PDDT",4)&&pd_decode(data,n,&decoded,&gen)&&gen==138);
 for(unsigned type=0;type<13;type++)for(unsigned i=0;i<3;i++)assert(decoded.defences.conditions[type][i]==(type+i+1)%7&&decoded.defences.temporary[type].conditions[i]==(type+i)%7);
 for(unsigned i=0;i<78;i++){pd_encode(&s,138,data);data[n-900+i]=7;crc(data,n);before=decoded;gen=99;assert(!pd_decode(data,n,&decoded,&gen)&&gen==99&&!memcmp(&decoded,&before,sizeof decoded));}
 setup(&s);n=pd_encode(&s,1,data);data[n-900]=1;crc(data,n);assert(!pd_decode(data,n,&decoded,&gen));
 printf("Conditional defences: %u independent rule/source/property/partial-context cases, complementary grants, immunity, grouping, frozen impact, lifecycle and strict PDD24 passed\n",cases);
}
