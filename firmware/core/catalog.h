#pragma once
#include "pocketdd.h"
#define PD_SEARCH_BYTES 21
enum { PD_CATALOG_ITEMS,PD_CATALOG_SPELLS,PD_CATALOG_FEATURES,PD_CATALOG_POWERS };
/* ASCII literal substring, case-insensitive, at most 20 characters. Trim outer
   spaces and display uppercase; empty means all. Invalid input preserves out.
   No game-state mutation, allocation, NVS or RNG. In-place normalization works. */
bool pd_search_query(char out[PD_SEARCH_BYTES],const char *text);
unsigned pd_catalog_count(const pd_state *state,unsigned catalog,const char *query);
/* Map a filtered position to the original item/power index. -1 if absent. */
int pd_catalog_index(const pd_state *state,unsigned catalog,const char *query,unsigned position);
