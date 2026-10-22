#pragma once
#include "history.h"
bool pd_history_store_load(uint32_t epoch,pd_history *history);
bool pd_history_store_save(const pd_history *history);
const char *pd_history_store_status(void);
