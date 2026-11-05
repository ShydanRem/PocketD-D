#define _POSIX_C_SOURCE 200809L
#include "spell_roll.h"
#include "library.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static pd_state s,before,next;
static pd_backup restored;
static unsigned char data[PD_RECORD_SIZE],legacy[PD_RECORD_SIZE];
static void crc(unsigned char *p,size_t n){uint32_t c=pd_crc32(p,n-4);for(unsigned i=0;i<4;i++)p[n-4+i]=c>>(8*i);}
int main(void){
    unsigned cases=0;pd_demo(&s);s.magic=(pd_magic){.available=true,.count=PD_POWERS};
    for(unsigned i=0;i<PD_POWERS;i++)s.magic.powers[i]=(pd_power){.name="Synthetic formula",.ability=5,.level=1,.flags=PD_PREPARED|PD_USES_SLOT};
    s.abilities[5]=18;before=s;
    for(unsigned index=0;index<PD_POWERS;index++)for(unsigned kind=1;kind<=2;kind++)for(unsigned scaling=0;scaling<=2;scaling++)for(unsigned level=1;level<=20;level++){
        s=before;s.level=level;s.magic.powers[index].level=scaling==2?0:1;
        pd_spell_formula f={.kind=kind,.count=2,.sides=8,.ability=1,.scaling=scaling,.step=scaling?2:0,.bonus=1};
        assert(pd_spell_formula_set(&s,index,f));pd_state *snapshot=&next;*snapshot=s;
        for(unsigned pact=0;pact<2;pact++){
            unsigned slot=scaling==2?0:3;pd_spell_roll_plan p;
            assert(pd_spell_formula_resolve(&s,index,slot,scaling==2?false:pact,&p));
            unsigned extra=scaling==1?2:(scaling==2?(unsigned)((level>=5)+(level>=11)+(level>=17)):0);
            assert(p.count==2+2*extra&&p.sides==8&&p.bonus==5&&p.kind==(kind==1?PD_SPELL_DAMAGE:PD_SPELL_HEAL_OTHER));
            assert(!memcmp(&s,snapshot,sizeof s));cases++;
        }
    }
    /* Exact max-sized current format; old empty records retain their bytes and
       all historical decoders continue to zero the newly introduced field. */
    s=before;for(unsigned i=0;i<PD_POWERS;i++)assert(pd_spell_formula_set(&s,i,(pd_spell_formula){.kind=2,.count=2,.sides=8,.ability=1,.scaling=1,.step=2,.source=i%2}));
    uint32_t gen;size_t n=pd_encode(&s,109,data);assert(n==PD_RECORD_SIZE&&!memcmp(data,PD_FORMULA_RECORD_MAGIC,4)&&pd_decode(data,n,&next,&gen)&&gen==109&&!memcmp(&s,&next,sizeof s));
    size_t old=n-8u*PD_POWERS;memcpy(legacy,data,old-4);memcpy(legacy,PD_RECORD_MAGIC,4);crc(legacy,old);assert(pd_decode(legacy,old,&next,&gen));for(unsigned i=0;i<PD_POWERS;i++)assert(!next.formulas[i].kind);assert(!memcmp(&next,&before,sizeof before));
    size_t same=pd_encode(&next,109,legacy);assert(same==PD_LEGACY_RECORD_SIZE&&!memcmp(legacy,PD_RECORD_MAGIC,4));
    /* A header relabel cannot make an older device accept truncated formulas. */
    memcpy(legacy,data,n);memcpy(legacy,PD_RECORD_MAGIC,4);crc(legacy,n);next=before;gen=100;assert(!pd_decode(legacy,n,&next,&gen)&&gen==100&&!memcmp(&next,&before,sizeof next));
    const unsigned corrupt[]={3,0,7,2,3,0,2,255};
    for(unsigned field=0;field<8;field++){
        pd_encode(&s,109,data);data[n-4-8+field]=corrupt[field];crc(data,n);next=before;gen=100;
        assert(!pd_decode(data,n,&next,&gen)&&gen==100&&!memcmp(&next,&before,sizeof next));
    }
    before=s;pd_spell_formula f=s.formulas[119];f.count=19;assert(!pd_spell_formula_set(&s,119,f)&&!memcmp(&s,&before,sizeof s));
    s.formulas[119].count=18;pd_spell_roll_plan p={.count=99};assert(!pd_spell_formula_resolve(&s,119,9,false,&p)&&p.count==99);s=before;
    s.formulas[119].bonus=60;assert(!pd_spell_formula_resolve(&s,119,1,false,&p)&&p.count==99);s=before;
    pd_history h;pd_history_reset(&h,s.history_epoch);size_t bytes;char *raw=pd_backup_encode(&s,&h,&bytes);assert(raw&&bytes<=110000&&pd_backup_decode(raw,bytes,&restored)&&!memcmp(&s,&restored.state,sizeof s));free(raw);
    /* Two local characters with the same name keep separate formulas and can
       switch/recover through the production SD library on a host filesystem. */
    char tmp[]="/tmp/pdd-formula-XXXXXX";assert(mkdtemp(tmp));char root[192];snprintf(root,sizeof root,"%s/characters",tmp);pd_library l;assert(pd_library_init(&l,root,109,NULL,NULL));unsigned slot;
    s.history_epoch=10;pd_history_reset(&h,10);assert(pd_library_save(&l,0,&s,&h,&slot)==PD_LIBRARY_OK&&slot==1);
    next=s;next.history_epoch=20;next.formulas[119].count=3;pd_history_reset(&h,20);assert(pd_library_save(&l,0,&next,&h,&slot)==PD_LIBRARY_OK&&slot==2);
    assert(pd_library_read(&l,1,&restored)==PD_LIBRARY_OK&&restored.state.formulas[119].count==2&&restored.state.history_epoch==10);
    assert(pd_library_read(&l,2,&restored)==PD_LIBRARY_OK&&restored.state.formulas[119].count==3&&restored.state.history_epoch==20);
    for(unsigned i=1;i<=2;i++){char file[256];snprintf(file,sizeof file,"%s/pg%02ua.pdl",root,i);assert(!unlink(file));}assert(!rmdir(root)&&!rmdir(tmp));
    printf("Spell profiles: %u source/target/level/scaling/payment cases, atomic setter, full PDDf/legacy byte retention, CRC-valid malformed rejection, no truncation/downgrade, backup and two SD characters passed (synthetic host).\n",cases);
}
