#include "damage.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;
static uint32_t face;
static uint32_t rng(void *unused){(void)unused;calls++;return face;}
static unsigned adjust(unsigned amount,unsigned flags){if(flags&4)return 0;if(flags&1)amount/=2;if(flags&2)amount*=2;return amount;}
static void flags(pd_state *s,unsigned type,unsigned value){unsigned bit=1u<<type;s->defences.resistance=(s->defences.resistance&~bit)|((value&1)?bit:0);s->defences.vulnerability=(s->defences.vulnerability&~bit)|((value&2)?bit:0);s->defences.immunity=(s->defences.immunity&~bit)|((value&4)?bit:0);}
static void concentrate(pd_state *s){pd_demo(s);s->magic.count=1;s->magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};s->magic.concentration=1;s->defences.configured=8191;}
int main(void){
 static pd_state s,before,next;pd_damage_event event;pd_damage_event_summary report,reference,untouched;pd_roll roll;unsigned cases=0,splits=0;
 pd_demo(&s);s.hp=s.max_hp=999;s.temp_hp=3;s.defences.configured=8191;
 /* Independent expectations for ordered type pairs, every flag combination,
    odd/even amounts, grouping and rounding before one HP/temp application. */
 for(unsigned a=0;a<13;a++)for(unsigned b=0;b<13;b++)for(unsigned fa=0;fa<8;fa++)for(unsigned fb=0;fb<(a==b?1:8);fb++){
  s.defences.resistance=s.defences.vulnerability=s.defences.immunity=0;flags(&s,a,fa);if(a!=b)flags(&s,b,fb);assert(pd_valid(&s));
  for(unsigned x=1;x<=7;x++)for(unsigned y=1;y<=7;y++){
   event=(pd_damage_event){.count=2,.components={{a,x},{b,y}}};unsigned total=a==b?adjust(x+y,fa):adjust(x,fa)+adjust(y,fb),absorbed=total<3?total:3;
   assert(pd_damage_event_preview(&s,&event,&report)&&report.count==(a==b?1:2)&&report.raw==x+y&&!report.missing&&report.total.configured&&report.total.allowed&&report.total.adjusted==total&&report.total.absorbed==absorbed&&report.total.temp_after==3-absorbed&&report.total.hp_after==999-total+absorbed&&!report.total.needs_concentration);
   if(a==b)assert(report.groups[0].amount==x+y&&report.groups[0].flags==fa&&report.groups[0].adjusted==total);
   else{assert(report.groups[0].type==(a<b?a:b)&&report.groups[1].type==(a<b?b:a));}
   if(x==7&&y==7){next=s;assert(pd_damage_event_apply(&next,&event,&report)&&next.hp==999-total+absorbed&&next.temp_hp==3-absorbed&&pd_valid(&next));}
   cases++;
  }
 }
 /* Splitting the same type into 1..6 rows cannot change rounding, including zeros. */
 for(unsigned type=0;type<13;type++)for(unsigned f=0;f<8;f++)for(unsigned total=0;total<=31;total++)for(unsigned n=1;n<=6;n++){
  pd_demo(&s);s.defences.configured=8191;flags(&s,type,f);event=(pd_damage_event){.count=n};
  for(unsigned i=0;i<n;i++)event.components[i]=(pd_damage_component){type,total/n+(i<total%n)};
  assert(pd_damage_event_preview(&s,&event,&report)&&report.raw==total&&report.count==1&&report.groups[0].amount==total&&report.total.adjusted==adjust(total,f));splits++;
 }
 /* All 720 row permutations yield the same canonical breakdown and total. */
 concentrate(&s);s.temp_hp=99;flags(&s,0,1);flags(&s,5,2);flags(&s,12,4);
 const pd_damage_component parts[]={{0,3},{5,7},{0,4},{12,11},{5,2},{3,1}};
 event=(pd_damage_event){.count=6};memcpy(event.components,parts,sizeof parts);assert(pd_damage_event_preview(&s,&event,&reference));unsigned permutations=0;
 for(unsigned a=0;a<6;a++)for(unsigned b=0;b<6;b++)if(b!=a)for(unsigned c=0;c<6;c++)if(c!=a&&c!=b)for(unsigned d=0;d<6;d++)if(d!=a&&d!=b&&d!=c)for(unsigned e=0;e<6;e++)if(e!=a&&e!=b&&e!=c&&e!=d)for(unsigned f=0;f<6;f++)if(f!=a&&f!=b&&f!=c&&f!=d&&f!=e){
  const unsigned order[]={a,b,c,d,e,f};for(unsigned i=0;i<6;i++)event.components[i]=parts[order[i]];
  assert(pd_damage_event_preview(&s,&event,&report)&&!memcmp(&reference,&report,sizeof report));permutations++;
 }
 assert(permutations==720&&reference.count==4&&reference.total.adjusted==22&&reference.total.concentration_dc==11);
 /* Unknown profiles block the whole impact, including immune or zero-valued rows. */
 for(unsigned missing=0;missing<6;missing++){
  concentrate(&s);s.defences.configured&=~(1u<<missing);event=(pd_damage_event){.count=6};for(unsigned i=0;i<6;i++)event.components[i]=(pd_damage_component){i,i==missing?0:1};
  before=s;assert(pd_damage_event_preview(&s,&event,&report)&&report.missing==(1u<<missing)&&!report.total.configured&&!report.total.allowed&&!report.total.adjusted&&report.total.hp_after==s.hp&&report.total.temp_after==s.temp_hp);
  memset(&report,0x55,sizeof report);untouched=report;assert(!pd_damage_event_apply(&s,&event,&report)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&report,&untouched,sizeof report));
  calls=0;next=before;assert(!pd_concentration_event_roll(&s,&event,0,0,rng,NULL,&next,&roll,&report,false)&&!calls&&!memcmp(&next,&before,sizeof next));
 }
 /* Maximum six 9999 components: totals exceed uint16, no truncation; massive blocked. */
 concentrate(&s);event=(pd_damage_event){.count=6};flags(&s,0,2);for(unsigned i=0;i<6;i++)event.components[i]=(pd_damage_component){0,9999};
 assert(pd_damage_event_preview(&s,&event,&report)&&report.raw==59994&&report.groups[0].amount==59994&&report.total.adjusted==119988&&report.total.massive&&!report.total.allowed);
 before=s;assert(!pd_damage_event_apply(&s,&event,&report)&&!memcmp(&s,&before,sizeof s));flags(&s,0,4);before=s;assert(pd_damage_event_apply(&s,&event,&report)&&!report.total.adjusted&&!memcmp(&s,&before,sizeof s));
 /* One concentration check sees all defences at impact, even when failure ends them.
    A linked maximum is capped once with the result, never between components. */
 for(unsigned mode=0;mode<3;mode++)for(unsigned natural=1;natural<=20;natural++){
  concentrate(&s);s.hp=32;s.temp_hp=99;
  assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=1}));s.hp=41;
  assert(pd_defence_temporary_set(&s,0,(pd_defence_temporary){.flags=1,.active=true,.concentration=1}));assert(pd_defence_temporary_set(&s,5,(pd_defence_temporary){.flags=4,.active=true,.concentration=1}));
  event=(pd_damage_event){.count=3,.components={{0,21},{5,80},{0,4}}};before=s;face=natural-1;calls=0;
  int selected_mode=(int)mode-1;assert(pd_concentration_event_roll(&s,&event,selected_mode,0,rng,NULL,&next,&roll,&report,false)&&calls==(mode==1?1:2)&&report.raw==105&&report.total.adjusted==12&&report.total.concentration_dc==10&&next.temp_hp==87&&!memcmp(&s,&before,sizeof s));
  bool pass=natural+2>=10;assert(next.magic.concentration==(pass?1:0)&&next.hp==(pass?41:32)&&report.total.ended_effects==(pass?0:3)&&pd_defence_flags(&next,5)==(pass?4:0)&&pd_valid(&next));
 }
 /* Aggregate DC, explicit cap 30, manual choice and candidate PDD20 roundtrip. */
 concentrate(&s);s.hp=s.max_hp=999;s.temp_hp=999;event=(pd_damage_event){.count=2,.components={{0,80},{5,60}}};face=19;calls=0;
 assert(pd_concentration_event_roll(&s,&event,0,30,rng,NULL,&next,&roll,&report,false)&&calls==1&&report.total.concentration_dc==70&&!next.magic.concentration&&next.temp_hp==859);
 assert(pd_concentration_event_roll(&s,&event,0,30,rng,NULL,&next,&roll,&report,true)&&report.total.concentration_dc==30&&next.magic.concentration==1&&next.temp_hp==859);
 event.concentration=PD_CONCENTRATION_END;before=s;assert(pd_damage_event_apply(&s,&event,&report)&&s.temp_hp==859&&!s.magic.concentration);static uint8_t data[PD_RECORD_SIZE];uint32_t gen;size_t size=pd_encode(&s,136,data);assert(pd_decode(data,size,&next,&gen)&&gen==136&&next.temp_hp==859&&!next.magic.concentration);
 /* Aggregate thresholds: individually non-massive components may be massive together. */
 concentrate(&s);event=(pd_damage_event){.count=2,.components={{0,28},{5,28}}};assert(pd_damage_event_preview(&s,&event,&report)&&report.total.massive&&!report.total.allowed);
 event.components[0].amount=12;event.components[1].amount=12;assert(pd_damage_event_preview(&s,&event,&report)&&report.total.allowed&&report.total.ends_concentration&&!report.total.needs_concentration&&!report.total.hp_after);
 assert(pd_damage_event_apply(&s,&event,&report)&&!s.hp&&!s.magic.concentration);
 /* Malformed plans/mode/bonus/state reject before RNG and leave all outputs untouched. */
 concentrate(&s);event=(pd_damage_event){.count=2,.components={{0,2},{5,3}}};
 for(unsigned bad=0;bad<5;bad++){
  pd_damage_event invalid=event;if(bad==0)invalid.count=0;if(bad==1)invalid.count=7;if(bad==2)invalid.components[1].type=13;if(bad==3)invalid.components[1].amount=10000;if(bad==4)invalid.concentration=3;
  before=s;next=s;memset(&report,0x33,sizeof report);untouched=report;calls=0;
  assert(!pd_damage_event_preview(&s,&invalid,&report)&&!memcmp(&report,&untouched,sizeof report));assert(!pd_damage_event_apply(&s,&invalid,&report)&&!memcmp(&s,&before,sizeof s));
  assert(!pd_concentration_event_roll(&s,&invalid,0,0,rng,NULL,&next,&roll,&report,false)&&!calls&&!memcmp(&next,&before,sizeof next)&&!memcmp(&report,&untouched,sizeof report));
 }
 calls=0;assert(!pd_concentration_event_roll(&s,&event,2,0,rng,NULL,&next,&roll,&report,false)&&!pd_concentration_event_roll(&s,&event,0,31,rng,NULL,&next,&roll,&report,false)&&!pd_concentration_event_roll(&s,&event,0,0,rng,NULL,&s,&roll,&report,false)&&!calls);
 s.training.available=false;assert(!pd_concentration_event_roll(&s,&event,0,0,rng,NULL,&next,&roll,&report,false)&&!calls);s.training.available=true;s.training.blocked=1u<<26;assert(!pd_concentration_event_roll(&s,&event,0,0,rng,NULL,&next,&roll,&report,false)&&!calls);
 assert(!pd_damage_event_preview(NULL,&event,&report)&&!pd_damage_event_preview(&s,NULL,&report)&&!pd_damage_event_preview(&s,&event,NULL)&&!pd_damage_event_apply(&s,&event,NULL));
 printf("Mixed damage: %u independent type/flag/amount pairs, %u split/rounding cases, 720 permutations, impact snapshot, one CON save, aggregate thresholds, unknown/malformed rejection and PDD20 candidate passed\n",cases,splits);
}
