#include "damage.h"
#include "magic.h"
#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void setup(pd_state *s){pd_demo(s);s->hp=s->max_hp=999;s->defences.configured=8191;s->magic.count=1;s->magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};s->magic.concentration=1;}
static unsigned adjusted(unsigned flags){if(flags&4)return 0;unsigned n=17;if(flags&1)n/=2;if(flags&2)n*=2;return n;}
static bool applies(unsigned rule,unsigned t){bool m=t&1,a=t&2,s=t&4,d=t&8;switch(rule){case 0:return true;case 1:return m;case 2:return !m;case 3:return a&&!m;case 4:return a&&!m&&!s;case 5:return a&&!m&&!d;default:return a&&!m&&!s&&!d;}}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned j=0;j<4;j++)p[n-4+j]=c>>(8*j);}
static unsigned calls;
static uint32_t rng(void *p){(void)p;calls++;return 0;}
int main(void){
 static pd_state s,before,next,decoded;pd_damage_event_summary r;unsigned cases=0,partial=0;
 /* Full flag truth table: base plus three independent sources and active subsets. */
 for(unsigned type=0;type<13;type++)for(unsigned base=0;base<8;base++)for(unsigned x=0;x<8;x++)for(unsigned y=0;y<8;y++)for(unsigned z=0;z<8;z++){
  setup(&s);assert(pd_defence_set(&s,type,true,base));unsigned flags[]={x,y,z};for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=flags[slot],.active=flags[slot]!=0,.timed=true,.rounds=slot+1}));
  for(unsigned round=0;round<4;round++){
   unsigned expected=base;for(unsigned slot=round;slot<3;slot++)expected|=flags[slot];
   pd_damage_event e={.count=1,.components={{type,17}}};assert(pd_damage_event_preview(&s,&e,&r)&&r.total.configured&&r.groups[0].flags==expected&&r.total.adjusted==adjusted(expected));
   if(round<3){pd_round_summary tick;assert(pd_round_advance(&s,&tick));for(unsigned slot=0;slot<3;slot++){const pd_defence_temporary *d=pd_defence_grant(&s,type,slot);bool advancing=flags[slot]&&slot>=round;uint64_t bit=UINT64_C(1)<<(slot*13+type);assert(!!(tick.defences_advanced&bit)==advancing&&!!(tick.defences_expired&bit)==(advancing&&slot==round));assert(d->rounds==(flags[slot]?(slot>=round?slot-round:0):slot+1));}}
   cases++;
  }
 }
 /* Three conditional grants, all predicates, each property and partial source.
    Expected truth uses independent natural boolean rules over complete sources. */
 for(unsigned prop=0;prop<3;prop++)for(unsigned a=0;a<7;a++)for(unsigned b=0;b<7;b++)for(unsigned c=0;c<7;c++){
  setup(&s);unsigned rules[]={a,b,c};for(unsigned slot=0;slot<3;slot++){pd_defence_temporary d={.flags=1u<<prop,.active=true};d.conditions[prop]=rules[slot];assert(pd_defence_grant_set(&s,12,slot,d));}
  for(unsigned known=0;known<16;known++)for(unsigned values=0;values<16;values++)if(!(values&~known)){
   bool any=false,all=true;for(unsigned full=0;full<16;full++)if((full&known)==values){bool yes=applies(a,full)||applies(b,full)||applies(c,full);any|=yes;all&=yes;}
   pd_damage_event e={.count=1,.components={{12,17}},.contexts={{known,values}}};assert(pd_damage_event_preview(&s,&e,&r)&&r.total.configured==!(any&&!all)&&r.groups[0].flags==(all?1u<<prop:0));if(r.total.configured)assert(r.total.adjusted==adjusted(all?1u<<prop:0));partial++;
  }
 }
 /* All 39 round bits including >31, paused grants, and atomic numeric expiry. */
 setup(&s);for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=slot+1}));
 pd_round_summary tick={0};assert(pd_round_advance(&s,&tick)&&tick.defences_advanced==((UINT64_C(1)<<39)-1)&&tick.defences_expired==8191);
 assert(pd_defence_grant_set(&s,12,2,(pd_defence_temporary){.flags=1,.timed=true,.rounds=1}));before=s;assert(pd_round_advance(&s,&tick)&&pd_defence_grant(&s,12,2)->rounds==1&&!pd_defence_grant(&s,12,2)->active);
 setup(&s);assert(pd_effect_set(&s,0,(pd_effect){.target=PD_EFFECT_ARMOR,.value=10,.active=true})&&pd_effect_set(&s,1,(pd_effect){.target=PD_EFFECT_ARMOR,.value=-20,.active=true,.timed=true,.rounds=1}));s.effects[0].timed=true;s.effects[0].rounds=1;s.effects[1].timed=false;s.effects[1].rounds=0;assert(pd_valid(&s));assert(pd_defence_grant_set(&s,12,2,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1}));before=s;memset(&tick,0x55,sizeof tick);pd_round_summary old=tick;assert(!pd_round_advance(&s,&tick)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&tick,&old,sizeof tick));
 /* Every concentration end trigger clears all bindings, not unbound protection. */
 for(unsigned end=0;end<6;end++){
  setup(&s);for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&s,5,slot,(pd_defence_temporary){.flags=1u<<slot,.active=true,.timed=true,.rounds=2,.concentration=slot<2?1:0}));
  assert(pd_concentration_effects(&s)==2);if(end==0)pd_concentration_end(&s);if(end==1)assert(pd_magic_use(&s,0,0,false));if(end==2)pd_hp(&s,-9999);if(end==3)assert(pd_rest_apply(&s,&(pd_rest_plan){.long_rest=true}));if(end==4){s.magic.count=2;s.magic.powers[1]=(pd_power){.name="Ira",.kind=1,.effect=1,.maximum=1,.remaining=1};s.combat.rage_bonus=2;assert(pd_magic_use(&s,1,0,false)&&s.combat.raging);}if(end==5){pd_damage_event e={.count=1,.components={{0,12}}};pd_roll roll;calls=0;assert(pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&calls==1&&r.total.ended_effects==2);s=next;}
  for(unsigned slot=0;slot<3;slot++){const pd_defence_temporary *d=pd_defence_grant(&s,5,slot);assert(d->active==(slot==2)&&!d->concentration&&d->rounds==2&&d->flags==(1u<<slot));}assert(pd_valid(&s));
 }
 /* Additional grants never make an unknown base implicitly normal. */
 pd_demo(&s);assert(pd_defence_grant_set(&s,12,2,(pd_defence_temporary){.flags=4,.active=true}));pd_damage_event unknown={.count=1,.components={{12,17}}};assert(pd_damage_event_preview(&s,&unknown,&r)&&!r.total.configured&&r.missing==(1u<<12));before=s;assert(!pd_damage_event_apply(&s,&unknown,&r)&&!memcmp(&s,&before,sizeof s));
 /* Maximal linked count and inactive bindings clear in the same end transition. */
 setup(&s);for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&s,type,slot,(pd_defence_temporary){.flags=1,.active=true,.concentration=1}));for(unsigned i=0;i<8;i++)assert(pd_effect_set(&s,i,(pd_effect){.value=1,.active=true,.concentration=1}));assert(pd_concentration_effects(&s)==47);s.defences.additional[12][1].active=false;assert(pd_concentration_effects(&s)==46);pd_concentration_end(&s);assert(!pd_concentration_effects(&s)&&!pd_defence_grant(&s,12,2)->concentration&&pd_valid(&s));
 /* Strict validation and largest record: all 208 appended bytes have schema checks. */
 setup(&s);before=s;assert(!pd_defence_grant(&s,0,3)&&!pd_defence_grant(&s,13,0)&&!pd_defence_grant(NULL,0,0));assert(!pd_defence_grant_set(&s,0,3,(pd_defence_temporary){0})&&!memcmp(&s,&before,sizeof s));
 s.magic.count=120;for(unsigned i=1;i<120;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};
 for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++){pd_defence_temporary d={.flags=7,.active=true,.timed=true,.rounds=99,.concentration=1,.conditions={1,2,6}};assert(pd_defence_grant_set(&s,type,slot,d));}
 static uint8_t data[PD_RECORD_SIZE];size_t n=pd_encode(&s,142,data);uint32_t gen;assert(n==PD_RECORD_SIZE&&!memcmp(data,"PDDT",4)&&pd_decode(data,n,&decoded,&gen)&&gen==142);
 for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(!memcmp(pd_defence_grant(&s,type,slot),pd_defence_grant(&decoded,type,slot),sizeof(pd_defence_temporary)));
 unsigned bad[]={8,2,2,100,2,7,7,7};for(unsigned i=0;i<208;i++){pd_encode(&s,142,data);data[n-822+i]=bad[i%8];crc(data,n);before=decoded;gen=17;assert(!pd_decode(data,n,&decoded,&gen)&&gen==17&&!memcmp(&decoded,&before,sizeof before));}
 setup(&s);n=pd_encode(&s,1,data);data[n-822+5]=1;crc(data,n);assert(!pd_decode(data,n,&decoded,&gen));
 printf("Multiple defences: %u flag/type/expiry cases, %u conditional partial contexts, 39 timers, bound/unbound endings, atomic rollback, strict PDD24 and 208 malformed appended fields passed\n",cases,partial);
}
