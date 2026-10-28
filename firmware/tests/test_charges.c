#include "damage.h"
#include "rest.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;
static uint32_t rng(void *p){(void)p;calls++;return 0;}
static void setup(pd_state *s){pd_demo(s);s->hp=s->max_hp=999;s->temp_hp=7;s->defences.configured=8191;s->equipped=PD_NO_WEAPON;s->item_count=60;for(unsigned i=0;i<60;i++)s->items[i]=(pd_item){.name="Oggetto sintetico",.quantity=1};}
static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
 static pd_state s,before,next,out,scratch;pd_damage_event_summary r;unsigned cases=0;
 const unsigned maxima[]={1,9,999};
 for(unsigned index=0;index<60;index++)for(unsigned m=0;m<3;m++)for(unsigned balance=0;balance<3;balance++)for(unsigned qty=0;qty<2;qty++)for(unsigned stage=0;stage<2;stage++)for(unsigned reaction=0;reaction<2;reaction++)for(unsigned spent=0;spent<2;spent++)for(unsigned damage=0;damage<2;damage++){
  setup(&s);unsigned maximum=maxima[m],current=balance==0?0:balance==1?maximum/2:maximum;s.items[index].charges=(pd_charges){current,maximum,1};s.items[index].quantity=qty;s.reaction_spent=spent;
  pd_reduction_profile p={.value=3,.types=8191,.stage=stage,.active=true,.cost_kind=PD_COST_ITEM,.cost_index=index,.cost=maximum,.reaction=reaction};assert(pd_reduction_profile_set(&s,index%3,p));
  pd_damage_event e={.count=1,.concentration=PD_CONCENTRATION_KEEP,.components={{0,damage?31:0}},.profile=1+index%3,.allocation={stage?0:3}};before=s;
  assert(pd_damage_event_preview(&s,&e,&r)&&!memcmp(&s,&before,sizeof s));bool allowed=qty&&current>=maximum&&!(reaction&&spent);
  assert(r.cost_unavailable==!qty&&r.cost_missing==(current<maximum)&&r.total.configured==allowed&&r.costs[0].before==current&&r.costs[0].after==(allowed?current-maximum:current));
  assert(pd_damage_event_apply(&s,&e,&r)==allowed);
  if(allowed){assert(s.items[index].quantity==qty&&s.items[index].charges.current==current-maximum&&s.reaction_spent==(spent||reaction));before.items[index].charges.current=current-maximum;before.reaction_spent=spent||reaction;before.hp=s.hp;before.temp_hp=s.temp_hp;assert(!memcmp(&s,&before,sizeof s));}
  else assert(!memcmp(&s,&before,sizeof s));cases++;
 }
 /* Every row and recovery policy; missing objects keep their balances and quantities. */
 for(unsigned policy=0;policy<3;policy++)for(unsigned qty=0;qty<2;qty++)for(unsigned long_rest=0;long_rest<2;long_rest++){
  setup(&s);for(unsigned i=0;i<60;i++){s.items[i].quantity=qty;s.items[i].charges=(pd_charges){0,999,policy};}before=s;pd_rest_plan p={.long_rest=long_rest};pd_rest_summary summary;assert(pd_rest_preview(&s,&p,&scratch,&summary)&&!memcmp(&s,&before,sizeof s));assert(pd_rest_apply(&s,&p)&&!memcmp(&s,&scratch,sizeof s));bool recover=qty&&(policy==1||(long_rest&&policy==2));assert(summary.charges==(recover?59940:0));for(unsigned i=0;i<60;i++)assert(s.items[i].quantity==qty&&s.items[i].charges.current==(recover?999:0));
 }
 /* Manual edits clamp even at INT_MIN/MAX, preserve quantities and reject unavailable pools. */
 setup(&s);s.items[59].charges=(pd_charges){2,7,0};before=s;assert(pd_item_charges_change(&s,59,INT_MIN)&&!s.items[59].charges.current&&pd_item_charges_change(&s,59,INT_MAX)&&s.items[59].charges.current==7);before.items[59].charges.current=7;assert(!memcmp(&s,&before,sizeof s));s.items[59].quantity=0;before=s;assert(!pd_item_charges_change(&s,59,1)&&!pd_item_charges_change(&s,60,1)&&!pd_item_charges_change(&s,0,1)&&!pd_item_charges_change(NULL,0,1)&&!memcmp(&s,&before,sizeof s));
 assert(!pd_reduction_cost_max(NULL,PD_COST_ITEM,0)&&!pd_reduction_cost_max(&s,PD_COST_ITEM,60)&&!pd_reduction_cost_current(&s,PD_COST_ITEM,60)&&!pd_reduction_cost_name(&s,PD_COST_ITEM,60));s.inventory_available=false;assert(!pd_reduction_cost_max(&s,PD_COST_ITEM,59));
 /* Charges, HP, reaction and concentration end are one deterministic candidate. */
 setup(&s);s.magic.count=1;s.magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};s.magic.concentration=1;s.items[59].charges=(pd_charges){7,7,0};assert(pd_reduction_profile_set(&s,0,(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.concentration=1,.cost_kind=PD_COST_ITEM,.cost_index=59,.cost=2,.reaction=true}));pd_damage_event e={.count=1,.components={{0,31}},.profile=1};before=s;pd_roll roll;calls=0;assert(pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&calls==1&&!memcmp(&s,&before,sizeof s)&&next.items[59].charges.current==5&&next.items[59].quantity==1&&next.hp==978&&!next.magic.concentration&&!next.reductions[0].active&&next.reaction_spent);
 s.items[59].quantity=0;before=s;calls=0;assert(!pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&!calls&&!memcmp(&s,&before,sizeof s));
 /* PDD23: all rows, strict ranges, absent/unused pools and old-format enum boundaries. */
 setup(&s);s.magic.count=120;for(unsigned i=0;i<120;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};for(unsigned i=0;i<60;i++)s.items[i].charges=(pd_charges){i,999,i%3};static uint8_t data[PD_RECORD_SIZE];size_t n=pd_encode(&s,210,data);uint32_t gen;assert(n==PD_RECORD_SIZE&&!memcmp(data,PD_RECORD_MAGIC,4)&&pd_decode(data,n,&out,&gen)&&gen==210);for(unsigned i=0;i<60;i++)assert(out.items[i].charges.current==i&&out.items[i].charges.maximum==999&&out.items[i].charges.recovery==i%3);
 for(unsigned i=0;i<60;i++)s.items[i].charges.current=999;
 for(unsigned i=0;i<300;i++){pd_encode(&s,210,data);data[n-439+i]=255;crc(data,n);before=out;gen=7;assert(!pd_decode(data,n,&out,&gen)&&gen==7&&!memcmp(&out,&before,sizeof out));}
 setup(&s);s.item_count=1;for(unsigned i=0;i<3;i++){pd_encode(&s,210,data);n=pd_encoded_size(&s);data[n-439+5+(const unsigned[]){0,2,4}[i]]=1;crc(data,n);assert(!pd_decode(data,n,&out,&gen));}
 s.items[0].charges=(pd_charges){0,0,1};assert(!pd_valid(&s));s.items[0].charges=(pd_charges){1,0,0};assert(!pd_valid(&s));s.items[0].charges=(pd_charges){0,1,3};assert(!pd_valid(&s));
 setup(&s);s.items[0].charges=(pd_charges){1,7,0};assert(pd_reduction_profile_set(&s,0,(pd_reduction_profile){.cost_kind=PD_COST_ITEM,.cost=1}));n=pd_encode(&s,210,data);data[3]='K';size_t old=n-435;crc(data,old);assert(pd_record_known(data,old)&&!pd_decode(data,old,&out,&gen));
 printf("Item charges: %u independent row/capacity/balance/quantity/phase/reaction/damage cases; rest policies, manual limits, one CON candidate, no RNG on rejection and strict PDD24 passed\n",cases);
}
