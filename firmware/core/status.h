#ifndef PD_STATUS_H
#define PD_STATUS_H
#include "pocketdd.h"
enum { PD_DEATH_RESET,PD_DEATH_STABILIZE,PD_DEATH_SUCCESS,PD_DEATH_FAILURE,PD_DEATH_CRITICAL,PD_DEATH_MINUS_SUCCESS,PD_DEATH_MINUS_FAILURE };
bool pd_death_ready(const pd_state *s);
bool pd_death_change(pd_state *s,unsigned action);
bool pd_death_face(pd_state *s,unsigned face);
bool pd_death_roll(const pd_state *s,pd_random random,void *context,pd_state *next,pd_roll *roll);
/* Explicit circumstances for this roll only. Multiple sources of either sign
   do not stack; one of each cancels. No ability/proficiency or condition inference. */
typedef struct { bool advantage,disadvantage; int bonus; } pd_death_roll_options;
int pd_death_mode(pd_death_roll_options options);
/* Natural 1/20 retain their special effects, independently of the signed bonus.
   Other faces use face + bonus against 10. Bonus range -30..30. */
bool pd_death_face_bonus(pd_state *s,unsigned face,int bonus);
/* Rejects unavailable rolls/options before RNG; rejected inputs leave outputs
   untouched. On success only next is changed; source and persistent formats stay intact. */
bool pd_death_roll_configured(const pd_state *s,pd_death_roll_options options,pd_random random,void *context,pd_state *next,pd_roll *roll);
/* Conditions are annotations. Poison is an independent, explicit mechanical flag. */
bool pd_condition_add(pd_state *s,const char *name,unsigned level);
bool pd_condition_level(pd_state *s,unsigned index,int delta);
bool pd_condition_remove(pd_state *s,unsigned index);
#endif
