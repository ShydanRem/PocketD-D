#pragma once
#include <stdint.h>
/* Non-secret install identity, persisted before any SD checkpoint. A factory
   reset gets a new identity so stale SD epochs cannot overwrite another PG. */
uint64_t pd_library_owner(void);
