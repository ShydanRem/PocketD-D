/* Allocation failures must not turn a newer valid bank into an older recovery. */
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned allocation,fail_at;
static void *allocate(size_t size){unsigned n=allocation++;return n==fail_at?NULL:malloc(size);}
#define malloc allocate
#include "../core/pocketdd.c"
#undef malloc
int main(void){
 static pd_state state,out,before;static uint8_t a[PD_RECORD_SIZE],b[PD_RECORD_SIZE];pd_demo(&state);size_t size_a=pd_encode(&state,1,a);state.hp=10;size_t size_b=pd_encode(&state,2,b);
 pd_demo(&before);out=before;uint32_t gen=99;allocation=0;fail_at=0;
 assert(!pd_decode(b,size_b,&out,&gen)&&gen==99&&!memcmp(&out,&before,sizeof out));
 for(unsigned failure=0;failure<5;failure++) {
  out=before;gen=99;allocation=0;fail_at=failure;int selected=pd_select(a,size_a,b,size_b,&out,&gen);
  assert(selected==-1||selected==1);
  if(selected<0)assert(gen==99&&!memcmp(&out,&before,sizeof out));else assert(gen==2&&out.hp==10);
 }
 puts("Allocation faults: unchanged output on decode/select failure; no silent fallback over a newer valid bank passed");
}
