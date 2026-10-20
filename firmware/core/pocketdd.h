#ifndef POCKETDD_H
#define POCKETDD_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define PD_RESOURCES 8
#define PD_NAME_BYTES 49
#define PD_RESOURCE_BYTES 33
#define PD_RECORD_SIZE 384
#define PD_PACKET_MAX 4096

typedef struct { char name[PD_RESOURCE_BYTES]; uint16_t current, maximum; } pd_resource;
typedef struct { uint8_t skin,hair,cloak,style; } pd_avatar;
typedef struct {
    char name[PD_NAME_BYTES];
    uint16_t hp, max_hp, temp_hp;
    uint8_t level, armor, abilities[6], resource_count;
    pd_resource resources[PD_RESOURCES];
    pd_avatar avatar;
    bool idle_animation;
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
/* -1: neither record valid; otherwise newest valid bank, including uint32 wrap. */
int pd_select(const uint8_t *a, size_t as, const uint8_t *b, size_t bs,
              pd_state *state, uint32_t *generation);
/* Strict USB projection schema; never accepts a full Beyond response or a backup v9/v10. */
bool pd_parse_packet(const char *json, size_t size, pd_state *state);
#endif
