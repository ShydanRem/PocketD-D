#include "damage.h"
#include "magic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;
static uint32_t rng(void *p){(void)p;calls++;return 0;}
static unsigned subtract(unsigned n,unsigned r){return r<n?n-r:0;}
static unsigned defence(unsigned n,unsigned flags){if(flags&4)return 0;if(flags&1)n/=2;if(flags&2)n*=2;return n;}
static void setup(pd_state *s){pd_demo(s);s->hp=s->max_hp=999;s->temp_hp=7;s->defences.configured=8191;}
static unsigned permutations;
static void permute(pd_state *s,pd_damage_event *e,unsigned pos,const pd_damage_event_summary *expected){
 if(pos==e->count){pd_damage_event_summary r;assert(pd_damage_event_preview(s,e,&r)&&!memcmp(&r,expected,sizeof r));permutations++;return;}
 for(unsigned i=pos;i<e->count;i++){
  pd_damage_component c=e->components[pos];e->components[pos]=e->components[i];e->components[i]=c;
  unsigned before=e->reductions[pos];e->reductions[pos]=e->reductions[i];e->reductions[i]=before;
  permute(s,e,pos+1,expected);
  c=e->components[pos];e->components[pos]=e->components[i];e->components[i]=c;before=e->reductions[pos];e->reductions[pos]=e->reductions[i];e->reductions[i]=before;
 }
}
int main(void){
 static pd_state s,before,next,decoded;pd_damage_event_summary r,original;unsigned cases=0,splits=0;
 const unsigned amounts[]={0,1,2,3,7,11,31,99,999,9999},reductions[]={0,1,2,3,7,10,99,9999};
 /* Independently subtract, apply R/V/I, subtract once, then absorb HP/temp. */
 for(unsigned type=0;type<13;type++)for(unsigned flags=0;flags<8;flags++)for(unsigned a=0;a<10;a++)for(unsigned pre=0;pre<8;pre++)for(unsigned post=0;post<8;post++){
  setup(&s);assert(pd_defence_set(&s,type,true,flags));pd_damage_event e={.count=1,.components={{type,amounts[a]}},.reductions={reductions[pre]},.reduction_after=reductions[post]};
  unsigned reduced=subtract(amounts[a],reductions[pre]),mitigated=defence(reduced,flags),actual=subtract(mitigated,reductions[post]);assert(pd_damage_event_preview(&s,&e,&r));
  assert(r.total.configured&&r.raw==amounts[a]&&r.requested_before==reductions[pre]&&r.requested_after==reductions[post]&&r.reduction_before==amounts[a]-reduced&&r.mitigated==mitigated&&r.reduction_after==mitigated-actual&&r.total.adjusted==actual);
  assert(r.groups[0].amount==amounts[a]&&r.groups[0].reduction==amounts[a]-reduced&&r.groups[0].adjusted==mitigated&&r.total.temp_after==subtract(7,actual)&&r.total.hp_after==subtract(999,subtract(actual,7)));
  before=s;original=r;if(r.total.allowed){assert(pd_damage_event_apply(&s,&e,&r)&&!memcmp(&r,&original,sizeof r)&&s.hp==r.total.hp_after&&s.temp_hp==r.total.temp_after);}else{assert(!pd_damage_event_apply(&s,&e,&r)&&!memcmp(&s,&before,sizeof s)&&!memcmp(&r,&original,sizeof r));}cases++;
 }
 /* Splitting a group conserves the allocated total reduction, never repeats it. */
 for(unsigned flags=0;flags<8;flags++)for(unsigned amount=1;amount<=31;amount++)for(unsigned reduction=0;reduction<=amount+1;reduction++)for(unsigned left=0;left<=amount;left++){
  setup(&s);assert(pd_defence_set(&s,0,true,flags));unsigned applied=reduction<amount?reduction:amount,first=applied<left?applied:left;
  pd_damage_event e={.count=2,.components={{0,left},{0,amount-left}},.reductions={first,applied-first},.reduction_after=3};
  unsigned mitigated=defence(amount-applied,flags);assert(pd_damage_event_preview(&s,&e,&r)&&r.count==1&&r.groups[0].reduction==applied&&r.mitigated==mitigated&&r.total.adjusted==subtract(mitigated,3));splits++;
 }
 /* Canonical groups and the one post reduction are invariant under all 720 orders. */
 setup(&s);pd_damage_event e={.count=6,.reduction_after=3};for(unsigned i=0;i<6;i++){assert(pd_defence_set(&s,i,true,i));e.components[i]=(pd_damage_component){i,i+1};e.reductions[i]=i;}
 assert(pd_damage_event_preview(&s,&e,&original));permute(&s,&e,0,&original);assert(permutations==720);
 /* A final reduction is shared by the impact, not repeated for each damage type. */
 setup(&s);e=(pd_damage_event){.count=2,.components={{0,11},{5,11}},.reductions={3,0},.reduction_after=3};assert(pd_defence_set(&s,0,true,1)&&pd_defence_set(&s,5,true,2));assert(pd_damage_event_preview(&s,&e,&r)&&r.mitigated==26&&r.total.adjusted==23&&r.groups[0].adjusted==4&&r.groups[1].adjusted==22);
 /* Context/profile uncertainty still blocks the entire calculation even at zero. */
 assert(pd_defence_rule_set(&s,0,false,0,PD_RULE_NONMAGICAL));e.reductions[0]=9999;e.reductions[1]=9999;e.reduction_after=9999;before=s;next=s;calls=0;
 assert(pd_damage_event_preview(&s,&e,&r)&&r.context_missing&&!r.total.configured&&!r.total.adjusted&&!r.mitigated&&!r.reduction_after);
 assert(!pd_damage_event_apply(&s,&e,&r)&&!memcmp(&s,&before,sizeof s));pd_roll roll;assert(!pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&!calls&&!memcmp(&next,&before,sizeof next));
 assert(pd_defence_set(&s,0,false,0));assert(pd_damage_event_preview(&s,&e,&r)&&r.missing==1&&!r.total.configured);
 /* Zero effective damage does not consume randomness, end concentration or heal. */
 setup(&s);s.magic.count=1;s.magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};s.magic.concentration=1;e=(pd_damage_event){.count=1,.components={{0,11}},.reduction_after=9999};before=s;calls=0;assert(pd_damage_event_preview(&s,&e,&r)&&r.total.allowed&&!r.total.needs_concentration&&r.reduction_after==11&&!r.total.adjusted);assert(pd_damage_event_apply(&s,&e,&r)&&!memcmp(&s,&before,sizeof s));assert(!pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&!calls);
 /* Reduced damage defines one CON DC, frozen candidate, and impact-time defence. */
 assert(pd_defence_temporary_set(&s,5,(pd_defence_temporary){.flags=1,.active=true,.concentration=1}));s.temp_hp=9;e=(pd_damage_event){.count=2,.components={{0,31},{5,21}},.reductions={2,1},.reduction_after=7};before=s;calls=0;
 assert(pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&calls==1&&r.mitigated==39&&r.reduction_after==7&&r.total.adjusted==32&&r.total.concentration_dc==16&&r.total.ended_effects==1&&r.groups[1].flags==1&&r.groups[1].adjusted==10&&next.hp==976&&!next.temp_hp&&!next.magic.concentration&&!pd_defence_flags(&next,5)&&!memcmp(&s,&before,sizeof s));
 static uint8_t data[PD_RECORD_SIZE];size_t n=pd_encode(&next,150,data);uint32_t gen;assert(pd_decode(data,n,&decoded,&gen)&&gen==150&&decoded.hp==976&&!decoded.temp_hp&&!decoded.magic.concentration);
 /* Every malformed active-row reduction and post field rejects before RNG/mutation. */
 e=(pd_damage_event){.count=6,.components={{0,1},{1,1},{2,1},{3,1},{4,1},{5,1}}};for(unsigned field=0;field<7;field++){
  if(field<6)e.reductions[field]=10000;else e.reduction_after=10000;before=s;next=s;memset(&r,0x55,sizeof r);original=r;calls=0;
  assert(!pd_damage_event_preview(&s,&e,&r)&&!memcmp(&r,&original,sizeof r));assert(!pd_damage_event_apply(&s,&e,&r)&&!memcmp(&s,&before,sizeof s));assert(!pd_concentration_event_roll(&s,&e,0,0,rng,NULL,&next,&roll,&r,false)&&!calls&&!memcmp(&next,&before,sizeof next));
  if(field<6)e.reductions[field]=0;else e.reduction_after=0;
 }
 setup(&s);for(unsigned i=0;i<6;i++){assert(pd_defence_set(&s,i,true,2));e.components[i].amount=9999;}e.reduction_after=9999;assert(pd_damage_event_preview(&s,&e,&r)&&r.raw==59994&&r.mitigated==119988&&r.total.adjusted==109989&&r.total.massive&&!r.total.allowed);
 printf("Reductions: %u independent type/flags/pre/post/HP cases, %u allocation/split cases, 720 permutations, zero/unknown/malformed guards, one CON DC and frozen PDD22 candidate passed\n",cases,splits);
}
