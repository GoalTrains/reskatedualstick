/* Supported 20260929 states: ReSkate launches.h and native checks classify
 * 701 as grinding alongside 400..499. 700 is respawn, not a grind.
 * Classify the observed state directly; no remembered state or grace timer. */
#ifndef SC_PHYSICS_STATES_H
#define SC_PHYSICS_STATES_H
static int sc_is_grind_state(int state){return (state>=400 && state<500) || state==701;}
#endif
