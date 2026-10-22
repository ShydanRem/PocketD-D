#include "damage.h"
#include "rest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;
static uint32_t rng(void *p){(void)p;calls++;return 0;}
static void setup(pd_state *s){
 pd_demo(s);s->hp=s->max_hp=999;s->temp_hp=7;s->defences.configured=8191;s->equipped=PD_NO_WEAPON;s->resource_count=3;s->magic.count=3;
 for(unsigned i=0;i<3;i++){s->resources[i]=(pd_resource){.name="Contatore sintetico",.current=3,.maximum=999};s->magic.powers[i]=(pd_power){.name="Utilizzo sintetico",.kind=1,.maximum=9,.remaining=3,.recovery=1};s->items[i].charges=(pd_charges){3,999,2};s->magic.slots[i]=(pd_slot){3,9};s->magic.pact[i]=(pd_slot){3,9};}
}
static void balance(pd_state *s,pd_cost c,unsigned n){switch(c.kind){case PD_COST_COUNTER:s->resources[c.index].current=n;break;case PD_COST_POWER:s->magic.powers[c.index].remaining=n;break;case PD_COST_SLOT:s->magic.slots[c.index].current=n;break;case PD_COST_PACT:s->magic.pact[c.index].current=n;break;case PD_COST_ITEM:s->items[c.index].charges.current=n;break;}}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
 static pd_state s,before,expected,next,out;pd_damage_event_summary r;unsigned cases=0;
 for(unsigned a=1;a<PD_COST_COUNT;a++)for(unsigned b=1;b<PD_COST_COUNT;b++)for(unsigned c=1;c<PD_COST_COUNT;c++)for(unsigned profile=0;profile<3;profile++)for(unsigned stage=0;stage<2;stage++)for(unsigned mask=0;mask<8;mask++)for(unsigned reaction=0;reaction<2;reaction++)for(unsigned spent=0;spent<2;spent++)for(unsigned damage=0;damage<2;damage++){
  setup(&s);s.reaction_spent=spent;pd_reduction_profile p={.value=3,.types=8191,.stage=stage,.active=true,.reaction=reaction};const unsigned kinds[]={a,b,c};
  for(unsigned row=0;row<3;row++){pd_cost cost={kinds[row],row,2};assert(pd_profile_cost_set(&p,row,cost));balance(&s,cost,mask&(1u<<row)?1:3);}
  assert(pd_reduction_profile_set(&s,profile,p));before=s;pd_damage_event e={.count=1,.concentration=PD_CONCENTRATION_KEEP,.components={{0,damage?31:0}},.profile=profile+1,.allocation={stage?0:3}};
  assert(pd_damage_event_preview(&s,&e,&r)&&!memcmp(&s,&before,sizeof s));bool allowed=!mask&&!(reaction&&spent);
  assert(r.total.allowed==allowed&&r.cost_missing==(mask!=0)&&r.reaction_missing==(reaction&&spent));
  for(unsigned row=0;row<3;row++){unsigned old=mask&(1u<<row)?1:3;assert(r.costs[row].before==old&&r.costs[row].after==(allowed?old-2:old)&&r.costs[row].missing==!!(mask&(1u<<row))&&!r.costs[row].unavailable&&r.costs[row].name[0]);}
  if(allowed){expected=before;for(unsigned row=0;row<3;row++)balance(&expected,pd_profile_cost(&p,row),1);expected.reaction_spent=spent||reaction;if(damage){expected.temp_hp=0;expected.hp=978;}assert(pd_damage_event_apply(&s,&e,&r)&&!memcmp(&s,&expected,sizeof s));}
  else{pd_damage_event_summary untouched;memset(&untouched,0xa5,sizeof untouched);r=untouched;assert(!pd_damage_event_apply(&s,&e,&r)&&!memcmp(&r,&untouched,sizeof r)&&!memcmp(&s,&before,sizeof s));}cases++;
 }
 /* Repeated sources reject even at different expense positions. No missing-source fallback. */
 for(unsigned kind=1;kind<PD_COST_COUNT;kind++)for(unsigned first=0;first<3;first++)for(unsigned second=first+1;second<3;second++){
  setup(&s);before=s;pd_reduction_profile p={0};pd_profile_cost_set(&p,first,(pd_cost){kind,0,1});pd_profile_cost_set(&p,second,(pd_cost){kind,0,1});assert(!pd_reduction_profile_set(&s,0,p)&&!memcmp(&s,&before,sizeof s));
 }
 setup(&s);before=s;pd_reduction_profile p={0};assert(!pd_profile_cost_set(NULL,0,(pd_cost){0})&&!pd_profile_cost_set(&p,3,(pd_cost){1,0,1})&&!pd_profile_cost(NULL,0).kind&&!pd_profile_cost(&p,3).kind);
 const pd_cost bad[]={{PD_COST_COUNT,0,1},{PD_COST_COUNTER,3,1},{PD_COST_POWER,3,1},{PD_COST_SLOT,9,1},{PD_COST_PACT,9,1},{PD_COST_ITEM,60,1},{PD_COST_NONE,1,0},{PD_COST_NONE,0,1},{PD_COST_COUNTER,0,0},{PD_COST_COUNTER,0,1000}};
 for(unsigned row=0;row<3;row++)for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){p=(pd_reduction_profile){0};pd_profile_cost_set(&p,row,bad[i]);assert(!pd_reduction_profile_set(&s,0,p)&&!memcmp(&s,&before,sizeof s));}
 /* Holes are valid; a spent reaction, unknown source, or absent item blocks all three. */
 setup(&s);p=(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true};pd_profile_cost_set(&p,2,(pd_cost){PD_COST_ITEM,0,2});assert(pd_reduction_profile_set(&s,0,p));pd_damage_event e={.count=1,.components={{0,31}},.profile=1};before=s;assert(pd_damage_event_apply(&s,&e,&r)&&s.items[0].charges.current==1&&s.resources[0].current==3);
 for(unsigned reason=0;reason<4;reason++){
  setup(&s);p=(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.reaction=true};for(unsigned row=0;row<3;row++)pd_profile_cost_set(&p,row,(pd_cost){(const unsigned[]){PD_COST_COUNTER,PD_COST_POWER,PD_COST_ITEM}[row],row,2});
  if(reason==0)s.items[2].quantity=0;if(reason==1)s.reaction_spent=true;if(reason==2)s.defences.configured=0;if(reason==3){p.rule=PD_RULE_MAGICAL;e.contexts[0]=(pd_damage_context){0};}
  assert(pd_reduction_profile_set(&s,0,p));before=s;calls=0;pd_roll roll;assert(pd_damage_event_preview(&s,&e,&r)&&!r.total.configured&&!pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&!calls&&!memcmp(&s,&before,sizeof s));for(unsigned row=0;row<3;row++)assert(r.costs[row].before==r.costs[row].after);
 }
 /* One CON candidate spends all pools, ends bindings and survives PDD23 serialization. */
 setup(&s);s.magic.powers[0]=(pd_power){.name="Concentrazione",.flags=PD_PREPARED|PD_CONCENTRATES};s.magic.concentration=1;
 p=(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.concentration=1,.reaction=true};pd_profile_cost_set(&p,0,(pd_cost){PD_COST_COUNTER,0,2});pd_profile_cost_set(&p,1,(pd_cost){PD_COST_POWER,1,2});pd_profile_cost_set(&p,2,(pd_cost){PD_COST_ITEM,2,2});assert(pd_reduction_profile_set(&s,0,p));before=s;e=(pd_damage_event){.count=1,.components={{0,31}},.profile=1};pd_roll roll;calls=0;
 assert(pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&calls==1&&!memcmp(&s,&before,sizeof s)&&next.resources[0].current==1&&next.magic.powers[1].remaining==1&&next.items[2].charges.current==1&&next.items[2].quantity==before.items[2].quantity&&next.hp==978&&!next.magic.concentration&&!next.reductions[0].active&&next.reaction_spent);
 static uint8_t data[PD_RECORD_SIZE];uint32_t gen;size_t n=pd_encode(&next,220,data);assert(pd_decode(data,n,&out,&gen)&&gen==220&&out.reductions[0].extra_costs[1].kind==PD_COST_ITEM&&out.items[2].charges.current==1);
 /* Last valid power/item/level indices and maximum costs are exact in every position. */
 setup(&s);s.item_count=60;for(unsigned i=3;i<60;i++)s.items[i]=(pd_item){.name="Oggetto sintetico",.quantity=1};s.items[59].charges=(pd_charges){999,999,0};s.magic.count=120;for(unsigned i=3;i<120;i++)s.magic.powers[i]=(pd_power){.name="Risorsa sintetica",.kind=1,.maximum=99,.remaining=99};s.magic.pact[8]=(pd_slot){9,9};
 p=(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true};pd_profile_cost_set(&p,0,(pd_cost){PD_COST_POWER,119,99});pd_profile_cost_set(&p,1,(pd_cost){PD_COST_ITEM,59,999});pd_profile_cost_set(&p,2,(pd_cost){PD_COST_PACT,8,9});assert(pd_reduction_profile_set(&s,2,p));e=(pd_damage_event){.count=1,.concentration=PD_CONCENTRATION_KEEP,.components={{0,0}},.profile=3};assert(pd_damage_event_apply(&s,&e,&r)&&!s.magic.powers[119].remaining&&!s.items[59].charges.current&&!s.magic.pact[8].current&&s.resources[0].current==3&&s.magic.powers[118].remaining==99);
 n=pd_encode(&s,220,data);assert(pd_decode(data,n,&out,&gen)&&out.reductions[2].cost_index==119&&out.reductions[2].extra_costs[0].index==59&&out.reductions[2].extra_costs[1].index==8);
 /* All 24 extension bytes checked with valid CRC; maximum record and old L preserved. */
 setup(&s);s.magic.count=120;for(unsigned i=3;i<120;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};for(unsigned i=0;i<3;i++){p=(pd_reduction_profile){.cost_kind=PD_COST_COUNTER,.cost=1};pd_profile_cost_set(&p,1,(pd_cost){PD_COST_POWER,1,9});pd_profile_cost_set(&p,2,(pd_cost){PD_COST_ITEM,2,999});assert(pd_reduction_profile_set(&s,i,p));}
 n=pd_encode(&s,220,data);assert(n==PD_RECORD_SIZE&&!memcmp(data,"PDDO",4)&&pd_decode(data,n,&out,&gen));
 for(unsigned i=0;i<24;i++){pd_encode(&s,220,data);data[n-50+i]=255;crc(data,n);before=out;gen=5;assert(!pd_decode(data,n,&out,&gen)&&gen==5&&!memcmp(&out,&before,sizeof out));}
 for(unsigned i=0;i<3;i++){pd_encode(&s,220,data);data[n-50+i*8]=PD_COST_COUNTER;data[n-50+i*8+1]=0;data[n-50+i*8+2]=1;crc(data,n);assert(!pd_decode(data,n,&out,&gen));}
 pd_encode(&s,220,data);data[3]='L';size_t old=n-46;crc(data,old);assert(pd_decode(data,old,&out,&gen)&&out.reductions[0].cost_kind==PD_COST_COUNTER&&!out.reductions[0].extra_costs[0].kind&&out.items[2].charges.maximum==999);
 printf("Multiple costs: %u independent type triples/profile/phase/balance/reaction/damage cases, exact atomic debit, holes, duplicate rejection, no RNG on block, CON candidate and strict PDD24 passed\n",cases);
}
