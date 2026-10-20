#ifndef POCKETDD_H
#define POCKETDD_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define PD_RESOURCES 8
#define PD_NAME_BYTES 49
#define PD_RESOURCE_BYTES 33
#define PD_RECORD_SIZE 5184
#define PD_ITEMS 60
#define PD_ITEM_NAME_BYTES 41
#define PD_NO_WEAPON 255
#define PD_SKILLS 18
#define PD_PACKET_MAX 32768

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
typedef struct {
    char name[PD_ITEM_NAME_BYTES],damage_type[25];
    uint16_t quantity;
    uint8_t kind,ability,count,sides,flags,reach,thrown_range;
    int8_t attack_modifier,damage_modifier,healing_modifier;
} pd_item;
typedef struct { uint8_t rage_bonus,critical_dice; bool raging,power_attack,thrown; } pd_combat;
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
} pd_state;
typedef struct { uint8_t dice[2], count, natural; int total, bonus; } pd_roll;
typedef uint32_t (*pd_random)(void *context);
void pd_demo(pd_state *state);
bool pd_valid(const pd_state *state);
int pd_modifier(int score);
void pd_hp(pd_state *state, int delta);
bool pd_resource_change(pd_state *state, unsigned index, int delta);
pd_roll pd_d20(int bonus, int mode, pd_random random, void *context);
uint32_t pd_crc32(const uint8_t *data, size_t size);
void pd_encode(const pd_state *state, uint32_t generation, uint8_t out[PD_RECORD_SIZE]);
bool pd_decode(const uint8_t *data, size_t size, pd_state *state, uint32_t *generation);
bool pd_record_known(const uint8_t *data,size_t size);
/* -1: neither record valid; otherwise newest valid bank, including uint32 wrap. */
int pd_select(const uint8_t *a, size_t as, const uint8_t *b, size_t bs,
              pd_state *state, uint32_t *generation);
/* Strict USB projection schema; never accepts a full Beyond response or a backup v9/v10. */
bool pd_parse_packet(const char *json, size_t size, pd_state *state);
#endif
