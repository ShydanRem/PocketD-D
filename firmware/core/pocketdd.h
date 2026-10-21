#ifndef POCKETDD_H
#define POCKETDD_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define PD_RESOURCES 8
#define PD_NAME_BYTES 49
#define PD_RESOURCE_BYTES 33
#define PD_RECORD_SIZE 75093
#define PD_POWERS 120
#define PD_POWER_TEXT_BYTES 501
#define PD_ITEMS 60
#define PD_ITEM_NAME_BYTES 41
#define PD_NO_WEAPON 255
#define PD_SKILLS 18
#define PD_PACKET_MAX 262144

typedef struct { char name[PD_RESOURCE_BYTES]; uint16_t current, maximum; } pd_resource;
typedef struct { uint8_t skin,hair,cloak,style; } pd_avatar;
typedef struct {
    bool available,half_checks,poisoned,strength_advantage;
    uint8_t ranks[PD_SKILLS],saves;
    int8_t skill_bonus[PD_SKILLS],save_bonus[6],check_bonus[6];
    uint32_t half_skills,blocked;
} pd_training;
typedef enum { PD_GEAR,PD_CONSUMABLE,PD_HEALING,PD_WEAPON } pd_item_kind;
/* Flags: proficient, add ability to damage, ranged, heavy. Notes stay on PC. */
typedef struct { uint16_t current,maximum; uint8_t recovery; } pd_charges;
typedef struct {
    pd_charges charges; /* One pool per row, independent of quantity. Zero maximum: absent. */
    char name[PD_ITEM_NAME_BYTES],damage_type[25];
    uint16_t quantity;
    uint8_t kind,ability,count,sides,flags,reach,thrown_range;
    int8_t attack_modifier,damage_modifier,healing_modifier;
} pd_item;
typedef struct { uint8_t rage_bonus,critical_dice; bool raging,power_attack,thrown; } pd_combat;
enum { PD_CONCENTRATES=1,PD_PREPARED=2,PD_USES_SLOT=4,PD_LIMITED=8,PD_PASSIVE=16,PD_EXCERPT=32 };
typedef struct {
    char name[41],description[PD_POWER_TEXT_BYTES];
    uint8_t kind,level,flags,ability,maximum,remaining,recovery,effect;
    int8_t attack_modifier,save_modifier;
} pd_power;
typedef struct { uint8_t current,maximum; } pd_slot;
typedef struct {
    bool available;
    uint8_t count,concentration; /* 0: none, otherwise index + 1. */
    pd_slot slots[9],pact[9];
    pd_power powers[PD_POWERS];
} pd_magic;
#define PD_HIT_POOLS 20
typedef struct { char name[41]; uint8_t die,maximum,used; } pd_hit_die;
typedef struct { bool available; uint8_t count; pd_hit_die pools[PD_HIT_POOLS]; } pd_recovery;
#define PD_CONDITIONS 30
typedef struct { char name[61]; uint8_t level; } pd_condition;
typedef struct { bool available; uint8_t count,successes,failures; bool stabilized; pd_condition conditions[PD_CONDITIONS]; } pd_status;
#define PD_EFFECTS 8
enum { PD_EFFECT_ARMOR,PD_EFFECT_MAX_HP };
/* Explicit additive deltas to the imported bases; no rule inferred from a label. */
typedef struct { uint8_t target; int16_t value; bool active,timed; uint8_t rounds,concentration,tick; uint16_t minutes; /* concentration: 0 or current magic index + 1; tick: PD_TICK_*. */ } pd_effect;
#define PD_DAMAGE_TYPES 13
enum { PD_RESISTANCE=1,PD_VULNERABILITY=2,PD_IMMUNITY=4 };
enum { PD_RULE_ALWAYS,PD_RULE_MAGICAL,PD_RULE_NONMAGICAL,PD_RULE_NONMAGICAL_ATTACK,
    PD_RULE_NON_SILVERED_ATTACK,PD_RULE_NON_ADAMANTINE_ATTACK,PD_RULE_NEITHER_ATTACK,PD_RULE_COUNT };
/* Three independent grants per type. Equal flags do not stack; base stays separate. */
#define PD_DEFENCE_SLOTS 3
#define PD_DEFENCE_GRANTS (PD_DAMAGE_TYPES * PD_DEFENCE_SLOTS)
typedef struct { uint8_t flags; bool active,timed; uint8_t rounds,concentration,conditions[3],tick; uint16_t minutes; } pd_defence_temporary;
typedef struct {
    uint16_t configured,resistance,vulnerability,immunity;
    pd_defence_temporary temporary[PD_DAMAGE_TYPES];
    uint8_t conditions[PD_DAMAGE_TYPES][3];
    pd_defence_temporary additional[PD_DAMAGE_TYPES][PD_DEFENCE_SLOTS-1];
} pd_defences;
#define PD_REDUCTION_PROFILES 3
#define PD_TIMED_DEFENCES (PD_DEFENCE_GRANTS + PD_REDUCTION_PROFILES)
#define PD_PROFILE_COSTS 3
typedef struct { uint8_t kind,index; uint16_t amount; } pd_cost;
/* Local reusable budget; applying it to an impact is always explicit. */
typedef struct { uint16_t value,types; uint8_t stage,rule; bool active,timed; uint8_t rounds,concentration,cost_kind,cost_index; uint16_t cost; bool reaction; uint8_t tick; uint16_t minutes; pd_cost extra_costs[PD_PROFILE_COSTS-1]; } pd_reduction_profile;
typedef struct { uint8_t effects_advanced,effects_expired; uint64_t defences_advanced,defences_expired; /* Extra bits slot*13+type, profile bits 39+index. */ } pd_round_summary;
typedef struct {
    char name[PD_NAME_BYTES];
    uint16_t hp, max_hp, temp_hp;
    uint8_t level, armor, abilities[6], resource_count;
    pd_resource resources[PD_RESOURCES];
    pd_avatar avatar;
    bool idle_animation;
    pd_training training;
    bool inventory_available;
    uint8_t item_count,equipped;
    pd_combat combat;
    pd_item items[PD_ITEMS];
    pd_magic magic;
    pd_recovery recovery;
    pd_status status;
    uint8_t effect_count;
    pd_effect effects[PD_EFFECTS];
    pd_reduction_profile reductions[PD_REDUCTION_PROFILES];
    bool reaction_spent; /* Explicit turn marker; rounds and rests never reset it. */
    pd_defences defences; /* Explicit local configuration; unknown is not normal. */
    uint8_t brightness,dim_timeout; /* Device-local: 10..100%, off/30/60/120s. */
    uint8_t shortcuts[3]; /* PWR short, BOOT short, BOOT long; device-local. */
} pd_state;
typedef struct { uint8_t dice[2], count, natural; int total, bonus; } pd_roll;
typedef uint32_t (*pd_random)(void *context);
void pd_demo(pd_state *state);
bool pd_valid(const pd_state *state);
int pd_effect_bonus(const pd_state *state,unsigned target);
unsigned pd_armor(const pd_state *state);
unsigned pd_max_hp(const pd_state *state);
/* Active totals with concentration-bound effects excluded. */
unsigned pd_unconcentrated_max_hp(const pd_state *state);
/* Number of active bound effects; inactive bindings are still cleared at end. */
unsigned pd_concentration_effects(const pd_state *state);
/* Ends every binding, deactivates bound effects together and caps HP once.
   Accepted states guarantee valid totals without concentration; cannot fail. */
void pd_concentration_end(pd_state *state);
/* Index == count appends. Invalid totals reject without changing the state.
   A lower maximum caps current HP; a higher maximum never heals. */
bool pd_effect_set(pd_state *state,unsigned index,pd_effect effect);
bool pd_effect_remove(pd_state *state,unsigned index);
/* One explicit round for all active timed effects at once; output bitmasks
   and state remain untouched on invalid final totals. Zero rounds means expired. */
bool pd_effect_advance(pd_state *state,uint8_t *advanced,uint8_t *expired);
/* A single explicit round for all timed numeric effects and temporary defences.
   A rejected numeric expiry leaves all timers and output unchanged. */
bool pd_round_advance(pd_state *state,pd_round_summary *summary);
enum { PD_TICK_ROUND,PD_TICK_START,PD_TICK_END,PD_TICK_MINUTES,PD_TICK_HOURS,PD_TICK_COUNT };
/* Explicit own-turn edge; start also resets reaction, atomically with expiry. */
bool pd_turn_advance(pd_state *state,bool start,pd_round_summary *summary);
/* Elapsed game time 1..5940 minutes; chronological numeric expiry, atomic rollback. */
bool pd_time_advance(pd_state *state,unsigned minutes,pd_round_summary *summary);
unsigned pd_defence_base_flags(const pd_state *state,unsigned type);
/* Only explicit ALWAYS grants; conditional profiles need impact evaluation. */
unsigned pd_defence_flags(const pd_state *state,unsigned type);
bool pd_defence_conditional(const pd_state *state,unsigned type);
bool pd_defence_rule_set(pd_state *state,unsigned type,bool temporary,unsigned property,unsigned condition);
bool pd_defence_set(pd_state *state,unsigned type,bool configured,unsigned flags);
bool pd_defence_temporary_set(pd_state *state,unsigned type,pd_defence_temporary defence);
/* Legacy setters above address slot 0; slots are stable and never compacted. */
const pd_defence_temporary *pd_defence_grant(const pd_state *state,unsigned type,unsigned slot);
bool pd_defence_grant_set(pd_state *state,unsigned type,unsigned slot,pd_defence_temporary defence);
enum { PD_COST_NONE,PD_COST_COUNTER,PD_COST_POWER,PD_COST_SLOT,PD_COST_PACT,PD_COST_ITEM,PD_COST_COUNT };
/* Slot indices are zero-based levels 1..9; only existing, imported pools qualify. */
unsigned pd_reduction_cost_max(const pd_state *state,unsigned kind,unsigned index);
unsigned pd_reduction_cost_current(const pd_state *state,unsigned kind,unsigned index);
const char *pd_reduction_cost_name(const pd_state *state,unsigned kind,unsigned index);
/* Explicit annotation/reset; already-spent spending rejects, repeated reset succeeds. */
bool pd_reaction_change(pd_state *state,bool spent);
/* The original flat fields remain expense 0; extra slots are independent, not alternatives.
   Draft edits may be invalid; pd_reduction_profile_set validates all costs together. */
pd_cost pd_profile_cost(const pd_reduction_profile *profile,unsigned row);
bool pd_profile_cost_set(pd_reduction_profile *profile,unsigned row,pd_cost cost);
bool pd_reduction_profile_set(pd_state *state,unsigned index,pd_reduction_profile profile);
int pd_modifier(int score);
void pd_hp(pd_state *state, int delta);
bool pd_resource_change(pd_state *state, unsigned index, int delta);
/* Manual balance adjustment; absent/zero-quantity objects reject without mutation. */
bool pd_item_charges_change(pd_state *state,unsigned index,int delta);
pd_roll pd_d20(int bonus, int mode, pd_random random, void *context);
uint32_t pd_crc32(const uint8_t *data, size_t size);
size_t pd_encoded_size(const pd_state *state);
size_t pd_encode(const pd_state *state, uint32_t generation, uint8_t out[PD_RECORD_SIZE]);
bool pd_decode(const uint8_t *data, size_t size, pd_state *state, uint32_t *generation);
bool pd_record_known(const uint8_t *data,size_t size);
/* -1: neither record valid; otherwise newest valid bank, including uint32 wrap. */
int pd_select(const uint8_t *a, size_t as, const uint8_t *b, size_t bs,
              pd_state *state, uint32_t *generation);
/* Strict USB projection schema; never accepts a full Beyond response or a PC backup. */
bool pd_parse_packet(const char *json, size_t size, pd_state *state);
#endif
