#pragma once
#include "pocketdd.h"
bool pd_storage_load(pd_state *state);
bool pd_storage_save(const pd_state *state);
const char *pd_storage_status(void);
bool pd_storage_writable(void);
