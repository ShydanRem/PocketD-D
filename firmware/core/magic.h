#ifndef PD_MAGIC_H
#define PD_MAGIC_H
#include "pocketdd.h"
bool pd_magic_valid(const pd_state *s);
bool pd_magic_stats(const pd_state *s,unsigned index,int *attack,int *dc);
/* Validate all costs before changing anything. Effects are limited to recognized identities. */
bool pd_magic_use(pd_state *s,unsigned index,unsigned slot,bool pact);
bool pd_magic_recover(pd_state *s,unsigned index,int delta);
bool pd_slot_change(pd_state *s,unsigned level,bool pact,int delta);
void pd_end_rage(pd_state *s);
#endif
