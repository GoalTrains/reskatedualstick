#ifndef SC_OVERLAY_HANDOFF_H
#define SC_OVERLAY_HANDOFF_H
/* Drop synthetic gestures during another UI's ownership. Held foot positions
 * need release after return, so closing a menu cannot create a fresh pop. */
static void sc_overlay_suspend(sc_state* state) {
    sc_state empty = {0};
    *state = empty;
    state->d_front_ignored = state->d_back_ignored = 1;
    state->d_front_cign = state->d_back_cign = 1;
}
#endif
