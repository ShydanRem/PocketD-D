#pragma once
#include "history.h"
#define PD_BACKUP_MAX 110000
typedef struct { pd_state state;pd_history history; } pd_backup;
/* Complete native snapshot, not a reduced USB character projection.
   Caller owns the allocated JSON buffer. No NVS or entropy access. */
char *pd_backup_encode(const pd_state *state,const pd_history *history,size_t *size);
/* Current backup v1: PDD25/PDD26/PDD27 and PDH1, exact keys, canonical base64, matching epoch.
   Both native decoders validate before the output changes. */
bool pd_backup_decode(const char *json,size_t size,pd_backup *out);
typedef bool (*pd_backup_writer)(const char *data,size_t size,void *context);
bool pd_backup_transmit(const pd_state *state,const pd_history *history,pd_backup_writer writer,void *context);
