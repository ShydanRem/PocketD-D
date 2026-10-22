#pragma once
#include "history.h"
bool pd_history_store_load(uint32_t epoch,pd_history *history);
bool pd_history_store_save(const pd_history *history);
const char *pd_history_store_status(void);
bool pd_history_store_available(void);
/* Skip every identity still present in either bank, including abandoned restores. */
bool pd_history_store_next_epoch(uint32_t current,uint32_t *next);
/* Restore writes the inactive bank for a NEW epoch first. No active state changes
   until the caller has committed its matching character record. */
bool pd_history_store_prepare_restore(const pd_history *history);
void pd_history_store_finish_restore(void);
void pd_history_store_cancel_restore(void);
/* A failed state commit may already be durable: preserve BOTH logs until reboot. */
void pd_history_store_uncertain_restore(void);
