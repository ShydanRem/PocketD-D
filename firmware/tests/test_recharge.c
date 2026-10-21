#include "recharge.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static unsigned calls,face;
static uint32_t rng(void *p){(void)p;calls++;return face-1;}
int main(void){
 static pd_state s,before,next,expected,out;pd_pool roll;unsigned gained,cases=0;const unsigned sides[]={4,6,8,10,12,20},maxima[]={1,9,999};
 for(unsigned index=0;index<60;index++)for(unsigned d=0;d<6;d++)for(unsigned count=1;count<=6;count++)for(unsigned m=0;m<3;m++)for(unsigned start=0;start<2;start++)for(unsigned b=0;b<4;b++)for(unsigned high=0;high<2;high++){
  pd_demo(&s);s.item_count=60;for(unsigned i=3;i<60;i++)s.items[i]=(pd_item){.name="Oggetto sintetico",.quantity=1};s.items[index].charges=(pd_charges){start?maxima[m]-1:0,maxima[m],index%3};s.items[index].quantity=1;before=s;
  pd_recharge_plan p={count,sides[d],(const int[]){-15,-1,0,60}[b]};face=high?sides[d]:1;calls=0;int total=(int)(face*count)+p.bonus;if(total<0)total=0;unsigned space=maxima[m]-s.items[index].charges.current,wanted=(unsigned)total<space?(unsigned)total:space;
  assert(pd_item_recharge(&s,index,&p,rng,NULL,&next,&roll,&gained)&&calls==count&&!memcmp(&s,&before,sizeof s));expected=s;expected.items[index].charges.current+=wanted;assert(!memcmp(&next,&expected,sizeof next)&&gained==wanted&&roll.count==count&&roll.sides==sides[d]&&roll.sum==(int)(face*count)&&roll.bonus==p.bonus&&roll.total==total&&roll.damage&&pd_valid(&next));for(unsigned i=0;i<count;i++)assert(roll.dice[i]==face);cases++;
 }
 /* Rejection happens before RNG and leaves every output untouched. */
 pd_demo(&s);s.items[0].charges=(pd_charges){2,7,1};pd_recharge_plan good={1,6,0};const pd_recharge_plan bad[]={{0,6,0},{7,6,0},{1,3,0},{1,6,-16},{1,6,61},{UINT_MAX,20,0},{1,UINT_MAX,0},{1,6,INT_MAX},{1,6,INT_MIN}};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){
  before=s;memset(&next,0xa5,sizeof next);expected=next;memset(&roll,0xa5,sizeof roll);pd_pool untouched=roll;gained=123;calls=0;assert(!pd_item_recharge(&s,0,&bad[i],rng,NULL,&next,&roll,&gained)&&!calls&&gained==123&&!memcmp(&roll,&untouched,sizeof roll)&&!memcmp(&next,&expected,sizeof next)&&!memcmp(&s,&before,sizeof s));
 }
 for(unsigned reason=0;reason<6;reason++){
  pd_demo(&s);s.items[0].charges=(pd_charges){2,7,1};unsigned index=0;if(reason==0)s.items[0].charges.current=7;if(reason==1){s.items[0].quantity=0;s.equipped=PD_NO_WEAPON;}if(reason==2)s.items[0].charges=(pd_charges){0};if(reason==3)index=60;if(reason==4){s.item_count=0;s.inventory_available=false;s.equipped=PD_NO_WEAPON;s.items[0].charges=(pd_charges){0};}if(reason==5)s.items[0].charges.current=8;
  before=s;next=s;pd_pool untouched;memset(&untouched,0xa5,sizeof untouched);roll=untouched;gained=123;calls=0;assert(!pd_item_recharge(&s,index,&good,rng,NULL,&next,&roll,&gained)&&!calls&&!memcmp(&s,&before,sizeof s)&&!memcmp(&next,&before,sizeof next)&&!memcmp(&roll,&untouched,sizeof roll)&&gained==123);
 }
 pd_demo(&s);s.items[0].charges=(pd_charges){2,7,1};before=s;calls=0;assert(!pd_item_recharge(NULL,0,&good,rng,NULL,&next,&roll,&gained)&&!pd_item_recharge(&s,0,NULL,rng,NULL,&next,&roll,&gained)&&!pd_item_recharge(&s,0,&good,NULL,NULL,&next,&roll,&gained)&&!pd_item_recharge(&s,0,&good,rng,NULL,&s,&roll,&gained)&&!pd_item_recharge(&s,0,&good,rng,NULL,NULL,&roll,&gained)&&!pd_item_recharge(&s,0,&good,rng,NULL,&next,NULL,&gained)&&!pd_item_recharge(&s,0,&good,rng,NULL,&next,&roll,NULL)&&!calls&&!memcmp(&s,&before,sizeof s));
 /* Candidate persists as the existing PDD23; the original and unrelated pools remain exact. */
 face=6;assert(pd_item_recharge(&s,0,&good,rng,NULL,&next,&roll,&gained)&&gained==5&&next.items[0].charges.current==7);static uint8_t data[PD_RECORD_SIZE];uint32_t gen;size_t n=pd_encode(&next,230,data);assert(pd_decode(data,n,&out,&gen)&&gen==230&&out.items[0].charges.current==7&&out.items[0].quantity==1&&!memcmp(&s,&before,sizeof s));
 printf("Recharge: %u independent item/die/count/capacity/balance/bonus/face cases, zero floor, excess cap, exact candidates, rejection before RNG and unchanged PDD23 persistence passed\n",cases);
}
