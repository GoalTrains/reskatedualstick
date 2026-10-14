#ifndef SC_PAUSE_GATE_H
#define SC_PAUSE_GATE_H
/* Physics stalls are not evidence of a menu when ScreenData says closed.
 * Keep the inactivity fallback only for unavailable native UI information. */
static int sc_pause_gate(int native_menu,int actor_known,double quiet_ms) {
    return native_menu==1 || (native_menu<0 && actor_known && quiet_ms>200);
}
#endif
