/* v0.19r: every move run with the SAME settings the DLL uses by default (named fields, so adding a
 * setting can never silently shift them), so a change for one feature cannot quietly break another. */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "../src/translate.h"
static sc_config D(void) {
    sc_config c; memset(&c, 0, sizeof c);
    c.enabled = 1; c.grab_mode = 1; c.grab_threshold = 250; c.deadzone = 0.15f; c.steer_scale = 1.0f; c.steer_curve = 1.5f;
    c.active = 0.30f; c.point_ms = 16; c.pop_release_ollie = 1; c.shuv_degrees = 50; c.shuv360_degrees = 120; c.lean_ms = 120;
    c.ms_per_unit = 35; c.min_path_ms = 60; c.hold_end_ms = 50; c.direct = 1; c.low_pop_ms = 80; c.combo_wait_ms = 180;
    c.recenter_ms = 34; c.scoop_mirror = 0; c.windup_deg = 60; c.quick_pops = 1; c.air_controls = 1; c.grab_double_ms = 300;
    c.powerslide = 1; c.ps_delay_ms = 120; c.revert_ms = 0; c.revert_sweep_ms = 150; c.revert_shape = 1; c.switch_flip = 0; c.prewind = 1; c.prewind_ms = 60; c.grab_hold_ms = 100;
    c.bump_ms = 250;
    return c;
}
typedef struct { double t; float fx, fy, bx, by; } key;
static FILE* out;
static void run(const char* label, const char* expect, key* k, int n, int switched, double air_from, double air_to) {
    sc_config c = D(); sc_state st; memset(&st, 0, sizeof st);
    fprintf(out, "{\"move\":\"%s\",\"expect\":\"%s\",\"path\":[", label, expect);
    int first = 1;
    for (double t = 0; t <= k[n - 1].t + 250; t += 8.333) {
        int i = 0; while (i < n - 1 && k[i + 1].t < t) ++i;
        key a = k[i], b = k[i < n - 1 ? i + 1 : i];
        double u = b.t > a.t ? (t - a.t) / (b.t - a.t) : 1; if (u > 1) u = 1;
        float fx = a.fx + (b.fx - a.fx) * u, fy = a.fy + (b.fy - a.fy) * u, bx = a.bx + (b.bx - a.bx) * u, by = a.by + (b.by - a.by) * u;
        float mb = sqrtf(bx * bx + by * by); if (mb > 1) { bx /= mb; by /= mb; }
        float mf = sqrtf(fx * fx + fy * fy); if (mf > 1) { fx /= mf; fy /= mf; }
        if (t == 0) st.sw_prev = switched;
        st.switched = switched; st.air = t >= air_from && t < air_to;
        sc_pad in = {0, 0, 0, sc_short(fx), sc_short(fy), sc_short(bx), sc_short(by)};
        sc_pad o = sc_translate(&c, &st, in, t);
        float x = sc_axis(o.rx), y = sc_axis(o.ry);
        if (switched) y = -y;   /* compare in board terms */
        fprintf(out, "%s[%.3f,%.3f]", first ? "" : ",", x, -y); first = 0;
    }
    fprintf(out, "],\"fired\":\"%s\"}\n", st.fired && st.last_name ? st.last_name : "");
}
#define N(a) (int)(sizeof(a) / sizeof(a[0]))
int main(void) {
    out = stdout;
    for (int sw = 0; sw <= 0; ++sw) {
        char L[64];
        key ollie[] = {{0,0,0,0,0},{50,0,0,0,-1},{250,0,0,0,-1},{280,0,1,0,-1},{340,0,1,0,-1},{380,0,0,0,0}};
        snprintf(L, 64, "ollie%s", sw ? " (switch)" : ""); run(L, "ollie", ollie, N(ollie), sw, 1e9, 1e9);
        key nollie[] = {{0,0,0,0,0},{50,0,1,0,0},{250,0,1,0,0},{280,0,1,0,-1},{340,0,1,0,-1},{380,0,0,0,0}};
        snprintf(L, 64, "nollie%s", sw ? " (switch)" : ""); run(L, "nollie", nollie, N(nollie), sw, 1e9, 1e9);
        key kf[] = {{0,0,0,0,0},{50,0,0,0,-1},{250,0,0,0,-1},{290,0.8f,0.6f,0,-1},{350,0.8f,0.6f,0,-1},{390,0,0,0,0}};
        snprintf(L, 64, "kickflip%s", sw ? " (switch)" : ""); run(L, "kickflip|heelflip", kf, N(kf), sw, 1e9, 1e9);
        /* laser, stock style: back foot starts right, sweeps into down, front flicks up-left */
        key lw[] = {{0,0,0,0,0},{40,0,0,1,0},{70,0,0,1,0},{90,0,0,0.92f,-0.38f},{110,0,0,0.7f,-0.7f},{130,0,0,0.38f,-0.92f},{150,0,0,0,-1},
                    {180,-0.7f,0.7f,0,-1},{230,-0.7f,0.7f,0,-1},{270,0,0,0,0}};
        snprintf(L, 64, "laser wind-up%s", sw ? " (switch)" : ""); run(L, "laserflip", lw, N(lw), sw, 1e9, 1e9);
        /* laser, scoop style: pop, scoop round 160 deg, flick heel */
        key ls[] = {{0,0,0,0,0},{50,0,0,0,-1},{150,0,0,0,-1},{170,0,0,-0.5f,-0.87f},{190,0,0,-0.87f,-0.5f},{210,0,0,-1,0},{230,0,0,-0.87f,0.5f},{250,0,0,-0.5f,0.87f},
                    {280,-0.8f,0.6f,-0.5f,0.87f},{330,-0.8f,0.6f,-0.5f,0.87f},{380,0,0,0,0}};
        snprintf(L, 64, "laser scoop%s", sw ? " (switch)" : ""); run(L, "laserflip|360flip|360hardflip|360inwardheelflip", ls, N(ls), sw, 1e9, 1e9);
        key qo[] = {{0,0,0,0,0},{40,0,0,0,-1},{120,0,0,0,-1},{150,0,0,0,1},{200,0,0,0,1},{240,0,0,0,0}};  /* v0.26: back stick alone down then up */
        snprintf(L, 64, "quick ollie%s", sw ? " (switch)" : ""); run(L, "quickollie", qo, N(qo), sw, 1e9, 1e9);
        /* air with back foot held as a manual through the landing, then an ollie */
        key mo[] = {{0,0,0,0,0},{50,0,0,0,-0.55f},{900,0,0,0,-0.55f},{950,0,0,0,-1},{1150,0,0,0,-1},{1180,0,1,0,-1},{1240,0,1,0,-1},{1280,0,0,0,0}};
        snprintf(L, 64, "manual landing then ollie%s", sw ? " (switch)" : ""); run(L, "ollie", mo, N(mo), sw, 100, 400);
    }
    return 0;
}
