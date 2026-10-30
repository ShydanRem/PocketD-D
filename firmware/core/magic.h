#ifndef PD_MAGIC_H
#define PD_MAGIC_H
#include "pocketdd.h"
enum {PD_TRUE_STRIKE_2024=6,PD_HEX_2024=7,PD_HUNTERS_MARK_2024=8};
/* Active concentration only; target eligibility is always a caller decision. */
int pd_on_hit_spell(const pd_state *s);
bool pd_magic_valid(const pd_state *s);
bool pd_magic_stats(const pd_state *s,unsigned index,int *attack,int *dc);
/* Validate all costs before changing anything. Effects are limited to recognized identities. */
bool pd_magic_use(pd_state *s,unsigned index,unsigned slot,bool pact);
/* Explicit local preparation/availability of a spell. Does not enforce class
   selection limits, consume pools, or end existing concentration/bindings.
   Rejects missing/malformed state and feature rows without mutation. */
bool pd_magic_prepare(pd_state *s,unsigned index,bool prepared);
bool pd_magic_recover(pd_state *s,unsigned index,int delta);
bool pd_slot_change(pd_state *s,unsigned level,bool pact,int delta);
void pd_end_rage(pd_state *s);
#endif
