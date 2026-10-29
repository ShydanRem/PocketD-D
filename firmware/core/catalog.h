#pragma once
#include "pocketdd.h"
#define PD_SEARCH_BYTES 21
enum { PD_CATALOG_ITEMS,PD_CATALOG_SPELLS,PD_CATALOG_FEATURES,PD_CATALOG_POWERS };
enum { PD_CATALOG_ALL,PD_CATALOG_READY,PD_CATALOG_POUCH };
/* ASCII literal substring, case-insensitive, at most 20 characters. Trim outer
   spaces and display uppercase; empty means all. Invalid input preserves out.
   No game-state mutation, allocation, NVS or RNG. In-place normalization works. */
bool pd_search_query(char out[PD_SEARCH_BYTES],const char *text);
unsigned pd_catalog_count(const pd_state *state,unsigned catalog,const char *query);
/* Map a filtered position to the original item/power index. -1 if absent. */
int pd_catalog_index(const pd_state *state,unsigned catalog,const char *query,unsigned position);
/* View-only filters, preserving original indices. READY: prepared spells with
 * pools/slots, active-use features with available costs. POUCH: carried
 * consumables/healing items and objects with remaining charges. The actual
 * command still validates all rules and costs before execution. */
unsigned pd_catalog_filtered_count(const pd_state *state,unsigned catalog,const char *query,unsigned filter);
int pd_catalog_filtered_index(const pd_state *state,unsigned catalog,const char *query,unsigned filter,unsigned position);
