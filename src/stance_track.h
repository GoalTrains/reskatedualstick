/* v5.46 test: track turns relative to takeoff. Unrelated ground flags must not
 * overwrite a completed turn or swap the feet again under a held manual. */
#ifndef SC_STANCE_TRACK_H
#define SC_STANCE_TRACK_H
typedef struct {
    int initialized, prev_state, prev_raw, reference, takeoff_stance, stance;
    double turn_time;
    double landing_turn_ms, landed_at, late_turn_ms;
    int saw_air_edge, pending_reference, pending_stance;
    int settle_touchdown;
} sc_stance_track;

static int sc_track_stance(sc_stance_track* t, int state, int raw,
                           int spawned, int initial_stance, double now) {
    int valid = raw == 0 || raw == 1;
    t->landing_turn_ms = t->late_turn_ms = -1;
    if (!t->initialized || spawned) {
        t->initialized = 1;
        t->stance = valid ? raw : (initial_stance > 0);
        t->reference = t->takeoff_stance = t->pending_reference = -1;
        t->saw_air_edge = 0;
        t->prev_raw = -1;
        t->turn_time = 0;
    } else {
        int prev = t->prev_state;
        int takeoff = prev >= 100 && prev < 200 && prev != 101 && state >= 200 && state < 500;
        int revert = prev >= 100 && prev < 200 && prev != 101 && state == 101;
        int landed = prev >= 200 && prev < 500 && state >= 100 && state < 200;
        int end_slide = prev == 101 && state != 101;
        /* A landing into state 101 is still a landing: commit the turn once,
         * rather than waiting for the slide to end and reconsidering the feet. */
        if ((landed || end_slide) && t->reference >= 0 && t->takeoff_stance >= 0) {
            /* The game can publish the final edge on the touchdown sample.
             * Using the previous sample here locked in the temporary 1->0
             * blip from the v5.46 log instead of its final 1. Later resets
             * remain ignored after this landing decision is committed. */
            int final_raw = valid ? raw : t->prev_raw;
            if (final_raw >= 0) t->stance = final_raw != t->reference ? !t->takeoff_stance : t->takeoff_stance;
            /* Accept one late edge only when no turn was observed in flight. */
            if (landed && (!t->saw_air_edge || t->settle_touchdown) && final_raw == t->reference) {
                t->pending_reference = t->reference;
                t->pending_stance = t->takeoff_stance;
                t->landed_at = now;
            } else t->pending_reference = -1;
            t->reference = t->takeoff_stance = -1;
        }
        if (landed && t->turn_time > 0) {
            t->landing_turn_ms = now - t->turn_time;
            t->turn_time = 0;
        }
        if (takeoff || revert || (end_slide && state >= 200 && state < 500)) {
            t->reference = t->prev_raw >= 0 ? t->prev_raw : (valid ? raw : -1);
            t->saw_air_edge = 0; t->pending_reference = -1;
            t->takeoff_stance = t->stance;
            t->turn_time = 0;
        }
        if (t->reference >= 0 && t->takeoff_stance >= 0 && state >= 200 && state < 300 && valid) {
            int wanted = raw != t->reference ? !t->takeoff_stance : t->takeoff_stance;
            if (raw != t->reference) t->saw_air_edge = 1;
            if (wanted != t->stance) { t->stance = wanted; t->turn_time = now; }
        }
        if (t->pending_reference >= 0) {
            if (state < 100 || state >= 200 || now - t->landed_at > 500.0) t->pending_reference = -1;
            else if (valid && raw != t->pending_reference) {
                t->stance = !t->pending_stance;
                t->late_turn_ms = now - t->landed_at;
                t->pending_reference = -1;
            }
        }
    }
    t->prev_state = state;
    if (valid) t->prev_raw = raw;
    else if (spawned) t->prev_raw = -1;
    return t->stance;
}
/* v5.60: the goofy True Foot trace shows d returning to zero after a
 * completed 180, like b/c. Treat that route as takeoff-relative, while
 * preserving the tested direct d route for regular and the other modes.
 * This does not count arbitrary ground edges as turns. */
static int sc_resolve_stance(sc_stance_track* t, int source, int goofy,
                             int mode, int front_back, int state, int raw,
                             int spawned, int initial, double now) {
    if (source == 4 && !(goofy && mode == 3 && !front_back)) {
        t->landing_turn_ms = t->late_turn_ms = -1;
        return raw >= 0 ? raw : (initial >= 0 ? initial : 0);
    }
    /* A spawn grace period can include the first trick. Only reset the
     * goofy tracker on actual off-board/spawn states, not on every poll
     * for 1.5 seconds while already riding. */
    if (source == 4) spawned = state == 504 || state == 300 || state == 700;
    t->settle_touchdown = source == 4;
    return sc_track_stance(t, state, raw, spawned, initial, now);
}
#endif
