#pragma once
#include "pocketdd.h"
#define PD_BEYOND_SOURCE_MAX (1024u*1024u)
#define PD_BEYOND_HEAP_MAX (4u*1024u*1024u)
#define PD_BEYOND_REPORT_LINES 64
enum { PD_BEYOND_READY,PD_BEYOND_BLOCKED,PD_BEYOND_INVALID,PD_BEYOND_MEMORY,PD_BEYOND_TIMEOUT,PD_BEYOND_CANCELLED,PD_BEYOND_ENGINE };
typedef struct {unsigned blockers,limits,count;char lines[PD_BEYOND_REPORT_LINES][161];} pd_beyond_report;
typedef struct {
    size_t heap_limit,peak_bytes;
    bool exhausted;unsigned phase;
    /* Called also during regexp execution. Nonzero aborts, with a public code.
       Device callback owns time/cancellation; no platform dependency here. */
    unsigned (*interrupt)(void *);
    void *context;
} pd_beyond_control;
/* Bounded local conversion. Output PG changes only after the same strict C
   packet parser used by USB accepts the projection. No storage/network/RNG. */
unsigned pd_beyond_convert(const char *source,size_t size,const char *id,pd_state *out,pd_beyond_report *report,pd_beyond_control *control);
bool pd_beyond_id(const char *id);

bool pd_beyond_utf8(const unsigned char *source,size_t size);
