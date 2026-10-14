/* Experimental remount correction. The recorded skater facing vector is
 * compared with measured world displacement, never with trigger input.
 * A steady, aligned ground run establishes the XOR reference for the game's
 * turn flag. Slow, sideways, airborne and teleport samples cannot establish it.
 * Once established, the reference stays fixed until getting off or respawning. */
#ifndef SC_MOTION_STANCE_H
#define SC_MOTION_STANCE_H
typedef struct {
    double time, x, z, candidate_since;
    int sampled, candidate, candidate_valid, bias, bias_valid;
    int seen_riding, remount_pending;
    double remount_started;
} sc_motion_stance;
static int sc_motion_stance_step(sc_motion_stance* s, int state, int valid,
                                double x, double z, double fx, double fz,
                                int raw, double now) {
    if (state == 300 || state == 700) {
        sc_motion_stance zero={0};zero.bias_valid=1;*s=zero;
        return -1;
    }
    if (state == 504) {
        /* A spawn's walking state is not a dismount. Arm calibration only
         * after this skater has actually ridden, and keep it across walking. */
        if(s->seen_riding) {
            sc_motion_stance zero={0};zero.remount_pending=1;*s=zero;
        }
        s->sampled=s->candidate_valid=0;
        return -1;
    }
    if(!s->bias_valid && !s->remount_pending)s->bias_valid=1;
    if(state>=100 && state<300)s->seen_riding=1;
    if(s->remount_pending) {
        if(!s->remount_started && state>=100 && state<300)s->remount_started=now;
        /* A turn/trick before calibration invalidates facing/movement as a
         * remount reference. Never learn an inverted frame after that trick. */
        if((state>=200 && state<300) ||
           (s->remount_started && now-s->remount_started>2000)) {
            s->remount_pending=0;s->bias=0;s->bias_valid=1;
            s->sampled=s->candidate_valid=0;
        }
    }
    /* Ground turns/reverts and temporary turn-flag resets must not rewrite the
     * input-frame reference. It belongs to this mounting of the board. */
    if (s->bias_valid) return raw >= 0 ? raw ^ s->bias : -1;
    if (state != 100 || !valid || !(x > -1000000 && x < 1000000 &&
        z > -1000000 && z < 1000000 && fx >= -1.01 && fx <= 1.01 &&
        fz >= -1.01 && fz <= 1.01)) {
        s->sampled = s->candidate_valid = 0;
        return s->bias_valid && raw >= 0 ? raw ^ s->bias : -1;
    }
    if (s->sampled && now - s->time < 90) return s->bias_valid && raw >= 0 ? raw ^ s->bias : -1;
    double dt = now - s->time, dx = x - s->x, dz = z - s->z;
    int previous = s->sampled;
    s->sampled = 1; s->time = now; s->x = x; s->z = z;
    double distance2 = dx*dx + dz*dz, facing2 = fx*fx + fz*fz;
    double dot = dx*fx + dz*fz;
    /* 1..50 units/s and direction within about 25 degrees of forward/back. */
    if (!previous || !(dt >= 90 && dt <= 250 && raw >= 0 && raw <= 1 &&
        facing2 >= .7 && facing2 <= 1.05 &&
        distance2 >= dt*dt/1000000 && distance2 <= dt*dt*.0025 &&
        dot*dot >= distance2*facing2*.81)) {
        s->candidate_valid = 0;
        return s->bias_valid && raw >= 0 ? raw ^ s->bias : -1;
    }
    int bias = (dot < 0) ^ raw;
    if (!s->candidate_valid || bias != s->candidate) {
        s->candidate = bias; s->candidate_valid = 1; s->candidate_since = now;
    } else if (now - s->candidate_since >= 200) {
        s->bias = bias; s->bias_valid = 1;s->remount_pending=0;
    }
    return s->bias_valid ? raw ^ s->bias : -1;
}
#endif
