/* v2.1 diagnostics: why the game feels sluggish at the start of a session.
 *  - frame hitches: the game polls the pad once a frame, so a long gap between polls is a frame
 *    that took that long (logged when 100 ms or more, plus a 10 s summary)
 *  - tricks the game did not take: a trick the mod sent, with no crouch / pop in the next 0.6 s
 *  - shader caches: size and age of the game's and the graphics driver's shader caches, at start
 *    and every 5 minutes, so we can see whether they build up or get wiped between launches
 */
#ifndef SC_DIAG_H
#define SC_DIAG_H

typedef struct { UINT lo, hi; } SC_FT;
typedef struct {
    UINT attr; SC_FT created, accessed, written; UINT size_hi, size_lo, r0, r1;
    WCHAR name[260]; WCHAR alt[14];
} SC_FIND;
__declspec(dllimport) UINT WINAPI GetEnvironmentVariableW(const WCHAR*, WCHAR*, UINT);
__declspec(dllimport) HANDLE WINAPI FindFirstFileW(const WCHAR*, SC_FIND*);
__declspec(dllimport) BOOL WINAPI FindNextFileW(HANDLE, SC_FIND*);
__declspec(dllimport) BOOL WINAPI FindClose(HANDLE);
__declspec(dllimport) void WINAPI GetSystemTimeAsFileTime(SC_FT*);

static void dg_wcpy(WCHAR* d, int cap, const WCHAR* a, const WCHAR* b) {
    int n = 0;
    while (a && *a && n < cap - 1) d[n++] = *a++;
    while (b && *b && n < cap - 1) d[n++] = *b++;
    d[n] = 0;
}
typedef struct { UINT64 files, bytes, newest; } dg_tot;
static UINT64 dg_ft(SC_FT f) { return ((UINT64)f.hi << 32) | f.lo; }
static void dg_scan(const WCHAR* dir, int depth, dg_tot* t) {
    WCHAR pat[600]; dg_wcpy(pat, 600, dir, L"\\*");
    SC_FIND fd; HANDLE h = FindFirstFileW(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.name[0] == '.' && (fd.name[1] == 0 || (fd.name[1] == '.' && fd.name[2] == 0))) continue;
        if (fd.attr & 0x10) {
            if (depth < 3) { WCHAR sub[600]; dg_wcpy(sub, 600, dir, L"\\"); dg_wcpy(sub, 600, sub, fd.name); dg_scan(sub, depth + 1, t); }
        } else {
            t->files++; t->bytes += ((UINT64)fd.size_hi << 32) | fd.size_lo;
            UINT64 w = dg_ft(fd.written); if (w > t->newest) t->newest = w;
        }
    } while (FindNextFileW(h, &fd) && t->files < 200000);
    FindClose(h);
}
static void dg_report(const char* label, const WCHAR* dir) {
    dg_tot t = {0, 0, 0}; dg_scan(dir, 0, &t);
    char line[300]; unsigned n = 0;
    #define DPUT(str) do { const char* q_ = (str); while (*q_ && n < 290) line[n++] = *q_++; } while (0)
    #define DNUM(v) do { UINT64 iv_ = (UINT64)(v); char t_[24]; int k_ = 0; do { t_[k_++] = (char)('0' + iv_ % 10); iv_ /= 10; } while (iv_ && k_ < 20); while (k_ && n < 290) line[n++] = t_[--k_]; } while (0)
    DPUT("  shader cache "); DPUT(label); DPUT(": ");
    if (!t.files) DPUT("empty or not found");
    else {
        DNUM(t.files); DPUT(" files, "); DNUM(t.bytes / (1024 * 1024)); DPUT(" MB, newest written ");
        SC_FT now; GetSystemTimeAsFileTime(&now);
        UINT64 nf = dg_ft(now), mins = nf > t.newest ? (nf - t.newest) / 600000000ull : 0;
        if (mins < 2) DPUT("just now"); else if (mins < 120) { DNUM(mins); DPUT(" min ago"); }
        else if (mins < 2880) { DNUM(mins / 60); DPUT(" h ago"); } else { DNUM(mins / 1440); DPUT(" days ago"); }
    }
    line[n] = 0; log_text(line);
}
static DWORD WINAPI dg_thread(void* p) {
    (void)p;
    WCHAR la[300] = {0}; GetEnvironmentVariableW(L"LOCALAPPDATA", la, 300);
    WCHAR d[600];
    for (int round = 0; ; ++round) {
        Sleep(round ? 300000 : 8000);
        log_text(round ? "shader caches (5 min check):" : "shader caches at start:");
        dg_wcpy(d, 600, g_dir, L"shader_cache"); dg_report("game folder", d);
        if (la[0]) {
            dg_wcpy(d, 600, la, L"\\AMD\\DxcCache"); dg_report("AMD DX12", d);
            dg_wcpy(d, 600, la, L"\\AMD\\DxCache"); dg_report("AMD DX11", d);
            dg_wcpy(d, 600, la, L"\\D3DSCache"); dg_report("Windows D3D", d);
            dg_wcpy(d, 600, la, L"\\NVIDIA\\DXCache"); dg_report("NVIDIA", d);
        }
    }
    return 0;
}
static void dg_start(void) {
    static int started; if (started) return; started = 1;
    DWORD id; HANDLE th = CreateThread(0, 0, dg_thread, 0, 0, &id);
    if (th) CloseHandle(th);
}

/* frame timing, called once per pad poll */
static void dg_frame(double now_ms, int state) {
    static double last, win_start, worst; static unsigned frames, h50, logged; static double t0;
    if (last <= 0) { last = now_ms; win_start = now_ms; t0 = now_ms; return; }
    double dt = now_ms - last; last = now_ms;
    if (dt > 5000) { win_start = now_ms; frames = 0; h50 = 0; worst = 0; return; }   /* paused / alt-tab */
    frames++; if (dt > worst) worst = dt; if (dt >= 50) h50++;
    if (dt >= 100 && logged < 400) {
        logged++;
        char line[160]; unsigned n = 0;
        DPUT("  HITCH: one frame took "); DNUM((UINT64)dt); DPUT(" ms (game state "); DNUM(state); DPUT(", ");
        DNUM((UINT64)((now_ms - t0) / 1000.0)); DPUT(" s after the pad was first read)");
        line[n] = 0; log_text(line);
    }
    if (now_ms - win_start >= 10000.0) {
        char line[200]; unsigned n = 0;
        double secs = (now_ms - win_start) / 1000.0;
        DPUT("  frames last 10 s: "); DNUM((UINT64)(frames / secs + 0.5)); DPUT(" fps, worst "); DNUM((UINT64)worst);
        DPUT(" ms, "); DNUM(h50); DPUT(" frames over 50 ms");
        line[n] = 0; log_text(line);
        win_start = now_ms; frames = 0; h50 = 0; worst = 0;
    }
}

/* v2.4: the last frames of pad input / output, dumped when the game ignores a trick */
typedef struct { double t; short rlx, rly, rrx, rry, olx, oly, orx, ory; unsigned char rlt, rrt, olt, ort; unsigned short ob; int st; } dg_frame_rec;
static dg_frame_rec dg_ring[1024]; static unsigned dg_ring_n;
static void dg_record(double t, sc_pad raw, sc_pad out, int state) {
    dg_frame_rec* r = &dg_ring[dg_ring_n++ % 1024];
    r->t = t; r->rlx = raw.lx; r->rly = raw.ly; r->rrx = raw.rx; r->rry = raw.ry; r->rlt = raw.lt; r->rrt = raw.rt;
    r->olx = out.lx; r->oly = out.ly; r->orx = out.rx; r->ory = out.ry; r->olt = out.lt; r->ort = out.rt; r->ob = out.buttons; r->st = state;
}
static void dg_dump(double from_t) {
    log_text("    frames around it (ms from the trick, game state, YOUR sticks L R + triggers  ->  what the GAME got):");
    unsigned n = dg_ring_n < 1024 ? dg_ring_n : 1024, k0 = dg_ring_n - n; double lastp = -1e9;
    for (unsigned k = k0; k < dg_ring_n; ++k) {
        dg_frame_rec* r = &dg_ring[k % 1024];
        if (r->t < from_t || r->t - lastp < 15.0) continue;
        lastp = r->t;
        char line[220]; unsigned n2 = 0;
        #define RPUT(str) do { const char* q_ = (str); while (*q_ && n2 < 210) line[n2++] = *q_++; } while (0)
        #define RNUM(v) do { int iv_ = (int)(v); char t_[12]; int k_ = 0; if (iv_ < 0) { line[n2++] = '-'; iv_ = -iv_; } \
            do { t_[k_++] = (char)('0' + iv_ % 10); iv_ /= 10; } while (iv_ && k_ < 11); while (k_ && n2 < 210) line[n2++] = t_[--k_]; } while (0)
        #define RAX(v) RNUM(sc_axis(v) * 100.0f)
        RPUT("    "); RNUM(r->t - from_t - 900.0); RPUT(" ["); RNUM(r->st); RPUT("] L("); RAX(r->rlx); RPUT(","); RAX(r->rly); RPUT(") R(");
        RAX(r->rrx); RPUT(","); RAX(r->rry); RPUT(") T"); RNUM(r->rlt); RPUT("/"); RNUM(r->rrt);
        RPUT("  ->  L("); RAX(r->olx); RPUT(","); RAX(r->oly); RPUT(") R("); RAX(r->orx); RPUT(","); RAX(r->ory); RPUT(") T");
        RNUM(r->olt); RPUT("/"); RNUM(r->ort); if (r->ob) { RPUT(" btn "); RNUM(r->ob); }
        line[n2] = 0; log_text(line);
    }
}

/* v2.9: stick health, logged every 30 s: drift at rest, how far each stick reaches in each
 * direction, and how much of the time it is held part-way (40-90 %) rather than full or centred */
typedef struct { double frames, rest, part, full; float rest_max, reach[8]; } dg_stick;
static dg_stick dg_st[2];
static void dg_stick_add(dg_stick* d, short ix, short iy) {
    float x = sc_axis(ix), y = sc_axis(iy), m = sc_sqrt(x * x + y * y);
    d->frames++;
    if (m < 0.25f) { d->rest++; if (m > d->rest_max) d->rest_max = m; }
    else if (m < 0.9f) { if (m >= 0.4f) d->part++; }
    else d->full++;
    if (m >= 0.4f) {
        float a = sc_angle(x, y);   /* 0 = down, 90 = right, 180 = up, 270 = left */
        int k = (int)((a + 22.5f) / 45.0f) & 7;
        if (m > d->reach[k]) d->reach[k] = m;
    }
}
static void dg_stick_log(const char* nm, dg_stick* d) {
    char line[260]; unsigned n = 0;
    #define SPUTX(str) do { const char* q_ = (str); while (*q_ && n < 250) line[n++] = *q_++; } while (0)
    #define SNUMX(v) do { int iv_ = (int)(v); char t_[12]; int k_ = 0; if (iv_ < 0) { line[n++] = '-'; iv_ = -iv_; } \
        do { t_[k_++] = (char)('0' + iv_ % 10); iv_ /= 10; } while (iv_ && k_ < 11); while (k_ && n < 250) line[n++] = t_[--k_]; } while (0)
    SPUTX("  stick "); SPUTX(nm); SPUTX(": drift at rest up to "); SNUMX(d->rest_max * 100); SPUTX("%, reach");
    static const char* const lab[8] = {" D", " DR", " R", " UR", " U", " UL", " L", " DL"};
    for (int k = 0; k < 8; ++k) { SPUTX(lab[k]); SPUTX("="); SNUMX(d->reach[k] * 100); }
    double moved = d->frames - d->rest;
    SPUTX("  | of the time pushed: full "); SNUMX(moved > 0 ? d->full * 100 / moved : 0); SPUTX("%, part-way (40-90%) ");
    SNUMX(moved > 0 ? d->part * 100 / moved : 0); SPUTX("%");
    line[n] = 0; log_text(line);
}
static void dg_sticks(double now, sc_pad raw) {
    static double last;
    dg_stick_add(&dg_st[0], raw.lx, raw.ly); dg_stick_add(&dg_st[1], raw.rx, raw.ry);
    if (last <= 0) last = now;
    if (now - last >= 30000.0) {
        last = now;
        dg_stick_log("L (raw from the pad)", &dg_st[0]); dg_stick_log("R (raw from the pad)", &dg_st[1]);
        dg_stick z = {0}; dg_st[0] = z; dg_st[1] = z;
    }
}

/* tricks the game did not take */
static void dg_trick(double now_ms, const char* name, int fired_now, int state_now, int got_air_or_crouch) {
    static double t; static const char* nm; static double worst, last; static int watching;
    if (watching) {
        double dt = now_ms - last; if (dt > worst && dt < 5000) worst = dt;
        if (got_air_or_crouch) watching = 0;
        else if (now_ms - t > 600.0) {
            watching = 0;
            char line[200]; unsigned n = 0;
            DPUT("  trick NOT taken by the game: "); DPUT(nm ? nm : "?"); DPUT(" (no crouch / pop within 0.6 s; slowest frame meanwhile ");
            DNUM((UINT64)worst); DPUT(" ms)");
            line[n] = 0; log_text(line);
            { static int dumps; if (dumps++ < 12) dg_dump(t - 900.0); }
        }
    }
    last = now_ms;
    if (fired_now && state_now == 100) { watching = 1; t = now_ms; nm = name; worst = 0; }
}
#endif
