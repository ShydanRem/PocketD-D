#include "pocketdd.h"
#include <string.h>
#include <limits.h>

static bool text(const char *s, size_t cap) {
    size_t n = 0;
    while (n < cap && s[n]) { if ((unsigned char)s[n] < 32 || (unsigned char)s[n] > 126) return false; ++n; }
    return n > 0 && n < cap;
}
void pd_demo(pd_state *s) {
    *s = (pd_state){.name="Elian", .hp=24, .max_hp=32, .level=3, .armor=16,
        .abilities={14,16,14,10,12,8}, .resource_count=1,
        .resources={{.name="Slancio", .current=1, .maximum=1}},.idle_animation=true,.training={.available=true}};
}
bool pd_valid(const pd_state *s) {
    if (!text(s->name, sizeof s->name) || s->level < 1 || s->level > 20 ||
        s->armor < 1 || s->armor > 40 || !s->max_hp || s->max_hp > 999 ||
        s->hp > s->max_hp || s->temp_hp > 999 || s->resource_count > PD_RESOURCES ||
        s->avatar.skin>2 || s->avatar.hair>2 || s->avatar.cloak>2 || s->avatar.style>2) return false;
    for (int i=0;i<6;i++) if (s->abilities[i] < 1 || s->abilities[i] > 30) return false;
    const pd_training *t=&s->training;
    if(t->saves>63||t->half_skills>0x3ffff||t->blocked>0x3fffffff)return false;
    for(int i=0;i<PD_SKILLS;i++)if(t->ranks[i]>2||t->skill_bonus[i]<-30||t->skill_bonus[i]>30)return false;
    for(int i=0;i<6;i++)if(t->save_bonus[i]<-30||t->save_bonus[i]>30||t->check_bonus[i]<-30||t->check_bonus[i]>30)return false;
    for (int i=0;i<s->resource_count;i++) if (!text(s->resources[i].name, PD_RESOURCE_BYTES) ||
        !s->resources[i].maximum || s->resources[i].maximum > 999 ||
        s->resources[i].current > s->resources[i].maximum) return false;
    return true;
}
int pd_modifier(int score) { return score / 2 - 5; }
void pd_hp(pd_state *s, int delta) {
    /* Damage consumes temporary HP first. Healing never creates temporary HP. */
    int64_t amount = delta;
    if (amount < 0) {
        int64_t absorbed = -amount < s->temp_hp ? -amount : s->temp_hp;
        s->temp_hp -= absorbed; amount += absorbed;
    }
    amount += s->hp;
    s->hp = amount < 0 ? 0 : amount > s->max_hp ? s->max_hp : amount;
}
bool pd_resource_change(pd_state *s, unsigned index, int delta) {
    if (index >= s->resource_count) return false;
    int64_t n = (int64_t)s->resources[index].current + delta;
    s->resources[index].current = n < 0 ? 0 : n > s->resources[index].maximum ? s->resources[index].maximum : n;
    return true;
}
static uint8_t d20(pd_random random, void *context) {
    uint32_t n;
    do { n = random(context); } while (n >= UINT32_C(4294967280));
    return n % 20 + 1;
}
pd_roll pd_d20(int bonus, int mode, pd_random random, void *context) {
    pd_roll r = {.count=mode ? 2 : 1, .bonus=bonus};
    r.natural = r.dice[0] = d20(random, context);
    if (mode) {
        r.dice[1] = d20(random, context);
        if ((mode > 0 && r.dice[1] > r.natural) || (mode < 0 && r.dice[1] < r.natural)) r.natural = r.dice[1];
    }
    r.total = r.natural + bonus;
    return r;
}
uint32_t pd_crc32(const uint8_t *data, size_t size) {
    uint32_t crc = UINT32_MAX;
    for (size_t i=0;i<size;i++) {
        crc ^= data[i];
        for (int bit=0;bit<8;bit++) crc = (crc >> 1) ^ (UINT32_C(0xedb88320) & (0u-(crc&1)));
    }
    return ~crc;
}
static void put16(uint8_t **p, unsigned n) { *(*p)++=n; *(*p)++=n>>8; }
static unsigned get16(const uint8_t **p) { unsigned n=(*p)[0]|((*p)[1]<<8); *p+=2; return n; }
static void put32(uint8_t *p, uint32_t n) { for (int i=0;i<4;i++) p[i]=n>>(8*i); }
static uint32_t get32(const uint8_t *p) { return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24); }
void pd_encode(const pd_state *s, uint32_t gen, uint8_t out[PD_RECORD_SIZE]) {
    memset(out, 0, PD_RECORD_SIZE);
    memcpy(out, "PDD3", 4); put32(out+4,gen);
    uint8_t *p=out+8;
    memcpy(p,s->name,PD_NAME_BYTES); p+=PD_NAME_BYTES;
    put16(&p,s->hp); put16(&p,s->max_hp); put16(&p,s->temp_hp);
    *p++=s->level; *p++=s->armor;
    memcpy(p,s->abilities,6); p+=6; *p++=s->resource_count;
    for (int i=0;i<PD_RESOURCES;i++) {
        memcpy(p,s->resources[i].name,PD_RESOURCE_BYTES); p+=PD_RESOURCE_BYTES;
        put16(&p,s->resources[i].current); put16(&p,s->resources[i].maximum);
    }
    *p++=s->avatar.skin;*p++=s->avatar.hair;*p++=s->avatar.cloak;*p++=s->avatar.style;
    *p++=s->idle_animation?1:0;
    const pd_training *t=&s->training;
    *p++=t->available;*p++=t->half_checks;*p++=t->poisoned;*p++=t->strength_advantage;*p++=t->saves;
    put32(p,t->half_skills);p+=4;put32(p,t->blocked);p+=4;
    for(int i=0;i<PD_SKILLS;i++)*p++=t->ranks[i];
    for(int i=0;i<PD_SKILLS;i++)*p++=(uint8_t)(t->skill_bonus[i]+30);
    for(int i=0;i<6;i++)*p++=(uint8_t)(t->save_bonus[i]+30);
    for(int i=0;i<6;i++)*p++=(uint8_t)(t->check_bonus[i]+30);
    put32(out+PD_RECORD_SIZE-4,pd_crc32(out,PD_RECORD_SIZE-4));
}
bool pd_record_known(const uint8_t *data,size_t size) {
    return data&&((size==384&&(!memcmp(data,"PDD1",4)||!memcmp(data,"PDD2",4)))||
                  (size==PD_RECORD_SIZE&&!memcmp(data,"PDD3",4)));
}
bool pd_decode(const uint8_t *data,size_t size,pd_state *s,uint32_t *gen) {
    if (!pd_record_known(data,size) ||
        pd_crc32(data,size-4) != get32(data+size-4)) return false;
    pd_state candidate={0}; const uint8_t *p=data+8;
    memcpy(candidate.name,p,PD_NAME_BYTES); p+=PD_NAME_BYTES;
    candidate.hp=get16(&p); candidate.max_hp=get16(&p); candidate.temp_hp=get16(&p);
    candidate.level=*p++; candidate.armor=*p++;
    memcpy(candidate.abilities,p,6); p+=6; candidate.resource_count=*p++;
    for (int i=0;i<PD_RESOURCES;i++) {
        memcpy(candidate.resources[i].name,p,PD_RESOURCE_BYTES); p+=PD_RESOURCE_BYTES;
        candidate.resources[i].current=get16(&p); candidate.resources[i].maximum=get16(&p);
    }
    candidate.idle_animation=true;
    if(memcmp(data,"PDD1",4)) {
        candidate.avatar=(pd_avatar){p[0],p[1],p[2],p[3]};
        if(p[4]>1)return false;
        candidate.idle_animation=p[4]!=0;
    }
    if(!memcmp(data,"PDD3",4)) {
        p+=5;pd_training *t=&candidate.training;
        for(int i=0;i<4;i++)if(p[i]>1)return false;
        t->available=*p++;t->half_checks=*p++;t->poisoned=*p++;t->strength_advantage=*p++;t->saves=*p++;
        t->half_skills=get32(p);p+=4;t->blocked=get32(p);p+=4;
        for(int i=0;i<PD_SKILLS;i++)t->ranks[i]=*p++;
        for(int i=0;i<PD_SKILLS;i++) { if(*p>60)return false;t->skill_bonus[i]=(int)*p++-30; }
        for(int i=0;i<6;i++) { if(*p>60)return false;t->save_bonus[i]=(int)*p++-30; }
        for(int i=0;i<6;i++) { if(*p>60)return false;t->check_bonus[i]=(int)*p++-30; }
    }
    if (!pd_valid(&candidate)) return false;
    *s=candidate; *gen=get32(data+4); return true;
}
int pd_select(const uint8_t *a,size_t as,const uint8_t *b,size_t bs,pd_state *s,uint32_t *gen) {
    pd_state sa,sb; uint32_t ga=0,gb=0;
    bool va=pd_decode(a,as,&sa,&ga), vb=pd_decode(b,bs,&sb,&gb);
    if (!va && !vb) return -1;
    bool use_b=vb && (!va || (gb != ga && (uint32_t)(gb-ga)<UINT32_C(0x80000000)));
    *s=use_b?sb:sa; *gen=use_b?gb:ga; return use_b?1:0;
}
