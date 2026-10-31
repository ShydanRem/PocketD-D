#pragma once
#include "backup.h"
/* Device-local native snapshots, never Beyond JSON or account credentials. */
#define PD_LIBRARY_LIMIT 32
typedef enum { PD_LIBRARY_EMPTY,PD_LIBRARY_OK,PD_LIBRARY_RECOVERED,PD_LIBRARY_CORRUPT,
    PD_LIBRARY_FUTURE,PD_LIBRARY_IO,PD_LIBRARY_MEMORY,PD_LIBRARY_FULL,PD_LIBRARY_CONFLICT } pd_library_result;
typedef struct { unsigned slot;pd_library_result status;char name[PD_NAME_BYTES];uint8_t level;uint32_t epoch;uint64_t owner; } pd_library_entry;
typedef struct { pd_library_entry entries[PD_LIBRARY_LIMIT];unsigned count; } pd_library_catalog;
typedef struct { char root[192];uint64_t owner;void (*yield)(void *);void *context; } pd_library;
bool pd_library_init(pd_library *library,const char *root,uint64_t owner,void (*yield)(void *),void *context);
/* No output changes on an I/O, memory or unsupported-format failure. */
pd_library_result pd_library_read(const pd_library *library,unsigned slot,pd_backup *out);
bool pd_library_scan(const pd_library *library,pd_library_catalog *out);
/* slot=0 finds this local epoch or creates a new entry; no matching by name.
   Explicit slot replaces that entry's inactive bank only, for a confirmed switch.
   Both banks are validated; a future format is never overwritten. */
pd_library_result pd_library_save(const pd_library *library,unsigned slot,const pd_state *state,const pd_history *history,unsigned *saved_slot);
