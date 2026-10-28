#pragma once
#include "history.h"
enum {PD_SYNC_OFF,PD_SYNC_CONNECTING,PD_SYNC_BASELINE,PD_SYNC_READY,PD_SYNC_ERROR,PD_SYNC_CONFLICT,PD_SYNC_UNCERTAIN,PD_SYNC_REBIND,PD_SYNC_WAIT_NETWORK};
typedef struct {unsigned state,queued,observed;bool configured,offer;char character_id[16],host[16],fingerprint[33];} pd_sync_info;
void pd_sync_boot(void);
void pd_sync_observe(const pd_state *state,const pd_history *history,bool durable);
void pd_sync_snapshot(pd_sync_info *out);
bool pd_sync_stage_json(const char *json,size_t size);
bool pd_sync_decide(bool accept,uint32_t epoch);
bool pd_sync_forget(void);

bool pd_sync_pause(bool pause);
