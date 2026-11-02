#pragma once
#include "beyond_runtime.h"
#include "beyond_account.h"
enum {PD_BEYOND_IDLE,PD_BEYOND_CLOCK,PD_BEYOND_HTTP,PD_BEYOND_CONVERTING,PD_BEYOND_FINISHED,PD_BEYOND_FAILED,PD_BEYOND_CANCELLING};
enum {PD_BEYOND_NETWORK=100,PD_BEYOND_NO_WIFI,PD_BEYOND_TIME,PD_BEYOND_DENIED,PD_BEYOND_NOT_FOUND,PD_BEYOND_RATE,PD_BEYOND_REDIRECT,PD_BEYOND_MIME,PD_BEYOND_SIZE,PD_BEYOND_BUSY,PD_BEYOND_SESSION,PD_BEYOND_IDENTITY,PD_BEYOND_CATALOG_ERROR,PD_BEYOND_STORAGE};
enum {PD_BEYOND_PUBLIC,PD_BEYOND_LIST,PD_BEYOND_OWNED,PD_BEYOND_LINK};
typedef struct {unsigned phase,error,http,operation,count;size_t bytes,peak;bool running,configured,remembered,partial,locked;char id[16];} pd_beyond_info;
typedef struct {pd_state state;pd_beyond_report report;} pd_beyond_candidate;
void pd_beyond_boot(void);
bool pd_beyond_start(const char *id);
void pd_beyond_cancel(void);
void pd_beyond_snapshot(pd_beyond_info *out);
/* Serialize the USB parser with the transition into native conversion. A
   successful begin holds the service mutex until end; never call another
   service API while holding it. Prevents a check-then-parse cJSON race. */
bool pd_beyond_usb_begin(void);
void pd_beyond_usb_end(void);
/* Caller owns the returned candidate, only after the worker has freed source/VM. */
pd_beyond_candidate *pd_beyond_take(void);

bool pd_beyond_list(void);
bool pd_beyond_owned(unsigned index);
bool pd_beyond_link(const pd_account_access *access,bool remember);
bool pd_beyond_row(unsigned index,pd_account_row *out);
bool pd_beyond_forget(void);
