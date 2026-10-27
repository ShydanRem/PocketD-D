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
static bool turn_name_valid(const char name[PD_TURN_NAME_BYTES]){
    unsigned n=0;while(n<PD_TURN_NAME_BYTES&&name[n]){if((unsigned char)name[n]<32||(unsigned char)name[n]>126)return false;n++;}
    if(n==PD_TURN_NAME_BYTES||(n&&(name[0]==' '||name[n-1]==' ')))return false;
    for(unsigned i=n;i<PD_TURN_NAME_BYTES;i++)if(name[i])return false;
    return true;
}
bool pd_turn_name_set(pd_state *s,unsigned owner,const char *name){
    if(!s||!name||!owner||owner>=PD_TURN_SUBJECTS||!pd_valid(s))return false;
    unsigned n=0;while(n<PD_TURN_NAME_BYTES&&name[n]){if((unsigned char)name[n]<32||(unsigned char)name[n]>126)return false;n++;}
    if(n==PD_TURN_NAME_BYTES)return false;
    unsigned first=0;while(first<n&&name[first]==' ')first++;while(n>first&&name[n-1]==' ')n--;
    char clean[PD_TURN_NAME_BYTES]={0};memcpy(clean,name+first,n-first);memcpy(s->turn_names[owner-1],clean,sizeof clean);return true;
}
void pd_demo(pd_state *s) {
    static const pd_state demo = {.name="Elian", .hp=24, .max_hp=32, .level=3, .armor=16,
        .abilities={14,16,14,10,12,8}, .resource_count=1,
        .resources={{.name="Slancio", .current=1, .maximum=1}},.idle_animation=true,.training={.available=true},.inventory_available=true,.item_count=3,.equipped=0,
        .items={{.name="Spada",.damage_type="Tagliente",.quantity=1,.kind=PD_WEAPON,.ability=0,.count=1,.sides=8,.flags=3,.reach=1},
        {.name="Pozione",.quantity=2,.kind=PD_HEALING,.count=2,.sides=4,.healing_modifier=2},
        {.name="Torcia",.quantity=3,.kind=PD_CONSUMABLE}},.magic={.available=true},.shortcuts={1,3,5},.brightness=50};
    *s=demo;
}
bool pd_identity_set(pd_state *s,unsigned lineage,unsigned body){
    if(!s||!pd_valid(s)||lineage>=12||body>1)return false;
    s->lineage=lineage;s->body=body;return true;
}
const char *pd_scene_name(unsigned scene){
    static const char *const names[]={"Accampamento","Rovine lunari","Porto all'alba"};
    return scene<PD_SCENES?names[scene]:"Scena non valida";
}
bool pd_appearance_set(pd_state *s,const pd_avatar *avatar,unsigned scene){
    if(!s||!avatar||!pd_valid(s)||avatar->skin>2||avatar->hair>2||avatar->cloak>2||avatar->style>2||scene>=PD_SCENES)return false;
    s->avatar=*avatar;s->scene=scene;return true;
}
int pd_effect_bonus(const pd_state *s,unsigned target) {
    int total=0;
    for(unsigned i=0;i<s->effect_count&&i<PD_EFFECTS;i++)if(s->effects[i].active&&s->effects[i].target==target)total+=s->effects[i].value;
    return total;
}
unsigned pd_armor(const pd_state *s){return s->armor+pd_effect_bonus(s,PD_EFFECT_ARMOR);}
unsigned pd_max_hp(const pd_state *s){return s->max_hp+pd_effect_bonus(s,PD_EFFECT_MAX_HP);}
static int unbound_total(const pd_state *s,unsigned target) {
    int total=target==PD_EFFECT_ARMOR?s->armor:s->max_hp;
    for(unsigned i=0;i<s->effect_count&&i<PD_EFFECTS;i++)if(s->effects[i].active&&!s->effects[i].concentration&&s->effects[i].target==target)total+=s->effects[i].value;
    return total;
}
unsigned pd_unconcentrated_max_hp(const pd_state *s){return unbound_total(s,PD_EFFECT_MAX_HP);}
static pd_defence_temporary *grant(pd_state *s,unsigned type,unsigned slot) {
    return slot?&s->defences.additional[type][slot-1]:&s->defences.temporary[type];
}
const pd_defence_temporary *pd_defence_grant(const pd_state *s,unsigned type,unsigned slot) {
    if(!s||type>=PD_DAMAGE_TYPES||slot>=PD_DEFENCE_SLOTS)return NULL;
    return slot?&s->defences.additional[type][slot-1]:&s->defences.temporary[type];
}
unsigned pd_concentration_effects(const pd_state *s) {
    unsigned count=0;for(unsigned i=0;i<s->effect_count&&i<PD_EFFECTS;i++)if(s->effects[i].concentration&&s->effects[i].active)count++;
    for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++){const pd_defence_temporary *d=pd_defence_grant(s,i,slot);if(d->active&&d->concentration)count++;}
    for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++)if(s->reductions[i].active&&s->reductions[i].concentration)count++;
    return count;
}
void pd_concentration_end(pd_state *s) {
    for(unsigned i=0;i<s->effect_count&&i<PD_EFFECTS;i++)if(s->effects[i].concentration){s->effects[i].active=false;s->effects[i].concentration=0;}
    for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++){pd_defence_temporary *d=grant(s,i,slot);if(d->concentration){d->active=false;d->concentration=0;}}
    for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++)if(s->reductions[i].concentration){s->reductions[i].active=false;s->reductions[i].concentration=0;}
    s->magic.concentration=0;
    if(s->hp>pd_max_hp(s))s->hp=pd_max_hp(s);
}
static bool duration_valid(bool timed,bool active,unsigned tick,unsigned rounds,unsigned minutes,unsigned owner){
    if(tick>=PD_TICK_COUNT||owner>=PD_TURN_SUBJECTS||(owner&&(!timed||(tick!=PD_TICK_START&&tick!=PD_TICK_END))))return false;
    if(!timed)return !tick&&!rounds&&!minutes;
    if(tick>=PD_TICK_MINUTES)return !rounds&&minutes<=(tick==PD_TICK_MINUTES?99:5940)&&(!active||minutes);
    return !minutes&&rounds<=99&&(!active||rounds);
}
static bool effects_valid(const pd_state *s) {
    if(s->effect_count>PD_EFFECTS)return false;
    for(unsigned i=0;i<s->effect_count;i++) {
        const pd_effect *e=&s->effects[i];int bound=e->target==PD_EFFECT_ARMOR?20:999;
        if(e->target>PD_EFFECT_MAX_HP||e->value< -bound||e->value>bound||e->rounds>99||
           !duration_valid(e->timed,e->active,e->tick,e->rounds,e->minutes,e->owner)||
           (e->concentration&&e->concentration!=s->magic.concentration))return false;
    }
    for(unsigned i=s->effect_count;i<PD_EFFECTS;i++)if(s->effects[i].tick||s->effects[i].minutes||s->effects[i].owner)return false;
    /* A concentration loss must always be possible, including after random damage.
       Reject dependencies on bound bonuses instead of failing after RNG. */
    int armor=unbound_total(s,PD_EFFECT_ARMOR),hp=unbound_total(s,PD_EFFECT_MAX_HP);
    return armor>=1&&armor<=40&&hp>=1&&hp<=999&&pd_armor(s)>=1&&pd_armor(s)<=40&&pd_max_hp(s)>=1&&pd_max_hp(s)<=999;
}
static bool effect_change(pd_state *s,unsigned index,const pd_effect *e) {
    if(!s||!pd_valid(s)||index>s->effect_count||index>=PD_EFFECTS||(!e&&index==s->effect_count))return false;
    pd_effect old[PD_EFFECTS];memcpy(old,s->effects,sizeof old);unsigned count=s->effect_count;
    if(e){s->effects[index]=*e;if(index==count)s->effect_count++;}
    else {memmove(s->effects+index,s->effects+index+1,(count-index-1)*sizeof(pd_effect));memset(&s->effects[--s->effect_count],0,sizeof(pd_effect));}
    if(!effects_valid(s)){memcpy(s->effects,old,sizeof old);s->effect_count=count;return false;}
    if(s->hp>pd_max_hp(s))s->hp=pd_max_hp(s);
    return true;
}
bool pd_effect_set(pd_state *s,unsigned index,pd_effect e){return effect_change(s,index,&e);}
bool pd_effect_remove(pd_state *s,unsigned index){return effect_change(s,index,NULL);}
static bool effect_tick(pd_state *s,unsigned tick,unsigned owner,uint8_t *advanced,uint8_t *expired) {
    if(!s||!advanced||!expired||advanced==expired||!pd_valid(s))return false;
    pd_effect old[PD_EFFECTS];memcpy(old,s->effects,sizeof old);uint8_t a=0,e=0;
    for(unsigned i=0;i<s->effect_count;i++) {
        pd_effect *v=&s->effects[i];if(!v->active||!v->timed||v->tick!=tick||v->owner!=owner)continue;
        a|=1u<<i;if(!--v->rounds){v->active=false;e|=1u<<i;}
    }
    if(!effects_valid(s)){memcpy(s->effects,old,sizeof old);return false;}
    if(s->hp>pd_max_hp(s))s->hp=pd_max_hp(s);
    *advanced=a;*expired=e;return true;
}
bool pd_effect_advance(pd_state *s,uint8_t *a,uint8_t *e){return effect_tick(s,PD_TICK_ROUND,0,a,e);}
static bool duration_tick(pd_state *s,unsigned tick,unsigned owner,pd_round_summary *out) {
    if(!out||owner>=PD_TURN_SUBJECTS)return false;
    pd_round_summary r={0};
    /* Numeric expiry validates/rolls back before any defence timer changes. */
    if(!effect_tick(s,tick,owner,&r.effects_advanced,&r.effects_expired))return false;
    for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++) {
        pd_defence_temporary *d=grant(s,i,slot);if(!d->active||!d->timed||d->tick!=tick||d->owner!=owner)continue;
        uint64_t bit=UINT64_C(1)<<(slot*PD_DAMAGE_TYPES+i);r.defences_advanced|=bit;if(!--d->rounds){d->active=false;r.defences_expired|=bit;}
    }
    for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++){
        pd_reduction_profile *p=&s->reductions[i];if(!p->active||!p->timed||p->tick!=tick||p->owner!=owner)continue;
        uint64_t bit=UINT64_C(1)<<(PD_DEFENCE_GRANTS+i);r.defences_advanced|=bit;if(!--p->rounds){p->active=false;r.defences_expired|=bit;}
    }
    *out=r;return true;
}
bool pd_round_advance(pd_state *s,pd_round_summary *out){return duration_tick(s,PD_TICK_ROUND,0,out);}
bool pd_turn_advance(pd_state *s,bool start,pd_round_summary *out){
    return pd_turn_advance_subject(s,0,start,out);
}
bool pd_turn_advance_subject(pd_state *s,unsigned owner,bool start,pd_round_summary *out){
    if(!duration_tick(s,start?PD_TICK_START:PD_TICK_END,owner,out))return false;
    if(start&&!owner)s->reaction_spent=false;
    return true;
}
bool pd_time_advance(pd_state *s,unsigned minutes,pd_round_summary *out){
    if(!s||!out||!minutes||minutes>5940||!pd_valid(s))return false;
    pd_round_summary r={0};pd_effect old[PD_EFFECTS];memcpy(old,s->effects,sizeof old);unsigned hp=s->hp,elapsed=0;
    for(unsigned i=0;i<s->effect_count;i++)if(old[i].active&&old[i].timed&&old[i].tick>=PD_TICK_MINUTES)r.effects_advanced|=1u<<i;
    /* Validate each simultaneous expiry in chronological order, including HP caps.
       A later increase of the maximum must never restore previously lost HP. */
    for(;;){
        unsigned next=minutes+1;
        for(unsigned i=0;i<s->effect_count;i++)if((r.effects_advanced&(1u<<i))&&s->effects[i].active&&old[i].minutes>elapsed&&old[i].minutes<next)next=old[i].minutes;
        if(next>minutes)break;
        for(unsigned i=0;i<s->effect_count;i++)if((r.effects_advanced&(1u<<i))&&old[i].minutes==next){s->effects[i].active=false;r.effects_expired|=1u<<i;}
        if(!effects_valid(s)){memcpy(s->effects,old,sizeof old);s->hp=hp;return false;}
        if(s->hp>pd_max_hp(s))s->hp=pd_max_hp(s);
        elapsed=next;
    }
    for(unsigned i=0;i<s->effect_count;i++)if(r.effects_advanced&(1u<<i))s->effects[i].minutes=old[i].minutes>minutes?old[i].minutes-minutes:0;
    for(unsigned type=0;type<PD_DAMAGE_TYPES;type++)for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++){
        pd_defence_temporary *d=grant(s,type,slot);if(!d->active||!d->timed||d->tick<PD_TICK_MINUTES)continue;
        uint64_t bit=UINT64_C(1)<<(slot*PD_DAMAGE_TYPES+type);r.defences_advanced|=bit;d->minutes=d->minutes>minutes?d->minutes-minutes:0;if(!d->minutes){d->active=false;r.defences_expired|=bit;}
    }
    for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++){
        pd_reduction_profile *p=&s->reductions[i];if(!p->active||!p->timed||p->tick<PD_TICK_MINUTES)continue;
        uint64_t bit=UINT64_C(1)<<(PD_DEFENCE_GRANTS+i);r.defences_advanced|=bit;p->minutes=p->minutes>minutes?p->minutes-minutes:0;if(!p->minutes){p->active=false;r.defences_expired|=bit;}
    }
    *out=r;return true;
}
unsigned pd_defence_base_flags(const pd_state *s,unsigned type) {
    if(type>=PD_DAMAGE_TYPES)return 0;
    unsigned bit=1u<<type;
    return ((s->defences.resistance&bit)?PD_RESISTANCE:0)|((s->defences.vulnerability&bit)?PD_VULNERABILITY:0)|((s->defences.immunity&bit)?PD_IMMUNITY:0);
}
unsigned pd_defence_flags(const pd_state *s,unsigned type) {
    if(type>=PD_DAMAGE_TYPES)return 0;
    unsigned flags=0,base=pd_defence_base_flags(s,type);
    for(unsigned i=0;i<3;i++){
        unsigned bit=1u<<i;if((base&bit)&&!s->defences.conditions[type][i])flags|=bit;
        for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++){const pd_defence_temporary *d=pd_defence_grant(s,type,slot);if(d->active&&(d->flags&bit)&&!d->conditions[i])flags|=bit;}
    }
    return flags;
}
bool pd_defence_conditional(const pd_state *s,unsigned type) {
    if(type>=PD_DAMAGE_TYPES)return false;
    unsigned base=pd_defence_base_flags(s,type);
    for(unsigned i=0;i<3;i++){
        if((base&(1u<<i))&&s->defences.conditions[type][i])return true;
        for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++){const pd_defence_temporary *d=pd_defence_grant(s,type,slot);if(d->active&&(d->flags&(1u<<i))&&d->conditions[i])return true;}
    }
    return false;
}
static bool rules_valid(unsigned flags,const uint8_t conditions[3]) {
    for(unsigned i=0;i<3;i++)if(conditions[i]>=PD_RULE_COUNT||(!(flags&(1u<<i))&&conditions[i]))return false;
    return true;
}
bool pd_defence_rule_set(pd_state *s,unsigned type,bool temporary,unsigned property,unsigned condition) {
    if(!s||!pd_valid(s)||type>=PD_DAMAGE_TYPES||property>=3||condition>=PD_RULE_COUNT)return false;
    unsigned flags=temporary?s->defences.temporary[type].flags:pd_defence_base_flags(s,type);
    if(condition&&!(flags&(1u<<property)))return false;
    if(temporary)s->defences.temporary[type].conditions[property]=condition;else s->defences.conditions[type][property]=condition;
    return true;
}
static bool temporary_valid(const pd_state *s,const pd_defence_temporary *d) {
    return d->flags<=7&&rules_valid(d->flags,d->conditions)&&(!d->active||d->flags)&&d->rounds<=99&&
        duration_valid(d->timed,d->active,d->tick,d->rounds,d->minutes,d->owner)&&
        (!d->concentration||d->concentration==s->magic.concentration);
}
bool pd_defence_grant_set(pd_state *s,unsigned type,unsigned slot,pd_defence_temporary d) {
    if(!s||!pd_valid(s)||type>=PD_DAMAGE_TYPES||slot>=PD_DEFENCE_SLOTS||!temporary_valid(s,&d))return false;
    *grant(s,type,slot)=d;return true;
}
bool pd_defence_temporary_set(pd_state *s,unsigned type,pd_defence_temporary d) {return pd_defence_grant_set(s,type,0,d);}
bool pd_defence_set(pd_state *s,unsigned type,bool configured,unsigned flags) {
    if(!s||!pd_valid(s)||type>=PD_DAMAGE_TYPES||flags>7||(!configured&&flags))return false;
    uint16_t bit=1u<<type;pd_defences *d=&s->defences;
    d->configured=(d->configured&~bit)|(configured?bit:0);
    d->resistance=(d->resistance&~bit)|((flags&PD_RESISTANCE)?bit:0);
    d->vulnerability=(d->vulnerability&~bit)|((flags&PD_VULNERABILITY)?bit:0);
    d->immunity=(d->immunity&~bit)|((flags&PD_IMMUNITY)?bit:0);
    for(unsigned i=0;i<3;i++)if(!(flags&(1u<<i)))d->conditions[type][i]=0;
    return true;
}
unsigned pd_reduction_cost_max(const pd_state *s,unsigned kind,unsigned index){
    if(!s)return 0;
    if(kind==PD_COST_COUNTER&&s->resource_count<=PD_RESOURCES&&index<s->resource_count)return s->resources[index].maximum;
    if(kind==PD_COST_POWER&&s->magic.count<=PD_POWERS&&index<s->magic.count){const pd_power *p=&s->magic.powers[index];if(!(p->flags&PD_PASSIVE)&&(p->kind||(p->flags&PD_LIMITED)))return p->maximum;}
    if(kind==PD_COST_ITEM&&s->inventory_available&&s->item_count<=PD_ITEMS&&index<s->item_count)return s->items[index].charges.maximum;
    if((kind==PD_COST_SLOT||kind==PD_COST_PACT)&&s->magic.available&&index<9)return (kind==PD_COST_PACT?s->magic.pact:s->magic.slots)[index].maximum;
    return 0;
}
unsigned pd_reduction_cost_current(const pd_state *s,unsigned kind,unsigned index){
    if(!pd_reduction_cost_max(s,kind,index))return 0;
    if(kind==PD_COST_COUNTER)return s->resources[index].current;
    if(kind==PD_COST_POWER)return s->magic.powers[index].remaining;
    if(kind==PD_COST_ITEM)return s->items[index].charges.current;
    return (kind==PD_COST_PACT?s->magic.pact:s->magic.slots)[index].current;
}
const char *pd_reduction_cost_name(const pd_state *s,unsigned kind,unsigned index){
    static const char *const ordinary[]={"Slot ordinario livello 1","Slot ordinario livello 2","Slot ordinario livello 3","Slot ordinario livello 4","Slot ordinario livello 5","Slot ordinario livello 6","Slot ordinario livello 7","Slot ordinario livello 8","Slot ordinario livello 9"};
    static const char *const pact[]={"Slot patto livello 1","Slot patto livello 2","Slot patto livello 3","Slot patto livello 4","Slot patto livello 5","Slot patto livello 6","Slot patto livello 7","Slot patto livello 8","Slot patto livello 9"};
    if(kind==PD_COST_NONE&&!index)return "Nessuna risorsa";
    if(!pd_reduction_cost_max(s,kind,index))return NULL;
    if(kind==PD_COST_COUNTER)return s->resources[index].name;
    if(kind==PD_COST_POWER)return s->magic.powers[index].name;
    if(kind==PD_COST_ITEM)return s->items[index].name;
    return kind==PD_COST_PACT?pact[index]:ordinary[index];
}
bool pd_condition_effects_set(pd_state *s,unsigned mask){if(!s||mask>PD_ROLL_CONDITIONS||!pd_valid(s))return false;s->roll_conditions=mask;return true;}
bool pd_reaction_change(pd_state *s,bool spent){if(!s||!pd_valid(s)||(spent&&s->reaction_spent))return false;s->reaction_spent=spent;return true;}
pd_cost pd_profile_cost(const pd_reduction_profile *p,unsigned row){
    if(!p||row>=PD_PROFILE_COSTS)return (pd_cost){0};
    return row?p->extra_costs[row-1]:(pd_cost){p->cost_kind,p->cost_index,p->cost};
}
bool pd_profile_cost_set(pd_reduction_profile *p,unsigned row,pd_cost cost){
    if(!p||row>=PD_PROFILE_COSTS)return false;
    if(row)p->extra_costs[row-1]=cost;else{p->cost_kind=cost.kind;p->cost_index=cost.index;p->cost=cost.amount;}return true;
}
static bool costs_valid(const pd_state *s,const pd_reduction_profile *p){
    if(p->payment>PD_PAYMENT_CHOOSE)return false;
    unsigned count=0;
    for(unsigned row=0;row<PD_PROFILE_COSTS;row++){
        pd_cost c=pd_profile_cost(p,row);count+=c.kind!=0;
        if(c.kind>=PD_COST_COUNT||(c.kind?(!c.amount||c.amount>pd_reduction_cost_max(s,c.kind,c.index)):(c.amount||c.index)))return false;
        for(unsigned previous=0;previous<row;previous++){pd_cost other=pd_profile_cost(p,previous);if(c.kind&&c.kind==other.kind&&c.index==other.index)return false;}
    }
    return p->payment!=PD_PAYMENT_CHOOSE||count>=2;
}
static bool reduction_valid(const pd_state *s,const pd_reduction_profile *p){
    return costs_valid(s,p)&&p->value<=9999&&p->types<=8191&&p->stage<=1&&p->rule<PD_RULE_COUNT&&p->rounds<=99&&
        (!p->active||(p->value&&p->types))&&duration_valid(p->timed,p->active,p->tick,p->rounds,p->minutes,p->owner)&&
        (!p->concentration||p->concentration==s->magic.concentration);
}
bool pd_reduction_profile_set(pd_state *s,unsigned i,pd_reduction_profile p){
    if(!s||i>=PD_REDUCTION_PROFILES||!pd_valid(s)||!reduction_valid(s,&p))return false;
    s->reductions[i]=p;return true;
}
bool pd_valid(const pd_state *s) {
    for(unsigned i=0;i<3;i++)if(!turn_name_valid(s->turn_names[i]))return false;
    for(unsigned i=0;i<PD_COINS;i++)if(s->wallet.coins[i]>PD_COIN_MAX||(!s->wallet.available&&s->wallet.coins[i]))return false;
    if (s->roll_conditions>PD_ROLL_CONDITIONS || !text(s->name, sizeof s->name) || s->level < 1 || s->level > 20 ||
        s->armor < 1 || s->armor > 40 || !s->max_hp || s->max_hp > 999 ||
        !effects_valid(s) || s->hp > pd_max_hp(s) || s->temp_hp > 999 || s->resource_count > PD_RESOURCES ||
        s->lineage>=12 || s->body>1 || s->scene>=PD_SCENES || s->avatar.skin>2 || s->avatar.hair>2 || s->avatar.cloak>2 || s->avatar.style>2) return false;
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
    for(unsigned i=0;i<PD_ITEMS;i++) {
        const pd_charges *c=&s->items[i].charges;
        if(c->maximum>999||c->current>c->maximum||c->recovery>2||(!c->maximum&&c->recovery)||(i>=s->item_count&&(c->current||c->maximum||c->recovery)))return false;
    }
    if(s->recovery.count>PD_HIT_POOLS||(!s->recovery.available&&s->recovery.count))return false;
    for(unsigned i=0;i<s->recovery.count;i++) {
        const pd_hit_die *v=&s->recovery.pools[i];
        if(!text(v->name,sizeof v->name)||!(v->die==6||v->die==8||v->die==10||v->die==12)||!v->maximum||v->maximum>20||v->used>v->maximum)return false;
    }
    if(s->status.count>PD_CONDITIONS||s->status.successes>3||s->status.failures>3||(!s->status.available&&(s->status.successes||s->status.failures||s->status.stabilized)))return false;
    for(unsigned i=0;i<s->status.count;i++)if(!text(s->status.conditions[i].name,61)||s->status.conditions[i].level>6)return false;
    const pd_defences *d=&s->defences;
    if(d->configured>8191||((d->resistance|d->vulnerability|d->immunity)&~d->configured))return false;
    for(unsigned i=0;i<PD_DAMAGE_TYPES;i++){
        if(!rules_valid(pd_defence_base_flags(s,i),d->conditions[i]))return false;
        for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++)if(!temporary_valid(s,pd_defence_grant(s,i,slot)))return false;
    }
    for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++)if(!reduction_valid(s,&s->reductions[i]))return false;
    return s->brightness>=10&&s->brightness<=100&&s->dim_timeout<=3&&pd_magic_valid(s)&&s->shortcuts[0]<=14&&s->shortcuts[1]<=14&&s->shortcuts[2]<=14;
}
int pd_modifier(int score) { return score / 2 - 5; }
void pd_hp(pd_state *s, int delta) {
    /* Damage consumes temporary HP first. Healing never creates temporary HP. */
    if(delta>0&&!s->hp&&s->status.available&&s->status.failures==3)return;
    unsigned old=s->hp;
    int64_t amount = delta;
    if (amount < 0) {
        int64_t absorbed = -amount < s->temp_hp ? -amount : s->temp_hp;
        s->temp_hp -= absorbed; amount += absorbed;
    }
    amount += s->hp;
    s->hp = amount < 0 ? 0 : amount > pd_max_hp(s) ? pd_max_hp(s) : amount;
    if(s->status.available&&((old&&!s->hp)||(!old&&s->hp))){s->status.successes=0;s->status.failures=0;s->status.stabilized=false;}
    if(!s->hp) {pd_concentration_end(s);s->combat.raging=false;if(s->magic.available)s->training.strength_advantage=false;}
}
bool pd_vitals_apply(pd_state *s,unsigned kind,unsigned amount){
    if(!s||!pd_valid(s)||kind>=PD_VITAL_COUNT||amount>999||(!amount&&kind!=PD_VITAL_TEMP_SET))return false;
    if(kind==PD_VITAL_HEAL){
        if(!s->hp&&s->status.available&&s->status.failures==3)return false;
        pd_hp(s,(int)amount);
    }else if(kind==PD_VITAL_TEMP_SET||amount>s->temp_hp)s->temp_hp=amount;
    return true;
}
bool pd_resource_change(pd_state *s, unsigned index, int delta) {
    if (index >= s->resource_count) return false;
    int64_t n = (int64_t)s->resources[index].current + delta;
    s->resources[index].current = n < 0 ? 0 : n > s->resources[index].maximum ? s->resources[index].maximum : n;
    return true;
}
bool pd_item_quantity_set(pd_state *s,unsigned index,unsigned quantity){
    if(!s||!pd_valid(s)||!s->inventory_available||index>=s->item_count||quantity>999)return false;
    s->items[index].quantity=quantity;
    if(!quantity&&s->equipped==index){s->equipped=PD_NO_WEAPON;s->combat.thrown=false;}
    return true;
}
bool pd_item_charges_change(pd_state *s,unsigned index,int delta) {
    if(!s||!pd_valid(s)||!s->inventory_available||index>=s->item_count||!s->items[index].quantity||!s->items[index].charges.maximum)return false;
    pd_charges *c=&s->items[index].charges;int64_t n=(int64_t)c->current+delta;
    c->current=n<0?0:n>c->maximum?c->maximum:n;return true;
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
bool pd_wallet_set(pd_state *s,const pd_wallet *wallet){
    if(!s||!wallet||!pd_valid(s))return false;
    for(unsigned i=0;i<PD_COINS;i++)if(wallet->coins[i]>PD_COIN_MAX||(!wallet->available&&wallet->coins[i]))return false;
    s->wallet=*wallet;return true;
}
bool pd_wallet_change(pd_state *s,unsigned coin,int32_t delta){
    if(!s||!pd_valid(s)||!s->wallet.available||coin>=PD_COINS)return false;
    int64_t next=(int64_t)s->wallet.coins[coin]+delta;
    if(next<0||next>PD_COIN_MAX)return false;
    s->wallet.coins[coin]=(uint32_t)next;return true;
}
uint64_t pd_wallet_copper(const pd_wallet *wallet){
    static const unsigned rate[PD_COINS]={1,10,50,100,1000};uint64_t total=0;
    if(!wallet||!wallet->available)return 0;
    for(unsigned i=0;i<PD_COINS;i++)total+=(uint64_t)wallet->coins[i]*rate[i];
    return total;
}
static void put16(uint8_t **p, unsigned n) { *(*p)++=n; *(*p)++=n>>8; }
static unsigned get16(const uint8_t **p) { unsigned n=(*p)[0]|((*p)[1]<<8); *p+=2; return n; }
static void put32(uint8_t *p, uint32_t n) { for (int i=0;i<4;i++) p[i]=n>>(8*i); }
static uint32_t get32(const uint8_t *p) { return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24); }
size_t pd_encoded_size(const pd_state *s){return 8945u+552u*s->magic.count;}
size_t pd_encode(const pd_state *s, uint32_t gen, uint8_t out[PD_RECORD_SIZE]) {
    memset(out, 0, pd_encoded_size(s));
    memcpy(out, "PDDU", 4); put32(out+4,gen);
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
    *p++=s->status.available;*p++=s->status.count;*p++=s->status.successes;*p++=s->status.failures;*p++=s->status.stabilized;
    for(unsigned i=0;i<PD_CONDITIONS;i++){memcpy(p,s->status.conditions[i].name,61);p+=61;*p++=s->status.conditions[i].level;}
    *p++=s->brightness;*p++=s->dim_timeout;
    *p++=s->effect_count;
    for(unsigned i=0;i<PD_EFFECTS;i++){const pd_effect *e=&s->effects[i];*p++=e->target;put16(&p,e->value+999);*p++=e->active;}
    for(unsigned i=0;i<PD_EFFECTS;i++){*p++=s->effects[i].timed;*p++=s->effects[i].rounds;}
    put16(&p,s->defences.configured);put16(&p,s->defences.resistance);put16(&p,s->defences.vulnerability);put16(&p,s->defences.immunity);
    for(unsigned i=0;i<PD_EFFECTS;i++)*p++=s->effects[i].concentration;
    for(unsigned i=0;i<PD_DAMAGE_TYPES;i++){const pd_defence_temporary *d=&s->defences.temporary[i];*p++=d->flags;*p++=d->active;*p++=d->timed;*p++=d->rounds;*p++=d->concentration;}
    for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)for(unsigned j=0;j<3;j++)*p++=s->defences.conditions[i][j];
    for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)for(unsigned j=0;j<3;j++)*p++=s->defences.temporary[i].conditions[j];
    for(unsigned slot=1;slot<PD_DEFENCE_SLOTS;slot++)for(unsigned i=0;i<PD_DAMAGE_TYPES;i++){
        const pd_defence_temporary *d=pd_defence_grant(s,i,slot);*p++=d->flags;*p++=d->active;*p++=d->timed;*p++=d->rounds;*p++=d->concentration;for(unsigned j=0;j<3;j++)*p++=d->conditions[j];
    }
    for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++){const pd_reduction_profile *r=&s->reductions[i];put16(&p,r->value);put16(&p,r->types);*p++=r->stage|(r->payment<<1);*p++=r->rule;*p++=r->active;*p++=r->timed;*p++=r->rounds;*p++=r->concentration;}
    for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++){const pd_reduction_profile *r=&s->reductions[i];*p++=r->cost_kind;*p++=r->cost_index;put16(&p,r->cost);*p++=r->reaction;}
    *p++=s->reaction_spent;
    for(unsigned i=0;i<PD_EFFECTS;i++)*p++=s->effects[i].tick|(s->effects[i].owner<<3);
    for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++)for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)*p++=pd_defence_grant(s,i,slot)->tick|(pd_defence_grant(s,i,slot)->owner<<3);
    for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++)*p++=s->reductions[i].tick|(s->reductions[i].owner<<3);
    for(unsigned i=0;i<PD_EFFECTS;i++)put16(&p,s->effects[i].minutes);
    for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++)for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)put16(&p,pd_defence_grant(s,i,slot)->minutes);
    for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++)put16(&p,s->reductions[i].minutes);
    for(unsigned i=0;i<PD_ITEMS;i++){const pd_charges *c=&s->items[i].charges;put16(&p,c->current);put16(&p,c->maximum);*p++=c->recovery;}
    for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++)for(unsigned row=1;row<PD_PROFILE_COSTS;row++){pd_cost c=pd_profile_cost(&s->reductions[i],row);*p++=c.kind;*p++=c.index;put16(&p,c.amount);}
    *p++=s->roll_conditions;
    *p++=s->wallet.available;
    for(unsigned i=0;i<PD_COINS;i++){put32(p,s->wallet.coins[i]);p+=4;}
    put32(p,s->history_epoch);p+=4;
    memcpy(p,s->turn_names,sizeof s->turn_names);p+=sizeof s->turn_names;
    *p++=s->scene;*p++=s->lineage;*p++=s->body;
    size_t size=pd_encoded_size(s);put32(out+size-4,pd_crc32(out,size-4));return size;
}
static bool record_payments(const uint8_t *data){return !memcmp(data,"PDDS",4)||!memcmp(data,"PDDT",4)||!memcmp(data,"PDDU",4);}
bool pd_record_known(const uint8_t *data,size_t size) {
    if(!data)return false;
    if(size==384&&(!memcmp(data,"PDD1",4)||!memcmp(data,"PDD2",4)))return true;
    if(size==512&&!memcmp(data,"PDD3",4))return true;
    if(size==5184&&!memcmp(data,"PDD4",4))return true;
    if(size>=5168&&size<=71408&&!memcmp(data,"PDD5",4)&&data[5123]<=PD_POWERS)return size==5168u+552u*data[5123];
    if(size>=6050&&size<=72290&&!memcmp(data,"PDD6",4)&&data[5123]<=PD_POWERS)return size==6050u+552u*data[5123];
    if(size>=7915&&size<=74155&&!memcmp(data,"PDD7",4)&&data[5123]<=PD_POWERS)return size==7915u+552u*data[5123];
    if(size>=7917&&size<=74157&&!memcmp(data,"PDD8",4)&&data[5123]<=PD_POWERS)return size==7917u+552u*data[5123];
    if(size>=7950&&size<=74190&&!memcmp(data,"PDD9",4)&&data[5123]<=PD_POWERS)return size==7950u+552u*data[5123];
    if(size>=7966&&size<=74206&&!memcmp(data,"PDDA",4)&&data[5123]<=PD_POWERS)return size==7966u+552u*data[5123];
    if(size>=7974&&size<=74214&&!memcmp(data,"PDDB",4)&&data[5123]<=PD_POWERS)return size==7974u+552u*data[5123];
    if(size>=7982&&size<=74222&&!memcmp(data,"PDDC",4)&&data[5123]<=PD_POWERS)return size==7982u+552u*data[5123];
    if(size>=8047&&size<=74287&&!memcmp(data,"PDDD",4)&&data[5123]<=PD_POWERS)return size==8047u+552u*data[5123];
    if(size>=8125&&size<=74365&&!memcmp(data,"PDDE",4)&&data[5123]<=PD_POWERS)return size==8125u+552u*data[5123];
    if(size>=8333&&size<=74573&&!memcmp(data,"PDDF",4)&&data[5123]<=PD_POWERS)return size==8333u+552u*data[5123];
    if(size>=8363&&size<=74603&&!memcmp(data,"PDDG",4)&&data[5123]<=PD_POWERS)return size==8363u+552u*data[5123];
    if(size>=8379&&size<=74619&&(!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4))&&data[5123]<=PD_POWERS)return size==8379u+552u*data[5123];
    if(size>=8429&&size<=74669&&!memcmp(data,"PDDJ",4)&&data[5123]<=PD_POWERS)return size==8429u+552u*data[5123];
    if(size>=8529&&size<=74769&&!memcmp(data,"PDDK",4)&&data[5123]<=PD_POWERS)return size==8529u+552u*data[5123];
    if(size>=8829&&size<=75069&&!memcmp(data,"PDDL",4)&&data[5123]<=PD_POWERS)return size==8829u+552u*data[5123];
    if(size>=8853&&size<=75093&&!memcmp(data,"PDDM",4)&&data[5123]<=PD_POWERS)return size==8853u+552u*data[5123];
    if(size>=8854&&size<=75094&&!memcmp(data,"PDDN",4)&&data[5123]<=PD_POWERS)return size==8854u+552u*data[5123];
    if(size>=8875&&size<=75115&&!memcmp(data,"PDDO",4)&&data[5123]<=PD_POWERS)return size==8875u+552u*data[5123];
    if(size>=8945&&size<=PD_RECORD_SIZE&&!memcmp(data,"PDDU",4)&&data[5123]<=PD_POWERS)return size==8945u+552u*data[5123];
    if(size>=8943&&size<=PD_RECORD_SIZE&&!memcmp(data,"PDDT",4)&&data[5123]<=PD_POWERS)return size==8943u+552u*data[5123];
    if(size>=8942&&size<=PD_RECORD_SIZE&&(!memcmp(data,"PDDR",4)||!memcmp(data,"PDDS",4))&&data[5123]<=PD_POWERS)return size==8942u+552u*data[5123];
    return size>=8879&&size<=PD_RECORD_SIZE&&(!memcmp(data,"PDDP",4)||!memcmp(data,"PDDQ",4))&&data[5123]<=PD_POWERS&&size==8879u+552u*data[5123];
}
static bool decode_into(const uint8_t *data,size_t size,pd_state *allocated,uint32_t *gen) {
    if (!pd_record_known(data,size) ||
        pd_crc32(data,size-4) != get32(data+size-4)) return false;
    memset(allocated,0,sizeof *allocated);
    #define candidate (*allocated)
    candidate.brightness=50;candidate.equipped=PD_NO_WEAPON;candidate.shortcuts[0]=1;candidate.shortcuts[1]=3;candidate.shortcuts[2]=5;
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
    if(!memcmp(data,"PDD4",4)||!memcmp(data,"PDD5",4)||!memcmp(data,"PDD6",4)||!memcmp(data,"PDD7",4)||!memcmp(data,"PDD8",4)||!memcmp(data,"PDD9",4)||!memcmp(data,"PDDA",4)||!memcmp(data,"PDDB",4)||!memcmp(data,"PDDC",4)||!memcmp(data,"PDDD",4)||!memcmp(data,"PDDE",4)||!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))) {
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
    if(!memcmp(data,"PDD5",4)||!memcmp(data,"PDD6",4)||!memcmp(data,"PDD7",4)||!memcmp(data,"PDD8",4)||!memcmp(data,"PDD9",4)||!memcmp(data,"PDDA",4)||!memcmp(data,"PDDB",4)||!memcmp(data,"PDDC",4)||!memcmp(data,"PDDD",4)||!memcmp(data,"PDDE",4)||!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))) {
        pd_magic *m=&candidate.magic;if(*p>1)goto invalid;m->available=*p++;m->count=*p++;m->concentration=*p++;
        for(int i=0;i<9;i++){m->slots[i].current=*p++;m->slots[i].maximum=*p++;m->pact[i].current=*p++;m->pact[i].maximum=*p++;}
        memcpy(candidate.shortcuts,p,3);p+=3;
        if(memcmp(data,"PDDT",4)&&memcmp(data,"PDDU",4)&&(candidate.shortcuts[0]>13||candidate.shortcuts[1]>13||candidate.shortcuts[2]>13))goto invalid;
        if((memcmp(data,"PDDP",4)&&(memcmp(data,"PDDQ",4)&&(memcmp(data,"PDDR",4)&&!record_payments(data))))&&(candidate.shortcuts[0]>12||candidate.shortcuts[1]>12||candidate.shortcuts[2]>12))goto invalid;
        for(int i=0;i<m->count;i++) {
            pd_power *v=&m->powers[i];memcpy(v->name,p,41);p+=41;memcpy(v->description,p,PD_POWER_TEXT_BYTES);p+=PD_POWER_TEXT_BYTES;
            v->kind=*p++;v->level=*p++;v->flags=*p++;v->ability=*p++;v->maximum=*p++;v->remaining=*p++;v->recovery=*p++;v->effect=*p++;
            if(p[0]>45||p[1]>45)goto invalid;
            v->attack_modifier=(int)*p++-15;v->save_modifier=(int)*p++-15;
        }
    }
    if(!memcmp(data,"PDD6",4)||!memcmp(data,"PDD7",4)||!memcmp(data,"PDD8",4)||!memcmp(data,"PDD9",4)||!memcmp(data,"PDDA",4)||!memcmp(data,"PDDB",4)||!memcmp(data,"PDDC",4)||!memcmp(data,"PDDD",4)||!memcmp(data,"PDDE",4)||!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))) {
        if(*p>1)goto invalid;
        candidate.recovery.available=*p++;candidate.recovery.count=*p++;
        for(unsigned i=0;i<PD_HIT_POOLS;i++) {
            pd_hit_die *v=&candidate.recovery.pools[i];memcpy(v->name,p,41);p+=41;v->die=*p++;v->maximum=*p++;v->used=*p++;
        }
    }
    if(!memcmp(data,"PDD7",4)||!memcmp(data,"PDD8",4)||!memcmp(data,"PDD9",4)||!memcmp(data,"PDDA",4)||!memcmp(data,"PDDB",4)||!memcmp(data,"PDDC",4)||!memcmp(data,"PDDD",4)||!memcmp(data,"PDDE",4)||!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))) {
        if(p[0]>1||p[4]>1)goto invalid;
        candidate.status.available=*p++;candidate.status.count=*p++;candidate.status.successes=*p++;candidate.status.failures=*p++;candidate.status.stabilized=*p++;
        for(unsigned i=0;i<PD_CONDITIONS;i++){memcpy(candidate.status.conditions[i].name,p,61);p+=61;candidate.status.conditions[i].level=*p++;}
    }
    if(!memcmp(data,"PDD8",4)||!memcmp(data,"PDD9",4)||!memcmp(data,"PDDA",4)||!memcmp(data,"PDDB",4)||!memcmp(data,"PDDC",4)||!memcmp(data,"PDDD",4)||!memcmp(data,"PDDE",4)||!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))){candidate.brightness=*p++;candidate.dim_timeout=*p++;}
    if(!memcmp(data,"PDD9",4)||!memcmp(data,"PDDA",4)||!memcmp(data,"PDDB",4)||!memcmp(data,"PDDC",4)||!memcmp(data,"PDDD",4)||!memcmp(data,"PDDE",4)||!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))) {
        candidate.effect_count=*p++;
        for(unsigned i=0;i<PD_EFFECTS;i++) {
            pd_effect *e=&candidate.effects[i];e->target=*p++;unsigned value=get16(&p);
            if(e->target>PD_EFFECT_MAX_HP||value>1998||*p>1)goto invalid;
            e->value=(int)value-999;e->active=*p++;
            int bound=e->target==PD_EFFECT_ARMOR?20:999;if(e->value< -bound||e->value>bound)goto invalid;
        }
    }
    if(!memcmp(data,"PDDA",4)||!memcmp(data,"PDDB",4)||!memcmp(data,"PDDC",4)||!memcmp(data,"PDDD",4)||!memcmp(data,"PDDE",4)||!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))) {
        for(unsigned i=0;i<PD_EFFECTS;i++) {
            pd_effect *e=&candidate.effects[i];if(p[0]>1||p[1]>99)goto invalid;
            e->timed=*p++;e->rounds=*p++;
        }
    }
    if(!memcmp(data,"PDDB",4)||!memcmp(data,"PDDC",4)||!memcmp(data,"PDDD",4)||!memcmp(data,"PDDE",4)||!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))){candidate.defences.configured=get16(&p);candidate.defences.resistance=get16(&p);candidate.defences.vulnerability=get16(&p);candidate.defences.immunity=get16(&p);}
    if(!memcmp(data,"PDDC",4)||!memcmp(data,"PDDD",4)||!memcmp(data,"PDDE",4)||!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data)))))for(unsigned i=0;i<PD_EFFECTS;i++){candidate.effects[i].concentration=*p++;if(candidate.effects[i].concentration&&(i>=candidate.effect_count||candidate.effects[i].concentration!=candidate.magic.concentration))goto invalid;}
    if(!memcmp(data,"PDDD",4)||!memcmp(data,"PDDE",4)||!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data)))))for(unsigned i=0;i<PD_DAMAGE_TYPES;i++){
        pd_defence_temporary *d=&candidate.defences.temporary[i];
        if(p[1]>1||p[2]>1)goto invalid;
        d->flags=*p++;d->active=*p++;d->timed=*p++;d->rounds=*p++;d->concentration=*p++;
    }
    if(!memcmp(data,"PDDE",4)||!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))){
        for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)for(unsigned j=0;j<3;j++)candidate.defences.conditions[i][j]=*p++;
        for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)for(unsigned j=0;j<3;j++)candidate.defences.temporary[i].conditions[j]=*p++;
    }
    if(!memcmp(data,"PDDF",4)||!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data)))))for(unsigned slot=1;slot<PD_DEFENCE_SLOTS;slot++)for(unsigned i=0;i<PD_DAMAGE_TYPES;i++){
        pd_defence_temporary *d=grant(&candidate,i,slot);if(p[1]>1||p[2]>1)goto invalid;
        d->flags=*p++;d->active=*p++;d->timed=*p++;d->rounds=*p++;d->concentration=*p++;for(unsigned j=0;j<3;j++)d->conditions[j]=*p++;
    }
    if(!memcmp(data,"PDDG",4)||!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data)))))for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++){
        pd_reduction_profile *r=&candidate.reductions[i];r->value=get16(&p);r->types=get16(&p);r->stage=*p++;if(record_payments(data)){r->payment=r->stage>>1;r->stage&=1;}r->rule=*p++;if(p[0]>1||p[1]>1)goto invalid;r->active=*p++;r->timed=*p++;r->rounds=*p++;r->concentration=*p++;
    }
    if(!memcmp(data,"PDDH",4)||!memcmp(data,"PDDI",4)||!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))){
        for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++){pd_reduction_profile *r=&candidate.reductions[i];r->cost_kind=*p++;if((!memcmp(data,"PDDH",4)&&r->cost_kind>PD_COST_POWER)||(memcmp(data,"PDDL",4)&&memcmp(data,"PDDM",4)&&(memcmp(data,"PDDN",4)&&memcmp(data,"PDDO",4)&&(memcmp(data,"PDDP",4)&&(memcmp(data,"PDDQ",4)&&(memcmp(data,"PDDR",4)&&!record_payments(data)))))&&r->cost_kind>PD_COST_PACT))goto invalid;r->cost_index=*p++;r->cost=get16(&p);if(*p>1)goto invalid;r->reaction=*p++;}
        if(*p>1)goto invalid;
        candidate.reaction_spent=*p++;
    }
    if(!memcmp(data,"PDDJ",4)||!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))){
        for(unsigned i=0;i<PD_EFFECTS;i++){candidate.effects[i].tick=*p++;if((!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data)))){candidate.effects[i].owner=candidate.effects[i].tick>>3;candidate.effects[i].tick&=7;}if(i>=candidate.effect_count&&(candidate.effects[i].tick||candidate.effects[i].owner))goto invalid;}
        for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++)for(unsigned i=0;i<PD_DAMAGE_TYPES;i++){pd_defence_temporary *d=grant(&candidate,i,slot);d->tick=*p++;if((!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data)))){d->owner=d->tick>>3;d->tick&=7;}}
        for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++){pd_reduction_profile *r=&candidate.reductions[i];r->tick=*p++;if((!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data)))){r->owner=r->tick>>3;r->tick&=7;}}
    }
    if(!memcmp(data,"PDDJ",4)){
        for(unsigned i=0;i<PD_EFFECTS;i++)if(candidate.effects[i].tick>=PD_TICK_MINUTES)goto invalid;
        for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++)for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)if(grant(&candidate,i,slot)->tick>=PD_TICK_MINUTES)goto invalid;
        for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++)if(candidate.reductions[i].tick>=PD_TICK_MINUTES)goto invalid;
    }
    if(!memcmp(data,"PDDK",4)||!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))){
        for(unsigned i=0;i<PD_EFFECTS;i++)candidate.effects[i].minutes=get16(&p);
        for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++)for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)grant(&candidate,i,slot)->minutes=get16(&p);
        for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++)candidate.reductions[i].minutes=get16(&p);
    }
    if(!memcmp(data,"PDDL",4)||!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data)))))for(unsigned i=0;i<PD_ITEMS;i++){pd_charges *c=&candidate.items[i].charges;c->current=get16(&p);c->maximum=get16(&p);c->recovery=*p++;}
    if(!memcmp(data,"PDDM",4)||!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data)))))for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++)for(unsigned row=1;row<PD_PROFILE_COSTS;row++){pd_cost c;c.kind=*p++;c.index=*p++;c.amount=get16(&p);pd_profile_cost_set(&candidate.reductions[i],row,c);}
    if(!memcmp(data,"PDDN",4)||!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data)))))candidate.roll_conditions=*p++;
    if((!memcmp(data,"PDDO",4)||(!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data)))))){
        if(*p>1)goto invalid;
        candidate.wallet.available=*p++;
        for(unsigned i=0;i<PD_COINS;i++){candidate.wallet.coins[i]=get32(p);p+=4;}
    }
    if((!memcmp(data,"PDDP",4)||(!memcmp(data,"PDDQ",4)||(!memcmp(data,"PDDR",4)||record_payments(data))))){candidate.history_epoch=get32(p);p+=4;}
    if((!memcmp(data,"PDDR",4)||record_payments(data))){memcpy(candidate.turn_names,p,sizeof candidate.turn_names);p+=sizeof candidate.turn_names;}
    if(!memcmp(data,"PDDT",4)||!memcmp(data,"PDDU",4))candidate.scene=*p++;
    if(!memcmp(data,"PDDU",4)){candidate.lineage=*p++;candidate.body=*p++;}
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
