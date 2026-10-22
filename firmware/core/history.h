#pragma once
#include "pocketdd.h"
#define PD_HISTORY_LIMIT 20
#define PD_HISTORY_LABEL 41
#define PD_HISTORY_NOTE 49
#define PD_HISTORY_DICE 18
#define PD_HISTORY_RECORD 2481
enum { PD_H_FREE,PD_H_CHECK,PD_H_SKILL,PD_H_SAVE,PD_H_ATTACK,PD_H_DAMAGE,
       PD_H_HEAL,PD_H_HIT_DIE,PD_H_DEATH,PD_H_CONCENTRATION,PD_H_RECHARGE,PD_H_KINDS };
typedef struct {
    uint32_t sequence;
    char label[PD_HISTORY_LABEL],note[PD_HISTORY_NOTE];
    uint8_t kind,count,sides,dice[PD_HISTORY_DICE];
    int8_t mode; /* Effective: -1 disadvantage, 0 sum/single, 1 advantage. */
    bool floor_zero;
    int bonus,total;
} pd_history_entry;
typedef struct { uint32_t epoch,sequence;uint8_t count;pd_history_entry entries[PD_HISTORY_LIMIT]; } pd_history;
void pd_history_reset(pd_history *history,uint32_t epoch);
bool pd_history_entry_valid(const pd_history_entry *entry);
bool pd_history_valid(const pd_history *history);
/* Newest first; a failed append leaves history untouched. */
bool pd_history_append(pd_history *history,const pd_history_entry *entry);
size_t pd_history_encode(const pd_history *history,uint32_t generation,uint8_t out[PD_HISTORY_RECORD]);
bool pd_history_decode(const uint8_t *data,size_t size,pd_history *history,uint32_t *generation);
