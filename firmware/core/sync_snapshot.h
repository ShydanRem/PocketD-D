#pragma once
#include "history.h"
typedef struct {uint32_t epoch;uint16_t hp,max,temp;char name[49];pd_history history;} pd_sync_sample;
/* Only the saved history and HP snapshot: no PG JSON, configuration or credentials. Caller frees. */
char *pd_sync_encode(const pd_sync_sample *s,uint32_t counter,const char *boot);
