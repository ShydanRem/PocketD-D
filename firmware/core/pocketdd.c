#include "pocketdd.h"
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include "magic.h"

static bool text(const char *s, size_t cap) {
    size_t n = 0;
    while (n < cap && s[n]) { if ((unsigned char)s[n] < 32 || (unsigned char)s[n] > 126) return false; ++n; }
    return n > 0 && n < cap;
}
void pd_demo(pd_state *s) {
    static const pd_state demo = {.name="Elian", .hp=24, .max_hp=32, .level=3, .armor=16,
        .abilities={14,16,14,10,12,8}, .resource_count=1,
        .resources={{.name="Slancio", .current=1, .maximum=1}},.idle_animation=true,.training={.available=true},.inventory_available=true,.item_count=3,.equipped=0,
        .items={{.name="Spada",.damage_type="Tagliente",.quantity=1,.kind=PD_WEAPON,.ability=0,.count=1,.sides=8,.flags=3,.reach=1},
        {.name="Pozione",.quantity=2,.kind=PD_HEALING,.count=2,.sides=4,.healing_modifier=2},
        {.name="Torcia",.quantity=3,.kind=PD_CONSUMABLE}},.magic={.available=true},.shortcuts={1,3,5}};
    *s=demo;
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
    if(s->item_count>PD_ITEMS || (!s->inventory_available&&s->item_count) ||
       (s->equipped!=PD_NO_WEAPON&&(s->equipped>=s->item_count||s->items[s->equipped].kind!=PD_WEAPON||!s->items[s->equipped].quantity)))return false;
    if(s->combat.rage_bonus>10||s->combat.critical_dice>6||(s->combat.raging&&!s->combat.rage_bonus))return false;
    if(s->combat.thrown&&(s->equipped==PD_NO_WEAPON||!s->items[s->equipped].thrown_range))return false;
    for(int i=0;i<s->item_count;i++) {
        const pd_item *v=&s->items[i];
        if(!text(v->name,sizeof v->name)||v->quantity>999||v->kind>PD_WEAPON||v->ability>5||v->flags>15||v->reach>120||v->thrown_range>120||
           v->attack_modifier< -10||v->attack_modifier>10||v->damage_modifier< -10||v->damage_modifier>10||v->healing_modifier<0||v->healing_modifier>30)return false;
        if(v->damage_type[0]&&!text(v->damage_type,sizeof v->damage_type))return false;
        if(v->kind>=PD_HEALING) {
            if(v->count<1||v->count>6||!(v->sides==4||v->sides==6||v->sides==8||v->sides==10||v->sides==12))return false;
            if(v->kind==PD_WEAPON&&!v->reach)return false;
        }
    }
    if(s->recovery.count>PD_HIT_POOLS||(!s->recovery.available&&s->recovery.count))return false;
    for(unsigned i=0;i<s->recovery.count;i++) {
        const pd_hit_die *v=&s->recovery.pools[i];
        if(!text(v->name,sizeof v->name)||!(v->die==6||v->die==8||v->die==10||v->die==12)||!v->maximum||v->maximum>20||v->used>v->maximum)return false;
    }
    return pd_magic_valid(s)&&s->shortcuts[0]<=7&&s->shortcuts[1]<=7&&s->shortcuts[2]<=7;
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
    if(!s->hp) {s->magic.concentration=0;s->combat.raging=false;if(s->magic.available)s->training.strength_advantage=false;}
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
size_t pd_encoded_size(const pd_state *s){return 6050u+552u*s->magic.count;}
size_t pd_encode(const pd_state *s, uint32_t gen, uint8_t out[PD_RECORD_SIZE]) {
    memset(out, 0, pd_encoded_size(s));
    memcpy(out, "PDD6", 4); put32(out+4,gen);
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
    *p++=s->inventory_available;*p++=s->item_count;*p++=s->equipped;
    *p++=s->combat.rage_bonus;*p++=s->combat.critical_dice;*p++=s->combat.raging;*p++=s->combat.power_attack;*p++=s->combat.thrown;
    for(int i=0;i<PD_ITEMS;i++) {
        const pd_item *v=&s->items[i];
        memcpy(p,v->name,41);p+=41;memcpy(p,v->damage_type,25);p+=25;put16(&p,v->quantity);
        *p++=v->kind;*p++=v->ability;*p++=v->count;*p++=v->sides;*p++=v->flags;*p++=v->reach;*p++=v->thrown_range;
        *p++=v->attack_modifier+10;*p++=v->damage_modifier+10;*p++=v->healing_modifier;
    }
    const pd_magic *m=&s->magic;
    *p++=m->available;*p++=m->count;*p++=m->concentration;
    for(int i=0;i<9;i++){*p++=m->slots[i].current;*p++=m->slots[i].maximum;*p++=m->pact[i].current;*p++=m->pact[i].maximum;}
    memcpy(p,s->shortcuts,3);p+=3;
    for(int i=0;i<m->count;i++) {
        const pd_power *v=&m->powers[i];memcpy(p,v->name,41);p+=41;memcpy(p,v->description,PD_POWER_TEXT_BYTES);p+=PD_POWER_TEXT_BYTES;
        *p++=v->kind;*p++=v->level;*p++=v->flags;*p++=v->ability;*p++=v->maximum;*p++=v->remaining;*p++=v->recovery;*p++=v->effect;
        *p++=v->attack_modifier+15;*p++=v->save_modifier+15;
    }
    *p++=s->recovery.available;*p++=s->recovery.count;
    for(unsigned i=0;i<PD_HIT_POOLS;i++) {
        const pd_hit_die *v=&s->recovery.pools[i];memcpy(p,v->name,41);p+=41;*p++=v->die;*p++=v->maximum;*p++=v->used;
    }
    size_t size=pd_encoded_size(s);put32(out+size-4,pd_crc32(out,size-4));return size;
}
bool pd_record_known(const uint8_t *data,size_t size) {
    if(!data)return false;
    if(size==384&&(!memcmp(data,"PDD1",4)||!memcmp(data,"PDD2",4)))return true;
    if(size==512&&!memcmp(data,"PDD3",4))return true;
    if(size==5184&&!memcmp(data,"PDD4",4))return true;
    if(size>=5168&&size<=71408&&!memcmp(data,"PDD5",4)&&data[5123]<=PD_POWERS)return size==5168u+552u*data[5123];
    return size>=6050&&size<=PD_RECORD_SIZE&&!memcmp(data,"PDD6",4)&&data[5123]<=PD_POWERS&&size==6050u+552u*data[5123];
}
static bool decode_into(const uint8_t *data,size_t size,pd_state *allocated,uint32_t *gen) {
    if (!pd_record_known(data,size) ||
        pd_crc32(data,size-4) != get32(data+size-4)) return false;
    memset(allocated,0,sizeof *allocated);
    #define candidate (*allocated)
    candidate.equipped=PD_NO_WEAPON;candidate.shortcuts[0]=1;candidate.shortcuts[1]=3;candidate.shortcuts[2]=5;
    const uint8_t *p=data+8;
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
        if(p[4]>1)goto invalid;
        candidate.idle_animation=p[4]!=0;
    }
    if(size>=512) {
        p+=5;pd_training *t=&candidate.training;
        for(int i=0;i<4;i++)if(p[i]>1)goto invalid;
        t->available=*p++;t->half_checks=*p++;t->poisoned=*p++;t->strength_advantage=*p++;t->saves=*p++;
        t->half_skills=get32(p);p+=4;t->blocked=get32(p);p+=4;
        for(int i=0;i<PD_SKILLS;i++)t->ranks[i]=*p++;
        for(int i=0;i<PD_SKILLS;i++) { if(*p>60)goto invalid;t->skill_bonus[i]=(int)*p++-30; }
        for(int i=0;i<6;i++) { if(*p>60)goto invalid;t->save_bonus[i]=(int)*p++-30; }
        for(int i=0;i<6;i++) { if(*p>60)goto invalid;t->check_bonus[i]=(int)*p++-30; }
    }
    if(!memcmp(data,"PDD4",4)||!memcmp(data,"PDD5",4)||!memcmp(data,"PDD6",4)) {
        if(p[0]>1||p[5]>1||p[6]>1||p[7]>1)goto invalid;
        candidate.inventory_available=*p++;candidate.item_count=*p++;candidate.equipped=*p++;
        candidate.combat.rage_bonus=*p++;candidate.combat.critical_dice=*p++;candidate.combat.raging=*p++;candidate.combat.power_attack=*p++;candidate.combat.thrown=*p++;
        for(int i=0;i<PD_ITEMS;i++) {
            pd_item *v=&candidate.items[i];memcpy(v->name,p,41);p+=41;memcpy(v->damage_type,p,25);p+=25;v->quantity=get16(&p);
            v->kind=*p++;v->ability=*p++;v->count=*p++;v->sides=*p++;v->flags=*p++;v->reach=*p++;v->thrown_range=*p++;
            if(p[0]>20||p[1]>20||p[2]>30)goto invalid;
            v->attack_modifier=(int)*p++-10;v->damage_modifier=(int)*p++-10;v->healing_modifier=*p++;
        }
    }
    if(!memcmp(data,"PDD5",4)||!memcmp(data,"PDD6",4)) {
        pd_magic *m=&candidate.magic;if(*p>1)goto invalid;m->available=*p++;m->count=*p++;m->concentration=*p++;
        for(int i=0;i<9;i++){m->slots[i].current=*p++;m->slots[i].maximum=*p++;m->pact[i].current=*p++;m->pact[i].maximum=*p++;}
        memcpy(candidate.shortcuts,p,3);p+=3;
        for(int i=0;i<m->count;i++) {
            pd_power *v=&m->powers[i];memcpy(v->name,p,41);p+=41;memcpy(v->description,p,PD_POWER_TEXT_BYTES);p+=PD_POWER_TEXT_BYTES;
            v->kind=*p++;v->level=*p++;v->flags=*p++;v->ability=*p++;v->maximum=*p++;v->remaining=*p++;v->recovery=*p++;v->effect=*p++;
            if(p[0]>45||p[1]>45)goto invalid;
            v->attack_modifier=(int)*p++-15;v->save_modifier=(int)*p++-15;
        }
    }
    if(!memcmp(data,"PDD6",4)) {
        if(*p>1)goto invalid;
        candidate.recovery.available=*p++;candidate.recovery.count=*p++;
        for(unsigned i=0;i<PD_HIT_POOLS;i++) {
            pd_hit_die *v=&candidate.recovery.pools[i];memcpy(v->name,p,41);p+=41;v->die=*p++;v->maximum=*p++;v->used=*p++;
        }
    }
    if (!pd_valid(&candidate)) goto invalid;
    *gen=get32(data+4);return true;
invalid:
    return false;
    #undef candidate
}
bool pd_decode(const uint8_t *data,size_t size,pd_state *s,uint32_t *gen) {
    if(!s||!gen||!pd_record_known(data,size))return false;
    pd_state *candidate=malloc(sizeof *candidate);if(!candidate)return false;
    uint32_t sequence;bool ok=decode_into(data,size,candidate,&sequence);
    if(ok){*s=*candidate;*gen=sequence;}free(candidate);return ok;
}
int pd_select(const uint8_t *a,size_t as,const uint8_t *b,size_t bs,pd_state *s,uint32_t *gen) {
    pd_state *states=malloc(2*sizeof *states);if(!states)return -1;
    uint32_t ga=0,gb=0;
    bool va=decode_into(a,as,&states[0],&ga),vb=decode_into(b,bs,&states[1],&gb);
    if(!va&&!vb){free(states);return -1;}
    bool use_b=vb&&(!va||(gb!=ga&&(uint32_t)(gb-ga)<UINT32_C(0x80000000)));
    *s=states[use_b?1:0];*gen=use_b?gb:ga;free(states);return use_b?1:0;
}
