/* Session-style dual-stick controls for skate. (reSkate) - v0.4 trick engine.
 * Pure C, no Windows dependencies, so every trick can be unit tested.
 *
 * Physical pad (Session layout):
 *   left stick = left foot, right stick = right foot (regular: left foot is the front foot)
 *   LT / RT    = turn left / right (and spin in the air), LB / RB = grabs
 * What the game gets (skate. flick-it layout):
 *   right stick = flick-it, left stick X = steer / spin, triggers = grabs
 *
 * How a trick is read (rider space, regular):
 *   POP     pull a foot stick down. Back foot = ollie family, front foot = nollie family.
 *           While held, the game sees the pop direction (the skater crouches).
 *   SCOOP   sweep the popping foot's stick sideways (shove-it), or about halfway round (360).
 *           A full circle is an impossible (circling to the right) or the other foot's
 *           impossible (circling to the left).
 *   FLICK   move the OTHER foot's stick: straight = ollie / nollie, out to the right =
 *           kickflip side, out to the left = heelflip side. The flick fires the trick.
 *   Releasing the popping foot after a scoop with no flick fires the shove-it.
 * The trick is then drawn on the flick stick using the game's own stick path for it
 * (tricks.h, generated from the game's gesture assets).
 */
#ifndef SESSION_TRANSLATE_H
#define SESSION_TRANSLATE_H

#include "tricks.h"

typedef struct {
    unsigned short buttons;
    unsigned char lt, rt;
    short lx, ly, rx, ry;
} sc_pad;

typedef struct {
    int enabled;          /* translation on */
    int goofy;            /* right foot is the front foot */
    int feet_swapped;     /* runtime toggle for riding switch */
    int flip_x;           /* mirror sideways flicks and scoops */
    int grab_mode;        /* 0 = no grabs, 1 = LB/RB grab, 2 = also a fully pulled trigger */
    int grab_threshold;   /* 0-255, mode 2 only */
    float deadzone;       /* per foot stick, 0-1 */
    float steer_scale;    /* trigger steering strength */
    float steer_curve;    /* 1 = linear, >1 = softer near zero */
    float active;         /* deflection (after deadzone) that counts as a pop / flick, 0-1 */
    float point_ms;       /* how long each point of a drawn trick is held */
    int pop_release_ollie;/* releasing a pop with no flick or scoop = ollie / nollie */
    float shuv_degrees;   /* scoop sweep that counts as a shove-it */
    float shuv360_degrees;/* scoop sweep that counts as a 360 shove-it */
    float lean_ms;        /* a light lean must be held this long before it becomes a manual */
    float walk_ms;        /* left stick held forward/sideways this long with no pop = walking (0 = off) */
    float ms_per_unit;    /* drawing speed: ms per unit of stick travel (down->up is about 1.9 units) */
    float min_path_ms;    /* a drawn trick never takes less than this */
    float hold_end_ms;    /* hold the last point of a drawn trick this long before letting go */
    int direct;           /* 1 = no trick drawing: pass the feet through (last foot to move wins) */
    float low_pop_ms;     /* direct mode: low-pop moves may travel 1 unit of stick per this many ms */
    float combo_wait_ms;  /* direct mode: after a scoop, how long to wait for the other foot's flick */
    float recenter_ms;    /* before drawing a trick that starts out to the side, centre the stick this long */
    int scoop_mirror;     /* direct mode: swap which scoop direction is backside / frontside */
    float windup_deg;     /* direct mode: a pop foot first pressed further than this from its pop
                           * direction (side / diagonal) is a stock-style wind-up, passed straight
                           * through (0 = off) */
    int quick_pops;       /* direct mode: pop foot held + other foot flicked towards the pop end
                           * (back down then front down) = quick ollie / quick nollie */
    int air_controls;     /* direct mode: in the air both feet steer the board (grinds), Session style */
    float grind_sensitivity; /* rail adjustment gain only; zero keeps the historical 1.0 gain */
    float grab_double_ms; /* >0: grab = double-press and hold LB / RB; a single press stays a bumper
                           * (handplants). 0 = any press grabs */
    int powerslide;       /* both sticks sideways the same way = powerslide */
    float ps_delay_ms;    /* hold this long before the powerslide starts (keeps a double tap clean) */
    float revert_ms;      /* revert = double-tap LT / RT: most ms between the two taps (0 = off) */
    int ps_flip;          /* swap toe side / heel side */
    int revert_flip;      /* swap revert spin direction */
    float revert_sweep_ms; /* how long the revert spin of the right stick takes (ms) */
    float revert_gap_ms;  /* the revert starts at least this long after the first tap was let go */
    float revert_min_gap_ms; /* double taps quicker than this (release to press) are not reverts */
    int revert_shape;     /* 1-4, see the revert code */
    float bump_ms;        /* v0.23: how long a grind bump-out holds the left stick to the side */
    int prewind;          /* v0.21: turning while crouched winds up a spin. 0 = off, 1-3 = stick shape */
    int prewind_steer;    /* light trigger steers while crouched; strong pull winds up */
    int prewind_percent;  /* strong net trigger threshold, 50-90 percent */
    float prewind_ms;     /* how long the move into the wind-up takes */
    float grab_hold_ms;   /* in the air, a bumper held this long is a grab (shorter = dark catch tap) */
    int switch_flip;      /* 1 = riding switch turns the flick stick round (y), 2 = y and x, 3 = true foot */
    int switch_invert_x, switch_invert_y; /* optional custom foot-input directions; default unchanged */
    int disable_hold_fakie; /* native hold-fakie compatibility; saved preferences untouched */
    int xl_layout;        /* physical feet; keep each physical stick setup direction in switch */
    int pop_on_release;   /* v1.1: letting go of a full crouch pops (Skater XL style) */
    int landing_manuals;  /* v2.6: a lean held into a landing is a manual: 0 never, 1 after a 180 only, 2 always */
    float manual_max;     /* v2.8: a lean up to this much is a manual; further is a crouch (0-1, default 0.6) */
    int pure;             /* v3.0: PURE - nothing invented: each foot's stick goes straight to the game */
    int revert_input;     /* v5.7: 0 = off, 1 = double tap LT / RT, 2 = squeeze both triggers */
    int switch_goofy_flip; /* v4.3: goofy riding switch also turns the flick stick round (your test: it must not) */
    float bf_grab_ms;     /* v3.5: how long a body flip holds its grab (stock: ~450 ms) */
    int one_foot_flips;   /* v5.27: 1 = a foot on its own may sweep round into the far half (one-stick heelflips, impossibles) */
    int front_back;       /* v5.26: 1 = FRONT / BACK FOOT layout (left stick always the front foot, Session's other option) */
    int stick_steer;      /* v5.15: front-foot stick pushed sideways (other stick let go) turns, Skater XL style */
    int stock_grab_air;   /* optional: stock sticks/triggers after a steep grounded ramp approach */
    int stock_transition_ground; /* optional: switch during the qualifying climb */
    int body_flips;       /* v3.2: click the front-foot stick = front flip, back-foot stick = backflip */
} sc_config;

enum { SC_IDLE, SC_ARMED, SC_EMIT, SC_COOLDOWN };

typedef struct {
    int phase;
    int pop_back;         /* 1 = back foot popped (ollie family), 0 = front foot (nollie family) */
    float sweep;          /* signed degrees the popping foot has swept since the pop */
    float last_angle;
    int tracking;         /* the popping stick is far enough out for its angle to mean something */
    float side;           /* the popping foot's furthest sideways reach (signed) */
    int trick;            /* index into sc_tricks while drawing */
    double emit_start;
    int emit_index;       /* point being drawn; advances at most once per poll */
    const char* last_name;/* last trick fired, for the log */
    unsigned fired;       /* count of tricks fired */
    double lean_since;    /* when the current light lean started (0 = none) */
    double walk_since;    /* when the left stick started looking like walking (0 = no) */
    int walk_request;     /* set when walking is detected; the caller turns Session mode off */
    /* diagnostics for the log */
    double pop_start;
    float pop_angle, pop_mag, max_mag;
    int event;            /* 1 = pop started, 2 = pop ended (fired or not), 3 = strong move that is not a pop */
    float ev_angle, ev_mag;
    int ev_foot_back;
    int ev_logged;
    /* drawing */
    float ox, oy;         /* last flick-stick output */
    float from_x, from_y; /* where the stick was when the trick fired */
    int seg;              /* next path vertex that has not been shown yet (1-based; 0 = start point) */
    double hold_start;
    float emit_ms_per_unit; /* drawing speed for this trick (slower = low pop) */
    float emit_last;      /* v1.9: how far along the path the game was last shown */
    int game_known;       /* set by the caller: 1 = the game's state is being read (board detection on) */
    int emit_wait; double emit_wait_t; float emit_crouch_y; /* v2.3: hold the crouch until the game has crouched */
    double recenter_until;  /* stick held at centre until then, before the drawing starts */
    const float (*path)[2]; int path_n; /* test mode: draw this path instead of sc_tricks[trick] */
    /* direct mode (last foot to move wins) */
    int d_front_on, d_back_on, d_front_spent, d_back_spent;
    unsigned d_front_seq, d_back_seq, d_seq;
    int d_front_flick, d_back_flick; /* survives setup-foot release until flick stick centres */
    int d_two;
    double d_t;
    int d_front_ignored, d_back_ignored; /* moved the wrong way first: ignored until released */
    int d_pop;            /* 1 = back foot started this (ollie side), 2 = front foot (nollie side) */
    float d_sweep, d_last_ang, d_side; int d_track;
    int d_pending; double d_pend_since; float d_pend_peak; /* a scoop is being held back, waiting for a flick */
    int d_latch; float d_lx, d_ly;  /* a slowed flick keeps going to where the foot went */
    double d_windup_t;    /* when the wind-up started */
    int d_back_full, d_front_full;  /* v1.1 pop on release: the crouch went all the way */
    int rp_two_foot; /* second-foot flick is not another release pop */
    double rp_t; float rp_y, d_back_peak, d_front_peak; /* v1.1 pop on release in progress / deepest crouch */
    int d_back_was_down, d_front_was_up; double d_back_rel_t, d_front_rel_t; /* single-stick quick pops */
    int d_windup;         /* this press started from the side: pass through, no scoop detection */
    int ps_side, ps_tap_side, rv_side, ps_logged; double ps_pop_t, ps_start, ps_tap_t, rv_start; /* powerslides / reverts */
    int tr_down[2], rv_block, rv_hold, rv_both; /* v4.7 rv_hold: 1 = LT / 2 = RT of a revert still held: no steering from it */ double tr_press[2], tr_rel[2]; /* LT / RT double taps */
    int g_down[2], g_grab[2]; double g_press[2], g_rel[2], g_pulse[2]; /* LB / RB: handplant / dark catch / grab */
    int switched;         /* set by the caller: riding the other way round from the spawn */
    int xl_absolute_raw;
    int game_mirror_known, game_mirror; /* owned rider animation frame relative to normal baseline */
    int board_frame_valid, board_frame_left; /* continuous XL air/rail adjustment frame */
    int game_switch_known, game_switch; /* native input frame can reset separately on remount */
    int feet_map_ok, feet_map_left; /* v5.45: which physical stick owns each foot history */
    int sw_lat, sw_lat_ok; /* v5.42: the switch turn-round in use on the game stick (kept while it is held) */
    int sw_prev; double sw_blend_until, sw_t; float sw_x, sw_y; /* easing across a stance change */
    int pw_side, pw_logged, pw_nollie, pw_block, pw_was_air; double pw_t, pw_crouch_t; /* spin wind-up (prewind) in progress: -1 left, +1 right */
    int bump_side, bump_y, bump_prev; double bump_until,bump_neutral_until,bump_output_time; float bump_output_x,bump_output_y; /* directional grind / lip exit */
    int stock_air_active, stock_air_block_l, stock_air_block_r;
    float stock_air_langle, stock_air_rangle;
    int pump_r3;          /* standalone R3: stock R2 / RT crouch for pumping */
    double pump_press_t, pump_release_t; float pump_level, pump_release_level;
    double pf_t[2]; int pf_hold; /* v4.1 pure: when each foot started (0 front, 1 back) */
    int lp_blk[2], lp_dip[2]; float lp_ang[2], lp_mag[2]; /* v3.8 pure: one foot alone can't flick through to the other end */
    int bf_arm, bf_side, bf_was_air; double bf_arm_t, bf_t, bf_air_t; /* v3.2 body flips: -1 front flip, +1 backflip */
    int game_crouch;      /* set by the caller: -1 unknown, 0/1 = the game says crouched (state 103) */
    int stock_ramp_valid, stock_ramp_eligible, stock_ramp_was_air, stock_ramp_departing;
    float stock_ramp_vertical;
    double stock_ramp_since, stock_ramp_last_ground;
    double stock_ramp_evidence, stock_ramp_history_t[8];
    float stock_ramp_history_y[8];
    int stock_ramp_history_n, stock_ramp_history_next, stock_ramp_curve;
    int stock_early_active;
    int air;              /* set by the caller: the game says the skater is in the air / grinding */
    int d_was_air;
    int d_front_carry, d_back_carry; /* v1.5: foot held since take-off (left over from the pop / flick) */
    int d_front_cign, d_back_cign;   /* v1.5: ignored because it was carried through a landing */
    double d_land_t;      /* v2.5: when we last landed (pre-crouch grace) */
    int sw_raw, sw_takeoff; /* v2.6: the game's facing flag right now (-1 unknown) / at take-off */
    double ps_lt, ps_rt;  /* v2.5: when each stick went sideways (powerslide timing) */
    double d_takeoff_t;   /* v1.8: when the board left the ground */
    int takeoff;          /* v1.6, set by the caller: 1 = the game says in the air RIGHT NOW (no delay), 0 = not, -1 = unknown */
    int d_took_off;
    int d_land_hold;      /* feet held through a landing: they are manual leans, not pops */
    int grind;            /* set by the caller: the game says the skater is grinding */
    int grind_prev, grind_pop, grind_fpop, grind_bpop;
    int rail_scoop_track, rail_scoop_exit; /* 1 front / 2 back: explicit quick scoop versus rail pivot */
    float rail_scoop_angle, rail_scoop_sweep, rail_scoop_x, rail_scoop_y;
    double rail_scoop_last, rail_scoop_turn;
    int grind_fflick, grind_bflick; double grind_fpop_t, grind_bpop_t; /* separate rail adjustment from a fresh pop setup */
    int hold_foot; float hold_x, hold_y; /* v5.19: held trick: 1 front / 2 back foot still holding its flick, and the stick the game had */
    float tk_fa, tk_ba;   /* v5.16: angle each held foot had at take-off */
    float lone_a0[2], lone_last[2], lone_sw[2], lone_out[2]; int lone_unl[2]; /* v5.28: impossible circles */
    int lone_seen[2], lone_from[2]; /* v5.31: side last seen, and the side a sweep came from through the own end */
    int lone_side[2];     /* v5.27: a lone foot that started out at the side: which side (+1 right / -1 left) */
    double dp_t;          /* v5.25: last time a D-pad button was held (radial menus) */
    int st_latch;         /* v5.15: the front-foot stick is steering until it comes back to the middle */
    int d_log; float d_log_a, d_log_b; /* for the log: 1 = wind-up start (angle), 2 = wind-up flick
                                        * (angle, from-angle), 3 = combo (sweep, flick x) */
} sc_state;

/* Preserve the physical foot's history when TRUE FOOT swaps front/back roles.
 * Without this, an already-held stick inherited the other foot's spent/ignored
 * state and could disappear or become a fresh flick during a 180. */
static void sc_remap_foot_history(sc_state* s) {
    s->rail_scoop_track=s->rail_scoop_exit=0; s->rail_scoop_last=s->rail_scoop_turn=0;
#define SC_SWAP_FIELD(type, a, b) do { type v = s->a; s->a = s->b; s->b = v; } while (0)
    SC_SWAP_FIELD(int, d_front_on, d_back_on);
    SC_SWAP_FIELD(int, d_front_spent, d_back_spent);
    SC_SWAP_FIELD(int, d_front_flick, d_back_flick);
    SC_SWAP_FIELD(unsigned, d_front_seq, d_back_seq);
    SC_SWAP_FIELD(int, d_front_ignored, d_back_ignored);
    SC_SWAP_FIELD(int, d_front_carry, d_back_carry);
    SC_SWAP_FIELD(int, d_front_cign, d_back_cign);
    SC_SWAP_FIELD(float, tk_fa, tk_ba);
    SC_SWAP_FIELD(double, pf_t[0], pf_t[1]);
    SC_SWAP_FIELD(int, lp_blk[0], lp_blk[1]);
    SC_SWAP_FIELD(int, lp_dip[0], lp_dip[1]);
    SC_SWAP_FIELD(float, lp_ang[0], lp_ang[1]);
    SC_SWAP_FIELD(float, lp_mag[0], lp_mag[1]);
    SC_SWAP_FIELD(float, lone_a0[0], lone_a0[1]);
    SC_SWAP_FIELD(float, lone_last[0], lone_last[1]);
    SC_SWAP_FIELD(float, lone_sw[0], lone_sw[1]);
    SC_SWAP_FIELD(float, lone_out[0], lone_out[1]);
    SC_SWAP_FIELD(int, lone_unl[0], lone_unl[1]);
    SC_SWAP_FIELD(int, lone_seen[0], lone_seen[1]);
    SC_SWAP_FIELD(int, lone_from[0], lone_from[1]);
    SC_SWAP_FIELD(int, lone_side[0], lone_side[1]);
#undef SC_SWAP_FIELD
    if (s->hold_foot) s->hold_foot = 3 - s->hold_foot;
    if (s->d_pop) s->d_pop = 3 - s->d_pop;
    s->pf_hold = -s->pf_hold;
    /* A role change is not a new release-pop or quick-pop gesture. */
    s->d_front_full = s->d_back_full = 0;
    s->d_front_peak = s->d_back_peak = 0;
    s->d_front_was_up = s->d_back_was_down = 0;
}

#define SC_DPAD_UP 0x0001
#define SC_LTHUMB 0x0040
#define SC_RTHUMB 0x0080
#define SC_LSHOULDER 0x0100
#define SC_RSHOULDER 0x0200

/* v0.16: paths recorded from the player's own stock-controls tricks override the game-data shapes. */
static float sc_ovr_xy[SC_TRICK_COUNT][24][2];
static int sc_ovr_n[SC_TRICK_COUNT];
static float sc_ovr_ms[SC_TRICK_COUNT];   /* recorded duration of the motion */

static float sc_abs(float v) { return v < 0 ? -v : v; }
/* v4.3: does riding switch turn the flick stick round? Regular: yes. Goofy: no - your goofy switch
 * test had ollie / nollie the wrong way round with it (the game already handles it for goofy). */
/* XL uses the live native frame for either stance. Keep the historical goofy
 * exception confined to the older layouts that were tested with it. */
#define SC_SWFLIP(c, s) (((s)->game_switch_known ? (s)->game_switch : (s)->switched) && (c)->switch_flip && ((c)->switch_flip != 3 || (c)->front_back || (c)->xl_layout) && (!(c)->goofy || (c)->switch_goofy_flip || (c)->xl_layout))
#define SC_MMAX(c) ((c)->manual_max > 0.2f && (c)->manual_max < 0.95f ? (c)->manual_max : 0.85f)
static float sc_sqrt(float v) { return v <= 0 ? 0 : __builtin_sqrtf(v); }
static float sc_sin_deg(float d) { /* small, freestanding */
    while (d > 180) d -= 360; while (d < -180) d += 360;
    float x = d * 0.0174532925f, x2 = x * x;
    return x * (1 - x2 / 6 * (1 - x2 / 20 * (1 - x2 / 42 * (1 - x2 / 72 * (1 - x2 / 110)))));
}
static float sc_axis(short v) { return v < 0 ? (float)v / 32768.0f : (float)v / 32767.0f; }
static short sc_short(float v) {
    if (v > 1.0f) v = 1.0f;
    if (v < -1.0f) v = -1.0f;
    return (short)(v < 0 ? v * 32768.0f : v * 32767.0f);
}
static float sc_pow(float b, float e) { /* e in [1,3]; good enough for a response curve */
    if (b <= 0) return 0;
    if (e <= 1.0f) return b;
    float r = b;
    float whole = (float)(int)e;
    for (int i = 1; i < (int)whole; ++i) r *= b;
    float frac = e - whole;
    if (frac > 0) r *= (1.0f - frac) + frac * b;
    return r;
}
/* Angle of a stick in degrees, 0 = straight down, 90 = right, 180 = up (XInput +y is up). */
static float sc_angle(float x, float y) {
    float dy = -y; /* down-positive */
    float ax = sc_abs(x), ay = sc_abs(dy);
    float mx = ax > ay ? ax : ay, mn = ax > ay ? ay : ax;
    if (mx <= 0) return 0;
    float a = mn / mx, s = a * a;
    float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a; /* atan, radians */
    if (ax > ay) r = 1.57079637f - r;   /* angle from the down axis */
    if (dy < 0) r = 3.14159274f - r;
    if (x < 0) r = 6.28318548f - r;
    return r * 57.2957795f;
}
static float sc_wrap(float d) { while (d > 180) d -= 360; while (d < -180) d += 360; return d; }
static int sc_streq(const char* a, const char* b) { while (*a && *a == *b) { ++a; ++b; } return *a == *b; }
static int sc_find(const char* name) {
    for (int i = 0; i < SC_TRICK_COUNT; ++i) if (sc_streq(sc_tricks[i].name, name)) return i;
    return -1;
}

/* Radial deadzone with rescale. */
static void sc_deadzone(float* x, float* y, float dz) {
    float m = sc_sqrt(*x * *x + *y * *y);
    if (m <= dz || m <= 0) { *x = *y = 0; return; }
    float scaled = (m - dz) / (1.0f - dz);
    if (scaled > 1.0f) scaled = 1.0f;
    *x = *x / m * scaled;
    *y = *y / m * scaled;
}

/* Name of the trick for a pop family, scoop (0 none, 1 shuv, 2 360) with side (+1 right / -1 left),
 * and flip (0 none, +1 kickflip side, -1 heelflip side). */
static const char* sc_trick_name(int pop_back, int scoop, int scoop_side, int flip) {
    static const char* const ollie[3][3][2] = {
        /* flip none */  {{"ollie", "ollie"}, {"fspopshuvit", "popshuvit"}, {"fs360popshuvit", "360popshuvit"}},
        /* kickflip */   {{"kickflip", "kickflip"}, {"hardflip", "varialkickflip"}, {"360hardflip", "360flip"}},
        /* heelflip */   {{"heelflip", "heelflip"}, {"varialheelflip", "inwardheelflip"}, {"laserflip", "360inwardheelflip"}},
    };
    static const char* const nollie[3][3][2] = {
        {{"nollie", "nollie"}, {"n_fspopshuvit", "n_popshuvit"}, {"n_fs360popshuvit", "n_360popshuvit"}},
        {{"n_kickflip", "n_kickflip"}, {"n_hardflip", "n_varialkickflip"}, {"n_360hardflip", "n_360flip"}},
        {{"n_heelflip", "n_heelflip"}, {"n_varialheelflip", "n_inwardheelflip"}, {"n_laserflip", "n_360inwardheelflip"}},
    };
    int f = flip > 0 ? 1 : flip < 0 ? 2 : 0;
    int side = scoop_side > 0 ? 1 : 0;
    return pop_back ? ollie[f][scoop][side] : nollie[f][scoop][side];
}

static void sc_fire(sc_state* s, const char* name, double now) {
    s->event = 2;
    s->d_latch = 0;
    int i = sc_find(name);
    if (i < 0) { s->phase = SC_COOLDOWN; return; }
    s->trick = i;
    s->emit_start = now;
    s->emit_index = 0;
    s->from_x = s->ox; s->from_y = s->oy;
    s->seg = 1;
    s->hold_start = 0;
    s->emit_last = 0;
    /* v2.3: a drawn trick is only drawn once the game has actually crouched (state 103). Fast
     * scoops and tricks straight after a landing used to start drawing before the game had taken
     * the crouch: the game ignored the whole trick (your logs: "trick NOT taken", no crouch), and
     * the crouch that followed became a manual. */
    s->emit_wait = 1; s->emit_wait_t = now; s->emit_crouch_y = sc_tricks[i].nollie ? 0.95f : -0.95f;
    s->emit_ms_per_unit = 0;
    s->recenter_until = 0;
    s->path_n = 0;
    s->phase = SC_EMIT;
    s->last_name = name;
    s->fired++;
}

/* Draw the current trick one poll at a time (smooth, never skipping a point, then a hold). */
static void sc_emit_step(const sc_config* c, sc_state* s, double now, float* ox, float* oy) {
        if (s->emit_wait) {
            int ready = !s->game_known || s->path_n > 0 || s->game_crouch > 0 || s->grind || s->air || s->takeoff == 1;
            if (ready || now - s->emit_wait_t >= 100.0) {   /* v2.6: 100 ms (250 felt like a stuck crouch) */
                s->emit_wait = 0;
                if (now > s->emit_wait_t) {   /* we waited: the drawing starts from the crouch the game took */
                    if (s->recenter_until > 0) s->recenter_until = now + (s->recenter_until - s->emit_wait_t);
                    else { s->from_x = 0; s->from_y = s->emit_crouch_y; }
                }
                s->emit_start = now;
            } else { *ox = 0; *oy = s->emit_crouch_y; s->emit_start = now; return; }
        }
        if (s->recenter_until > now) { *ox = 0; *oy = 0; s->emit_start = now; return; }
        /* Draw the trick like a thumb would: one smooth movement from where the stick was,
         * through every point of the game's path, then a short hold at the end. v0.4-v0.7
         * jumped from point to point in single 16 ms steps, which the game mostly ignored. */
        int rec = s->path_n <= 0 && sc_ovr_n[s->trick] > 0;
        const float (*pts)[2] = s->path_n > 0 ? s->path : rec ? (const float (*)[2])sc_ovr_xy[s->trick] : sc_tricks[s->trick].xy;
        int n = s->path_n > 0 ? s->path_n : rec ? sc_ovr_n[s->trick] : sc_tricks[s->trick].count;
        float vx[25], vy[25], cum[25];
        vx[0] = s->from_x; vy[0] = s->from_y; cum[0] = 0;
        for (int k = 0; k < n; ++k) {
            vx[k + 1] = pts[k][0]; vy[k + 1] = pts[k][1];
            float dx = vx[k + 1] - vx[k], dy = vy[k + 1] - vy[k];
            cum[k + 1] = cum[k] + sc_sqrt(dx * dx + dy * dy);
        }
        float total = cum[n];
        double dur = total * (s->emit_ms_per_unit > 0 ? s->emit_ms_per_unit : c->ms_per_unit);
        if (rec && sc_ovr_ms[s->trick] > 0) {    /* replay at the speed it was recorded (slower for low pop) */
            dur = sc_ovr_ms[s->trick];
            if (s->emit_ms_per_unit > c->ms_per_unit && c->ms_per_unit > 0) dur *= s->emit_ms_per_unit / c->ms_per_unit;
        }
        if (dur < c->min_path_ms) dur = c->min_path_ms;
        float want = dur > 0 ? (float)((now - s->emit_start) / dur) * total : total;
        if (want > total) want = total;
        /* v1.9: never move more than half a stick-width between two game frames. When the game
         * hitches (shaders compiling early in a session, a busy handheld) a timed drawing could
         * jump straight to its end in one frame and the game did not see the trick at all - no pop.
         * On a smooth 60 fps this changes nothing. */
        if (want > s->emit_last + 0.5f) want = s->emit_last + 0.5f;
        if (s->seg <= n) {
            float limit = cum[s->seg];
            float p = want < limit ? want : limit;   /* never skip past the next unseen point */
            int k = 0;
            while (k < n && cum[k + 1] < p) ++k;
            float segl = cum[k + 1] - cum[k];
            float u = segl > 0 ? (p - cum[k]) / segl : 1;
            *ox = vx[k] + (vx[k + 1] - vx[k]) * u;
            *oy = vy[k] + (vy[k + 1] - vy[k]) * u;
            s->emit_last = p;
            if (p >= limit - 1e-4f) s->seg++;
        } else {
            *ox = vx[n]; *oy = vy[n];
            if (s->hold_start <= 0) s->hold_start = now + 1;
            if (now + 1 - s->hold_start >= c->hold_end_ms) { s->phase = SC_COOLDOWN; }
        }
}

/* The flick stick the game sees, from the two feet (XInput space, +y up). now = milliseconds. */
static void sc_feet_to_flick(const sc_config* c, sc_state* s, double now,
                             float fx, float fy, float bx, float by, float* out_x, float* out_y) {
    sc_deadzone(&fx, &fy, c->deadzone);
    sc_deadzone(&bx, &by, c->deadzone);
    if ((c->flip_x != c->goofy) != (c->switch_flip == 3 && s->switched)) { fx = -fx; bx = -bx; }   /* v0.28: goofy mirrors sideways moves */
    float fm = sc_sqrt(fx * fx + fy * fy), bm = sc_sqrt(bx * bx + by * by);
    float on = c->active, off = c->active * 0.6f;
    float x = 0, y = 0;

    if (s->phase == SC_IDLE) {
        /* A pop is a foot pulled down (within 60 degrees of straight down). */
        float ba = sc_angle(bx, by), fa = sc_angle(fx, fy);
        int back_pop = bm >= on && (ba <= 60 || ba >= 300);
        int front_pop = fm >= on && (fa <= 60 || fa >= 300);
        if (back_pop || front_pop) {
            s->phase = SC_ARMED;
            s->pop_back = back_pop;
            s->sweep = 0;
            s->last_angle = back_pop ? ba : fa;
            s->tracking = 0;
            s->side = 0;
            s->pop_start = now;
            s->pop_angle = back_pop ? ba : fa;
            s->pop_mag = back_pop ? bm : fm;
            s->max_mag = s->pop_mag;
            s->event = 1;
        } else if (bm < on && fm < on && (bm > 0 || fm > 0)) {
            /* Light leans pass through for manuals: back foot -> tail, front foot -> nose.
             * Only once held steady: every pop passes through this zone on its way down,
             * and passing it straight through made the skater flicker into a manual. */
            if (s->lean_since <= 0) s->lean_since = now + 1; /* +1: 0 means unset */
            if (now + 1 - s->lean_since >= c->lean_ms) {
                if (bm >= fm) { x = bx; y = -sc_abs(by); } else { x = fx; y = sc_abs(fy); }
            }
        }
        if (s->phase != SC_IDLE || (bm >= on || fm >= on) || (bm <= 0 && fm <= 0)) s->lean_since = 0;
        /* else: a foot moved without a pop (e.g. front foot pushed up first): nothing. */
        if (s->phase == SC_IDLE && (bm >= on || fm >= on) && !s->ev_logged) {
            s->event = 3; s->ev_foot_back = bm >= fm; s->ev_angle = bm >= fm ? ba : fa; s->ev_mag = bm >= fm ? bm : fm;
            s->ev_logged = 1;
        }
        if (bm < off && fm < off) s->ev_logged = 0;
    }

    if (s->phase == SC_ARMED) {
        float px = s->pop_back ? bx : fx, py = s->pop_back ? by : fy, pm = s->pop_back ? bm : fm;
        float ox = s->pop_back ? fx : bx, oy = s->pop_back ? fy : by, om = s->pop_back ? fm : bm;
        (void)py; (void)oy;
        /* Only read a scoop while the stick is pushed well out. Near the centre (letting go,
         * the stick springing back) its angle is noise, and v0.4 counted that as a scoop. */
        if (pm >= 0.7f) {
            float a = sc_angle(px, py);
            if (s->tracking) {
                float d = sc_wrap(a - s->last_angle);
                if (sc_abs(d) <= 75.0f) s->sweep += d; /* bigger jumps in one poll are not a scoop */
            }
            s->tracking = 1;
            s->last_angle = a;
            if (sc_abs(px) > sc_abs(s->side)) s->side = px;
        } else {
            s->tracking = 0;
        }
        if (pm > s->max_mag) s->max_mag = pm;
        float swept = sc_abs(s->sweep);
        int scoop = swept >= c->shuv360_degrees ? 2 : swept >= c->shuv_degrees ? 1 : 0;
        int scoop_side = s->sweep > 0 ? 1 : -1; /* sweeping from down towards the right is positive */
        if (scoop == 0 && sc_abs(s->side) >= 0.75f) { scoop = 1; scoop_side = s->side > 0 ? 1 : -1; }

        if (swept >= 290.0f) {
            /* Circling through the right side is an impossible; through the left side is the
             * other foot's impossible, as the game's own gesture data tells them apart. */
            if (s->pop_back) sc_fire(s, s->sweep > 0 ? "impossible" : "frontfoot_impossible", now);
            else sc_fire(s, s->sweep > 0 ? "n_impossible" : "n_backfoot_impossible", now);
        } else if (om >= on) {
            float nx = ox / om;
            int flip = nx > 0.45f ? 1 : nx < -0.45f ? -1 : 0;
            sc_fire(s, sc_trick_name(s->pop_back, scoop, scoop_side, flip), now);
        } else if (pm < off) {
            if (scoop) sc_fire(s, sc_trick_name(s->pop_back, scoop, scoop_side, 0), now);
            else if (c->pop_release_ollie) sc_fire(s, s->pop_back ? "ollie" : "nollie", now);
            else s->phase = SC_IDLE;
        } else {
            /* Holding the pop: the skater crouches. */
            float m = pm > 0.95f ? 0.95f : pm;
            x = 0; y = s->pop_back ? -m : m;
        }
    }

    if (s->phase == SC_EMIT) sc_emit_step(c, s, now, &x, &y);

    if (s->phase == SC_COOLDOWN) {
        x = y = 0;
        if (bm < off && fm < off) s->phase = SC_IDLE; /* both feet back to rest */
    }

    float m = sc_sqrt(x * x + y * y);
    if (m > 1.0f) { x /= m; y /= m; }
    *out_x = x;
    *out_y = y;
}

/* v0.14: a trick whose drawing starts well away from where the stick is now (e.g. a laser flip
 * starting out to the right while the stick is held down in the crouch) is drawn from the centre,
 * after a short pause there. Sweeping from the crouch round to the start point made the game read
 * a backside spin first (laser flip came out as a 360 inward heelflip). */
static void sc_maybe_recenter(const sc_config* c, sc_state* s, double now) {
    const sc_trick* t = &sc_tricks[s->trick];
    if (sc_ovr_n[s->trick] > 0) return;
    if (c->recenter_ms <= 0 || t->count < 1) return;
    float fx = s->from_x, fy = s->from_y, fm = sc_sqrt(fx * fx + fy * fy);
    float px = t->xy[0][0], py = t->xy[0][1], pm = sc_sqrt(px * px + py * py);
    if (fm < 0.2f || pm < 0.2f) return;
    float cosang = (fx * px + fy * py) / (fm * pm);
    if (cosang > 0.5f) return;                  /* within 60 degrees: carry on from the crouch */
    s->recenter_until = now + c->recenter_ms;
    s->from_x = 0; s->from_y = 0;
}

/* Direct mode (v0.3 behaviour): each foot folded onto its half of the flick stick, the
 * last foot to move drives it, an overtaken foot is ignored until released.
 * v0.10 adds pop height by which foot does what:
 *   first foot held down, second foot flicked UP   -> passed straight through (full pop)
 *   first foot held down, second foot flicked DOWN -> same flick, slowed down (low pop)
 *   one foot on its own (shove-its, leans, letting go) -> slowed down (low pop), so a single
 *   stick springing back can't snap out an ollie or nollie. */
static void sc_direct(const sc_config* c, sc_state* s, double now, float fx, float fy, float bx, float by,
                      float* out_x, float* out_y) {
    sc_deadzone(&fx, &fy, c->deadzone);
    sc_deadzone(&bx, &by, c->deadzone);
    /* v0.28 goofy: a goofy rider's toes point the other way across the screen, so every
     * sideways move on the ground (kickflip / heelflip side, scoops, wind-ups) is mirrored.
     * The game does not mirror its gestures for goofy, so kickflips came out as heelflips.
     * Air / grind controls move the board's ends across the screen and are not mirrored. */
    int mirror = c->flip_x;
    if (c->goofy && !(c->air_controls && s->air && s->phase != SC_EMIT)) mirror = !mirror;
    /* v0.33 true foot switch: riding switch you stand the other way round, like goofy */
    if (c->switch_flip == 3 && s->switched && !(c->air_controls && s->air && s->phase != SC_EMIT)) mirror = !mirror;
    if (mirror) { fx = -fx; bx = -bx; }
    float ffx = fx, ffy = sc_abs(fy), bbx = bx, bby = -sc_abs(by);
    float fm = sc_sqrt(ffx * ffx + ffy * ffy), bm = sc_sqrt(bbx * bbx + bby * bby);
    float on = c->active, off = c->active * 0.6f;
    /* v0.19 air / grind: Session style, each foot moves its own end of the board (front foot the
     * nose half, back foot the tail half, both together add up: both sideways = boardslide).
     * A trick still being drawn is finished first. On landing, feet still held are ignored until
     * they are let go, so a held grind position does not turn into a crouch / quick ollie. */
    /* v1.6: the carried feet are decided at the moment the board leaves the ground (the game's
     * own state, no delay), not when air controls switch on 120 ms later: by then a thumb had often
     * eased off the crouch to a light lean, which was then taken as a manual on landing (and a
     * sideways flick still held turned the board in the air). Every foot still held at take-off
     * is left over from the trick. */
    if (s->d_front_carry && fm < off) s->d_front_carry = 0;
    if (s->d_back_carry && bm < off) s->d_back_carry = 0;
    if (s->takeoff > 0 && !s->d_took_off) {
        s->d_took_off = 1; s->d_takeoff_t = now; s->sw_takeoff = s->sw_raw;
        s->d_front_carry = fm >= c->deadzone; s->d_back_carry = bm >= c->deadzone;
    } else if (s->takeoff > 0 && now - s->d_takeoff_t < 150.0) {
        /* v1.8: the flick often comes a moment AFTER the board leaves the ground (the game takes
         * the pop on the crouch, the flip on the flick). A foot pressed in the first moments of the
         * air is still part of the trick (first 150 ms): held, it tilted the board and you landed in a manual
         * (your logs: both sticks were already back in the middle at touchdown) */
        if (fm >= c->active) s->d_front_carry = 1;
        if (bm >= c->active) s->d_back_carry = 1;
    } else if (s->takeoff == 0 && s->d_took_off) {
        s->d_took_off = 0;
        if (!s->air && !s->d_was_air && !s->grind) {   /* a hop too short for air controls */
            if (s->d_front_carry || (fm >= 0.9f && fy > 0.5f * fm)) { s->d_front_ignored = 1; s->d_front_on = 0; s->d_front_cign = 1; }
            if (s->d_back_carry || (bm >= 0.9f && by < -0.5f * bm)) { s->d_back_ignored = 1; s->d_back_on = 0; s->d_back_cign = 1; }
            s->d_front_carry = s->d_back_carry = 0;
        }
    }
    if (!c->air_controls && s->air && s->phase != SC_EMIT) {
        /* v2.5: air controls OFF: nothing goes to the game in the air. It used to run the ground
         * controls, so pulling the back foot down in the air (getting ready for the next ollie)
         * tilted the tail down and you landed in a manual - and the trick after it didn't pop. */
        s->d_was_air = 1;
        s->phase = SC_IDLE; s->d_pop = 0; s->d_pending = 0; s->d_latch = 0; s->d_windup = 0; s->path_n = 0;
        s->d_front_on = s->d_back_on = 0; s->d_front_spent = s->d_back_spent = 0; s->d_two = 0;
        /* v2.8: ...except a LIGHT up / down lean (a manual set up for the landing): the game has
         * to see it before touchdown, or you can't manual out of a trick. A push past ManualMaxPercent
         * is a crouch for the next pop and is kept back. Feet left over from the trick never count. */
        float mm = SC_MMAX(c), lx2 = 0, ly2 = 0;
        int allow = c->landing_manuals >= 2 || (c->landing_manuals == 1 &&
            (s->sw_raw < 0 || s->sw_takeoff < 0 || s->sw_raw != s->sw_takeoff));
        if (allow) {
            if (!s->d_front_carry && fy > 0 && fm >= c->deadzone && fm < mm) { lx2 = 0; ly2 = fy; }
            if (!s->d_back_carry && by < 0 && bm >= c->deadzone && bm < mm && bm > sc_abs(ly2)) { lx2 = 0; ly2 = by; }
        }
        s->d_t = now; *out_x = lx2; *out_y = ly2;
        return;
    }
    if (c->air_controls && s->air && s->phase != SC_EMIT) {
        /* v1.5: a foot still held from the pop / flick when you take off is not an air control
         * (and not a manual on landing) until you let go of it once. Holding the flick through
         * the air sent the board's nose / tail down at touchdown = an accidental (nose) manual.
         * Let go and press again in the air to set up a grind or a manual on purpose. */
        if (!s->d_was_air && s->takeoff < 0) {   /* take-off moment unknown: only feet held in their manual direction */
            /* a light lean held from before take-off is a manual set up on purpose: only a foot
             * still pushed most of the way (left over from the pop / flick) is carried */
            s->d_front_carry = fm >= 0.7f && fy > 0.5f * fm;
            s->d_back_carry = bm >= 0.7f && by < -0.5f * bm;
        }
        /* v2.2: a foot pushed right to the edge in the air is a (late) flick, not a manual lean:
         * it is carried too. A lighter lean set up in the air stays live, so a 180 into a manual
         * (or a manual set up on purpose) still lands in the manual. */
        if (fm >= 0.9f && fy > 0.5f * fm) s->d_front_carry = 1;    /* (a full sideways push is a grind set-up) */
        if (bm >= 0.9f && by < -0.5f * bm) s->d_back_carry = 1;
        /* v2.8: an up / down push past ManualMaxPercent is a crouch for the next pop: no tilt */
        if (fy > 0.85f * fm && fm >= SC_MMAX(c)) ffy = 0;
        if (by < -0.85f * bm && bm >= SC_MMAX(c)) bby = 0;
        if (s->d_front_carry) fm = 0;
        if (s->d_back_carry) bm = 0;
        {   /* v2.6: an up / down lean in the air tilts the board for a manual landing: only allowed
             * when LandingManuals would keep it (always, or after a 180 once you have turned) */
            int allow = c->landing_manuals >= 2 || (c->landing_manuals == 1 &&
                (s->sw_raw < 0 || s->sw_takeoff < 0 || s->sw_raw != s->sw_takeoff));
            if (!allow) { ffy = 0; bby = 0; }
        }
        s->d_was_air = 1;
        s->phase = SC_IDLE; s->d_pop = 0; s->d_pending = 0; s->d_latch = 0; s->d_windup = 0; s->path_n = 0;
        s->d_front_on = s->d_back_on = 0; s->d_front_spent = s->d_back_spent = 0; s->d_two = 0;
        float x = (fm >= c->deadzone ? ffx : 0) + (bm >= c->deadzone ? bbx : 0);
        float y = (fm >= c->deadzone ? ffy : 0) + (bm >= c->deadzone ? bby : 0);
        float m = sc_sqrt(x * x + y * y);
        if (m > 1.0f) { x /= m; y /= m; }
        s->d_t = now;
        *out_x = x; *out_y = y;
        return;
    }
    if (s->d_was_air) {
        s->d_was_air = 0;
        /* Landing: feet still held are ignored until let go. Onto a rail (v0.19d): feet held in
         * the grind position count straight away, so a held back foot is already the crouch and
         * one flick of the other foot tricks out, like on the ground. */
        /* v0.19q: feet held through a landing are simply live again, exactly like on a rail and
         * like the normal controls (a held back foot is the manual or the next crouch, as the game
         * decides). Ignoring them got you stuck in manuals, unable to ollie. */
        s->ox = s->oy = 0; s->d_t = now;
        /* v1.5: ...except a foot held ever since take-off (not onto a rail, where a held foot is the
         * crouch for popping out): ignored until let go, so it cannot land you in a manual */
        /* v2.2: feet carried from the trick, or still pushed right to the edge, are ignored until
         * you ease off / let go. A lighter lean set up in the air is a manual on purpose and stays
         * (v2.0 ignored every held foot, which also cancelled 180-into-manuals). */
        if (!s->grind) {
            /* v2.6: a lighter lean held into the landing lands a manual only when LandingManuals
             * allows it: 1 (default) = only after a 180 (the game's facing flag changed in the air,
             * your "180 into a manual"), 2 = always, 0 = never. Your logs: the back stick resting
             * about half way down in the air gave manuals you never meant. */
            int keep_lean = c->landing_manuals >= 2 ||
                (c->landing_manuals == 1 && s->sw_raw >= 0 && s->sw_takeoff >= 0 && s->sw_raw != s->sw_takeoff) ||
                (c->landing_manuals == 1 && (s->sw_raw < 0 || s->sw_takeoff < 0));   /* facing unknown: keep */
            int f_lean = !keep_lean && fm >= c->deadzone, b_lean = !keep_lean && bm >= c->deadzone;
            if (s->d_front_carry || f_lean || (fm >= 0.9f && fy > 0.5f * fm)) { s->d_front_ignored = 1; s->d_front_on = 0; s->d_front_cign = 1; }
            if (s->d_back_carry || b_lean || (bm >= 0.9f && by < -0.5f * bm)) { s->d_back_ignored = 1; s->d_back_on = 0; s->d_back_cign = 1; }
            /* v2.5: a foot held right down in its pop direction at touchdown is you getting ready for
             * the next pop: it is kept from the game for 150 ms (so the landing is not a manual), then
             * becomes a normal crouch by itself - no need to let go and press again */
            if (fm >= SC_MMAX(c) && fy > 0.85f * fm) { s->d_front_ignored = 1; s->d_front_on = 0; s->d_front_cign = 3; }
            if (bm >= SC_MMAX(c) && by < -0.85f * bm) { s->d_back_ignored = 1; s->d_back_on = 0; s->d_back_cign = 3; }
            s->d_land_t = now;
        }
        s->d_front_carry = s->d_back_carry = 0;
    }
    /* v0.11: a foot can only START a trick in its own pop direction, like Session:
     * back foot DOWN (towards the tail) = ollie, front foot UP (towards the nose) = nollie.
     * Back foot up / front foot down on their own do nothing. Once one foot is held, the
     * other foot may flick any way (that is the second foot, which sets the pop height). */
    float fa = sc_angle(fx, fy), ba = sc_angle(bx, by);       /* 0 = down, 180 = up */
    float f_off = sc_abs(sc_wrap(fa - 180.0f)), b_off = sc_abs(sc_wrap(ba)); /* degrees from pop direction */
    float pop_lim = c->windup_deg > 0 && c->windup_deg < 75.0f ? c->windup_deg : 75.0f;
    int front_pop_dir = fy > 0 && f_off <= pop_lim;   /* close to up */
    int back_pop_dir = by < 0 && b_off <= pop_lim;    /* close to down */
    /* v0.18 wind-up: started from the side or a diagonal (up to 30 deg past sideways), like
     * skate.'s own 360 flip / laser flip wind-up. Never from the wrong end of the stick. */
    int front_windup = c->windup_deg > 0 && !front_pop_dir && f_off <= 120.0f;
    int back_windup = c->windup_deg > 0 && !back_pop_dir && b_off <= 120.0f;
    /* v1.5: a foot carried through the landing comes back to life when it is let go, or pushed
     * right to the edge in its pop direction (the next ollie straight away) */
    if (s->d_front_cign == 3) {
        if (fm < off) { s->d_front_cign = 0; s->d_front_ignored = 0; }
        else if (now - s->d_land_t >= 150.0) {
            if (front_pop_dir && fm >= SC_MMAX(c)) { s->d_front_cign = 0; s->d_front_ignored = 0; } else s->d_front_cign = 1;
        }
    }
    if (s->d_back_cign == 3) {
        if (bm < off) { s->d_back_cign = 0; s->d_back_ignored = 0; }
        else if (now - s->d_land_t >= 150.0) {
            if (back_pop_dir && bm >= SC_MMAX(c)) { s->d_back_cign = 0; s->d_back_ignored = 0; } else s->d_back_cign = 1;
        }
    }
    if (s->d_front_cign == 1 && fm < 0.75f) s->d_front_cign = 2;   /* eased off: a fresh push can pop */
    if (s->d_back_cign == 1 && bm < 0.75f) s->d_back_cign = 2;
    if (s->d_front_cign && (fm < off || (s->d_front_cign == 2 && front_pop_dir && fm >= 0.9f))) { s->d_front_cign = 0; s->d_front_ignored = 0; }
    if (s->d_back_cign && (bm < off || (s->d_back_cign == 2 && back_pop_dir && bm >= 0.9f))) { s->d_back_cign = 0; s->d_back_ignored = 0; }
    if (!s->d_front_on && fm >= on && !s->d_front_ignored) {
        if (s->d_back_on || front_pop_dir || front_windup) {
            s->d_front_on = 1; s->d_front_seq = ++s->d_seq;
            if (!s->d_back_on && !front_pop_dir) { s->d_windup = 1; s->d_windup_t = now; s->d_log = 1; s->d_log_a = fa; s->d_log_b = 0; }
        }
        else s->d_front_ignored = 1;
    } else if (fm < off) {
        if (s->d_front_on && !s->d_front_spent && s->d_front_was_up) s->d_front_rel_t = now;
        s->d_front_on = 0; s->d_front_spent = 0; s->d_front_ignored = 0; s->d_front_was_up = 0;
    }
    if (!s->d_back_on && bm >= on && !s->d_back_ignored) {
        if (s->d_front_on || back_pop_dir || back_windup) {
            s->d_back_on = 1; s->d_back_seq = ++s->d_seq;
            if (!s->d_front_on && !back_pop_dir) { s->d_windup = 1; s->d_windup_t = now; s->d_log = 1; s->d_log_a = ba; s->d_log_b = 1; }
        }
        else s->d_back_ignored = 1;
    } else if (bm < off) {
        if (s->d_back_on && !s->d_back_spent && s->d_back_was_down) s->d_back_rel_t = now;
        s->d_back_on = 0; s->d_back_spent = 0; s->d_back_ignored = 0; s->d_back_was_down = 0;
    }

    /* two_feet: the second foot came in while the first was still held. */
    if (!s->d_front_on && !s->d_back_on) { if (fm < off && bm < off) s->d_two = 0; }
    else if (s->d_front_on && s->d_back_on) s->d_two = 1;

    /* v0.26 quick ollie / nollie on ONE stick: the back foot flicked down and straight back up
     * (an ollie motion on the right stick alone) = quick ollie; the front foot up and straight
     * back down = quick nollie. (replaces v0.19's back held + front flicked down) */
    if (s->d_back_on && !s->d_back_spent && by < -0.5f) s->d_back_was_down = 1;
    if (s->d_front_on && !s->d_front_spent && fy > 0.5f) s->d_front_was_up = 1;
    if (c->quick_pops && s->phase == SC_IDLE && !s->d_windup && s->d_pop >= 0) {
        int quick = 0;
        /* a straight flick only (a scoop sweeping round towards the top is a shove-it / laser) */
        int scooping = s->d_pop && sc_abs(s->d_sweep) > 60.0f;
        if (scooping) {}
        else if (!s->d_front_on && by > 0.45f && sc_abs(bx) < 0.5f * by &&
            ((s->d_back_on && !s->d_back_spent && s->d_back_was_down) || (s->d_back_rel_t > 0 && now - s->d_back_rel_t < 200.0))) quick = 1;
        else if (!s->d_back_on && fy < -0.45f && sc_abs(fx) < -0.5f * fy &&
            ((s->d_front_on && !s->d_front_spent && s->d_front_was_up) || (s->d_front_rel_t > 0 && now - s->d_front_rel_t < 200.0))) quick = 2;
        if (quick) {
            static const float qo[3][2] = {{0, 0}, {0, -0.95f}, {0, -0.95f}};
            static const float qn[3][2] = {{0, 0}, {0, 0.95f}, {0, 0.95f}};
            s->d_back_rel_t = s->d_front_rel_t = 0;
            if (quick == 1) { s->d_back_on = 0; s->d_back_ignored = 1; s->d_back_was_down = 0; }
            else { s->d_front_on = 0; s->d_front_ignored = 1; s->d_front_was_up = 0; }
            s->d_pop = 0; s->d_pending = 0; s->d_latch = 0;
            sc_fire(s, quick == 1 ? "quickollie" : "quicknollie", now);
            s->path = quick == 1 ? qo : qn; s->path_n = 3;
            s->emit_ms_per_unit = c->ms_per_unit;
            if (quick == 2) { s->recenter_until = now + 70.0; s->from_x = 0; s->from_y = 0; }
            float ex = 0, ey = 0; sc_emit_step(c, s, now, &ex, &ey); s->d_t = now; *out_x = ex; *out_y = ey;
            return;
        }
    }

    /* v1.1 pop on release (Skater XL style): crouch all the way with one foot (back foot down /
     * front foot up), then just LET GO of that stick = ollie / nollie. A flick of the other foot
     * while still crouched is still a flip trick, and a scoop (stick swept round, not let go) is
     * still a shove-it, because only the stick coming back to the middle pops. */
    if (c->pop_on_release) {
        float bmr = sc_sqrt(bx * bx + by * by), fmr = sc_sqrt(fx * fx + fy * fy);
        /* v1.4: a fast release can jump from the crouch nearly to the middle in one poll, which
         * also lets go of the foot (d_back_on = 0) in the same poll. v1.3 forgot the crouch then
         * and did not pop. The crouch is now remembered until it pops or something else happens. */
        int busy = s->phase != SC_IDLE || s->d_windup || s->d_pending;
        if (busy || s->d_front_on || s->d_back_spent) { s->d_back_full = 0; s->d_back_peak = 0; }
        else if (s->d_back_on && bmr >= 0.8f && by < 0 && sc_abs(bx) <= -by) { s->d_back_full = 1; if (bmr > s->d_back_peak) s->d_back_peak = bmr; }
        if (busy || s->d_back_on || s->d_front_spent) { s->d_front_full = 0; s->d_front_peak = 0; }
        else if (s->d_front_on && fmr >= 0.8f && fy > 0 && sc_abs(fx) <= fy) { s->d_front_full = 1; if (fmr > s->d_front_peak) s->d_front_peak = fmr; }
        int rel = 0;
        if (!busy && s->d_back_full && bmr < s->d_back_peak - 0.12f && (by > -0.85f * s->d_back_peak || bmr < 0.75f)) rel = 1;
        else if (!busy && s->d_front_full && fmr < s->d_front_peak - 0.12f && (fy < 0.85f * s->d_front_peak || fmr < 0.75f)) rel = 2;
        if (rel) {
            if (rel == 1) { s->d_back_on = 0; s->d_back_ignored = bmr >= c->active * 0.6f; s->d_back_full = 0; s->d_back_peak = 0; s->d_back_was_down = 0; }
            else { s->d_front_on = 0; s->d_front_ignored = fmr >= c->active * 0.6f; s->d_front_full = 0; s->d_front_peak = 0; s->d_front_was_up = 0; }
            s->d_back_rel_t = s->d_front_rel_t = 0;
            s->d_pop = 0; s->d_pending = 0; s->d_latch = 0;
            /* the pop goes to the game at once: the stick snaps from the crouch to the other end */
            s->rp_t = now; s->rp_y = rel == 1 ? 0.95f : -0.95f;
            s->last_name = rel == 1 ? "ollie (let go)" : "nollie (let go)"; s->fired++;
        }
    }
    if (s->rp_t > 0 && now - s->rp_t < 70.0) { s->d_t = now; *out_x = 0; *out_y = s->rp_y; return; }
    s->rp_t = 0;

    /* ---- v0.12 combos ------------------------------------------------------------------
     * The game wants a combo's wind-up FIRST (varial kickflip = down-left, down, up-right).
     * Passing a scoop straight through completed a pop shove-it on its own, and the flick
     * that followed became a second trick. So a scoop is held back (the game keeps seeing
     * the crouch) until the other foot flicks (-> the combo, drawn in the game's own shape)
     * or the wait runs out / the foot lets go (-> the shove-it). */
    if (s->phase == SC_EMIT) {
        float ex = 0, ey = 0;
        sc_emit_step(c, s, now, &ex, &ey);
        s->d_t = now;
        *out_x = ex; *out_y = ey;
        return;
    }
    if (s->phase == SC_COOLDOWN) {
        s->d_t = now;
        *out_x = 0; *out_y = 0;
        if (fm < off && bm < off) { s->phase = SC_IDLE; s->d_pop = 0; s->d_pending = 0; s->d_latch = 0; s->d_windup = 0; s->path_n = 0; }
        return;
    }
    /* v1.2: a scoop let go before its wait ran out is still the shove-it (it used to be dropped) */
    if (!s->d_front_on && !s->d_back_on && !s->d_pending) { s->d_pop = 0; s->d_windup = 0; }
    else if (!s->d_pop) {
        if (s->d_back_on && !s->d_front_on) { s->d_pop = 1; s->d_last_ang = 0; }
        else if (s->d_front_on && !s->d_back_on) { s->d_pop = 2; s->d_last_ang = 180; }
        s->d_sweep = 0; s->d_side = 0; s->d_track = 0; s->d_pending = 0;
    }
    if (s->d_pop && !s->d_windup) {
        int back = s->d_pop == 1;
        float px = back ? bx : fx, py = back ? by : fy, pm = sc_sqrt(px * px + py * py);
        int pop_live = back ? (s->d_back_on && !s->d_back_spent) : (s->d_front_on && !s->d_front_spent);
        int other_on = back ? s->d_front_on : s->d_back_on;
        /* v1.9: the scooping foot's latest position still counts in the frame the other foot
         * flicks (at 30 fps that last step was lost: a laser came out as a varial heelflip) */
        if (pop_live && other_on && s->d_pending && s->d_track && pm >= 0.7f) {
            float a = sc_angle(px, py), d = sc_wrap(a - s->d_last_ang);
            if (sc_abs(d) <= 110.0f) { s->d_sweep += d; s->d_last_ang = a; }
            if (sc_abs(px) > sc_abs(s->d_side)) s->d_side = px;
        }
        if (pop_live && !other_on) {
            if (pm >= 0.7f) {
                float a = sc_angle(px, py);
                if (s->d_track) { float d = sc_wrap(a - s->d_last_ang); if (sc_abs(d) <= 110.0f) s->d_sweep += d;   /* v1.9: 110 (was 75): at 30 fps a fast scoop moves further per frame */ }
                s->d_track = 1; s->d_last_ang = a;
                if (sc_abs(px) > sc_abs(s->d_side)) s->d_side = px;
            } else s->d_track = 0;
            float swept = sc_abs(s->d_sweep);
            if (swept >= 290.0f) {
                /* full circle: impossible (direction picks which foot's), single foot = low pop */
                int right = (s->d_side > 0) != (c->scoop_mirror != 0);
                sc_fire(s, back ? (right ? "impossible" : "frontfoot_impossible") : (right ? "n_impossible" : "n_backfoot_impossible"), now);
                s->emit_ms_per_unit = c->low_pop_ms;
                sc_maybe_recenter(c, s, now);
                float ex = 0, ey = 0; sc_emit_step(c, s, now, &ex, &ey); s->d_t = now; *out_x = ex; *out_y = ey; return;
            }
            if (!s->d_pending && (swept >= c->shuv_degrees || sc_abs(s->d_side) >= 0.75f)) { s->d_pending = 1; s->d_pend_since = now; s->d_pend_peak = 0; }
        }
        if (s->d_pending) {
            float swept = sc_abs(s->d_sweep);
            int scoop = swept >= c->shuv360_degrees ? 2 : 1;
            int side = s->d_side >= 0 ? 1 : -1;
            if (c->scoop_mirror) side = -side;
            float ox2 = back ? fx : bx, oy2 = back ? fy : by, om2 = sc_sqrt(ox2 * ox2 + oy2 * oy2);
            const char* name = 0; int full = 0;
            if (other_on && om2 >= on) {
                float nx = ox2 / om2;
                int flip = nx > 0.45f ? 1 : nx < -0.45f ? -1 : 0;
                name = sc_trick_name(back, scoop, side, flip);
                full = back ? oy2 >= 0 : oy2 <= 0;     /* same pop-height rule as ollies */
                s->d_log = 3; s->d_log_a = s->d_sweep; s->d_log_b = nx;
            } else if (c->pop_on_release) {
                /* v1.2 pop style RELEASE: the shove-it pops (full height, at once) as soon as the
                 * scooping stick starts coming back, like an ollie; no waiting time */
                if (pm > s->d_pend_peak) s->d_pend_peak = pm;
                if (!pop_live || (s->d_pend_peak >= 0.7f && pm < s->d_pend_peak - 0.12f)) {
                    name = sc_trick_name(back, scoop, side, 0); full = 1;
                }
            } else if (!pop_live || now - s->d_pend_since >= c->combo_wait_ms) {
                name = sc_trick_name(back, scoop, side, 0);
                /* v2.3: drawn at full speed. The slow "low pop" drawing (80 ms per unit) was often
                 * too slow for the game to see a shove-it at all */
                full = 1;
            }
            if (name) {
                sc_fire(s, name, now);
                s->emit_ms_per_unit = full ? c->ms_per_unit : c->low_pop_ms;
                s->d_pending = 0;
                sc_maybe_recenter(c, s, now);
                float ex = 0, ey = 0; sc_emit_step(c, s, now, &ex, &ey); s->d_t = now; *out_x = ex; *out_y = ey; return;
            }
            /* still waiting: the game keeps seeing the crouch */
            float cm = pm > 1.0f ? 1.0f : pm;
            s->d_t = now;
            s->ox = 0; s->oy = back ? -cm : cm;
            *out_x = 0; *out_y = back ? -cm : cm;
            return;
        }
    }

    float x = 0, y = 0;
    int full = 0;
    int fl = s->d_front_on && !s->d_front_spent, bl = s->d_back_on && !s->d_back_spent;
    /* Pop height of the second foot: ollie (front second) full when flicked up, low when down;
     * nollie (back second) full when flicked down, low when up. */
    if (fl && bl) {
        if (s->d_front_seq > s->d_back_seq) { s->d_back_spent = 1; x = ffx; y = ffy; full = fy>=0; }
        else { s->d_front_spent = 1; x = bbx; y = bby; full = by<=0; }
        if (s->d_windup) {
            /* the flick after a wind-up goes straight through, at full speed */
            s->d_log = 2; s->d_log_a = sc_angle(x, y); s->d_log_b = sc_angle(s->ox, s->oy); full = 1;
        }
    } else if (fl) { x = ffx; y = ffy; full = s->d_windup || !s->d_two || (s->d_back_spent && (fy>=0)); }
    else if (bl) { x = bbx; y = bby; full = s->d_windup || !s->d_two || (s->d_front_spent && (by<=0)); }
    /* v0.24: a foot on its own (the crouch) goes through at full speed. It used to be slowed like a
     * low-pop flick, so every crouch reached the game ~80 ms late (the input lag people felt). */
    else if (!s->d_front_on && !s->d_back_on) {
        /* light leans for manuals, only in each foot's own direction */
        /* v0.19o: a foot held through the landing (a manual / nose manual set up in the air) keeps
         * leaning, as a light lean, instead of being dropped */
        s->d_land_hold = 0;
        /* v0.19p: pushing a held foot right to the edge means a pop, not a manual: it counts as a
         * fresh press from the next poll (you could not ollie after landing in a manual) */
        if (s->d_land_hold && ((s->d_front_ignored && fm >= 0.85f) || (s->d_back_ignored && bm >= 0.85f))) {
            s->d_land_hold = 0; s->d_front_ignored = 0; s->d_back_ignored = 0;
        }
        int lh = s->d_land_hold;
        int front_lean = fy > 0 && (!s->d_front_ignored || lh), back_lean = by < 0 && (!s->d_back_ignored || lh);
        if (front_lean && (!back_lean || fm >= bm)) { x = ffx; y = ffy; }
        else if (back_lean) { x = bbx; y = bby; }
        /* (v0.19p: no cap any more - the game dropped the manual straight after landing) */
    }
    /* else: only a spent foot is held: neutral (v0.3), unless a slowed flick is still finishing */
    else full = !s->d_latch;
    /* (kept outside the if / else chain above on purpose) */
    if (!s->d_two && !s->d_windup && (fl != bl)) {
        /* the crouch is shown straight down / up: a scoop starting sideways must not reach the game
         * before it is recognised (it would start a shove-it on its own) */
        float m = sc_sqrt(x * x + y * y); x = 0; y = fl ? m : -m;
        /* v2.8: past ManualMaxPercent it is a crouch: send a full crouch (a handheld stick that only
         * reaches ~75 % gave the game a manual instead of a crouch) */
        if (m >= SC_MMAX(c) && !s->d_latch) y = fl ? 0.97f : -0.97f;
    }
    /* v0.19g: a one-foot sideways start waits a moment: if the other stick joins sideways, it is a
     * powerslide and the game must not see a wind-up first */
    /* v0.19r: only 40 ms - the full 120 ms ate the start of quick wind-ups (laser flips broke) */
    if (s->d_windup && !(fl && bl) && c->powerslide && now - s->d_windup_t < 40.0) { x = 0; y = 0; full = 1; }
    float m = sc_sqrt(x * x + y * y);
    if (m > 1.0f) { x /= m; y /= m; }

    /* Low pop: limit how fast the flick stick may move. A slowed flick is finished even if the
     * thumb lets go first (a quick flick would otherwise stop halfway and do nothing). */
    double dt = s->d_t > 0 ? now - s->d_t : 0;
    s->d_t = now;
    if (full) s->d_latch = 0;
    else if (m >= on && (!s->d_latch || m >= sc_sqrt(s->d_lx * s->d_lx + s->d_ly * s->d_ly) - 0.01f ||
                         (x * s->d_lx + y * s->d_ly) < 0.5f * m * sc_sqrt(s->d_lx * s->d_lx + s->d_ly * s->d_ly))) {
        /* keep the furthest point of this flick; a new direction starts a new one */
        s->d_latch = 1; s->d_lx = x; s->d_ly = y;
    } else if (m >= on) {
        x = s->d_lx; y = s->d_ly; /* thumb easing back: keep going to the furthest point */
        float ex = s->d_lx - s->ox, ey = s->d_ly - s->oy;
        if (sc_sqrt(ex * ex + ey * ey) < 0.08f) { s->d_latch = 0; x = s->ox; y = s->oy; }
    }
    else if (s->d_latch) {
        float ex = s->d_lx - s->ox, ey = s->d_ly - s->oy;
        if (sc_sqrt(ex * ex + ey * ey) < 0.08f) s->d_latch = 0;
        else { x = s->d_lx; y = s->d_ly; }
    }
    if (!full && c->low_pop_ms > 0) {
        float dx = x - s->ox, dy = y - s->oy, d = sc_sqrt(dx * dx + dy * dy);
        float step = (float)(dt / c->low_pop_ms);
        if (d > step && d > 0) { x = s->ox + dx / d * step; y = s->oy + dy / d * step; }
    }
    *out_x = x; *out_y = y;
}

/* v3.0 PURE mode. The mod invents nothing: no drawn tricks, no quick pops, no held-back scoops,
 * no landing guesses, no slowed flicks. The rules are only the ones a pair of feet needs:
 *   - a foot STARTS in its own pop direction (back foot down / front foot up) or from the side
 *     (a stock wind-up); started the wrong way it does nothing until let go
 *   - the started foot's stick goes to the game exactly as it is (scoops, wind-ups, flicks: the
 *     game reads its own gestures from your real stick motion, like the stock controls)
 *   - the second foot to move takes over; the first is then "used" and gives nothing until let go
 *   - a foot still held when the board leaves the ground is used too (so the pop / flick you are
 *     still holding can't tilt the landing into a manual); let go and press again in the air to
 *     set up a manual or a grind
 *   - with neither foot started, a light lean in a foot's own direction goes through (manuals)
 * Goofy / switch / true foot, powerslides, reverts, grabs, spin wind-up and bump-outs are
 * unchanged (they are in sc_translate). PopOnRelease still works: letting go of a full crouch
 * snaps the stick to the other end, as you asked for that option. */

/* v5.27 ONE-FOOT LIMIT: a foot on its own (the other let go) may sweep round the edge for shove-its,
 * 360 shove-its and wind-ups, but not on into the far half of the stick (a one-stick heelflip /
 * kickflip), and a sweep that started out at one side stops 65 degrees round on the other side (where
 * the game's 360 shove-it ends; going on to flat side read as a one-stick tre).
 * v5.28: IMPOSSIBLES are a different move - a circle that starts at the foot's own end (back foot
 * down / front foot up) and keeps going round (the game wants about 240 degrees). Once such a circle
 * has gone 240 degrees round (over the top: past any one-stick heelflip / kickflip) it is let through,
 * catching up from the limit in a few quick steps so the game still sees the whole circle.
 * k = 1 back foot (own end down), 0 front foot (own end up). */
static float sc_own_off(int k, float a) { return k ? sc_abs(sc_wrap(a)) : sc_abs(sc_wrap(a - 180.0f)); }
static void sc_vec_at(int k, float off_signed, float m, float* x, float* y) {
    /* off_signed: degrees from the own end, + towards +x */
    float o = off_signed < 0 ? -off_signed : off_signed, sx = off_signed < 0 ? -1.0f : 1.0f;
    float sn = sc_sin_deg(o), cs = sc_sin_deg(90.0f - o);
    *x = sx * m * sn; *y = k ? -m * cs : m * cs;
}
static void sc_lone_start(sc_state* s, int k, float x, float y) {
    float a = sc_angle(x, y);
    s->lone_a0[k] = a; s->lone_last[k] = a; s->lone_sw[k] = 0; s->lone_unl[k] = 0; s->lone_out[k] = 0;
    s->lone_seen[k] = 0; s->lone_from[k] = 0;
}
static void sc_lone_clamp(sc_state* s, int k, float* x, float* y) {
    float m = sc_sqrt(*x * *x + *y * *y);
    if (m < 0.05f) return;
    float a = sc_angle(*x, *y);
    s->lone_sw[k] += sc_wrap(a - s->lone_last[k]); s->lone_last[k] = a;
    int start_side = s->lone_side[k];
    float off = sc_own_off(k, a);
    /* signed offset from the own end (+ = the +x side) */
    float soff = (*x < 0) ? -off : off;
    float base = k ? 0.0f : 180.0f;   /* the foot's own end, in sc_angle degrees */
    if (!start_side && sc_own_off(k, s->lone_a0[k]) <= 35.0f && sc_abs(s->lone_sw[k]) >= 240.0f && m>=0.8f && !s->lone_unl[k]) {
        float dir = s->lone_sw[k] > 0 ? 1.0f : -1.0f;
        s->lone_unl[k] = 1;
        s->lone_out[k] = dir * 75.0f + sc_wrap(base - s->lone_a0[k]);   /* where the limit held it, measured from the start */
    }
    if (s->lone_unl[k]) {   /* impossible: catch up along the circle, at most 30 degrees a frame */
        float d = s->lone_sw[k] - s->lone_out[k];
        if (d > 30.0f) d = 30.0f;
        if (d < -30.0f) d = -30.0f;
        s->lone_out[k] += d;
        float aa = s->lone_a0[k] + s->lone_out[k];
        while (aa < 0) aa += 360.0f;
        while (aa >= 360.0f) aa -= 360.0f;
        *x = m * sc_sin_deg(aa); *y = -m * sc_sin_deg(90.0f - aa);   /* sc_angle: 0 down, 90 right, 180 up */
        return;
    }
    /* v5.30: held 15 degrees BELOW flat side. At 98 degrees (just above flat) the held position was the
     * game's kickflip / heelflip end point - your one-stick log: down, then swung out to the side =
     * a kickflip. Shove-its still reach their end (the game's shove-it ends 60-90 degrees round). */
    float lim = 75.0f;
    /* v5.31: a sweep that came round THROUGH the foot's own end from one side (the 360 shove-it path,
     * however it started - your log: from one side over the top, then back round through the bottom)
     * stops 65 degrees round on the other side. Flat side there is the game's one-stick tre / laser. */
    {
        int side_now = *x > 0 ? 1 : -1;
        if (off >= 30.0f) {
            if (s->lone_seen[k] && side_now != s->lone_seen[k] && s->lone_sw[k] != 0) { /* crossed over */ }
            s->lone_seen[k] = side_now;
        }
        if (off < 25.0f && s->lone_seen[k]) s->lone_from[k] = s->lone_seen[k];
        int from = s->lone_from[k] ? s->lone_from[k] : start_side;
        if (from && side_now != from) lim = 65.0f;
    }
    /* a sweep that went round past the far end stays held on the side it went round on */
    if (sc_abs(s->lone_sw[k]) > 90.0f) { float sd = s->lone_sw[k] > 0 ? 1.0f : -1.0f; if (!k) sd = -sd; if (sd * soff < 0) off = 360.0f - off; soff = sd * off; }
    if (off <= lim) return;
    sc_vec_at(k, soff < 0 ? -lim : lim, m, x, y);
}
/* A quick continuous sweep away from a foot's own pop end is a trick-out,
 * not the slow/held tail pivot. Decide from actual motion, without a wait or
 * a generated trick path. The rest of the gesture uses the ground translator. */
static int sc_rail_scoop(sc_state* s, double now, float fx, float fy, float bx, float by, float off) {
    float fm=sc_sqrt(fx*fx+fy*fy), bm=sc_sqrt(bx*bx+by*by);
    int foot = fm>=.5f && bm<off ? 1 : bm>=.5f && fm<off ? 2 : 0;
    if(!s->grind || s->grind_pop || !foot) { s->rail_scoop_track=0; return 0; }
    float x=foot==1?fx:bx, y=foot==1?fy:by;
    float own=foot==1?y:-y, a=sc_angle(x,y);
    double dt=now-s->rail_scoop_last;
    if(s->rail_scoop_track==foot && dt>0 && dt<=100) {
        float delta=sc_wrap(a-s->rail_scoop_angle);
        if(sc_abs(delta)<=110) {
            s->rail_scoop_sweep+=delta;
            if(sc_abs(s->rail_scoop_sweep)<8) s->rail_scoop_turn=0;
            else if(!s->rail_scoop_turn) s->rail_scoop_turn=s->rail_scoop_last;
            if(s->rail_scoop_turn) {
                double elapsed=now-s->rail_scoop_turn;
                if(sc_abs(s->rail_scoop_sweep)>=35 && elapsed>0 &&
                   sc_abs(s->rail_scoop_sweep)*1000.0>=400.0*elapsed) {
                    s->rail_scoop_exit=foot; s->rail_scoop_track=0; return foot;
                }
            }
            s->rail_scoop_angle=a;s->rail_scoop_last=now;
            /* Slow motion stays in the pivot frame; don't let a later twitch
             * reinterpret an already sideways-held board adjustment. */
            if(sc_abs(s->rail_scoop_sweep)<35 || own>.6f) return 0;
        }
        s->rail_scoop_track=0;
    }
    if(own>=.35f && sc_abs(x)<=.7f*own) {
        s->rail_scoop_track=foot;s->rail_scoop_angle=a;s->rail_scoop_last=now;
        s->rail_scoop_sweep=0;s->rail_scoop_turn=0;s->rail_scoop_x=x;s->rail_scoop_y=y;
    } else s->rail_scoop_track=0;
    return 0;
}
static void sc_pure(const sc_config* c, sc_state* s, double now, float fx, float fy, float bx, float by,
                    float* out_x, float* out_y) {
    sc_deadzone(&fx, &fy, c->deadzone);
    sc_deadzone(&bx, &by, c->deadzone);
    int airborne = s->air || s->takeoff > 0;
    int mirror = c->flip_x;
    /* v5.14: no goofy mirror any more. reSkate mirrors its own flick gestures for a goofy skater, so
     * mirroring here as well turned kickflips into heelflips (and scoops round) - your v5.12 test:
     * goofy was only right with "Kick / heel sides" = MIRRORED, which cancelled this out. */
    /* True Foot swaps foot jobs, not kick/heel directions. The extra regular
     * switch mirror reversed successful flips in the v5.90 gameplay reports. */
    if (mirror) { fx = -fx; bx = -bx; }
    float fm = sc_sqrt(fx * fx + fy * fy), bm = sc_sqrt(bx * bx + by * by);
    /* a stick that reaches 95 % is all the way: worn / handheld sticks rarely give 100 % */
    if (fm >= 0.95f) { fx /= fm; fy /= fm; fm = 1.0f; }
    if (bm >= 0.95f) { bx /= bm; by /= bm; bm = 1.0f; }
    float on = c->active, off = c->active * 0.6f;
    s->phase = SC_IDLE; s->d_pending = 0; s->d_latch = 0; s->path_n = 0;

    /* Preserve a two-foot gesture across setup release, including a short
     * neutral gap covered by the existing 70 ms release pulse. */
    if(c->pop_on_release && !airborne &&
       ((fm>=on && ((s->d_back_on && !s->d_back_spent) || s->d_back_full || (s->rp_t>0 && now-s->rp_t<70 && s->rp_y>0))) ||
        (bm>=on && ((s->d_front_on && !s->d_front_spent) || s->d_front_full || (s->rp_t>0 && now-s->rp_t<70 && s->rp_y<0))))) {
        s->rp_two_foot=1;s->rp_t=0;
    }
    if(s->rp_two_foot) {
        s->d_front_full=s->d_back_full=0;
        s->d_front_peak=s->d_back_peak=0;
        if(fm<off && bm<off)s->rp_two_foot=0;
    }
    /* Recognize the second foot before clearing a setup released in this poll. */
    if(fm>=on && !s->d_front_on && s->d_back_on && !s->d_back_spent)s->d_front_flick=1;
    if(bm>=on && !s->d_back_on && s->d_front_on && !s->d_front_spent)s->d_back_flick=1;
    if(fm<off)s->d_front_flick=0;
    if(bm<off)s->d_back_flick=0;
    /* let go = fresh foot */
    if (fm < off) { s->d_front_on = 0; s->d_front_spent = 0; s->d_front_ignored = 0; }
    if (bm < off) { s->d_back_on = 0; s->d_back_spent = 0; s->d_back_ignored = 0; }
    if (!s->d_front_on && !s->d_back_on) s->d_windup = 0;

    /* take-off: whatever is still held is left over from the trick */
    if (airborne && !s->d_took_off) {
        s->d_took_off = 1;
        if (fm >= off) { s->d_front_on = 1; s->d_front_spent = 1; s->tk_fa = sc_angle(fx, fy); }
        if (bm >= off) { s->d_back_on = 1; s->d_back_spent = 1; s->tk_ba = sc_angle(bx, by); }
        s->d_windup = 0; s->rp_t = 0; s->d_back_full = s->d_front_full = 0;
        /* v5.19 HELD TRICKS: the game holds a flip (held kickflip etc.) while the flick stick stays out
         * where the flick ended (85 %+, each trick has its own window). The foot that flicked last and
         * is still pushed right out keeps the game's stick exactly where the flick left it. */
        s->hold_foot = 0;
        {
            /* A setup/wind-up stick is not a held flip. In the grind trace,
             * that false capture kept the back foot in Skate's flick frame
             * until rail contact, then reversed it into a tail pivot. */
            int fh = fm >= 0.8f && (s->d_front_flick || (c->one_foot_flips && fy < -0.2f*fm));
            int bh = bm >= 0.8f && (s->d_back_flick || (c->one_foot_flips && by > 0.2f*bm));
            if (fh && (!bh || s->d_front_seq > s->d_back_seq)) s->hold_foot = 1;
            else if (bh) s->hold_foot = 2;
            float om = sc_sqrt(s->ox * s->ox + s->oy * s->oy);
            float ha = s->hold_foot == 1 ? sc_angle(fx, fy) : sc_angle(bx, by);
            /* only when that foot points where the game's stick is (the flick it made), not e.g. the
             * ollie crouch still held down while the stick shows the front foot's flick up */
            if (s->hold_foot && om >= 0.8f && sc_abs(sc_wrap(ha - sc_angle(s->ox, s->oy))) < 45.0f) { s->hold_x = s->ox / om; s->hold_y = s->oy / om; }
            else s->hold_foot = 0;
        }
    } else if (!airborne) { s->d_took_off = 0; s->hold_foot = 0; }

    float fa = sc_angle(fx, fy), ba = sc_angle(bx, by);       /* 0 = down, 180 = up */
    float f_off = sc_abs(sc_wrap(fa - 180.0f)), b_off = sc_abs(sc_wrap(ba));
    float pop_lim = c->windup_deg > 0 && c->windup_deg < 75.0f ? c->windup_deg : 75.0f;
    int front_pop_dir = fy > 0 && f_off <= pop_lim, back_pop_dir = by < 0 && b_off <= pop_lim;
    float side_lim = c->windup_deg > 0 ? 120.0f : pop_lim;

    if(!c->air_controls) {s->rail_scoop_track=s->rail_scoop_exit=0;}
    /* A rail foot remains a board adjustment across vertical and sideways
     * positions. A fresh opposite-foot flick confirms a Flick exit; a full
     * vertical setup followed by release confirms a Release exit. */
    if (c->air_controls) {
        int fpop = fy >= 0.70f && sc_abs(fx) < 0.65f * fm;
        int bpop = by <= -0.70f && sc_abs(bx) < 0.65f * bm;
        int fflick = fm >= on && (fy > 0.2f * fm || sc_abs(fx) > 0.7f * fm);
        int bflick = bm >= on && (by < -0.2f * bm || sc_abs(bx) > 0.7f * bm);
        int release_back=0,release_front=0;
        int scoop_foot=sc_rail_scoop(s,now,fx,fy,bx,by,off);
        if (!s->grind) {
            s->grind_pop = 0; s->rail_scoop_exit=0; s->grind_fpop_t = s->grind_bpop_t = 0;
        } else {
            if(fpop && (!s->grind_prev || !s->grind_fpop))s->grind_fpop_t=now;
            if(bpop && (!s->grind_prev || !s->grind_bpop))s->grind_bpop_t=now;
            if(!fpop)s->grind_fpop_t=0;
            if(!bpop)s->grind_bpop_t=0;
            release_back=c->pop_on_release && s->grind_prev && s->grind_bpop && s->d_back_full && bm<off;
            release_front=c->pop_on_release && s->grind_prev && s->grind_fpop && s->d_front_full && fm<off;
            if (fm < off && bm < off && !(s->rp_t>0 && now-s->rp_t<70)) {s->grind_pop = 0;s->rail_scoop_exit=0;}
            /* A held stall position remains an adjustment. A fresh flick of
             * the opposite foot after a firm held pop is an explicit trick-out. */
            int held_back = s->grind_prev && bpop && fflick && !s->grind_fflick &&
                            s->grind_bpop_t > 0 && now-s->grind_bpop_t >= 70.0;
            int held_front = s->grind_prev && fpop && bflick && !s->grind_bflick &&
                             s->grind_fpop_t > 0 && now-s->grind_fpop_t >= 70.0;
            if (s->grind_prev && !s->grind_pop &&
                (scoop_foot || held_back || held_front || (bpop && fflick && !s->grind_fflick) ||
                 (fpop && bflick && !s->grind_bflick) || release_back || release_front)) {
                s->grind_pop = 1;
                s->d_front_on = s->d_back_on = 0;
                s->d_front_spent = s->d_back_spent = 0;
                s->d_front_ignored = s->d_back_ignored = 0;
                s->d_land_hold = s->d_front_carry = s->d_back_carry = 0;
                s->d_windup=0;
                if(scoop_foot) {
                    int k=scoop_foot==2;
                    if(k){s->d_back_on=1;s->d_back_seq=++s->d_seq;s->pf_t[1]=s->rail_scoop_turn;}
                    else {s->d_front_on=1;s->d_front_seq=++s->d_seq;s->pf_t[0]=s->rail_scoop_turn;}
                    s->lone_side[k]=0;sc_lone_start(s,k,s->rail_scoop_x,s->rail_scoop_y);
                    s->last_name=k?"back-foot rail scoop":"front-foot rail scoop";s->fired++;
                }
                else if(held_back || (bpop && fflick)) {s->d_back_on=1;s->d_back_seq=++s->d_seq;s->pf_t[1]=s->grind_bpop_t;}
                else if(held_front || (fpop && bflick)) {s->d_front_on=1;s->d_front_seq=++s->d_seq;s->pf_t[0]=s->grind_fpop_t;}
            }
        }
            if(release_back || release_front) {
                s->rp_t=now;s->rp_y=release_back?1.0f:-1.0f;s->rp_two_foot=0;
                s->d_back_full=s->d_front_full=0;
                s->last_name=release_back?"ollie (rail release)":"nollie (rail release)";s->fired++;
            }
        s->grind_prev = s->grind;
        s->grind_fpop = fpop; s->grind_bpop = bpop;
        s->grind_fflick = fflick; s->grind_bflick = bflick;
        if (s->grind && !s->grind_pop) {
            float x = fx - bx, y = sc_abs(fy) - sc_abs(by);
            float gain = c->grind_sensitivity > 0 ? c->grind_sensitivity : 1.0f;
            x *= gain; y *= gain;
            float m = sc_sqrt(x*x + y*y); if (m > 1) { x /= m; y /= m; }
            /* A single foot sweeping past vertical is board positioning,
             * not a trick-out. Track full Release setups without switching
             * frames; Flick exits require fresh input from the other foot. */
            s->d_front_full=fpop && fm>=.8f;s->d_back_full=bpop && bm>=.8f;
            /* No spent-foot or one-foot flick clamp while changing grinds. */
            s->d_front_on = s->d_back_on = 0;
            s->d_front_spent = s->d_back_spent = 0;
            s->d_front_ignored = s->d_back_ignored = 0;
            s->d_land_hold = s->d_front_carry = s->d_back_carry = 0;
            s->d_was_air = s->d_took_off = s->hold_foot = 0;
            s->d_windup = s->rp_t = 0;
            s->d_t = now; *out_x = x; *out_y = y; return;
        }
    }

    if (airborne) {
        if (s->hold_foot) {   /* v5.19: held trick - until that foot eases off or swings away */
            float hx = s->hold_foot == 1 ? fx : bx, hy = s->hold_foot == 1 ? fy : by, hm = sc_sqrt(hx * hx + hy * hy);
            float ta = s->hold_foot == 1 ? s->tk_fa : s->tk_ba;
            float other_m=s->hold_foot==1?bm:fm;
            float other_a=s->hold_foot==1?sc_angle(bx,by):sc_angle(fx,fy);
            float other_ta=s->hold_foot==1?s->tk_ba:s->tk_fa;
            int other_on=s->hold_foot==1?s->d_back_on:s->d_front_on;
            int pivot=other_m>=on && (!other_on || sc_abs(sc_wrap(other_a-other_ta))>=20.0f);
            /* New board positioning takes priority over a held flick; use
             * the same 20-degree movement threshold as ordinary air pivots. */
            if (!pivot && hm >= 0.7f && sc_abs(sc_wrap(sc_angle(hx, hy) - ta)) < 20.0f) {
                s->d_was_air = 1; s->d_t = now; *out_x = s->hold_x; *out_y = s->hold_y;
                return;
            }
            s->hold_foot = 0;
        }
        /* in the air: only feet pressed AFTER take-off count */
        /* v5.16: ...or a foot held from take-off that you then swing round (40 degrees or more), e.g. the
         * back foot still down from the ollie pushed out to the side for a smith / feeble. Before, a
         * held foot stayed used up until it came back to the middle, so the back foot (usually still
         * held from the pop) never set up a grind - only the front foot did. */
        if (s->d_front_on && s->d_front_spent && fm >= off && sc_abs(sc_wrap(sc_angle(fx, fy) - s->tk_fa)) >= 20.0f) s->d_front_spent = 0;   /* v5.18: 20 degrees like the back foot */
        if (s->d_back_on && s->d_back_spent && bm >= off && sc_abs(sc_wrap(sc_angle(bx, by) - s->tk_ba)) >= 20.0f) s->d_back_spent = 0;   /* v5.17: 20 degrees, quicker */
        int fl = s->d_front_on ? !s->d_front_spent : fm >= off;
        int bl = s->d_back_on ? !s->d_back_spent : bm >= off;
        if (fl && !s->d_front_on && fm >= on) { s->d_front_on = 1; s->d_front_seq = ++s->d_seq; }
        if (bl && !s->d_back_on && bm >= on) { s->d_back_on = 1; s->d_back_seq = ++s->d_seq; }
        float x = 0, y = 0;
        if (c->air_controls) {
            /* Session style: front foot moves the nose half, back foot the tail half */
            /* v5.18: finer control - a foot still held from the pop / flick steers its end SIDEWAYS
             * straight away (no swing or let-go needed); only its up / down part (the nose / tail
             * tilt that lands manuals) waits until you swing it round or let go */
            int fh = airborne && !fl && s->d_front_on && s->d_front_spent && fm >= off;   /* after AirDelayMs: the trick is in */
            int bh = airborne && !bl && s->d_back_on && s->d_back_spent && bm >= off;
            /* v5.20: a held foot now tilts its end too (back foot down = tail down for a 5-0, down to
             * the side = smith / feeble), exactly like a fresh press - so the back foot, which is
             * nearly always still held from the ollie, works like the front foot. Only a foot pushed
             * right out straight up / down (85 %+, within 30 degrees) gives no tilt: that is a crouch
             * held for the next pop, and tilting with it landed you in manuals. */
            int fcr = fh && fm >= 0.85f && sc_abs(fx) < 0.5f * fm;
            int bcr = bh && bm >= 0.85f && sc_abs(bx) < 0.5f * bm;
            if (fl) { x += fx; y += sc_abs(fy); } else if (fh) { x += fx; if (!fcr) y += sc_abs(fy); }
            /* v5.17: the back foot moves the TAIL, so pushing it one way swings the nose the other way:
             * its sideways push is reversed (your test: it set up grinds the wrong way round) */
            if (bl) { x -= bx; y -= sc_abs(by); } else if (bh) { x -= bx; if (!bcr) y -= sc_abs(by); }
        } else {
            /* stock-like: the lean you set up for the landing (manual / nose manual) */
            if (fl && fy > 0 && (!bl || fm >= bm)) y = fy;
            else if (bl && by < 0) y = by;
        }
        /* Match rail pivot strength while approaching it. Preserve vertical
         * manual intent; apply the rail gain only to sideways board pivot. */
        if(c->air_controls) x*=c->grind_sensitivity>0?c->grind_sensitivity:1.0f;
        float m = sc_sqrt(x * x + y * y); if (m > 1.0f) { x /= m; y /= m; }
        s->d_was_air = 1; s->d_t = now; *out_x = x; *out_y = y;
        return;
    }
    /* v5.45: carry a landing lean using the SAME air tilt in the new stance.
     * Reinterpreting a held up-stick as a newly started back-foot gesture on
     * touchdown can produce a scoop/pop instead of the intended manual. */
    if (s->d_was_air && c->air_controls) {
        s->d_front_carry = fm >= off && fm < SC_MMAX(c);
        s->d_back_carry = bm >= off && bm < SC_MMAX(c);
        s->d_land_hold = s->d_front_carry || s->d_back_carry;
    }
    s->d_was_air = 0;
    if (s->d_land_hold) {
        if (fm < off || (front_pop_dir && fm >= SC_MMAX(c))) s->d_front_carry = 0;
        if (bm < off || (back_pop_dir && bm >= SC_MMAX(c))) s->d_back_carry = 0;
        int new_foot = (!s->d_front_carry && fm >= on) || (!s->d_back_carry && bm >= on);
        if (!new_foot && (s->d_front_carry || s->d_back_carry)) {
            float x = (s->d_front_carry ? fx : 0) - (s->d_back_carry ? bx : 0);
            float y = (s->d_front_carry ? sc_abs(fy) : 0) - (s->d_back_carry ? sc_abs(by) : 0);
            float m = sc_sqrt(x * x + y * y); if (m > 1) { x /= m; y /= m; }
            s->d_t = now; *out_x = x; *out_y = y; return;
        }
        s->d_land_hold = 0;
    }

    /* v5.21 GRIND ADJUST: on a rail, light or sideways pushes of either foot move the board like the
     * air controls (both feet add up, back foot sideways reversed), so the back foot works like the
     * front foot in a grind too. A foot pushed firmly in its own pop direction (60 %+, back foot down /
     * front foot up) is the crouch for popping out: from then on it is the normal pop / flick controls. */
    if (s->grind && c->air_controls && !s->grind_pop && s->phase == SC_IDLE &&
        !(s->d_front_on && !s->d_front_spent) && !(s->d_back_on && !s->d_back_spent)) {
        int fpop = fy > 0 && fm >= 0.6f && sc_abs(fx) < 0.7f * fm, bpop = by < 0 && bm >= 0.6f && sc_abs(bx) < 0.7f * bm;
        if (!fpop && !bpop) {
            float x = 0, y = 0;
            if (fm >= off) { x += fx; y += sc_abs(fy); }
            if (bm >= off) { x -= bx; y -= sc_abs(by); }
            float m = sc_sqrt(x * x + y * y); if (m > 1.0f) { x /= m; y /= m; }
            /* feet used here are not pops: they must be let go (or pushed firmly) to pop */
            s->d_t = now; *out_x = x; *out_y = y;
            return;
        }
    }

    /* starting a foot */
    if (!s->d_front_on && fm >= on && !s->d_front_ignored) {
        if (s->d_back_on || s->d_front_flick || front_pop_dir || f_off <= side_lim) {
            s->d_front_on = 1; s->d_front_seq = ++s->d_seq; s->pf_t[0] = now;
            s->lone_side[0] = (!s->d_back_on && f_off >= 55.0f) ? (fx > 0 ? 1 : -1) : 0; sc_lone_start(s, 0, fx, fy);
            if (!s->d_back_on && !s->d_front_flick && !front_pop_dir) { s->d_windup = 1; s->d_windup_t = now; s->d_log = 1; s->d_log_a = fa; s->d_log_b = 0; }
        } else s->d_front_ignored = 1;
    }
    if (!s->d_back_on && bm >= on && !s->d_back_ignored) {
        if (s->d_front_on || s->d_back_flick || back_pop_dir || b_off <= side_lim) {
            s->d_back_on = 1; s->d_back_seq = ++s->d_seq; s->pf_t[1] = now;
            s->lone_side[1] = (!s->d_front_on && b_off >= 55.0f) ? (bx > 0 ? 1 : -1) : 0; sc_lone_start(s, 1, bx, by);
            if (!s->d_front_on && !s->d_back_flick && !back_pop_dir) { s->d_windup = 1; s->d_windup_t = now; s->d_log = 1; s->d_log_a = ba; s->d_log_b = 1; }
        } else s->d_back_ignored = 1;
    }
    int fl = s->d_front_on && !s->d_front_spent, bl = s->d_back_on && !s->d_back_spent;

    /* second foot in: it takes over, the first is used up (logged so the game's answer is checked) */
    /* v4.1: both feet pressed almost together (your goofy log: front foot up, back foot down 30 ms
     * later): the game saw the crouch for 2 frames only and did not pop. The first foot's crouch is
     * now shown for at least 70 ms (full depth) before the second foot takes over. */
    s->pf_hold = 0;
    if (fl && bl) {
        int front_first = s->d_front_seq < s->d_back_seq;
        double t0 = s->pf_t[front_first ? 0 : 1];
        if (!s->d_windup && t0 > 0 && now - t0 < 70.0) s->pf_hold = front_first ? 1 : -1;
    }
    if (fl && bl && !s->pf_hold) {
        if (s->d_front_seq > s->d_back_seq) { s->d_back_spent = 1; s->d_front_flick=1; bl = 0; }
        else { s->d_front_spent = 1; s->d_back_flick=1; fl = 0; }
        s->d_log = 2; s->d_log_a = fl ? fa : ba; s->d_log_b = 0;
    }

    if(c->pop_on_release && fl && bl)s->rp_two_foot=1;
    /* PopOnRelease: let go of a full one-foot crouch = the stick snaps to the other end */
    if (c->pop_on_release && !s->rp_two_foot && !s->rail_scoop_exit) {
        if (s->d_windup || s->d_front_on || s->d_back_spent) { s->d_back_full = 0; s->d_back_peak = 0; }
        else if (bl && bm >= 0.8f && back_pop_dir) { s->d_back_full = 1; if (bm > s->d_back_peak) s->d_back_peak = bm; }
        if (s->d_windup || s->d_back_on || s->d_front_spent) { s->d_front_full = 0; s->d_front_peak = 0; }
        else if (fl && fm >= 0.8f && front_pop_dir) { s->d_front_full = 1; if (fm > s->d_front_peak) s->d_front_peak = fm; }
        int rel = 0;
        if (s->d_back_full && !s->d_front_on && bm < s->d_back_peak - 0.12f && (by > -0.85f * s->d_back_peak || bm < 0.75f)) rel = 1;
        else if (s->d_front_full && !s->d_back_on && fm < s->d_front_peak - 0.12f && (fy < 0.85f * s->d_front_peak || fm < 0.75f)) rel = 2;
        if (rel) {
            if (rel == 1) { if (s->d_back_on) s->d_back_spent = 1; s->d_back_full = 0; s->d_back_peak = 0; }
            else { if (s->d_front_on) s->d_front_spent = 1; s->d_front_full = 0; s->d_front_peak = 0; }
            s->rp_t = now; s->rp_y = rel == 1 ? 1.0f : -1.0f;
            s->last_name = rel == 1 ? "ollie (let go)" : "nollie (let go)"; s->fired++;
        }
        if (s->rp_t > 0 && now - s->rp_t < 70.0) { s->d_t = now; *out_x = 0; *out_y = s->rp_y; return; }
        s->rp_t = 0;
    }

    /* v3.8: ONE foot on its own cannot flick through the middle into the other end of the stick
     * (that is the stock one-stick kickflip / heelflip / ollie: a nollie crouch on the left stick then
     * a flick down-left was a nollie kickflip with no back foot). A flip needs the other foot.
     * Sweeping round the edge (shove-its, 360 shove-its, wind-ups) is still fine. */
    for (int k = 0; k < 2; ++k) {
        int live = k ? (bl && !s->d_front_on && !s->d_back_flick) : (fl && !s->d_back_on && !s->d_front_flick);
        float px = k ? bx : fx, py = k ? by : fy, pm = k ? bm : fm;
        float own = k ? -py : py;                    /* + = towards this foot's own end */
        if (c->one_foot_flips || !live) { s->lp_blk[k] = 0; s->lp_dip[k] = 0; s->lp_mag[k] = 0; continue; }
        float a = sc_angle(px, py);
        if (own > 0.3f && pm >= 0.5f) { s->lp_dip[k] = 0; s->lp_blk[k] = 0; }
        else if (pm < 0.5f) s->lp_dip[k] = 1;
        else if (s->lp_mag[k] >= 0.5f && sc_abs(sc_wrap(a - s->lp_ang[k])) > 100.0f) s->lp_dip[k] = 1;  /* jumped across */
        if (own < -0.15f && s->lp_dip[k]) s->lp_blk[k] = 1;
        s->lp_ang[k] = a; s->lp_mag[k] = pm;
    }
    float x = 0, y = 0;
    if (s->pf_hold) { x = 0; y = s->pf_hold > 0 ? 1.0f : -1.0f; }
    else if (fl && !s->lp_blk[0]) { x = fx; y = fy; if (!c->one_foot_flips && !s->d_back_on && !s->d_front_flick) sc_lone_clamp(s, 0, &x, &y); }
    else if (bl && !s->lp_blk[1]) { x = bx; y = by; if (!c->one_foot_flips && !s->d_front_on && !s->d_back_flick) sc_lone_clamp(s, 1, &x, &y); }
    else if (fl || bl) { x = 0; y = 0; }
    else if (!s->d_front_on && !s->d_back_on) {
        /* light leans (manuals), each foot in its own direction only */
        int front_lean = fy > 0 && !s->d_front_ignored, back_lean = by < 0 && !s->d_back_ignored;
        if (front_lean && (!back_lean || fm >= bm)) { x = fx; y = fy; }
        else if (back_lean) { x = bx; y = by; }
    }
    /* (only a used foot held: centred until it is let go) */
    /* v4.1: the SECOND foot's flick (the other foot is used: this is the pop) made straight sideways
     * never popped (your goofy log: front foot flicked flat left / right from a held crouch, the game
     * saw down -> side = nothing). A flick more sideways than towards its own end is turned to the
     * diagonal, which is the game's kickflip / heelflip (nollie: the same towards the tail). */
    if (!s->pf_hold && ((fl && (s->d_back_spent || s->d_front_flick)) || (bl && (s->d_front_spent || s->d_back_flick)))) {
        float sgn = fl ? 1.0f : -1.0f, own = y * sgn, ax = sc_abs(x), m = sc_sqrt(x * x + y * y);
        if (m >= on && ax > 0.2f && own < 0.7f * ax) { x = (x > 0 ? 0.707f : -0.707f) * m; y = sgn * 0.707f * m; }
    }

    /* a one-foot sideways start waits 40 ms in case the other stick joins (a powerslide) */
    if (s->d_windup && !(s->d_front_on && s->d_back_on) && c->powerslide && now - s->d_windup_t < 40.0) { x = 0; y = 0; }
    float m = sc_sqrt(x * x + y * y);
    if (m > 1.0f) { x /= m; y /= m; }
    s->d_t = now;
    *out_x = x; *out_y = y;
}

static sc_pad sc_pump_output(sc_state* s,sc_pad out,double now) {
    if (s->pump_r3) {
        /* v5.56: R3 substitutes for the stock R2 / RT pumping crouch.
         * Foot gestures stay neutral; release R3 releases that trigger.
         * Preserve movement-stick steering and independently held grabs. */
        /* Stock reference: full R2 in 29-60ms, release over ~50ms.
         * Start at a low trigger value and reach full over 45ms. */
        float u = (float)((now - s->pump_press_t) / 45.0);
        if (u > 1) u = 1; if (u < 0) u = 0;
        s->pump_level = 0.05f + 0.95f*u;
        unsigned char pump_rt = (unsigned char)(255*s->pump_level);
        if (pump_rt > out.rt) out.rt = pump_rt;
        out.rx = out.ry = 0;
        out.buttons &= (unsigned short)~SC_RTHUMB;
    } else if (s->pump_release_t > 0) {
        float u = (float)((now + 1 - s->pump_release_t) / 50.0);
        if (u >= 1) { s->pump_release_t = 0; s->pump_level = 0; }
        else {
            s->pump_level = s->pump_release_level * (1-u);
            unsigned char pump_rt = (unsigned char)(255*s->pump_level);
            if (pump_rt > out.rt) out.rt = pump_rt;
        }
    }
    return out;
}

/* Translate one physical pad state into the state the game should see. */
static sc_pad sc_translate(const sc_config* c, sc_state* s, sc_pad in, double now) {
    if (!c->enabled) {s->board_frame_valid=0;return in;}
    /* v5.25 RADIAL MENUS: holding the D-pad opens the game's radial menus, where you pick with the
     * sticks. While a D-pad button is held (and 0.25 s after, while you let go on your pick) the pad
     * goes to the game untouched - the sticks were being turned into foot moves / steering. */
    if (in.buttons & 0x000F) s->dp_t = now + 1;
    if (s->dp_t > 0 && now + 1 - s->dp_t < 250.0) {
        s->pump_r3 = 0;
        s->pump_release_t = 0; s->pump_level = 0;
        s->d_front_on = s->d_back_on = 0; s->d_front_spent = s->d_back_spent = 0; s->phase = SC_IDLE;
        s->d_front_ignored = s->d_back_ignored = 1;   /* a stick still held after the pick is ignored until let go */
        return in;
    }
    /* Optional grab-air experiment. Ground pop/prewind remains two-foot.
     * Once a trigger is held in confirmed air, use physical stock sticks and
     * analogue grab triggers until landing. Horizontal body steering is always
     * live; only a carried vertical body-flip or board gesture needs release
     * or a deliberate direction change. */
    int stock_airborne = (s->takeoff == 1 || s->air) && !s->grind;
    /* Sustained steep orientation or a rapid grounded curve qualifies.
     * In-air tilting on a flat ollie cannot arm this gate. Unknown data fails
     * closed. Preserve qualification through the air, then clear on landing. */
    if (!stock_airborne) {
        if(!s->stock_ramp_was_air && !s->grind && s->takeoff==0 &&
           s->stock_ramp_departing && s->stock_ramp_eligible &&
           now+1-s->stock_ramp_last_ground<=180.0) {
            /* A qualified ramp pop briefly reports 103 (or 200 before the
             * takeoff flag updates). Preserve prior ground evidence only;
             * these transition states can never create qualification. */
        } else if(s->stock_ramp_was_air || s->grind || s->takeoff!=0 || !s->stock_ramp_valid) {
            s->stock_ramp_since=0;s->stock_ramp_eligible=0;
            s->stock_ramp_history_n=s->stock_ramp_history_next=s->stock_ramp_curve=0;
            s->stock_ramp_evidence=0;
        } else {
            s->stock_ramp_last_ground=now+1;
            float tilt=s->stock_ramp_vertical;
            if(sc_abs(tilt)>=0.5f) {
                if(!s->stock_ramp_since)s->stock_ramp_since=now+1;
                if(now+1-s->stock_ramp_since>=80.0)s->stock_ramp_evidence=now+1;
            } else s->stock_ramp_since=0;
            /* About 10 degrees minimum tilt and about 7 degrees of change
             * over 100-250 ms. Eight 30-ms samples bound the short history.
             * Only validated ground samples enter it; air/pop never arm it. */
            int curve=0;
            for(int i=0;i<s->stock_ramp_history_n;i++) {
                double age=now+1-s->stock_ramp_history_t[i];
                if(age>=100.0 && age<=250.0 && sc_abs(tilt)>=0.17f &&
                   sc_abs(tilt-s->stock_ramp_history_y[i])>=0.12f)curve=1;
            }
            if(curve){s->stock_ramp_evidence=now+1;s->stock_ramp_curve=1;}
            int prev=(s->stock_ramp_history_next+7)%8;
            if(!s->stock_ramp_history_n || now+1-s->stock_ramp_history_t[prev]>=30.0) {
                int i=s->stock_ramp_history_next;
                s->stock_ramp_history_y[i]=tilt;s->stock_ramp_history_t[i]=now+1;
                s->stock_ramp_history_next=(i+1)%8;
                if(s->stock_ramp_history_n<8)s->stock_ramp_history_n++;
            }
            s->stock_ramp_eligible=s->stock_ramp_evidence>0 && now+1-s->stock_ramp_evidence<=450.0;
            if(!s->stock_ramp_eligible)s->stock_ramp_curve=0;
        }
    }
    s->stock_ramp_was_air=stock_airborne;
    int early_ground=c->stock_transition_ground && s->stock_ramp_eligible &&
        s->takeoff==0 && !stock_airborne && !s->grind;
    if (s->stock_air_active && ((!stock_airborne && !early_ground) || !c->stock_grab_air)) {
        s->stock_air_active=0;
        s->stock_early_active=0;
        s->d_front_on=s->d_back_on=s->d_front_spent=s->d_back_spent=0;
        s->d_front_ignored=s->d_back_ignored=1;
        s->d_land_hold=s->d_front_carry=s->d_back_carry=0;
        s->d_was_air=s->d_took_off=s->hold_foot=s->d_windup=0;
        s->rp_t=0;s->d_back_full=s->d_front_full=0;s->phase=SC_IDLE;
        s->rv_start=0;s->tr_rel[0]=s->tr_rel[1]=0;
        s->tr_down[0]=in.lt>=100;s->tr_down[1]=in.rt>=100;
        s->tr_press[0]=s->tr_press[1]=now;
        /* A grab held over touchdown becomes ground steering immediately.
         * Seed trigger edges above so it cannot invent a fresh revert. */
        s->rv_hold=0;
        s->bf_side=s->bf_arm=0;s->bf_air_t=0;
        for(int i=0;i<2;i++){s->g_down[i]=(in.buttons&(i?SC_RSHOULDER:SC_LSHOULDER))!=0;s->g_grab[i]=0;s->g_pulse[i]=0;}
    }
    if (c->stock_grab_air && s->stock_ramp_eligible &&
        (early_ground || (stock_airborne && (s->stock_air_active || in.lt>=30 || in.rt>=30)))) {
        float lx=sc_axis(in.lx),ly=sc_axis(in.ly),rx=sc_axis(in.rx),ry=sc_axis(in.ry);
        float lm=sc_sqrt(lx*lx+ly*ly),rm=sc_sqrt(rx*rx+ry*ry);
        if(!s->stock_air_active) {
            s->stock_air_active=1;s->stock_early_active=early_ground;s->d_log=early_ground?11:10;
            s->stock_air_block_l=lm>=c->active;
            s->stock_air_block_r=rm>=c->active;
            s->stock_air_langle=sc_angle(lx,ly);s->stock_air_rangle=sc_angle(rx,ry);
        }
        if(lm<c->active*.6f || sc_abs(sc_wrap(sc_angle(lx,ly)-s->stock_air_langle))>=30.0f)s->stock_air_block_l=0;
        if(rm<c->active*.6f || sc_abs(sc_wrap(sc_angle(rx,ry)-s->stock_air_rangle))>=30.0f)s->stock_air_block_r=0;
        sc_pad stock=in;
        if(s->stock_air_block_l)stock.ly=0;
        if(s->stock_air_block_r)stock.rx=stock.ry=0;
        s->ps_side=s->ps_logged=0;s->rv_start=0;s->bf_side=s->bf_arm=0;
        if(early_ground) {
            /* R3 keeps its pumping crouch during the stock ramp approach.
             * Ground stock steering/pop/prewind use the physical sticks. */
            int held=(in.buttons&(SC_LTHUMB|SC_RTHUMB))==SC_RTHUMB;
            float limit=s->pump_r3?c->active:c->deadzone;
            int neutral=lm<=limit && rm<=limit;
            int was=s->pump_r3;
            if(!held || !neutral)s->pump_r3=0;
            else if(!was){s->pump_r3=1;s->pump_press_t=now;s->pump_release_t=0;}
            if(was && !s->pump_r3) {
                if(neutral && !held){s->pump_release_t=now+1;s->pump_release_level=s->pump_level;}
                else{s->pump_release_t=0;s->pump_level=0;}
            }
            if(!neutral || (in.buttons&(SC_LTHUMB|SC_RTHUMB))==(SC_LTHUMB|SC_RTHUMB)) {
                s->pump_release_t=0;if(!s->pump_r3)s->pump_level=0;
            }
            return sc_pump_output(s,stock,now);
        }
        s->pump_r3=0;s->pump_release_t=0;s->pump_level=0;
        return stock;
    }
    sc_pad out = in;

    /* Walking: the physical left stick held forward or sideways (not a pop) with no trick
     * going on. Nothing on the board asks for that, so hand the pad back to the game. */
    if (c->walk_ms > 0) {
        float wx = sc_axis(in.lx), wy = sc_axis(in.ly);
        sc_deadzone(&wx, &wy, c->deadzone);
        float wm = sc_sqrt(wx * wx + wy * wy), wa = sc_angle(wx, wy);
        int walking_shape = wm >= 0.5f && wa > 75.0f && wa < 285.0f; /* not pulled down */
        if (walking_shape && s->phase == SC_IDLE) {
            if (s->walk_since <= 0) s->walk_since = now + 1;
            if (now + 1 - s->walk_since >= c->walk_ms) { s->walk_request = 1; s->walk_since = 0; return in; }
        } else {
            s->walk_since = 0;
        }
    }

    /* v5.26 stick layout: LEFT / RIGHT FOOT (default) = each stick is one of your feet, so goofy puts the
     * left stick on your back foot; FRONT / BACK FOOT = the left stick is always the front foot */
    int left_is_front = c->front_back ? 1 : !c->goofy;
    if (c->feet_swapped) left_is_front = !left_is_front;
    /* v0.33 switch stance 3 = TRUE FOOT (Skater XL / Session): each stick stays on its foot, and the
     * feet stay on their ends of the board, so riding switch the front-foot stick becomes the back
     * foot (regular: left stick down + right stick up = switch ollie). */
    if (c->switch_flip == 3 && s->switched && !c->front_back) left_is_front = !left_is_front;
    /* XL board positioning keeps its takeoff foot frame through a continuous
     * air/rail sequence. Riding stance can still update for the next trick.
     * A body turn is not a new inward/outward thumb gesture. */
    int board_position=c->pure && c->xl_layout && c->air_controls &&
                       (s->air || s->takeoff>0 || s->grind);
    if(board_position) {
        if(!s->board_frame_valid){
            s->board_frame_valid=1;
            s->board_frame_left=s->feet_map_ok?s->feet_map_left:left_is_front;
        }
        left_is_front=s->board_frame_left;
    }else s->board_frame_valid=0;
    /* A late ground correction must not flip a held manual into a fresh pop.
     * Keep that manual's roles until it ends; fresh input then uses the stance. */
    if (s->feet_map_ok && s->d_land_hold && !s->air && s->takeoff <= 0)
        left_is_front = s->feet_map_left;
    if (s->feet_map_ok && left_is_front != s->feet_map_left) sc_remap_foot_history(s);
    s->feet_map_ok = 1; s->feet_map_left = left_is_front;
    float lx = sc_axis(in.lx), ly = sc_axis(in.ly), rx = sc_axis(in.rx), ry = sc_axis(in.ry);
    float fx = left_is_front ? lx : rx, fy = left_is_front ? ly : ry;
    float bx = left_is_front ? rx : lx, by = left_is_front ? ry : ly;

    /* v5.15 STICK STEERING (Skater XL style, End menu "Stick steering"): rolling with the back foot
     * let go, the front-foot stick pushed mostly sideways turns the board, like the triggers. It
     * stays steering until that stick comes back to the middle, then it is a foot again (pops,
     * flicks, manuals). Not in the air or on a rail (air controls / grinds keep the sticks). */
    float stick_turn = 0;
    if (c->stick_steer) {
        int ground = !s->air && !s->grind && s->takeoff <= 0;
        int steering_left=c->stick_steer==2 ? 1 : left_is_front;
        float sx=steering_left?lx:rx,sy=steering_left?ly:ry;
        float ox=steering_left?rx:lx,oy=steering_left?ry:ly;
        float fax=sc_abs(sx),fmag=sc_sqrt(sx*sx+sy*sy),bmag=sc_sqrt(ox*ox+oy*oy);
        int foot_busy=s->d_front_on || s->d_back_on || s->d_front_full || s->d_back_full ||
                      (s->rp_t>0 && now-s->rp_t<70);
        if (!s->st_latch) {
            if (ground && !foot_busy && s->phase == SC_IDLE && !s->ps_side && bmag < 0.3f && fax >= 0.15f && fax > 1.5f*sc_abs(sy)) s->st_latch=1;
        } else if (fmag < 0.10f || !ground) s->st_latch=0;
        if (s->st_latch) {
            stick_turn=sx;
            if(steering_left==left_is_front){fx=0;fy=0;}else{bx=0;by=0;}
            if(steering_left){lx=0;ly=0;}else{rx=0;ry=0;}
        }
    }

    /* Custom switch directions affect foot gestures, not steering, pumping,
     * trigger reverts or the physical inward/outward powerslide gesture. */
    /* XL keeps the physical stick direction while its front/back job swaps. */
    int foot_switch=s->switched;
    if(board_position && !c->front_back && c->switch_flip==3)
        foot_switch=left_is_front!=((!c->goofy)!=!!c->feet_swapped);
    if(c->xl_layout && !c->front_back && c->switch_flip==3 && foot_switch){fy=-fy;by=-by;}
    if(foot_switch) {
        if(c->switch_invert_x){fx=-fx;bx=-bx;}
        if(c->switch_invert_y){fy=-fy;by=-by;}
    }

    /* v0.19g powerslides: sticks pushed towards each other and held = toe side (frontside:
     * skate.'s left stick down-right), away from each other = heel side (backside: down-left). Not in the air or on a rail.
     * Reverts: double-tap RT = revert one way, LT = the other (skate.: a circle on the left
     * stick). A single press still steers. */
    int ps_active = 0, ps_pending=0; float psx = 0, psy = 0;
    if (c->revert_ms > 0) {
        unsigned char tv[2] = {in.lt, in.rt};
        for (int i = 0; i < 2; ++i) {
            int down = tv[i] >= 100;
            if (down && !s->tr_down[i]) {
                /* v0.22i: a double tap quicker than RevertMinGapMs never reverted in your logs (it turned into a
                 * powerslide), so it is ignored: that press just steers */
                if (c->revert_input == 1 && s->tr_rel[i] > 0 && now - s->tr_rel[i] <= c->revert_ms && now - s->tr_rel[i] >= c->revert_min_gap_ms &&
                    s->tr_rel[i] - s->tr_press[i] <= c->revert_ms && !s->grind &&
                    /* v1.7: only while just rolling: a quick tap-tap of a trigger to steer while
                     * crouched / popping set off the revert motion mid-trick (the board turned and
                     * steering went dead for 0.4 s) */
                    /* v4.5: a foot held as a manual lean (not a full crouch) no longer blocks it: revert out of manuals */
                    !s->air && s->takeoff != 1 && s->game_crouch != 1 && s->phase == SC_IDLE &&
                    !(s->d_front_on && sc_sqrt(fx * fx + fy * fy) >= SC_MMAX(c)) && !(s->d_back_on && sc_sqrt(bx * bx + by * by) >= SC_MMAX(c))) {
                    /* v0.22g: you found reverts work when the double tap is not too quick. The first
                     * tap's turn is still settling in the game, so the revert waits until at least
                     * RevertGapMs after the first tap was let go (stick centred meanwhile). */
                    double gap = now - s->tr_rel[i], start = s->tr_rel[i] + c->revert_gap_ms;
                    s->rv_start = start > now ? start : now; s->rv_side = i ? 1 : -1; s->tr_rel[i] = 0; s->rv_block = 1; s->rv_hold = i + 1;
                    s->d_log = 4; s->d_log_a = (float)s->rv_side; s->d_log_b = (float)gap;
                }
                s->tr_press[i] = now;
            } else if (!down && s->tr_down[i]) {
                s->tr_rel[i] = s->rv_start > 0 && now - s->rv_start < 400.0 ? 0 : now;
            }
            s->tr_down[i] = down;
        }
        /* v5.3: squeeze BOTH triggers (within 120 ms of each other) = revert, towards the trigger you
         * pressed first. No turn goes to the game first (both triggers cancel out), unlike a double
         * tap whose first tap carved the board - your log: those often became powerslides. */
        if (c->revert_input == 2 && s->tr_down[0] && s->tr_down[1] && !s->rv_both && sc_abs(s->tr_press[0] - s->tr_press[1]) <= 120.0 &&
            (s->rv_start <= 0 || now - s->rv_start > 600.0) && !s->grind && !s->air && s->takeoff != 1 && s->game_crouch != 1 &&
            !(s->d_front_on && sc_sqrt(fx * fx + fy * fy) >= SC_MMAX(c)) && !(s->d_back_on && sc_sqrt(bx * bx + by * by) >= SC_MMAX(c))) {
            int first_r = s->tr_press[1] < s->tr_press[0];
            s->rv_start = now; s->rv_side = first_r ? 1 : -1; s->rv_block = 1; s->rv_hold = 3; s->rv_both = 1;
            s->tr_rel[0] = s->tr_rel[1] = 0;
            s->d_log = 4; s->d_log_a = (float)s->rv_side; s->d_log_b = 0;
        }
        if (!s->tr_down[0] && !s->tr_down[1]) s->rv_both = 0;
    }
    /* v0.19k reverts, as in the base game: spin the RIGHT stick all the way round, fast. RT = spun
     * right (clockwise, toe side), LT = spun left (heel side). Starts and ends at the side, so the
     * flick recognizer never sees a crouch-then-pop. Drawn on the flick stick; steering is held
     * still while it runs and for a moment after. */
    int rv_active = 0; float rvx = 0, rvy = 0;
    /* v3.1: a revert set off by a quick tap-tap of a trigger just before a pop is cancelled the
     * moment a foot crouches or the board leaves the ground. It used to keep the left stick for
     * ~0.4 s (the revert circle, then centred), so the trigger you held for the spin did nothing:
     * no rotation in the air. */
    /* v4.6: a foot held as a manual lean no longer cancels it (only a full crouch / the pop / the air):
     * it cancelled every revert out of a manual */
    if (s->rv_start > 0 && ((s->d_front_on && sc_sqrt(fx * fx + fy * fy) >= SC_MMAX(c)) || (s->d_back_on && sc_sqrt(bx * bx + by * by) >= SC_MMAX(c)) ||
                            s->air || s->takeoff == 1 || s->game_crouch == 1)) {
        s->rv_start = 0; s->d_log = 8;
    }
    if (s->rv_start > 0) {
        /* v0.19q: RevertShape picks the left-stick motion, so it can be changed live in the ini:
         * 1 = full circle from the top (the only one that reverted in your v0.19l log, default),
         * 2 = half circle bottom -> side -> top (the game's data read one way),
         * 3 = half circle top -> side -> bottom (the data read the other way),
         * 4 = side -> bottom -> other side (the trick list icons). RT and LT spin opposite ways. */
        double spin = c->revert_sweep_ms > 30 ? c->revert_sweep_ms : 160.0;
        double lead = 0.0;   /* v4.8: shape 5 first pulls the stick down and holds it */
        double t = now - s->rv_start, dur = c->revert_shape == 5 ? spin * 580.0 / 300.0 : lead + spin + 20.0, quiet = dur + 100.0;
        int cw = (s->rv_side * (c->revert_flip ? -1 : 1) * (c->goofy ? -1 : 1)) > 0;   /* goofy: FS / BS swap sides */
        if (t < 0) { rv_active = 2; }
        else if (t < dur) {
            float u = t > spin ? 1.0f : (float)(t / spin), th;  /* 0 up, 90 right, 180 down, 270 left */
            switch (c->revert_shape) {
            case 2: th = cw ? 180.0f + 180.0f * u : 180.0f - 180.0f * u; break;
            case 3: th = cw ? 360.0f - 180.0f * u : 180.0f * u; break;
            case 4: th = cw ? 90.0f + 180.0f * u : 270.0f - 180.0f * u; break;
            case 1: th = cw ? 360.0f * u : -360.0f * u; break;
            default: th = 0; break;
            }
            rvx = sc_sin_deg(th) * 0.95f;
            rvy = sc_sin_deg(90.0f - th) * 0.95f;
            if (c->revert_shape == 5) {
                /* v4.6 = your STOCK recording: left stick straight DOWN, then swept round through the
                 * side up over the top (about 225 degrees), in about a quarter second. RT goes round
                 * the right side, LT the left. */
                /* v5.5, from all your stock reverts side by side (the ones that turned you vs. the ones
                 * that powerslid): out to one side, a fairly SLOW quarter down to the bottom (that is
                 * where the game starts its slide, state 101), then a QUICK swing across to the other
                 * side, just above level, and HOLD it there (a quarter to half a second). Stopping at
                 * the bottom = powerslide; letting go straight after the swing = often nothing. */
                float sd = (s->rv_side * (c->revert_flip ? -1 : 1)) > 0 ? 1.0f : -1.0f;
                double k = spin / 300.0, t1 = 40.0 * k, t2 = t1 + 160.0 * k, t3 = t2 + 110.0 * k;
                float a;   /* 0 up, 90 right, 180 down, 270 left - on the starting side's half */
                if (t < t1) { rvx = sd * (float)(t / t1); rvy = 0; a = -1; }
                else if (t < t2) a = 90.0f + 90.0f * (float)((t - t1) / (t2 - t1));          /* side -> bottom */
                else if (t < t3) a = 180.0f + 105.0f * (float)((t - t2) / (t3 - t2));        /* bottom -> other side, a bit above */
                else a = 285.0f;                                                            /* hold */
                if (a >= 0) { float ar = sd > 0 ? a : 360.0f - a; rvx = sc_sin_deg(ar); rvy = sc_sin_deg(90.0f - ar); }
            } else if (c->revert_shape < 1 || c->revert_shape > 4) {
                /* 5 (default, how you described the base game): from the middle straight out to
                 * the side (RT right, LT left), then a half circle down round the bottom to the
                 * other side. */
                /* 6 = the trick list icons exactly: the arc stops at about 4-5 / 7-8 o'clock
                 * (135 degrees round), not all the way to the other side */
                float side = cw ? 1.0f : -1.0f, arc = c->revert_shape == 6 ? 135.0f : 180.0f;
                if (u < 0.3f) { rvx = side * 0.95f * (u / 0.3f); rvy = 0; }
                else {
                    float v = (u - 0.3f) / 0.7f;
                    float a = cw ? 90.0f + arc * v : 270.0f - arc * v;   /* right->down->left or left->down->right */
                    rvx = sc_sin_deg(a) * 0.95f; rvy = sc_sin_deg(90.0f - a) * 0.95f;
                }
            }
            rv_active = 1;
        } else if (t < quiet) { rv_active = 2; }
        else s->rv_start = 0;
    }
    /* Leaving the slide context must clear its latch before foot translation.
     * A latched slide used to skip sc_pure throughout a stall, even after release. */
    if (!c->powerslide || s->air || s->takeoff == 1 || s->grind) {
        s->ps_side = s->ps_logged = 0; s->ps_lt = s->ps_rt = 0; s->ps_pop_t=0;
    }
    if (c->powerslide && !s->air && s->takeoff != 1 && !s->grind && s->phase != SC_EMIT) {
        float alx = sc_abs(lx), arx = sc_abs(rx);
        int side = 0;
        /* v0.19g: the sticks pushed TOWARDS each other (left stick right, right stick left) = toe
         * side, AWAY from each other = heel side */
        float slide_on = s->ps_side ? 0.35f : 0.4f;
        float slide_angle = s->ps_side ? 0.9f : 1.2f;
        int lside = alx >= slide_on && alx > slide_angle * sc_abs(ly);
        int rside = arx >= slide_on && arx > slide_angle * sc_abs(ry);
        if (!lside) s->ps_lt = 0; else if (s->ps_lt <= 0) s->ps_lt = now + 1;
        if (!rside) s->ps_rt = 0; else if (s->ps_rt <= 0) s->ps_rt = now + 1;
        /* Ordinary sideways inputs use a joining window; strong opposing
         * thumbs can recover a slide even when they arrive farther apart.
         * Protect recent vertical setup inputs, not stale spent-foot flags. */
        int recent_pop=s->ps_pop_t>0 && now+1-s->ps_pop_t<300.0;
        /* A sideways-start scoop becomes a trick setup once it passes its
         * full pop end. Do not let the later opposite-foot flick steal it
         * as a powerslide merely because d_windup is still set. */
        int foot_busy=c->pure && recent_pop;
        if((fy>=.85f && sc_abs(fx)<.5f) || (by<=-.85f && sc_abs(bx)<.5f))
            s->ps_pop_t=now+1;
        /* Reserve a developing opposing-stick slide before both thumbs
         * cross the threshold. The v5.94 trace emitted a synthetic flick
         * at L48%/R58%, then sent the slide: that first poll can spoil it. */
        ps_pending=!foot_busy && alx>=.25f && arx>=.25f &&
            alx>1.2f*sc_abs(ly) && arx>1.2f*sc_abs(ry) && (lx>0)!=(rx>0);
        if (lside && rside && (lx > 0) != (rx > 0) && (s->ps_side || (!foot_busy && (sc_abs(s->ps_lt - s->ps_rt) <= 220.0 || (alx>=.65f && arx>=.65f)))))
            side = lx > 0 ? 1 : -1;
        if (side && side != s->ps_side) { s->ps_start = now; s->ps_side = side; }
        else if (!side && s->ps_side) { s->ps_side = 0; s->ps_logged = 0; }
        if (!ps_active && s->ps_side) {
            int dir = s->ps_side * (c->ps_flip ? -1 : 1) * (c->goofy ? -1 : 1);   /* goofy: toe / heel side swap */
            psx = dir > 0 ? 0.75f : -0.75f;
            /* Keep the familiar neutral-thumb diagonal. Fore/aft movement of
             * both thumbs adjusts its downward angle continuously for coping. */
            psy = -0.75f + 0.5f * (ly + ry) * 0.5f;
            if (psy < -1.0f) psy = -1.0f;
            if (psy > -0.35f) psy = -0.35f;
            ps_active = 1;
            if (!s->ps_logged) { s->ps_logged = 1; s->d_log = 5; s->d_log_a = (float)s->ps_side; }
        }
        if (s->ps_side) {
            /* the feet are busy sliding: no pops from them until they are let go */
            s->d_front_ignored = s->d_back_ignored = 1;
            s->d_front_on = s->d_back_on = 0; s->d_pop = 0; s->d_pending = 0; s->d_windup = 0; s->d_latch = 0;
            if (s->phase == SC_COOLDOWN || s->phase == SC_ARMED) s->phase = SC_IDLE;
        }
    }
    float vx, vy;
    if (s->ps_side || ps_active == 1 || ps_pending) { vx = 0; vy = 0; }
    else if (c->pure) sc_pure(c, s, now, fx, fy, bx, by, &vx, &vy);
    else if (c->direct) sc_direct(c, s, now, fx, fy, bx, by, &vx, &vy);
    else sc_feet_to_flick(c, s, now, fx, fy, bx, by, &vx, &vy);
    /* Skate mirrors trick recognition when its animation frame changes.
     * XL preserves physical flick/scoop directions across that change.
     * Air and rail pivots have their own continuous frame above. */
    if(c->pure && c->xl_layout && s->game_mirror_known && s->game_mirror &&
       !board_position)vx=-vx;
    s->ox = vx; s->oy = vy;
    /* v0.19k switch: the game says you are riding the other way round from your spawn, so the
     * board's nose is now behind you. Turn the flick stick round so the front foot still pops the
     * end in front of you (nollie) and the back foot the end behind you (ollie). */
    /* v5.42: a stance change only turns the game stick round once it is back near the middle: a
     * stick held through a 180 (nose manual set up in the air) keeps meaning the same end until you
     * let it go - turning it round under the held stick read as a fresh push = an ollie. */
    {
        int want = SC_SWFLIP(c, s) ? 1 : 0;
        int held = vx * vx + vy * vy >= 0.3f * 0.3f;
        if (!s->sw_lat_ok || !held) { s->sw_lat = want; s->sw_lat_ok = 1; }
        else if (want != s->sw_lat && (s->air || s->takeoff == 1)) {
            /* v5.44: in the air (spinning round) a held stick moves straight over to the new end.
             * (v5.43 let it go for 60 ms and eased it back: the game turns the flag late in the
             * spin, so that gap landed right on touchdown and the manual was lost) */
            s->sw_lat = want;
        }
        if (s->sw_lat) { vy = -vy; if (c->switch_flip == 2) vx = -vx; }

    }
    /* v0.19o: the stance flips half way round a 180, often while a foot is held (a nose manual
     * set up in the air). Jumping the stick from one end to the other read as a flick and glitched,
     * so for 250 ms after a change the stick eases across (about 1 stick-width per 120 ms). */
    if (s->switched != s->sw_prev) { s->sw_prev = s->switched; if (c->switch_flip) s->sw_blend_until = now + 250.0; }
    /* v2.0: no easing any more - the stance only changes with both sticks let go, and the easing
     * kept the last air output (stick pulled down) alive for 0.25 s after a landing = a manual */
    if (0 && now < s->sw_blend_until && !rv_active) {
        double dt = s->sw_t > 0 ? now - s->sw_t : 0;
        float dx = vx - s->sw_x, dy = vy - s->sw_y, d = sc_sqrt(dx * dx + dy * dy), step = (float)(dt / 120.0);
        if (d > step && d > 0) { vx = s->sw_x + dx / d * step; vy = s->sw_y + dy / d * step; }
    }
    s->sw_t = now; s->sw_x = vx; s->sw_y = vy;
    /* v5.7: a manual lean on the flick stick stays during a revert (zeroing it cancelled the manual) */
    if (rv_active && sc_sqrt(vx * vx + vy * vy) >= SC_MMAX(c)) { vx = 0; vy = 0; }
    out.rx = sc_short(vx);
    out.ry = sc_short(vy);

    /* v4.7: the trigger still held from the revert's double tap does not steer until you let go of it:
     * after the revert it kept turning the board (your "stretched out turn") */
    if (s->rv_hold == 3 ? (!s->tr_down[0] && !s->tr_down[1]) : (s->rv_hold && !s->tr_down[s->rv_hold - 1])) s->rv_hold = 0;
    float steer_l = (s->rv_hold == 1 || s->rv_hold == 3) ? 0.0f : (float)in.lt, steer_r = (s->rv_hold == 2 || s->rv_hold == 3) ? 0.0f : (float)in.rt;
    /* (v0.22f: the v0.22e ease-in made turning feel unresponsive and did not help reverts: removed) */
    float steer = (steer_r - steer_l) / 255.0f;
    /* v0.22d: after a revert, the trigger you are still holding from the double tap does not steer
     * until you let go (steering straight after the revert turned it into a powerslide) */
    /* (v0.22f: the v0.22d steering block after a revert left you unable to turn for a while: removed) */
    s->rv_block = 0;
    float steer_out = steer + stick_turn; if (steer_out > 1) steer_out = 1; if (steer_out < -1) steer_out = -1;
    float mag = sc_pow(sc_abs(steer_out), c->steer_curve) * c->steer_scale;
    out.lx = sc_short(steer_out < 0 ? -mag : mag);
    out.ly = 0;
    /* v0.21 spin wind-up. skate. winds up a spin (FS / BS prewind) instead of turning when the
     * left stick is pushed to the side while you are crouched for an ollie or nollie. The game's
     * own gesture (Gesture_Body_Prewind_Left / _Right) is: stick from UP round to UP-LEFT /
     * UP-RIGHT, held there at least 85 % out. Our turning on the triggers never made that shape,
     * so: crouched + a trigger pulled = that motion, held while the trigger is held. It carries
     * on into the air (the spin) until you let the trigger go. */
    if (c->prewind && (c->direct || c->pure)) {
        int back_crouch = s->d_back_on && !s->d_back_spent && !s->d_front_on;
        int front_crouch = s->d_front_on && !s->d_front_spent && !s->d_back_on;
        /* v0.22e: only a real crouch (the game's own crouch state, or the stick right at the edge
         * when that is unknown). A manual lean is not a crouch: winding up from a manual made the
         * stick go to the powerslide corner and you got a manual powerslide. */
        float pm = back_crouch ? sc_sqrt(bx * bx + by * by) : sc_sqrt(fx * fx + fy * fy);
        /* v3.4: your stock recording shows the game stays in state 100 all through a crouch (103 is the
         * pop itself), so waiting for 103 only let the wind-up start as you popped - too late. A foot
         * held 85 % out in its pop direction is the crouch. */
        int real = pm >= 0.85f || s->game_crouch > 0;
        int crouched = !s->air && !s->d_windup && (back_crouch || front_crouch) && s->phase == SC_IDLE && real;
        float wind_threshold=0.3f;
        if(c->prewind_steer && !s->air && s->takeoff!=1) {
            int percent=c->prewind_percent; if(percent<50 || percent>90)percent=70;
            wind_threshold=percent/100.0f;
        }
        int want = steer >= wind_threshold ? 1 : steer <= -wind_threshold ? -1 : 0;
        /* v2.0: the wind-up ends when you land. It used to stay on while the trigger was still
         * held, so the left stick sat pulled back and to the side after landing: you landed in a
         * manual and the board turned (with air controls off too). Let go of the trigger to turn
         * normally again... it now turns normally at once. */
        int airborne = s->air || s->takeoff == 1;
        if (s->pw_side && s->pw_was_air && !airborne) { s->pw_side = 0; s->pw_logged = 0; s->pw_block = want; }
        if (s->pw_side) s->pw_was_air = airborne;
        if (crouched || airborne) s->pw_crouch_t = now;
        /* never pops (crouch let go, still on the ground a moment later): plain turning again */
        if (s->pw_side && !airborne && !crouched && now - s->pw_crouch_t > 250.0) { s->pw_side = 0; s->pw_logged = 0; s->pw_block = want; }
        if (!want || want != s->pw_block) s->pw_block = 0;
        if (!want) s->pw_side = 0;
        else if (want != s->pw_side && crouched && !s->pw_block) { s->pw_side = want; s->pw_t = now; s->pw_nollie = front_crouch; s->pw_was_air = 0; }
        if (s->pw_side && want == s->pw_side) {
            float sx = (float)s->pw_side, hx, hy, ax, ay;      /* hold point / start point */
            /* v0.21b: 1 = match the crouch (you: UP worked from a nollie crouch, not an ollie one, so
             * an ollie crouch winds up from DOWN), 2 = always down, 3 = straight side, 4 = always up */
            /* v0.31 switch: the game's stick is turned round, so match the crouch the GAME sees */
            int game_nollie = s->pw_nollie != SC_SWFLIP(c, s);
            /* v3.2: the game's own gestures (Gesture_Body_Prewind_Left / _Right): the LEFT stick from UP
             * round to up-left / up-right, held out at the side (>= 85 %). The _Nollie versions start
             * from DOWN. Shape 1 now follows that (it had them the other way round); 5 = the old order. */
            /* v3.4: your stock prewind: left stick DOWN, then round to the side, held through the pop
             * (the game's gesture data has y the other way up). Ollie = from down, nollie = from up. */
            int from_up = c->prewind == 4 || (c->prewind == 1 && game_nollie) || (c->prewind == 5 && !game_nollie);
            if (c->prewind == 3) { ax = 0; ay = 0; hx = sx; hy = 0; }
            else if (from_up) { ax = 0; ay = 1.0f; hx = 0.8f * sx; hy = 0.6f; }
            else { ax = 0; ay = -1.0f; hx = 0.8f * sx; hy = -0.6f; }
            double ms = c->prewind_ms > 0 ? c->prewind_ms : 60.0, t = now - s->pw_t;
            float u = t >= ms ? 1.0f : (float)(t / ms);
            /* start point first (a short move out from the centre), then round to the hold point */
            float px, py;
            if (u < 0.4f) { float v = u / 0.4f; px = ax * v; py = ay * v; }
            else { float v = (u - 0.4f) / 0.6f; px = ax + (hx - ax) * v; py = ay + (hy - ay) * v;
                   float m = sc_sqrt(px * px + py * py); if (m > 0.01f) { px *= 0.97f / m; py *= 0.97f / m; } }
            /* v3.2: once the gesture's end point has been shown, settle straight out to the side: the middle
             * of the game's hold range (left 225-315 / right 45-135 degrees, 85 % out) */
            if (c->prewind != 3 && t >= ms + 30.0) { px = 0.97f * sx; py = 0; }
            out.lx = sc_short(px); out.ly = sc_short(py);
            if (!s->pw_logged) { s->pw_logged = 1; s->d_log = 6; s->d_log_a = sx; }
        } else s->pw_logged = 0;
    }
    /* v0.23 bump out of a grind: on a rail, click L3 = bump off to the left, R3 = to the right
     * (skate.'s own bump is the left stick pushed left / right). The click itself is kept from
     * the game while grinding. Both clicked together is still the Session on/off switch. */
    {
        unsigned short th = (unsigned short)(in.buttons & (SC_LTHUMB | SC_RTHUMB));
        unsigned short down = (unsigned short)(th & ~s->bump_prev);
        int pump_was_held = s->pump_r3;
        int pump_ground = !s->grind && !s->air && s->takeoff != 1;
        /* Use the foot activation threshold, with hysteresis: small pad
         * drift must not repeatedly cancel and restart a held pump. */
        float pump_limit = s->pump_r3 ? c->active : c->deadzone;
        int pump_neutral = lx*lx + ly*ly <= pump_limit*pump_limit &&
                           rx*rx + ry*ry <= pump_limit*pump_limit;
        if (!pump_ground || !pump_neutral || th != SC_RTHUMB) s->pump_r3 = 0;
        /* Recover if the click began before touchdown or while a stick
         * was off-centre. Centred held R3 is pump intent, not a new flip. */
        else if (s->phase == SC_IDLE &&
                 !s->d_front_on && !s->d_back_on && s->game_crouch != 1)
            { if(!s->pump_r3){s->pump_press_t=now;s->pump_release_t=0;}
              s->pump_r3=1;s->bf_arm=s->bf_side=0; }
        if (pump_was_held && !s->pump_r3) {
            /* A physical release gets the same short analogue fall as
             * stock R2. Foot gestures and grinds cancel it immediately. */
            if (pump_neutral && !s->grind && !(th & SC_RTHUMB)) {
                s->pump_release_t = now + 1;
                s->pump_release_level = s->pump_level;
            } else { s->pump_release_t = 0; s->pump_level = 0; }
        }
        if (!pump_neutral || s->grind || th == (SC_LTHUMB|SC_RTHUMB)) {
            s->pump_release_t = 0;
            if (!s->pump_r3) s->pump_level = 0;
        }
        if (s->grind && c->bump_ms > 0 && down && th != (SC_LTHUMB | SC_RTHUMB)) {
            /* v5.51: ramp lip exits need the game's LEFT stick vertical input.
             * Use the clicked physical stick, independent of stance / foot layout.
             * A clear vertical tilt chooses that exit; a centred/sideways click
             * keeps the original L3-left / R3-right rail bump. */
            float cx = sc_axis((down & SC_RTHUMB) ? in.rx : in.lx);
            float cy = sc_axis((down & SC_RTHUMB) ? in.ry : in.ly);
            s->bump_y = sc_abs(cy) >= 0.5f && sc_abs(cy) > sc_abs(cx) ? (cy > 0 ? 1 : -1) : 0;
            s->bump_side = s->bump_y ? 0 : (down & SC_RTHUMB) ? 1 : -1;
            /* A click must make a fresh native movement gesture, even if
             * the same direction is already held by a prior exit/prewind.
             * Only re-arm an already-held direction; fresh exits stay immediate. */
            int held_direction=s->bump_y?sc_axis(out.ly)*s->bump_y>.5f:sc_axis(out.lx)*s->bump_side>.5f;
            if(now-s->bump_output_time<=50 && s->bump_output_time>0)
                held_direction|=s->bump_y?s->bump_output_y*s->bump_y>.5f:s->bump_output_x*s->bump_side>.5f;
            s->bump_neutral_until=held_direction?now+20:0;
            s->bump_until = now + c->bump_ms + (held_direction?20:0);
            s->d_log = 7; s->d_log_a = (float)s->bump_side; s->d_log_b = (float)s->bump_y;
        }
        /* v3.2 body flips (the game's Gesture_Body_FrontFlip: the LEFT stick from the middle to DOWN
         * and held there; backflip: to UP). Click the FRONT foot's stick = front flip, the BACK foot's
         * stick = backflip: while crouched it waits for the pop, in the air it starts at once. The
         * left stick is held there until you land. */
        int airborne = s->air || s->takeoff == 1;
        if (c->body_flips && !s->grind && !s->pump_r3 && down && th != (SC_LTHUMB | SC_RTHUMB)) {
            int front_click = (down & SC_LTHUMB) ? left_is_front : !left_is_front;
            int dir = front_click ? 1 : -1;   /* v3.4 stock recording: front flip = left stick UP, backflip = DOWN */
            int crouched = s->d_front_on || s->d_back_on || s->game_crouch == 1;
            if (airborne) { s->bf_side = dir; s->bf_t = now; s->bf_arm = 0; s->d_log = 9; s->d_log_a = (float)dir; }
            else if (crouched) { s->bf_arm = dir; s->bf_arm_t = now; s->d_log = 9; s->d_log_a = (float)(dir * 2); }
        }
        /* v3.4: starts with the pop (the other foot's flick), as in your stock flips, or at take-off */
        if (s->bf_arm && (airborne || s->d_log == 2 || s->rp_t == now)) { s->bf_side = s->bf_arm; s->bf_t = now; s->bf_arm = 0; }
        if (s->bf_arm && !airborne && now - s->bf_arm_t > 700.0) s->bf_arm = 0;   /* never popped */
        if (s->bf_side && s->bf_was_air && !airborne) s->bf_side = 0;            /* landed */
        if (s->bf_side && !airborne && !s->bf_was_air && now - s->bf_t > 400.0) s->bf_side = 0; /* never left the ground */
        if (s->bf_side && airborne && !s->bf_air_t) s->bf_air_t = now;
        if (!s->bf_side) s->bf_air_t = 0;
        s->bf_was_air = airborne;
        s->bump_prev = th;
        if ((s->grind || (c->body_flips && (airborne || s->bf_arm || s->d_front_on || s->d_back_on || s->game_crouch == 1)))
            && th != (SC_LTHUMB | SC_RTHUMB)) out.buttons &= (unsigned short)~(SC_LTHUMB | SC_RTHUMB);
    }
    if (ps_active) { out.lx = sc_short(psx); out.ly = sc_short(psy); }
    if (rv_active == 1) { out.lx = sc_short(rvx); out.ly = sc_short(rvy); }
    if (s->bf_side) {
        /* a frame just off the middle (where the game's gesture starts), then out to the end and held */
        float y = now - s->bf_t < 20.0 ? -0.15f * s->bf_side : 0.97f * s->bf_side;
        /* keep the trigger turn (spins) but inside the game's hold range (45 degrees either side) */
        float sx = sc_axis(out.lx); if (sx > 0.6f) sx = 0.6f; if (sx < -0.6f) sx = -0.6f;
        out.lx = sc_short(sx); out.ly = sc_short(y);
    }
    if (now < s->bump_until) {
        int neutral=now<s->bump_neutral_until;
        out.lx = neutral?0:sc_short((float)s->bump_side); out.ly = neutral?0:sc_short((float)s->bump_y);
        /* The held foot/board stick must not keep rail lock engaged while
         * the movement stick requests an explicit clicked exit. */
        out.rx=out.ry=0;
    }
    else if (rv_active == 2) { out.lx = 0; out.ly = 0; }

    unsigned char glt = 0, grt = 0;
    if (c->grab_mode >= 1 && c->grab_double_ms > 0) {
        /* v0.20c bumpers:
         *   press and hold (ground or air)  = grab with that hand (after GrabHoldMs)
         *   quick tap in the air            = dark catch / flip: a short bumper press
         *   double-tap and hold on the ground = handplant (riding up to a lip): bumper held
         *   (in the air every tap is a dark catch, double taps included) */
        static const unsigned short bit[2] = {SC_LSHOULDER, SC_RSHOULDER};
        double hold = c->grab_hold_ms > 0 ? c->grab_hold_ms : 100.0;
        for (int i = 0; i < 2; ++i) {
            int down = (in.buttons & bit[i]) != 0;
            out.buttons &= (unsigned short)~bit[i];
            if (down && !s->g_down[i]) {
                int dbl = s->g_rel[i] > 0 && now - s->g_rel[i] <= c->grab_double_ms && s->g_rel[i] - s->g_press[i] <= c->grab_double_ms;
                /* v3.7 (your stock controls): darkslides / dark catches = hold LB / RB as you FLICK the trick.
                 * So a bumper pressed on the ground is the real bumper, held for as long as you hold it
                 * (dark catch / darkslide, and handplants at a lip). Pressed in the air: held = grab,
                 * tap = dark catch. Double-tap and hold in the air also stays the real bumper. */
                int ground = !s->air && s->takeoff != 1;
                /* v5.24 ROLLING GRAB: pressed while just rolling (no foot crouched / flicking) it is the
                 * grab (state 4, follows below); pressed with a foot down it is the real bumper (dark
                 * catch / darkslide), and a double tap is still the real bumper (handplants) */
                int footy = s->d_front_on || s->d_back_on || s->game_crouch == 1 || s->phase != SC_IDLE;
                s->g_grab[i] = dbl ? 1 : ground ? (footy || s->grind ? 1 : 4) : 3;
                s->g_press[i] = now;
            } else if (!down && s->g_down[i]) {
                if (s->g_grab[i] == 3 && s->air) s->g_pulse[i] = now + 80.0;  /* air tap: dark catch */
                s->g_rel[i] = (s->g_grab[i] == 1 || s->g_grab[i] == 2 || s->g_grab[i] == 4) ? 0 : now;
                s->g_grab[i] = 0;
            }
            s->g_down[i] = down;
            if (down && s->g_grab[i] == 3 && now - s->g_press[i] >= hold) s->g_grab[i] = 2;
            if (down && s->g_grab[i] == 4) {   /* rolling grab: until a foot goes down or you take off */
                int ground = !s->air && s->takeoff != 1;
                int footy = s->d_front_on || s->d_back_on || s->game_crouch == 1 || s->phase != SC_IDLE;
                if (!ground) s->g_grab[i] = 2;            /* carried into the air: an air grab */
                else if (footy) s->g_grab[i] = 1;          /* crouching to flick: the real bumper (dark catch) */
                else { if (i == 0) glt = 255; else grt = 255; }
            }
            if (down && s->g_grab[i] == 1) out.buttons |= bit[i];
            if (down && s->g_grab[i] == 2) { if (i == 0) glt = 255; else grt = 255; }
            if (now < s->g_pulse[i]) out.buttons |= bit[i];
        }
    } else if (c->grab_mode >= 1) {
        if (in.buttons & SC_LSHOULDER) glt = 255;
        if (in.buttons & SC_RSHOULDER) grt = 255;
        out.buttons &= (unsigned short)~(SC_LSHOULDER | SC_RSHOULDER);
    }
    if (c->grab_mode == 2) {
        if (in.lt >= c->grab_threshold) glt = 255;
        if (in.rt >= c->grab_threshold) grt = 255;
    }
    /* v3.5: your stock flips are a grab too: front flip = left stick up + RT (right hand),
     * backflip = left stick down + LT, the grab held from the pop for about 0.45 s */
    /* v3.6: just a tap of the grab, from the moment you leave the ground (hardly visible) */
    if (s->bf_side && s->bf_air_t > 0 && now - s->bf_air_t < c->bf_grab_ms) { if (s->bf_side > 0) grt = 255; else glt = 255; }
    out.lt = glt;
    out.rt = grt;
    out=sc_pump_output(s,out,now);
    s->bump_output_x=sc_axis(out.lx);s->bump_output_y=sc_axis(out.ly);s->bump_output_time=now;
    return out;
}

#endif
