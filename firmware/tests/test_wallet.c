#include "pocketdd.h"
#include "fixtures/state-v23.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static void crc(uint8_t *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(i*8);}
int main(void){
    static pd_state s,before,out;static uint8_t data[PD_RECORD_SIZE];pd_demo(&s);
    pd_wallet unknown={0};assert(!s.wallet.available&&!pd_wallet_change(&s,0,1));
    assert(pd_wallet_set(&s,&unknown));unknown.coins[0]=1;before=s;assert(!pd_wallet_set(&s,&unknown)&&!memcmp(&s,&before,sizeof s));
    pd_wallet wallet={.available=true};assert(pd_wallet_set(&s,&wallet));
    const uint32_t values[]={0,1,99,65535,65536,999999998,PD_COIN_MAX};
    const int32_t deltas[]={INT32_MIN,-1000000000,-1,0,1,999999999,INT32_MAX};unsigned cases=0;
    for(unsigned coin=0;coin<PD_COINS;coin++)for(unsigned v=0;v<7;v++)for(unsigned d=0;d<7;d++){
        wallet.coins[coin]=values[v];assert(pd_wallet_set(&s,&wallet));before=s;
        int64_t expected=(int64_t)values[v]+deltas[d];bool ok=expected>=0&&expected<=999999999;
        assert(pd_wallet_change(&s,coin,deltas[d])==ok);
        if(ok){before.wallet.coins[coin]=(uint32_t)expected;assert(!memcmp(&s,&before,sizeof s));}
        else assert(!memcmp(&s,&before,sizeof s));
        cases++;
    }
    before=s;assert(!pd_wallet_change(&s,5,1)&&!pd_wallet_change(NULL,0,1)&&!pd_wallet_set(NULL,&wallet)&&!pd_wallet_set(&s,NULL)&&!memcmp(&s,&before,sizeof s));
    for(unsigned i=0;i<PD_COINS;i++)wallet.coins[i]=PD_COIN_MAX;
    assert(pd_wallet_copper(&wallet)==UINT64_C(1160999998839)&&!pd_wallet_copper(NULL));
    assert(pd_wallet_set(&s,&wallet));size_t n=pd_encode(&s,42,data);uint32_t generation=0;
    assert(!memcmp(data,"PDDO",4)&&pd_decode(data,n,&out,&generation)&&generation==42&&out.wallet.available&&!memcmp(out.wallet.coins,wallet.coins,sizeof wallet.coins));
    for(unsigned coin=0;coin<5;coin++){
        pd_encode(&s,42,data);uint32_t bad=PD_COIN_MAX+1;for(unsigned b=0;b<4;b++)data[n-24+coin*4+b]=bad>>(b*8);crc(data,n);
        before=out;generation=99;assert(!pd_decode(data,n,&out,&generation)&&generation==99&&!memcmp(&out,&before,sizeof out));
    }
    pd_encode(&s,42,data);data[n-25]=2;crc(data,n);assert(!pd_decode(data,n,&out,&generation));
    data[n-25]=0;crc(data,n);assert(!pd_decode(data,n,&out,&generation));
    assert(pd_decode(golden_v23,sizeof golden_v23,&out,&generation)&&generation==231&&out.roll_conditions==15&&!out.wallet.available);
    for(unsigned i=0;i<5;i++)assert(!out.wallet.coins[i]);
    /* Known zero survives as known, while every previous schema starts unknown. */
    pd_demo(&s);wallet=(pd_wallet){.available=true};assert(pd_wallet_set(&s,&wallet));n=pd_encode(&s,9,data);
    assert(pd_decode(data,n,&out,&generation)&&out.wallet.available&&generation==9);
    for(unsigned i=0;i<120;i++)s.magic.powers[i]=(pd_power){.name="Magia",.flags=PD_PREPARED};s.magic.count=120;
    assert(pd_valid(&s)&&pd_encode(&s,11,data)==PD_RECORD_SIZE&&pd_decode(data,PD_RECORD_SIZE,&out,&generation)&&out.wallet.available&&out.magic.count==120);
    printf("Wallet: %u boundary/delta cases, 64-bit equivalent, unknown/zero, atomic invalid input, CRC-valid corruption and actual PDD23 migration passed\n",cases);
}
