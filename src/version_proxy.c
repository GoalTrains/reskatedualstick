/* Session Controls for reSkate v0.2: a version.dll proxy.
 * Built freestanding (no CRT): clang --target=x86_64-pc-windows-msvc + lld-link.
 *
 * Skate.exe imports version.dll from its own folder, so this file loads into the game at
 * startup, before reSkate. It forwards every version.dll call to the real one in System32.
 * Inside Skate.exe only, it redirects the game's GetProcAddress import: when the game looks up
 * XInputGetState / XInputGetStateEx it gets a wrapper that calls the real function (including
 * reSkate's hook on it) and then applies the Session translation. No game code is patched.
 */
#include "translate.h"
#include "tricks_all.h"
#include "stance_track.h"
#include "motion_stance.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef int BOOL;
typedef unsigned int UINT;
typedef unsigned short WCHAR;
typedef void* HANDLE;
typedef void* HMODULE;
typedef long long INT64;
typedef unsigned long long UINT64;
typedef void* FARPROC;
#define WINAPI __stdcall
#define DLLEXPORT __declspec(dllexport)
#define ERROR_SUCCESS 0
#define ERROR_DEVICE_NOT_CONNECTED 1167
#define INVALID_HANDLE_VALUE ((HANDLE)(INT64)-1)
#define PAGE_READWRITE 0x04

typedef struct { DWORD dwPacketNumber; sc_pad Gamepad; } XINPUT_STATE;

__declspec(dllimport) HMODULE WINAPI LoadLibraryW(const WCHAR*);
__declspec(dllimport) FARPROC WINAPI GetProcAddress(HMODULE, const char*);
__declspec(dllimport) HMODULE WINAPI GetModuleHandleW(const WCHAR*);
__declspec(dllimport) DWORD WINAPI GetSystemDirectoryW(WCHAR*, DWORD);
__declspec(dllimport) DWORD WINAPI GetModuleFileNameW(HMODULE, WCHAR*, DWORD);
__declspec(dllimport) HANDLE WINAPI CreateFileW(const WCHAR*, DWORD, DWORD, void*, DWORD, DWORD, HANDLE);
__declspec(dllimport) BOOL WINAPI WriteFile(HANDLE, const void*, DWORD, DWORD*, void*);
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE);
__declspec(dllimport) DWORD WINAPI GetPrivateProfileIntW(const WCHAR*, const WCHAR*, int, const WCHAR*);
__declspec(dllimport) DWORD WINAPI GetPrivateProfileStringW(const WCHAR*, const WCHAR*, const WCHAR*, WCHAR*, DWORD, const WCHAR*);
__declspec(dllimport) BOOL WINAPI WritePrivateProfileStringW(const WCHAR*, const WCHAR*, const WCHAR*, const WCHAR*);
__declspec(dllimport) DWORD WINAPI GetPrivateProfileSectionW(const WCHAR*, WCHAR*, DWORD, const WCHAR*);
__declspec(dllimport) BOOL WINAPI WritePrivateProfileSectionW(const WCHAR*, const WCHAR*, const WCHAR*);
__declspec(dllimport) BOOL WINAPI DisableThreadLibraryCalls(HMODULE);
__declspec(dllimport) UINT64 WINAPI GetTickCount64(void);
__declspec(dllimport) DWORD WINAPI GetFileAttributesW(const WCHAR*);
__declspec(dllimport) BOOL WINAPI VirtualProtect(void*, UINT64, DWORD, DWORD*);
__declspec(dllimport) BOOL WINAPI QueryPerformanceCounter(INT64*);
__declspec(dllimport) void* WINAPI VirtualAlloc(void*, UINT64, DWORD, DWORD);
__declspec(dllimport) BOOL WINAPI FlushInstructionCache(HANDLE, const void*, UINT64);
__declspec(dllimport) HANDLE WINAPI GetCurrentProcess(void);
__declspec(dllimport) BOOL WINAPI QueryPerformanceFrequency(INT64*);

int _fltused = 0;
void* memset(void* d, int v, unsigned long long n) { BYTE* p = (BYTE*)d; while (n--) *p++ = (BYTE)v; return d; }
void* memcpy(void* d, const void* s, unsigned long long n) { BYTE* p = (BYTE*)d; const BYTE* q = (const BYTE*)s; while (n--) *p++ = *q++; return d; }

/* ---------------------------------------------------------------- strings */
static unsigned wlen(const WCHAR* s) { unsigned n = 0; while (s[n]) ++n; return n; }
static void wcat(WCHAR* d, unsigned cap, const WCHAR* s) {
    unsigned n = wlen(d), i = 0;
    while (s[i] && n + 1 < cap) d[n++] = s[i++];
    d[n] = 0;
}
static const WCHAR* wbase(const WCHAR* p) { const WCHAR* b = p; for (; *p; ++p) if (*p == '\\' || *p == '/') b = p + 1; return b; }
static WCHAR wlow(WCHAR c) { return (c >= 'A' && c <= 'Z') ? (WCHAR)(c + 32) : c; }
static int wieq(const WCHAR* a, const char* b) { while (*a && *b) { if (wlow(*a) != (WCHAR)*b) return 0; ++a; ++b; } return *a == 0 && *b == 0; }
static int wistarts(const WCHAR* a, const char* b) { while (*b) { if (!*a || wlow(*a) != (WCHAR)*b) return 0; ++a; ++b; } return 1; }
static int streq(const char* a, const char* b) { while (*a && *a == *b) { ++a; ++b; } return *a == *b; }

/* ---------------------------------------------------------------- state */
static HMODULE g_self;
static WCHAR g_dir[520], g_ini[560], g_log[560];
static sc_config g_cfg;
static sc_state g_state[4];
static WORD g_board_button;
static int g_debug;
static int g_precise_log = 1;
static int g_ramp_diag;
static int g_remount_diag;
static int g_motion_stance;
static volatile int g_ramp_diag_reset;
static int g_board_detect;
static int g_scoop_logged, g_mode_logged, g_stock_log = 1, g_ov_enabled = 1;
static volatile int g_game_goofy = -1;   /* v3.9: the game's own stance setting (-1 = unknown) */
static int g_auto_stance = 1, g_ini_goofy;
static int g_test_on;
static WORD g_test_prev;
static WCHAR g_test_trick[40];
static int g_test_items[64], g_test_count, g_test_next, g_test_last = -1;
static void test_build_list(void);
static void load_recordings(void);
static WCHAR g_rec_trick[40];
static int g_user_enabled = 1;     /* L3+R3 master switch */
static int g_toggle_ms = 3000;     /* v5.2: how long L3+R3 must be held to switch it */
static int g_switch_src = 4;       /* direct stance flag; safe default without an INI */
static int g_ps_enabled = 1;       /* PlayStationPads */
static volatile int g_ui_scale = 100; /* MenuScale, percent */
static WCHAR g_menu_key_name[24] = {'E','n','d',0};
static int g_show_startup_menu = 1;
static double g_rv_watch_t; static int g_rv_watch_b = -1, g_rv_shape, g_rv_ms; /* did that revert turn you? */
static int g_rv_calib, g_rv_calib_i;
static const int k_rv_shapes[2] = {5, 6}; static const int k_rv_speeds[4] = {120, 170, 230, 300};
static int read_switch_b(void);
static int g_takeoff = -1;         /* v1.6: in the air right now (game state 2xx), -1 unknown */
static int g_paused;               /* v0.26c: the game is paused (skater not ticking) */
static int g_menu;                 /* v0.19: a menu is open (Start / Back): standard controls */
static int g_menu_depth;
static int g_state_log;            /* StateLog=1: log every skater state change (for tuning) */
static int g_air_ms = 120;         /* AirDelayMs: state must be 'air' this long before air controls */
static WORD g_prev_buttons[4];
static UINT64 g_last_ini_check;
static int g_active;     /* inside Skate.exe with the import redirected */
static int g_seen_state; /* logged the first translated poll */

typedef FARPROC (WINAPI* PFN_GetProcAddress)(HMODULE, const char*);
typedef DWORD (WINAPI* PFN_GetState)(DWORD, XINPUT_STATE*);
static PFN_GetProcAddress g_real_gpa;
static PFN_GetState g_real_getstate, g_real_getstateex;

/* ---------------------------------------------------------------- log */
static void log_text(const char* s) {
    if (!g_log[0]) return;
    HANDLE h = CreateFileW(g_log, 4 /*FILE_APPEND_DATA*/, 1 | 2, 0, 4 /*OPEN_ALWAYS*/, 0x80, 0);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD w, n = 0; while (s[n]) ++n;
    WriteFile(h, s, n, &w, 0);
    WriteFile(h, "\r\n", 2, &w, 0);
    CloseHandle(h);
}
static void log_wide(const char* prefix, const WCHAR* w) {
    char buf[700]; unsigned n = 0;
    while (*prefix && n < 300) buf[n++] = *prefix++;
    while (*w && n < 690) { buf[n++] = (*w < 128) ? (char)*w : '?'; ++w; }
    buf[n] = 0;
    log_text(buf);
}
static void log_num(const char* prefix, UINT64 v) {
    char buf[200]; unsigned n = 0; char tmp[24]; int t = 0;
    while (*prefix && n < 170) buf[n++] = *prefix++;
    do { tmp[t++] = (char)('0' + v % 10); v /= 10; } while (v && t < 20);
    while (t) buf[n++] = tmp[--t];
    buf[n] = 0;
    log_text(buf);
}

/* ---------------------------------------------------------------- config */
static int ini_int(const WCHAR* key, int def) { return (int)GetPrivateProfileIntW(L"SessionControls", key, def, g_ini); }
static float ini_percent(const WCHAR* key, int def) { return (float)ini_int(key, def) / 100.0f; }
static void load_config(int keep_runtime) {
    int enabled = g_cfg.enabled, swapped = g_cfg.feet_swapped;
    WCHAR stance[16] = {0};
    GetPrivateProfileStringW(L"SessionControls", L"Stance", L"regular", stance, 16, g_ini);
    g_ini_goofy = (stance[0] == 'g' || stance[0] == 'G');
    /* v3.9: AutoStance=1 (default): follow the stance set in the game (DefaultStanceIsGoofy) */
    g_auto_stance = ini_int(L"AutoStance", 1) != 0;
    {
        int was = g_cfg.goofy;
        g_cfg.goofy = (g_auto_stance && g_game_goofy >= 0) ? g_game_goofy : g_ini_goofy;
        if (keep_runtime && was != g_cfg.goofy) log_text(g_cfg.goofy ? "stance: goofy" : "stance: regular");
    }
    g_cfg.flip_x = ini_int(L"MirrorFlicks", 0) != 0;   /* v5.12: ini only (was FlipSideways, a menu row) */
    g_cfg.grab_mode = ini_int(L"GrabMode", 1);
    g_cfg.grab_threshold = ini_int(L"GrabTriggerThreshold", 250);
    g_cfg.grab_double_ms = (float)ini_int(L"GrabDoubleTapMs", 300);
    g_cfg.grab_hold_ms = (float)ini_int(L"GrabHoldMs", 100);
    g_cfg.powerslide = ini_int(L"Powerslides", 1) != 0;
    g_cfg.ps_delay_ms = 0; /* legacy PowerslideDelayMs is ignored: immediate response */
    g_cfg.revert_ms = (float)ini_int(L"RevertDoubleTapMs", 300);
    if (g_cfg.revert_ms <= 0) g_cfg.revert_ms = 300;
    /* v5.7: RevertInput = 0 off, 1 double tap, 2 squeeze both triggers (default) */
    g_cfg.revert_input = ini_int(L"RevertInput", 2);
    if (g_cfg.revert_input < 0 || g_cfg.revert_input > 2) g_cfg.revert_input = 2;
    g_cfg.ps_flip = ini_int(L"PowerslideFlip", 0) != 0;
    g_cfg.revert_flip = ini_int(L"RevertFlip", 0) != 0;
    g_cfg.revert_sweep_ms = (float)ini_int(L"RevertSpinMs", 300);
    g_cfg.revert_gap_ms = (float)ini_int(L"RevertGapMs", 0);
    g_cfg.revert_min_gap_ms = (float)ini_int(L"RevertMinGapMs", 0);
    g_cfg.revert_shape = ini_int(L"RevertShape", 5);
    { int was = g_rv_calib; g_rv_calib = ini_int(L"RevertCalibrate", 0) != 0; if (g_rv_calib && !was) { g_rv_calib_i = 0; log_text("revert calibration ON: each double tap tries the next shape / speed"); }
      if (g_rv_calib) { g_cfg.revert_shape = k_rv_shapes[g_rv_calib_i / 4]; g_cfg.revert_sweep_ms = (float)k_rv_speeds[g_rv_calib_i % 4]; } }
    /* v0.32: switch stance ON by default. New key, so old ini files (SwitchFlip=0) get it too. */
    g_cfg.switch_flip = ini_int(L"SwitchStance", 1);
    if (g_cfg.switch_flip < 0 || g_cfg.switch_flip > 3) g_cfg.switch_flip = 1;
    g_cfg.prewind = ini_int(L"SpinWindUp", 1);
    g_cfg.body_flips = ini_int(L"BodyFlips", 1) != 0;
    g_cfg.stick_steer = ini_int(L"StickSteering", 0) != 0;   /* v5.15 */
    g_cfg.front_back = ini_int(L"FrontBackFoot", 0) != 0;     /* v5.26 */
    /* Front/back always keeps its jobs in switch. Preserve the stored key
     * so switching back to left/right restores that layout's switch choice. */
    if (g_cfg.front_back) g_cfg.switch_flip = 1;
    g_cfg.one_foot_flips = ini_int(L"OneFootFlips", 0) != 0;  /* v5.27 */
    g_toggle_ms = ini_int(L"ToggleHoldMs", 3000); if (g_toggle_ms < 400) g_toggle_ms = 400;
    g_cfg.switch_goofy_flip = ini_int(L"SwitchFlipGoofy", 0) != 0;
    g_cfg.bf_grab_ms = (float)ini_int(L"BodyFlipGrabMs", 80);
    g_stock_log = ini_int(L"StockInputLog", 1) != 0;
    g_ov_enabled = ini_int(L"MenuInGame", 1) != 0;
    g_cfg.bump_ms = (float)ini_int(L"GrindBumpMs", 250);
    { int v = ini_int(L"PlayStationPads", 1); g_ps_enabled = v <= 0 ? 0 : v >= 2 ? 2 : 1; }   /* v5.23: 2 = NATIVE */
    { int sc = ini_int(L"MenuScale", 100); if (sc < 50) sc = 50; if (sc > 300) sc = 300; g_ui_scale = sc; }
    GetPrivateProfileStringW(L"SessionControls", L"MenuKey", L"End", g_menu_key_name, 24, g_ini);
    g_show_startup_menu = ini_int(L"ShowMenuOnStartup", 1) != 0;
    g_cfg.prewind_ms = (float)ini_int(L"SpinWindUpMs", 60);
    g_switch_src = ini_int(L"SwitchSource", 4);
    g_cfg.deadzone = ini_percent(L"FootDeadzonePercent", 15);
    g_cfg.steer_scale = ini_percent(L"SteerStrengthPercent", 100);
    g_cfg.steer_curve = ini_percent(L"SteerCurvePercent", 150);
    if (g_cfg.deadzone < 0 || g_cfg.deadzone > 0.9f) g_cfg.deadzone = 0.15f;
    if (g_cfg.steer_curve < 1.0f) g_cfg.steer_curve = 1.0f;
    if (g_cfg.steer_curve > 3.0f) g_cfg.steer_curve = 3.0f;
    g_cfg.active = ini_percent(L"PopFlickPercent", 30);
    if (g_cfg.active < 0.1f || g_cfg.active > 0.9f) g_cfg.active = 0.30f;
    g_cfg.point_ms = (float)ini_int(L"TrickPointMs", 16);
    if (g_cfg.point_ms < 4 || g_cfg.point_ms > 100) g_cfg.point_ms = 16;
    g_cfg.pop_release_ollie = ini_int(L"PopReleaseOllie", 1) != 0;
    g_cfg.lean_ms = (float)ini_int(L"ManualLeanMs", 120);
    g_cfg.walk_ms = 0; /* v0.6 auto-walk removed: it fired while riding */
    g_cfg.ms_per_unit = (float)ini_int(L"TrickDrawMsPerUnit", 28);
    g_cfg.min_path_ms = (float)ini_int(L"TrickDrawMinMs", 60);
    g_cfg.hold_end_ms = (float)ini_int(L"TrickHoldEndMs", 50);
    {
        /* v5.12: PURE only. TrickInput / TrickMode are no longer read (the old ASSISTED mode is gone). */
        g_cfg.direct = 0; g_cfg.pure = 1;
        if (!g_mode_logged) { log_text("trick input: PURE (sticks straight through, nothing invented)"); g_mode_logged = 1; }
    }
    g_board_detect = ini_int(L"BoardDetect", 1) != 0;
    g_cfg.low_pop_ms = (float)ini_int(L"LowPopMsPerUnit", 80);
    g_cfg.combo_wait_ms = (float)ini_int(L"ComboWaitMs", 130);
    g_cfg.recenter_ms = (float)ini_int(L"RecenterMs", 34);
    {
        int was = g_cfg.scoop_mirror;
        /* v0.29: the old MirrorScoops=1 default had backside / frontside the wrong way round (tre flips
         * came out as 360 hardflips, varial heels as inward heels), so it is replaced by a new key
         * whose default is the fixed way round. Old ini files keep working (MirrorScoops is ignored). */
        g_cfg.scoop_mirror = ini_int(L"SwapShoveIts", 0) != 0;
        if (was != g_cfg.scoop_mirror || !g_scoop_logged) { log_text(g_cfg.scoop_mirror ? "shove-it sides: SWAPPED (SwapShoveIts=1)" : "shove-it sides: normal (SwapShoveIts=0)"); g_scoop_logged = 1; }
    }
    GetPrivateProfileStringW(L"SessionControls", L"RecordTrick", L"laserflip", g_rec_trick, 40, g_ini);
    load_recordings();
    {
        int was = g_test_on;
        WCHAR tt[40] = {0};
        g_test_on = 0;   /* v5.12: the ASSISTED test mode is gone */
        GetPrivateProfileStringW(L"SessionControls", L"TestTrick", L"laserflip", tt, 40, g_ini);
        int changed = 0;
        for (int i = 0; i < 40; ++i) { if (tt[i] != g_test_trick[i]) changed = 1; g_test_trick[i] = tt[i]; if (!tt[i]) break; }
        if (changed) { g_test_next = 0; g_test_last = -1; }
        test_build_list();
        if (g_test_on && (!was || changed)) { log_wide("test mode ON for ", g_test_trick); log_num("  shapes to try: ", (UINT64)g_test_count); }
    }
    g_debug = ini_int(L"InputLog", 1) != 0;
    g_precise_log = ini_int(L"PreciseInputLog", 1) != 0;
    g_cfg.shuv_degrees = (float)ini_int(L"ShuvScoopDegrees", 50);
    g_cfg.shuv360_degrees = (float)ini_int(L"Shuv360ScoopDegrees", 150);
    g_cfg.windup_deg = (float)ini_int(L"WindUpDegrees", 60);
    g_cfg.quick_pops = ini_int(L"QuickPops", 1) != 0;
    g_cfg.pop_on_release = ini_int(L"PopOnRelease", 0) != 0;
    g_cfg.landing_manuals = ini_int(L"LandingManuals", 2);
    g_cfg.manual_max = ini_percent(L"ManualMaxPercent", 85);
    g_cfg.stock_grab_air = ini_int(L"StockGrabAir", 0) != 0;
    g_cfg.stock_transition_ground = ini_int(L"StockTransitionGround", 0) != 0;
    g_remount_diag = ini_int(L"RemountDiagnostics", 0) != 0;
    g_motion_stance = ini_int(L"RemountStanceMotion", 1) != 0;
    g_ramp_diag = ini_int(L"RampDiagnostics", 0); if(g_ramp_diag<0 || g_ramp_diag>2)g_ramp_diag=0;
    g_cfg.air_controls = ini_int(L"AirControls", 1) != 0;
    g_cfg.grind_sensitivity = ini_percent(L"GrindSensitivityPercent", 120);
    if (g_cfg.grind_sensitivity < 0.5f) g_cfg.grind_sensitivity = 0.5f;
    if (g_cfg.grind_sensitivity > 2.0f) g_cfg.grind_sensitivity = 2.0f;
    g_air_ms = ini_int(L"AirDelayMs", 120);
    g_state_log = ini_int(L"StateLog", 1) != 0;
    if (g_cfg.shuv_degrees < 20) g_cfg.shuv_degrees = 20;
    if (g_cfg.shuv360_degrees <= g_cfg.shuv_degrees) g_cfg.shuv360_degrees = g_cfg.shuv_degrees + 60;
    WCHAR btn[16] = {0};
    GetPrivateProfileStringW(L"SessionControls", L"BoardButton", L"Y", btn, 16, g_ini);
    static const struct { const char* name; WORD bit; } names[] = {
        {"a", 0x1000}, {"b", 0x2000}, {"x", 0x4000}, {"y", 0x8000}, {"back", 0x0020}, {"view", 0x0020},
        {"dpadup", 0x0001}, {"dpaddown", 0x0002}, {"dpadleft", 0x0004}, {"dpadright", 0x0008},
        {"cross", 0x1000}, {"circle", 0x2000}, {"square", 0x4000}, {"triangle", 0x8000}};
    g_board_button = 0;
    for (unsigned i = 0; i < sizeof names / sizeof names[0]; ++i) if (wieq(btn, names[i].name)) g_board_button = names[i].bit;
    if (keep_runtime) { g_cfg.enabled = enabled; g_cfg.feet_swapped = swapped; }
    else { g_cfg.enabled = ini_int(L"EnabledAtStart", 1) != 0; g_cfg.feet_swapped = 0; g_user_enabled = g_cfg.enabled; }
}


/* ---------------------------------------------------------------- on-board detection
 * reSkate's No Bail hooks the game's physics-state selector, uint32(selector, current), at
 * RVA 0x4745a70 of this game build. Its result 504 means off the board (walking) and 300 a
 * wipeout. We chain in front of reSkate's hook (never touching its checks): follow the jump
 * reSkate placed at the function entry, and replace it with a jump to us that calls reSkate's
 * hook and records what state each skater's selector chose. */
#define SEL_RVA 0x4745a70u
#define CALLSITE_RVA 0x47da1cfu
static const BYTE k_callsite[32] = {
    0x48,0x8b,0x8b,0x40,0x04,0x00,0x00,0x8b,0xd0,0x8b,0xf0,0xe8,0x91,0xb8,0xf6,0xff,
    0x8b,0xf8,0xb8,0xf8,0x01,0x00,0x00,0x81,0xff,0x2c,0x01,0x00,0x00,0x0f,0x44,0xf8};
#define STATE_OFFBOARD 504u
#define STATE_WIPEOUT 300u
#define STATE_RIDE 700u   /* really the spawn / teleport flicker */

typedef DWORD (*PFN_Select)(UINT64, DWORD);
static PFN_Select g_sel_prev;
static int g_sel_status;          /* 0 = not tried, 1 = waiting for reSkate, 2 = installed, 3 = gave up */
static int g_sel_tries;
static UINT64 g_sel_next_try;
static double g_probe_ms;          /* last time we expect the local skater's state to change */
typedef struct { UINT64 sel; volatile DWORD state; volatile double seen, changed, react_t; volatile float react; volatile int score; volatile unsigned calls; } sel_slot;
static sel_slot g_slots[16];
static double g_now_ms;            /* updated every poll; read by the hook */
static int g_local = -1;           /* slot of the local skater, once known */
static double g_spawn_t;           /* when we last followed a (re)spawn */
static int g_walking;              /* game says the local skater is off the board */

/* State changes are queued here by the hook (game thread) and written to the log by process(). */
typedef struct { int slot; DWORD from, to; double t; } st_event;
static st_event g_ev[64];
static volatile unsigned g_ev_w, g_ev_r;
static double react_now(const sel_slot* sl) { /* reaction score, halving every 20 s */
    double dt = g_now_ms - sl->react_t, v = sl->react;
    while (dt > 20000.0 && v > 0.01) { v *= 0.5; dt -= 20000.0; }
    return v * (1.0 - dt / 40000.0);
}
static DWORD sel_hook(UINT64 selector, DWORD current) {
    DWORD r = g_sel_prev(selector, current);
    int free_slot = -1;
    for (int i = 0; i < 16; ++i) {
        if (g_slots[i].sel == selector) {
            if (g_slots[i].state != r) {
                unsigned w = g_ev_w;
                if (w - g_ev_r < 64) { st_event* e = &g_ev[w % 64]; e->slot = i; e->from = g_slots[i].state; e->to = r; e->t = g_now_ms; g_ev_w = w + 1; }
                g_slots[i].state = r;
                g_slots[i].changed = g_now_ms;
                if (g_now_ms - g_probe_ms < 900.0) {
                    g_slots[i].score++;
                    g_slots[i].react = (float)(react_now(&g_slots[i]) + 1.0);
                    g_slots[i].react_t = g_now_ms;
                }
            }
            g_slots[i].seen = g_now_ms;
            g_slots[i].calls++;
            return r;
        }
        if (free_slot < 0 && !g_slots[i].sel) free_slot = i;
    }
    if (free_slot < 0) { /* full: reuse the slot quiet the longest */
        double oldest = 1e300;
        for (int i = 0; i < 16; ++i) if (g_slots[i].seen < oldest) { oldest = g_slots[i].seen; free_slot = i; }
        if (free_slot == g_local) return r;
    }
    sel_slot z = {0};
    g_slots[free_slot] = z;
    g_slots[free_slot].state = r; g_slots[free_slot].seen = g_now_ms; g_slots[free_slot].changed = g_now_ms;
    g_slots[free_slot].react_t = g_now_ms;
    if (g_now_ms - g_probe_ms < 900.0) { g_slots[free_slot].react = 1.0f; g_slots[free_slot].score = 1; }
    g_slots[free_slot].sel = selector;
    {
        unsigned w = g_ev_w;
        if (w - g_ev_r < 64) { st_event* e = &g_ev[w % 64]; e->slot = free_slot; e->from = 0xFFFFFFFFu; e->to = r; e->t = g_now_ms; g_ev_w = w + 1; }
    }
    return r;
}

static int bytes_equal(const BYTE* a, const BYTE* b, unsigned n) { while (n--) if (*a++ != *b++) return 0; return 1; }

typedef struct { void* base; void* alloc; DWORD aprot; WORD part; WORD pad0; UINT64 size; DWORD state, prot, type, pad1; } SC_MBI;
__declspec(dllimport) UINT64 __stdcall VirtualQuery(const void*, SC_MBI*, UINT64);
/* v0.22b: find free memory within 2 GB of the game code by walking the address space
 * (probing every 64 KB with VirtualAlloc failed on one launch: "no memory near the game code") */
static BYTE* alloc_near(UINT64 target) {
    UINT64 lo = target > 0x7FF00000ull ? target - 0x7FF00000ull : 0x10000, hi = target + 0x7FF00000ull;
    BYTE* best = 0; UINT64 best_d = ~0ull;
    UINT64 a = lo;
    while (a < hi) {
        SC_MBI m;
        if (!VirtualQuery((const void*)a, &m, sizeof m)) break;
        UINT64 base = (UINT64)m.base, end = base + m.size;
        if (m.state == 0x10000 /*MEM_FREE*/) {
            UINT64 c = (base + 0xFFFF) & ~0xFFFFull;              /* allocation granularity */
            UINT64 cands[2] = {c, (end - 0x10000) & ~0xFFFFull};  /* both ends of the free block */
            for (int k = 0; k < 2; ++k) {
                UINT64 p = cands[k];
                if (p < base || p + 0x1000 > end || p < lo || p + 0x1000 > hi) continue;
                UINT64 d = p > target ? p - target : target - p;
                if (d < best_d) { best_d = d; best = (BYTE*)p; }
            }
        }
        if (end <= a) break;
        a = end;
    }
    return best ? (BYTE*)VirtualAlloc(best, 0x1000, 0x3000, 0x40) : 0;
}
static int readable(UINT64 a, UINT64 n);
/* v1.2: a run of int3 (0xCC) padding inside the game's executable sections, within 1.5 GB of
 * `near`. Only the middle of a run of 48+ bytes is used, so it can never be the end of a function. */
static BYTE* code_cave(BYTE* base, BYTE* near) {
    if (!readable((UINT64)base, 0x400) || base[0] != 'M' || base[1] != 'Z') return 0;
    BYTE* nt = base + *(int*)(base + 0x3C);
    if (!readable((UINT64)nt, 0x108) || nt[0] != 'P' || nt[1] != 'E') return 0;
    WORD nsec = *(WORD*)(nt + 6), optsz = *(WORD*)(nt + 20);
    BYTE* sec = nt + 24 + optsz;
    for (int i = 0; i < nsec && i < 96; ++i, sec += 40) {
        UINT chars = *(UINT*)(sec + 36), vsz = *(UINT*)(sec + 8), va = *(UINT*)(sec + 12);
        if (!(chars & 0x20000000u /*MEM_EXECUTE*/) || vsz < 64) continue;
        BYTE* p = base + va; BYTE* end = p + vsz;
        int run = 0;
        for (BYTE* q = p; q < end; ++q) {
            if (((UINT64)q & 0xFFF) == 0 && !readable((UINT64)q, 0x1000)) { q += 0xFFF; run = 0; continue; }
            if (*q == 0xCC) {
                if (++run == 20) {
                    BYTE* c = q - 19 + 3;    /* v4.5: 20-byte runs (48 was too rare on some builds): 3 in, 14 used, 3 spare */
                    INT64 d = (INT64)c - (INT64)near;
                    if (d < 0x60000000LL && d > -0x60000000LL) return c;
                }
            } else run = 0;
        }
    }
    return 0;
}
static void try_install_selector_hook(UINT64 now) {
    if (g_sel_status >= 2 || now < g_sel_next_try) return;
    g_sel_next_try = now + 1000;
    BYTE* base = (BYTE*)GetModuleHandleW(0);
    if (!bytes_equal(base + CALLSITE_RVA, k_callsite, 32)) {
        g_sel_status = 3; log_text("board detection off: game build does not match (using Y / L3+R3)"); return;
    }
    BYTE* entry = base + SEL_RVA;
    UINT64 prev = 0;
    if (entry[0] == 0xE9) prev = (UINT64)(entry + 5) + (INT64)*(int*)(entry + 1);
    else if (entry[0] == 0xFF && entry[1] == 0x25) prev = *(UINT64*)(entry + 6 + *(int*)(entry + 2));
    else if (entry[0] == 0x40 && entry[1] == 0x53) {
        if (g_sel_status == 0) log_text("board detection: waiting for reSkate's hook...");
        g_sel_status = 1;
        if (++g_sel_tries > 120) { g_sel_status = 3; log_text("board detection off: reSkate's No Bail hook never appeared (using Y / L3+R3)"); }
        return;
    } else { g_sel_status = 3; log_text("board detection off: unexpected code at the selector (using Y / L3+R3)"); return; }

    /* v4.5: reSkate's hook is an absolute jump through a pointer (FF 25): just point that pointer at
     * us - no memory near the game code is needed (a player's log: "no memory near the game code",
     * so walking / riding fell back to the Y button and got out of step on every respawn). */
    if (entry[0] == 0xFF && entry[1] == 0x25) {
        UINT64* slot = (UINT64*)(entry + 6 + *(int*)(entry + 2));
        DWORD op;
        if (readable((UINT64)slot, 8) && VirtualProtect(slot, 8, 0x40, &op)) {
            g_sel_prev = (PFN_Select)prev;
            __atomic_exchange_n((volatile UINT64*)slot, (UINT64)sel_hook, __ATOMIC_SEQ_CST);
            VirtualProtect(slot, 8, op, &op);
            g_sel_status = 2;
            log_text("board detection: on (chained onto reSkate's hook pointer)");
            return;
        }
    }
    /* A stub within 2 GB of the entry, so a 5-byte jump can reach it. */
    BYTE* stub = 0;
    UINT64 e = (UINT64)entry & ~(UINT64)0xFFFF;
    for (UINT64 off = 0x10000; off < 0x4000000 && !stub; off += 0x10000) {   /* quick try close by */
        stub = (BYTE*)VirtualAlloc((void*)(e - off), 0x1000, 0x3000, 0x40);
        if (!stub) stub = (BYTE*)VirtualAlloc((void*)(e + off), 0x1000, 0x3000, 0x40);
    }
    if (!stub) stub = alloc_near((UINT64)entry);
    if (!stub) {
        /* v1.2: no free memory could be had near the game code on some launches (board detection,
         * switch stance and the automatic walking / riding switch all went with it). Fall back to
         * a spare gap of int3 padding between two functions inside the game's own code. */
        stub = code_cave(base, entry);
        if (stub) log_text("board detection: using a padding gap in the game code for the jump");
    }
    if (!stub) { g_sel_status = 3; log_text("board detection off: no memory near the game code"); return; }
    INT64 rel = (INT64)stub - (INT64)(entry + 5);
    if (rel > 0x7FFFFFF0LL || rel < -0x7FFFFFF0LL) { g_sel_status = 3; log_text("board detection off: stub out of range"); return; }
    {
        DWORD sp = 0; int is_cave = stub >= base && stub < base + 0x40000000ull;
        if (is_cave && !VirtualProtect(stub, 14, 0x40, &sp)) { g_sel_status = 3; log_text("board detection off: cannot write the padding gap"); return; }
        stub[0] = 0xFF; stub[1] = 0x25; stub[2] = stub[3] = stub[4] = stub[5] = 0;
        *(UINT64*)(stub + 6) = (UINT64)sel_hook;
        if (is_cave) { VirtualProtect(stub, 14, sp, &sp); FlushInstructionCache(GetCurrentProcess(), stub, 14); }
    }
    g_sel_prev = (PFN_Select)prev;

    UINT64 old8 = *(volatile UINT64*)entry, new8 = old8;
    BYTE* nb = (BYTE*)&new8;
    nb[0] = 0xE9; *(int*)(nb + 1) = (int)rel;
    DWORD oldp;
    if (!VirtualProtect(entry, 8, 0x40, &oldp)) { g_sel_status = 3; log_text("board detection off: cannot write the jump"); return; }
    __atomic_exchange_n((volatile UINT64*)entry, new8, __ATOMIC_SEQ_CST); /* entry is 16-byte aligned */
    VirtualProtect(entry, 8, oldp, &oldp);
    FlushInstructionCache(GetCurrentProcess(), entry, 8);
    g_sel_status = 2;
    log_text("board detection: on (chained onto reSkate's physics-state hook)");
}

/* Which slot is the local skater: the only one ticking, or the one whose state keeps
 * changing right after our tricks / Y presses. */
/* v0.19: the local skater is the slot that keeps reacting to YOUR inputs (tricks, A/B/Y, Start,
 * Back), scored with a 20 s half-life, and we stick with it unless another slot clearly reacts
 * more. v0.18 switched to "the only one ticking" whenever your skater went quiet for 5 s, which
 * often picked someone else (controls going back to stock mid-session). Respawning makes a new
 * slot; it gets picked as soon as it reacts to you, or straight away if the old one has gone
 * silent after a bail. */
static void update_local(void) {
    int active = 0, only = -1, best = -1; double best_r = 0.5;
    for (int i = 0; i < 16; ++i) {
        if (!g_slots[i].sel || g_now_ms - g_slots[i].seen > 30000.0) continue;
        ++active; only = i;
        double r = react_now(&g_slots[i]);
        if (r > best_r) { best_r = r; best = i; }
    }
    int pick = -1; const char* why = 0;
    if (g_local < 0) {
        if (best >= 0) { pick = best; why = "board detection: found your skater by its reactions, state "; }
        else if (active == 1) { pick = only; why = "board detection: found your skater (only one ticking), state "; }
    } else {
        double mine = react_now(&g_slots[g_local]);
        double quiet = g_now_ms - g_slots[g_local].seen;
        if (best >= 0 && best != g_local && best_r > mine + 1.5 && g_slots[best].changed > g_spawn_t + 500.0) { pick = best; why = "board detection: switched to the skater reacting to you, state "; }
        else if (g_slots[g_local].state == STATE_WIPEOUT || g_slots[g_local].state == STATE_OFFBOARD) {
            /* v0.19b: respawning swaps between two skater slots (your log: the old one parks in 504,
             * the other flickers 700 and starts riding). So if ours is parked off the board and
             * another one has since started riding (not 504 / 300 / the 700 spawn flicker), that is you. */
            int nb = -1; double nt = g_slots[g_local].changed;
            for (int i = 0; i < 16; ++i) {
                DWORD st = g_slots[i].state;
                if (i == g_local || !g_slots[i].sel || st == STATE_OFFBOARD || st == STATE_WIPEOUT || st == STATE_RIDE) continue;
                if (g_slots[i].changed > nt && g_now_ms - g_slots[i].seen < 3000.0) { nt = g_slots[i].changed; nb = i; }
            }
            if (nb >= 0) { pick = nb; why = "board detection: respawned, following your skater, state "; }
        }
        if (quiet > 120000.0) { g_local = -1; g_walking = 0; } /* level change etc. */
    }
    if (pick >= 0 && pick != g_local) {
        g_local = pick;
        char buf[160]; unsigned n = 0; const char* q = why; while (*q && n < 120) buf[n++] = *q++;
        buf[n] = 0; log_num(buf, g_slots[pick].state);
    }
}
static int g_air;                  /* the game says airborne / grinding (state not ride, walk, bail) */
static double g_air_since;



/* ---------------------------------------------------------------- test mode (v0.15)
 * TestMode=1 in the ini. Hold LB and press D-pad UP: the mod performs the next shape the game
 * defines for TestTrick (each from the crouch, then from the centre) and logs which one it was,
 * so you can note what the game calls it. D-pad DOWN repeats the last one, LEFT/RIGHT step through
 * without performing. LB does not grab while test mode is on. */
static int g_test_running; static double g_test_t0; static int g_test_item;
static sc_state g_test_state;

static int wide_eq_ascii_ci(const WCHAR* w, const char* a) {
    while (*w && *a) { WCHAR c = wlow(*w); char d = *a; if (d >= 'A' && d <= 'Z') d += 32; if (c != (WCHAR)d) return 0; ++w; ++a; }
    return *w == 0 && *a == 0;
}
static void test_build_list(void) {
    g_test_count = 0;
    for (int i = 0; i < SC_VARIANT_COUNT && g_test_count < 62; ++i)
        if (wide_eq_ascii_ci(g_test_trick, sc_variants[i].name)) {
            g_test_items[g_test_count++] = i * 2;      /* from the crouch */
            g_test_items[g_test_count++] = i * 2 + 1;  /* from the centre */
        }
    if (g_test_next >= g_test_count) g_test_next = 0;
}
static void test_log_item(const char* what, int k) {
    char line[200]; unsigned n = 0;
    #define TP(str) do { const char* q_ = (str); while (*q_ && n < 190) line[n++] = *q_++; } while (0)
    #define TN(v) do { int iv_ = (int)(v); char t_[12]; int j_ = 0; do { t_[j_++] = (char)('0' + iv_ % 10); iv_ /= 10; } while (iv_ && j_ < 11); while (j_ && n < 190) line[n++] = t_[--j_]; } while (0)
    int item = g_test_items[k]; const sc_variant* v = &sc_variants[item / 2];
    TP(what); TP(" "); TN(k + 1); TP("/"); TN(g_test_count); TP(": "); TP(v->name); TP(" "); TP(v->desc);
    TP((item & 1) ? "  [from centre]" : "  [from crouch]");
    line[n] = 0; log_text(line);
}
static void test_controls(WORD b, WORD prev, double now) {
    if (!g_test_on || !(b & SC_LSHOULDER) || g_test_count == 0) return;
    WORD pressed = b & (WORD)~prev;
    if (pressed & 0x0001 && !g_test_running) {            /* D-pad up: perform next */
        g_test_item = g_test_next; g_test_last = g_test_next;
        g_test_next = (g_test_next + 1) % g_test_count;
        g_test_running = 1; g_test_t0 = now;
        test_log_item("TEST", g_test_item);
    } else if (pressed & 0x0002 && !g_test_running && g_test_last >= 0) { /* down: repeat last */
        g_test_item = g_test_last; g_test_running = 1; g_test_t0 = now;
        test_log_item("TEST (again)", g_test_item);
    } else if (pressed & 0x0008) { g_test_next = (g_test_next + 1) % g_test_count; test_log_item("next will be", g_test_next); }
    else if (pressed & 0x0004) { g_test_next = (g_test_next + g_test_count - 1) % g_test_count; test_log_item("next will be", g_test_next); }
}
/* Returns 1 and sets the flick stick while a test is running: crouch 300 ms, (centre 34 ms), draw. */
static int test_output(double now, float* x, float* y) {
    if (!g_test_running) return 0;
    int item = g_test_items[g_test_item]; const sc_variant* v = &sc_variants[item / 2];
    int centre = item & 1;
    double t = now - g_test_t0;
    if (g_test_running == 1) {
        if (t < 300) { *x = 0; *y = -1; return 1; }
        double draw_at = 300 + (centre ? 34 : 0);
        if (t < draw_at) { *x = 0; *y = 0; return 1; }
    }
    sc_state* ts = &g_test_state;
    if (g_test_running == 1) {                 /* first frame of the drawing */
        sc_state z = {0}; *ts = z;
        ts->path = v->xy; ts->path_n = v->count; ts->phase = SC_EMIT;
        ts->from_x = 0; ts->from_y = centre ? 0.0f : -1.0f; ts->seg = 1; ts->emit_start = now;
        g_test_running = 2;
    }
    if (g_test_running == 2) {
        sc_emit_step(&g_cfg, ts, now, x, y);
        if (ts->phase != SC_EMIT) { g_test_running = 3; g_test_t0 = now; *x = 0; *y = 0; }
        return 1;
    }
    /* finished: centre for 150 ms, then hand the stick back */
    *x = 0; *y = 0;
    if (now - g_test_t0 > 150) g_test_running = 0;
    return 1;
}


/* ---------------------------------------------------------------- recording (v0.16)
 * With Session controls OFF (L3+R3), do a trick the normal skate. way, then hold LB and tap D-pad
 * UP within ~3 s: the last real right-stick motion is saved to session_controls.ini under
 * [Recorded] as RecordTrick, and Session controls replay it for that trick from then on. */
typedef struct { double t; float x, y; } rec_sample;
static rec_sample g_ring[512];
static int g_ring_head, g_ring_count;

static void ring_push(double t, float x, float y) {
    g_ring[g_ring_head] = (rec_sample){t, x, y};
    g_ring_head = (g_ring_head + 1) & 511;
    if (g_ring_count < 512) ++g_ring_count;
}
static const rec_sample* ring_at(int i) { /* 0 = oldest */
    return &g_ring[(g_ring_head - g_ring_count + i + 1024) & 511];
}
static void ascii_to_wide(const char* a, WCHAR* w, int cap) { int i = 0; while (a[i] && i + 1 < cap) { w[i] = (WCHAR)a[i]; ++i; } w[i] = 0; }
static int put_num(WCHAR* out, int n, int cap, float v) { /* 3 decimals */
    if (n + 8 >= cap) return n;
    if (v < 0) { out[n++] = '-'; v = -v; }
    int whole = (int)v, frac = (int)((v - (float)whole) * 1000.0f + 0.5f);
    if (frac >= 1000) { whole++; frac -= 1000; }
    { char d[12]; int k = 0; do { d[k++] = (char)('0' + whole % 10); whole /= 10; } while (whole && k < 11); while (k && n < cap - 5) out[n++] = (WCHAR)d[--k]; }
    out[n++] = '.';
    out[n++] = (WCHAR)('0' + frac / 100); out[n++] = (WCHAR)('0' + (frac / 10) % 10); out[n++] = (WCHAR)('0' + frac % 10);
    return n;
}
static const char* dir_name(float x, float y) {
    static const char* const names[8] = {"down", "down-right", "right", "up-right", "up", "up-left", "left", "down-left"};
    if (sc_sqrt(x * x + y * y) < 0.3f) return "centre";
    float a = sc_angle(x, y) + 22.5f; if (a >= 360) a -= 360;
    return names[(int)(a / 45.0f) & 7];
}
static void describe_path(const float (*xy)[2], int n, char* out, unsigned cap) {
    unsigned k = 0; const char* last = 0;
    for (int i = 0; i < n; ++i) {
        const char* d = dir_name(xy[i][0], xy[i][1]);
        if (last && d == last) continue;
        if (last) { const char* sep = " > "; while (*sep && k + 1 < cap) out[k++] = *sep++; }
        while (*d && k + 1 < cap) out[k++] = *d++;
        last = dir_name(xy[i][0], xy[i][1]);
    }
    out[k] = 0;
}

/* Take the last real trick motion out of the ring and save it. */
static void record_now(double now) {
    int trick = -1;
    for (int i = 0; i < SC_TRICK_COUNT; ++i) if (wide_eq_ascii_ci(g_rec_trick, sc_tricks[i].name)) trick = i;
    if (trick < 0) { log_wide("record: unknown RecordTrick ", g_rec_trick); return; }
    /* find the last block where the stick was out (>= 0.2) with at least 1 unit of travel */
    int end = -1, start = -1;
    for (int i = g_ring_count - 1; i >= 0; --i) {
        const rec_sample* r = ring_at(i);
        if (now - r->t > 3000) break;
        float m = sc_sqrt(r->x * r->x + r->y * r->y);
        if (m >= 0.2f) {
            if (end < 0) end = i;
            start = i;
        } else if (end >= 0) {
            float travel = 0;
            for (int j = start + 1; j <= end; ++j) { float dx = ring_at(j)->x - ring_at(j - 1)->x, dy = ring_at(j)->y - ring_at(j - 1)->y; travel += sc_sqrt(dx * dx + dy * dy); }
            if (travel >= 1.0f) break;
            end = start = -1;   /* too small (a nudge): keep looking further back */
        }
    }
    if (end < 0 || start < 0) { log_text("record: no trick motion found in the last 3 s (do the trick, then LB + D-pad up)"); return; }
    /* compress: drop samples closer than 0.06 to the last kept one; time = movement time only */
    float kx[512], ky[512]; int kn = 0; double dur = 0, last_t = ring_at(start)->t;
    for (int i = start; i <= end; ++i) {
        const rec_sample* r = ring_at(i);
        if (kn == 0) { kx[kn] = r->x; ky[kn] = r->y; kn++; last_t = r->t; continue; }
        float dx = r->x - kx[kn - 1], dy = r->y - ky[kn - 1];
        if (sc_sqrt(dx * dx + dy * dy) >= 0.06f || i == end) {
            double dt = r->t - last_t; if (dt > 40) dt = 40; dur += dt; last_t = r->t;
            kx[kn] = r->x; ky[kn] = r->y; kn++;
        }
    }
    /* at most 24 points, evenly by distance travelled */
    float cum[512]; cum[0] = 0;
    for (int i = 1; i < kn; ++i) { float dx = kx[i] - kx[i - 1], dy = ky[i] - ky[i - 1]; cum[i] = cum[i - 1] + sc_sqrt(dx * dx + dy * dy); }
    int n = kn < 24 ? kn : 24;
    for (int k = 0; k < n; ++k) {
        float want = n > 1 ? cum[kn - 1] * k / (n - 1) : 0; int i = 0;
        while (i < kn - 1 && cum[i + 1] < want) ++i;
        float seg = i < kn - 1 ? cum[i + 1] - cum[i] : 0, u = seg > 0 ? (want - cum[i]) / seg : 0;
        sc_ovr_xy[trick][k][0] = i < kn - 1 ? kx[i] + (kx[i + 1] - kx[i]) * u : kx[i];
        sc_ovr_xy[trick][k][1] = i < kn - 1 ? ky[i] + (ky[i + 1] - ky[i]) * u : ky[i];
    }
    if (dur < 40) dur = 40; if (dur > 600) dur = 600;
    sc_ovr_n[trick] = n; sc_ovr_ms[trick] = (float)dur;
    /* save: "ms;x,y;x,y;..." */
    WCHAR val[1200]; int vn = 0;
    vn = put_num(val, vn, 1190, (float)dur);
    for (int k = 0; k < n; ++k) { val[vn++] = ';'; vn = put_num(val, vn, 1190, sc_ovr_xy[trick][k][0]); val[vn++] = ','; vn = put_num(val, vn, 1190, sc_ovr_xy[trick][k][1]); }
    val[vn] = 0;
    WCHAR key[40]; ascii_to_wide(sc_tricks[trick].name, key, 40);
    WritePrivateProfileStringW(L"Recorded", key, val, g_ini);
    char d[200]; describe_path((const float (*)[2])sc_ovr_xy[trick], n, d, sizeof d);
    char line[300] = "RECORDED "; unsigned ln = 9;
    for (const char* q = sc_tricks[trick].name; *q && ln < 250; ++q) line[ln++] = *q;
    const char* mid = ": "; while (*mid) line[ln++] = *mid++;
    for (const char* q = d; *q && ln < 295; ++q) line[ln++] = *q;
    line[ln] = 0; log_text(line);
    log_num("  points ", (UINT64)n); log_num("  ms ", (UINT64)dur);
}

static float parse_num(const WCHAR** p) {
    const WCHAR* q = *p; float sign = 1, v = 0, scale = 0;
    if (*q == '-') { sign = -1; ++q; }
    for (; *q; ++q) {
        if (*q >= '0' && *q <= '9') { if (scale > 0) { v += (float)(*q - '0') * scale; scale *= 0.1f; } else v = v * 10 + (float)(*q - '0'); }
        else if (*q == '.' && scale == 0) scale = 0.1f;
        else break;
    }
    *p = q; return sign * v;
}
/* Load every recorded trick from the ini. */
static void load_recordings(void) {
    int loaded = 0;
    for (int i = 0; i < SC_TRICK_COUNT; ++i) {
        WCHAR key[40], val[1200]; ascii_to_wide(sc_tricks[i].name, key, 40);
        val[0] = 0;
        GetPrivateProfileStringW(L"Recorded", key, L"", val, 1200, g_ini);
        if (!val[0]) { sc_ovr_n[i] = 0; continue; }
        const WCHAR* p = val; float ms = parse_num(&p); int n = 0;
        while (*p == ';' && n < 24) { ++p; float x = parse_num(&p); if (*p != ',') break; ++p; float y = parse_num(&p); sc_ovr_xy[i][n][0] = x; sc_ovr_xy[i][n][1] = y; ++n; }
        if (n >= 2) { sc_ovr_n[i] = n; sc_ovr_ms[i] = ms; ++loaded; } else sc_ovr_n[i] = 0;
    }
    static int last_loaded = -1;
    if (loaded != last_loaded) { log_num("recorded tricks in use: ", (UINT64)loaded); last_loaded = loaded; }
}

/* ---------------------------------------------------------------- translation */
/* L3+R3 together toggles Session mode; LB+L3+R3 swaps feet (for switch). */
static void handle_combos(DWORD user, XINPUT_STATE* s) {
    if (user > 3) return;
    WORD b = s->Gamepad.buttons, prev = g_prev_buttons[user];
    WORD both = SC_LTHUMB | SC_RTHUMB;
    static double both_since; static int both_done;
    if ((b & both) != both) { both_since = 0; both_done = 0; }
    else if (both_since <= 0) both_since = g_now_ms;
    /* any button you press is a chance to see which skater reacts (respawn, get on / off) */
    if ((b & ~prev) & (0x1000 | 0x2000 | 0x4000 | 0x8000 | 0x0010 | 0x0020)) g_probe_ms = g_now_ms;
    /* v0.19 menus: Start / Back open one; standard controls until it is closed again. A goes a
     * level deeper, B a level back; Start / Back always close. */
    {
        WORD down = (WORD)(b & ~prev);
        /* v0.30: Start / Back inside a menu opens a sub menu (it used to close our menu mode, so
         * the menus stopped working). Menu mode ends when B backs out of the first menu, or as
         * soon as you are skating again (crouch / air / grind / slide, see drain_state_events). */
        if (down & (0x0010 | 0x0020)) {
            if (!g_menu) { g_menu = 1; g_menu_depth = 1; log_text("menu opened (Start / Back): standard controls"); }
            else g_menu_depth++;
        } else if (g_menu && (down & 0x1000)) { g_menu_depth++; }
        else if (g_menu && (down & 0x2000)) {
            if (--g_menu_depth <= 0) { g_menu = 0; g_menu_depth = 0; log_text("menu closed (B): Dual Stick controls back"); }
        }
    }
    if ((b & both) == both && !both_done && g_now_ms - both_since >= 400.0 && (g_menu || (b & SC_LSHOULDER))) {
        both_done = 1; /* v0.19: L3+R3 must be HELD for 0.4 s (a quick double stick-click while skating toggled it) */
        if (g_menu) { g_menu = 0; g_menu_depth = 0; log_text("menu mode cleared (L3+R3): Dual Stick controls back"); }
        else {
            g_cfg.feet_swapped = !g_cfg.feet_swapped;
            log_text(g_cfg.feet_swapped ? "feet swapped (switch)" : "feet normal");
        }
    }
    /* v5.2: switching Dual Stick on / off needs L3+R3 HELD for ToggleHoldMs (3 s by default), so it
     * can't be hit by accident */
    if ((b & both) == both && !both_done && g_now_ms - both_since >= (double)g_toggle_ms) {
        both_done = 1;
        {
            g_user_enabled = !g_user_enabled;
            g_probe_ms = g_now_ms;
            log_text(g_user_enabled ? "Dual Stick mode: ON (L3+R3)" : "Dual Stick mode: OFF (L3+R3)");
        }
    }
    if (g_board_button && (b & g_board_button) && !(prev & g_board_button)) {
        g_probe_ms = g_now_ms;
        if (g_local < 0) { /* no board detection yet: fall back to toggling with the button */
            g_walking = !g_walking; /* the button still reaches the game */
            log_text(g_walking ? "board button: walking (Dual Stick controls off)" : "board button: riding (Dual Stick controls on)");
        }
    }
    g_prev_buttons[user] = b;
    if ((b & both) == both) s->Gamepad.buttons &= (WORD)~(both | SC_LSHOULDER);
}
static void drain_state_events(void) {
        while (g_ev_r != g_ev_w) {
            st_event e = g_ev[g_ev_r % 64];
            g_ev_r++;
            /* v0.19c: every (re)spawn shows up as the 700 flicker, on your board or on foot, often in
             * a different slot from before (the old one just goes quiet in whatever state it had).
             * So the slot that comes out of 700 is you. */
            if (e.from == STATE_RIDE && e.to != STATE_RIDE && e.slot != g_local) {
                g_local = e.slot; g_spawn_t = e.t;
                log_num("board detection: (re)spawn, following your skater, state ", e.to);
            }
            if (e.from == STATE_RIDE && e.to != STATE_RIDE) g_spawn_t = e.t;
            {
                DWORD st_now2 = g_local >= 0 ? g_slots[g_local].state : 0;
                int w2 = st_now2 == STATE_OFFBOARD || st_now2 == STATE_WIPEOUT;
                if (g_local == e.slot && w2 != g_walking) {
                    g_walking = w2;
                    log_text(w2 ? "game: off the board -> walking (Dual Stick controls off)" : "game: on the board -> Dual Stick controls on");
                }
            }
            if (g_menu && e.slot == g_local && e.from != 0xFFFFFFFFu && (e.to == STATE_WIPEOUT || e.from == STATE_WIPEOUT || e.from == STATE_OFFBOARD || e.to == STATE_OFFBOARD)) {
                g_menu = 0; g_menu_depth = 0; log_text("menu closed (your skater moved): Dual Stick controls back");
            }
            /* v0.30: crouching, popping, grinding or sliding can only happen with the menus shut */
            else if (g_menu && e.slot == g_local && e.from != 0xFFFFFFFFu && (e.to == 101 || e.to == 103 || e.to == 200 || e.to == 201 || (e.to >= 400 && e.to < 500))) {
                g_menu = 0; g_menu_depth = 0; log_text("menu closed (you are skating): Dual Stick controls back");
            }
            if (!g_state_log) continue;
            char line[160]; unsigned n = 0;
            #define SPUT(str) do { const char* q_ = (str); while (*q_ && n < 150) line[n++] = *q_++; } while (0)
            #define SNUM(v) do { UINT64 iv_ = (UINT64)(v); char t_[24]; int k_ = 0; \
                do { t_[k_++] = (char)('0' + iv_ % 10); iv_ /= 10; } while (iv_ && k_ < 20); while (k_ && n < 150) line[n++] = t_[--k_]; } while (0)
            SPUT("    state: slot "); SNUM(e.slot); SPUT(e.slot == g_local ? " (you) " : " ");
            if (e.from == 0xFFFFFFFFu) SPUT("new"); else SNUM(e.from);
            SPUT(" -> "); SNUM(e.to);
            SPUT("  at "); SNUM((UINT64)(e.t / 1000.0)); SPUT("s");
            line[n] = 0; log_text(line);
        }
}

/* ---------------------------------------------------------------- stance snapshots (v0.19h)
 * To find where the game keeps "riding switch": LB + D-pad LEFT while riding normally, LB + D-pad
 * RIGHT while riding switch. Each press writes the memory around your skater's physics-state object
 * (and what it points to) to sc_dump_R<n>.bin / sc_dump_S<n>.bin next to the game. Read-only. */
static int readable(UINT64 a, UINT64 n) {
    while (n) {
        SC_MBI m;
        if (a < 0x10000 || a > 0x7FFFFFFFFFFFull) return 0;
        if (!VirtualQuery((const void*)a, &m, sizeof m)) return 0;
        if (m.state != 0x1000 || (m.prot & 0x101) || !(m.prot & 0xEE)) return 0;
        UINT64 end = (UINT64)m.base + m.size;
        if (end >= a + n) return 1;
        n -= end - a; a = end;
    }
    return 1;
}
static void dump_block(HANDLE h, UINT64 a, DWORD n) {
    DWORD w;
    if (!readable(a, n)) return;
    WriteFile(h, &a, 8, &w, 0); WriteFile(h, &n, 4, &w, 0); WriteFile(h, (const void*)a, n, &w, 0);
}
static UINT64 g_dumped[4096]; static int g_ndumped;
static int seen_block(UINT64 a) {
    for (int i = 0; i < g_ndumped; ++i) if (g_dumped[i] == a) return 1;
    if (g_ndumped < 4096) g_dumped[g_ndumped++] = a;
    return 0;
}
static void dump_ptrs(HANDLE h, UINT64 from, UINT64 len, DWORD blk, int depth) {
    for (UINT64 p = from; p < from + len; p += 8) {
        if (!readable(p, 8)) { p = (p | 0xFFF) - 7; continue; }
        UINT64 v = *(volatile UINT64*)p;
        if ((v & 7) || v < 0x100000 || v > 0x7FFFFFFFFFFFull) continue;
        UINT64 a = v & ~(UINT64)0xFF;
        if (seen_block(a) || !readable(a, blk)) continue;
        dump_block(h, a, blk);
        if (depth > 0) dump_ptrs(h, a, 0x200, 0x800, depth - 1);
    }
}
static void stance_dump(int sw) {
    static int count[2];
    if (g_local < 0 || !g_slots[g_local].sel) { log_text("snapshot: no skater found yet"); return; }
    WCHAR path[600]; path[0] = 0; wcat(path, 600, g_dir); wcat(path, 600, sw ? L"sc_dump_S" : L"sc_dump_R");
    int k = ++count[sw]; WCHAR num[4] = {(WCHAR)('0' + k % 10), 0}; wcat(path, 600, num); wcat(path, 600, L".bin");
    HANDLE h = CreateFileW(path, 0x40000000, 1, 0, 2 /*CREATE_ALWAYS*/, 0x80, 0);
    if (h == INVALID_HANDLE_VALUE) { log_text("snapshot: cannot write the file"); return; }
    UINT64 sel = g_slots[g_local].sel;
    g_ndumped = 0;
    dump_block(h, sel - 0x1000, 0x3000);
    dump_ptrs(h, sel - 0x200, 0x1200, 0x1000, 1);   /* v0.19j: two levels of pointers */
    CloseHandle(h);
    log_num(sw ? "snapshot SWITCH saved, number " : "snapshot REGULAR saved, number ", (UINT64)k);
}
/* v0.19k: riding switch. Found with the snapshots: from your skater's physics-state object,
 * [[sel] + 0x60] + 0xA0 is 0 riding the way you spawned and 1 after a 180 (5 normal / 4 switch
 * snapshots, no exceptions). Read-only, checked before every read. */
static int g_switched = -1;
static int g_switch_probes[4] = {-1,-1,-1,-1};
static UINT64 rd_pointer(UINT64 address);
static int rd_copy(UINT64 address,void* out,UINT64 size);
static UINT64 rd8(UINT64 a) { return rd_pointer(a); }
static int rd1(UINT64 a) { BYTE v=0;return rd_copy(a,&v,1)?v:-1; }
/* Three places that all matched normal / switch in your snapshots; all logged, the first one used
 * unless SwitchSource says otherwise. */

static void read_switch_cands(int* c1, int* c2, int* c3) {
    *c1 = *c2 = *c3 = -1;
    if (g_local < 0 || !g_slots[g_local].sel) return;
    UINT64 sel = g_slots[g_local].sel, a = rd8(sel);
    if (a && !(a & 7)) {
        UINT64 b = rd8(a + 0x60); if (b && !(b & 3)) *c1 = rd1(b + 0xA0);
        UINT64 d = rd8(a + 0x78); if (d && !(d & 3)) *c3 = rd1(d + 0x350);
    }
    UINT64 p = rd8(sel + 0x10);
    if (p && !(p & 7)) { UINT64 q = rd8(p + 0x10); if (q && !(q & 7)) *c2 = rd1(q + 0x5e4); }
}
static UINT64 g_input_generation;
static void switch_snapshot(void);
static int read_switch_b(void) { switch_snapshot();return g_switch_probes[1]; }
/* Controlled regular/switch snapshots: the first three bytes at A0 remain
 * zero, while the fourth boolean (A3) changes 0 -> 1. Probe b and c later
 * reset on the ground. Keep this candidate separate and log it for testing. */
static int read_switch_d(void) {
    if (g_local < 0 || !g_slots[g_local].sel) return -1;
    UINT64 a = rd8(g_slots[g_local].sel);
    if (!a || (a & 7)) return -1;
    UINT64 p = rd8(a + 0x60);
    return p && !(p & 3) ? rd1(p + 0xA3) : -1;
}
static double g_rv_dump;   /* v4.9: dump the frames of a revert that did not turn */
static void revert_watch(void) {
    if (g_rv_watch_t <= 0) return;
    int b = read_switch_b();
    if (b >= 0 && g_rv_watch_b >= 0 && b != g_rv_watch_b) {
        char line[96]; unsigned n = 0; const char* q = "  revert WORKED (you turned round) with shape ";
        while (*q) line[n++] = *q++;
        line[n++] = (char)('0' + g_rv_shape % 10); line[n++] = ','; line[n++] = ' ';
        { int v = g_rv_ms; char t[8]; int k = 0; do { t[k++] = (char)('0' + v % 10); v /= 10; } while (v && k < 7); while (k) line[n++] = t[--k]; }
        q = " ms"; while (*q) line[n++] = *q++; line[n] = 0; log_text(line);
        g_rv_watch_t = 0;
    } else if (g_now_ms - g_rv_watch_t > 1500.0) { log_text("  revert: no turn"); g_rv_dump = g_rv_watch_t; g_rv_watch_t = 0; }
}
static void switch_snapshot(void) {
    static UINT64 generation=~0ull;static int slot=-2;
    if(generation==g_input_generation && slot==g_local)return;
    generation=g_input_generation;slot=g_local;
    read_switch_cands(&g_switch_probes[0],&g_switch_probes[1],&g_switch_probes[2]);
    g_switch_probes[3]=read_switch_d();
}
static int read_switched(void) {
    static int l1=-9,l2=-9,l3=-9,l4=-9;
    switch_snapshot();
    int c1=g_switch_probes[0],c2=g_switch_probes[1],c3=g_switch_probes[2],c4=g_switch_probes[3];
    if (c1 != l1 || c2 != l2 || c3 != l3 || c4 != l4) {
        char line[96] = "  switch probe: a="; unsigned n = 18;
        int v[4] = {c1, c2, c3, c4};
        for (int i = 0; i < 4; ++i) {
            if (i) { line[n++] = ' '; line[n++] = (char)('a' + i); line[n++] = '='; }
            if (v[i] < 0) { line[n++] = '?'; } else { int x = v[i]; if (x >= 100) line[n++] = (char)('0' + x / 100); if (x >= 10) line[n++] = (char)('0' + x / 10 % 10); line[n++] = (char)('0' + x % 10); }
        }
        line[n] = 0; if (g_state_log) log_text(line);
        l1 = c1; l2 = c2; l3 = c3; l4 = c4;
    }
    int v = g_switch_src == 4 ? c4 : g_switch_src == 2 ? c2 : g_switch_src == 3 ? c3 : c1;
    return v == 0 || v == 1 ? v : -1;
}
#include "overlay.h"
#include "ps_pad.h"
#include "diag.h"
#include "gstance.h"
#include "precise_trace.h"
#include "ramp_diag.h"
#include "overlay_handoff.h"

static int reskate_owns_input(double now) {
    typedef DWORD (WINAPI* Ownership)(void);
    static Ownership ownership;
    static double next_lookup;
    if (!ownership && now >= next_lookup) {
        next_lookup = now + 1000.0;
        HMODULE reskate = GetModuleHandleW(L"ReSkate.dll");
        if (reskate) ownership = (Ownership)GetProcAddress(reskate,"ReSkateOverlayOwnsInputV1");
    }
    return ownership ? ownership() != 0 : g_reskate_menu_visible;
}
static void process_inner(DWORD user, XINPUT_STATE* s, UINT64* timing) {
    ++g_input_generation;
    UINT64 now = GetTickCount64();
    {
        static INT64 f0;
        INT64 t0 = 0;
        if (!f0) QueryPerformanceFrequency(&f0);
        QueryPerformanceCounter(&t0);
        g_now_ms = f0 ? (double)t0 * 1000.0 / (double)f0 : (double)now;
    }
    dg_frame(g_now_ms, g_local >= 0 ? (int)g_slots[g_local].state : -1);
    if (g_board_detect) try_install_selector_hook(now);
    if (g_sel_status == 2) {
        update_local();
        if (g_local >= 0) {
            DWORD st_now = g_slots[g_local].state;
            int walking = st_now == STATE_OFFBOARD || st_now == STATE_WIPEOUT;
            if (walking != g_walking) {
                g_walking = walking;
                log_text(walking ? "game: off the board -> walking (Dual Stick controls off)" : "game: on the board -> Dual Stick controls on");
            }
            /* From your logs: 100 riding, 103 crouch, 200 / 201 in the air, 400-404 grinding, 504 off
             * the board, 300 bail, 700 a short flicker when spawning. Air controls = 200-299 only
             * (grinds use the normal pop / flick controls), capped at 6 s. */
            int air_state = st_now >= 200 && st_now < 300;
            g_takeoff = air_state;
            if (!air_state) { g_air_since = 0; if (g_air) { g_air = 0; if (g_state_log) log_text("    (air controls off)"); } }
            else {
                if (g_air_since <= 0) g_air_since = g_now_ms;
                double t = g_now_ms - g_air_since;
                if (!g_air && t >= g_air_ms && t < 6000.0) { g_air = 1; if (g_state_log) log_num("    (air controls on, state ", st_now); }
                if (g_air && t >= 6000.0) { g_air = 0; log_text("    (air controls off: 6 s without landing, probably not really in the air)"); }
            }
        }
        drain_state_events();
    }
    if (now - g_last_ini_check > 2000) { g_last_ini_check = now; load_config(1); }
    {
        static int was_owned;
        int owned = reskate_owns_input(now);
        if (owned != was_owned) {
            log_text(owned ? "reSkate overlay owns input: Dual Stick gestures suspended" : "reSkate overlay released input: Dual Stick controls back");
            was_owned = owned;
        }
        if (owned) {
            g_cfg.enabled = 0;
            sc_overlay_suspend(&g_state[user < 4 ? user : 0]);
            g_ui_open = g_ui_bind_mode = 0;
            g_ui_req_toggle = g_ui_req_nav = g_ui_req_change = g_ui_req_bound_key = 0;
            return; /* Keep reSkate's captured/neutral pad exactly as returned. */
        }
    }
    g_ui_key = key_from_name(g_menu_key_name);
    ui_start();
    if (!g_seen_state) { g_seen_state = 1; log_text("game is polling the controller through Dual Stick Controls"); dg_start(); gs_start(); }
    if(timing)timing[0]=pt_time();
    int menu_consumed=ui_poll(s);
    if(timing)timing[1]=pt_time();
    if(menu_consumed)return;
    handle_combos(user, s);
    sc_state* st = &g_state[user < 4 ? user : 0];
    /* v0.26c native menus: when the game pauses (pause menu, map, settings...) your skater's physics
     * stops ticking. While it is not ticking the pad goes to the game untouched, exactly like stock,
     * however deep you go into the menus and whichever button you leave them with. */
    {
        static int paused, menu_saw_pause;
        int p = g_sel_status == 2 && g_local >= 0 && g_slots[g_local].seen > 0 && g_now_ms - g_slots[g_local].seen > 200.0;
        if (p != paused) {
            paused = p;
            log_text(p ? "game paused (skater stopped): standard controls" : "game running again: Dual Stick controls back");
            if (p && g_menu) menu_saw_pause = 1;
            if (!p && g_menu && menu_saw_pause) { g_menu = 0; g_menu_depth = 0; }
        }
        if (!g_menu) menu_saw_pause = 0;
        g_paused = paused;
    }
    g_cfg.enabled = g_user_enabled && !g_walking && !g_menu && !g_paused;
    if (!g_cfg.enabled) { sc_state zero = {0}; *st = zero; }
    st->air = g_air;
    st->takeoff = g_sel_status == 2 && g_local >= 0 ? g_takeoff : -1;
    st->stock_ramp_valid=0;st->stock_ramp_vertical=0;
    st->stock_ramp_departing=g_sel_status==2 && g_local>=0 &&
        (g_slots[g_local].state==103 || g_slots[g_local].state==200 || g_slots[g_local].state==201);
    if(g_cfg.stock_grab_air && g_sel_status==2 && g_local>=0 &&
       g_slots[g_local].state==100 && st->takeoff==0) {
        /* Captured unit direction at selector->+10->+10 +150: grounded
         * vertical component near zero on flats, +/-1 at both half-pipe lips.
         * Experimental orientation cue, not a claimed collision normal. */
        UINT64 first=rd_pointer(g_slots[g_local].sel+0x10);
        UINT64 orientation=first?rd_pointer(first+0x10):0;
        float direction[3];
        if(orientation && rd_copy(orientation+0x150,direction,sizeof direction)) {
            float length=direction[0]*direction[0]+direction[1]*direction[1]+direction[2]*direction[2];
            if(length>=0.95f && length<=1.05f && direction[1]>=-1.01f && direction[1]<=1.01f) {
                st->stock_ramp_valid=1;st->stock_ramp_vertical=direction[1];
            }
        }
    }
    st->game_known = g_sel_status == 2 && g_local >= 0;
    st->game_crouch = (g_sel_status == 2 && g_local >= 0) ? (g_slots[g_local].state == 103) : -1;
    st->sw_raw = (g_sel_status == 2 && g_local >= 0) ? read_switch_b() : -1;   /* v2.6: facing now, for landing manuals */
    st->game_switch_known=0;
    if (g_sel_status == 2 && g_cfg.switch_flip) {
        int sw = read_switched();
        int native_sw=sw;
        int motion_sw = -1;
        static sc_motion_stance motion;
        static int motion_slot=-2, motion_enabled=-1;
        if (motion_slot!=g_local || motion_enabled!=g_motion_stance) {
            sc_motion_stance zero={0}; motion=zero;
            motion_slot=g_local; motion_enabled=g_motion_stance;
        }
        if (g_motion_stance && g_switch_src==4 && !g_cfg.goofy && g_local>=0) {
            float position[3]={0}, facing[3]={0};
            /* A fixed riding reference needs no more position/facing reads.
             * Collect motion only while an actual remount is calibrating. */
            int sample_motion=motion.remount_pending && g_slots[g_local].state==100 &&
                              (!motion.sampled || g_now_ms-motion.time>=90);
            UINT64 first=sample_motion?rd_pointer(g_slots[g_local].sel+0x10):0;
            UINT64 orientation=first?rd_pointer(first+0x10):0;
            int valid=orientation && rd_copy(orientation+0x100,position,sizeof position) &&
                      rd_copy(orientation+0x150,facing,sizeof facing);
            int old_valid=motion.bias_valid,old_pending=motion.remount_pending;
            motion_sw=(motion.remount_pending && g_slots[g_local].state==100 && !sample_motion) ? -1 : sc_motion_stance_step(&motion,(int)g_slots[g_local].state,valid,
                position[0],position[2],facing[0],facing[2],sw,g_now_ms);
            if (old_pending && !old_valid && motion.bias_valid) {
                if(!motion.candidate_valid)log_text("remount stance: calibration cancelled by takeoff or timeout; keeping native frame");
                else log_text(motion.bias ? "experimental stance: backwards remount reference established from aligned ground motion" :
                                           "experimental stance: normal remount reference established from aligned ground motion");
            }
        }
        /* v5.45 test: use one takeoff-relative decision. Probe c was allowed to
         * overwrite this after landing, which produced normal -> switch -> normal
         * under the same held stick. It is still logged, but never changes stance. */
        static sc_stance_track tracking;
        static int track_slot=-2, track_source=-1, track_goofy=-1, track_mode=-1, track_layout=-1;
        if (track_slot!=g_local || track_source!=g_switch_src || track_goofy!=g_cfg.goofy ||
            track_mode!=g_cfg.switch_flip || track_layout!=g_cfg.front_back) {
            sc_stance_track zero={0}; tracking=zero;
            track_slot=g_local; track_source=g_switch_src; track_goofy=g_cfg.goofy;
            track_mode=g_cfg.switch_flip; track_layout=g_cfg.front_back;
            if (g_switch_src==4 && g_cfg.goofy && g_cfg.switch_flip==3 && !g_cfg.front_back)
                log_text("goofy true foot: retaining completed turns through ground probe resets");
        }
        int state = g_local >= 0 ? (int)g_slots[g_local].state : 0;
        int spawned = g_now_ms - g_spawn_t < 1500.0;
        /* Keep direct d for the tested regular route. Goofy True Foot uses
         * one turn relative to takeoff because its d resets on the ground. */
        sw = sc_resolve_stance(&tracking,g_switch_src,g_cfg.goofy,g_cfg.switch_flip,
                               g_cfg.front_back,state,motion_sw>=0?motion_sw:sw,spawned,g_switched,g_now_ms);
        if (motion_sw >= 0) {
            /* The game's turn flag resets at remount. Use the corrected
             * riding direction for feet, but the native flag for its input
             * frame, so a fakie input does not become a switch ollie. */
            if (native_sw >= 0) {st->game_switch_known=1;st->game_switch=sw ^ (motion.bias_valid ? motion.bias : 0);}
        }
        if (g_state_log && tracking.landing_turn_ms >= 0)
            log_num("  stance turned in the air, ms before landing: ", (UINT64)tracking.landing_turn_ms);
        if (g_state_log && tracking.late_turn_ms >= 0)
            log_num("  stance turned AFTER landing, ms: ", (UINT64)tracking.late_turn_ms);
        if (sw != g_switched) {
            if (g_switched >= 0 || sw) log_text(sw ? (g_cfg.switch_flip == 3 && !g_cfg.front_back ? "stance: riding SWITCH (true foot: the sticks swap jobs)" : "stance: riding SWITCH (front foot pops the end in front of you)") : "stance: riding normal");
            g_switched = sw;
        }
        st->switched = sw;
    }
    st->grind = g_local >= 0 && g_slots[g_local].state >= 400 && g_slots[g_local].state < 500;
    static INT64 freq;
    INT64 ticks = 0;
    if (!freq) QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&ticks);
    double now_ms = freq ? (double)ticks * 1000.0 / (double)freq : (double)now;
    unsigned before = st->fired;
    WORD raw_buttons = s->Gamepad.buttons;
    ring_push(now_ms, sc_axis(s->Gamepad.rx), sc_axis(s->Gamepad.ry));
    /* v5.12: the old ASSISTED trick recorder (LB + D-pad up with the mod off) is gone: it also hid LB
     * from the game while the mod was off, so stock grabs with LB did not work */
    if (!g_test_on && (raw_buttons & SC_LSHOULDER)) {
        WORD pr = (WORD)(raw_buttons & ~g_test_prev);
        if (pr & 0x0004) stance_dump(0);
        if (pr & 0x0008) stance_dump(1);
    }
    if (g_test_on && (raw_buttons & SC_LSHOULDER)) {
        test_controls(raw_buttons, g_test_prev, now_ms);
        s->Gamepad.buttons &= (WORD)~(SC_LSHOULDER | 0x000F); /* LB and the D-pad stay with the test */
    }
    g_test_prev = raw_buttons;
    sc_pad raw_pad = s->Gamepad;
    int pump_before = st->pump_r3;
    if(timing)timing[2]=pt_time();
    s->Gamepad = sc_translate(&g_cfg, st, s->Gamepad, now_ms);
    if(timing)timing[3]=pt_time();
    if (st->pump_r3 != pump_before)
        log_text(st->pump_r3 ? "  R3 pumping crouch: game R2 / RT held" : "  R3 pumping crouch: released / cancelled");
    {   /* v2.1: was the trick taken by the game? */
        int lst = g_local >= 0 ? (int)g_slots[g_local].state : -1;
        dg_record(now_ms, raw_pad, s->Gamepad, lst);
        {   /* v5.29: a pop where only ONE of your sticks moved: log the frames (what you did, what the game got) */
            static double t_l, t_r; static int prev_st = -1, n_one;
            float lx = sc_axis(raw_pad.lx), ly = sc_axis(raw_pad.ly), rx = sc_axis(raw_pad.rx), ry = sc_axis(raw_pad.ry);
            if (lx * lx + ly * ly > 0.35f * 0.35f) t_l = now_ms;
            if (rx * rx + ry * ry > 0.35f * 0.35f) t_r = now_ms;
            int popped = (lst == 200 || lst == 201) && prev_st >= 100 && prev_st < 200;
            if (popped && g_cfg.enabled && n_one < 12) {
                int only_r = now_ms - t_r < 500.0 && (t_l <= 0 || now_ms - t_l > 900.0);
                int only_l = now_ms - t_l < 500.0 && (t_r <= 0 || now_ms - t_r > 900.0);
                if (only_r || only_l) {
                    ++n_one;
                    log_text(only_r ? "  ONE-STICK POP: only the right stick moved - frames:" : "  ONE-STICK POP: only the left stick moved - frames:");
                    dg_dump(now_ms - 900.0);
                }
            }
            prev_st = lst;
        }
        if (g_rv_dump > 0) { static int n_rv; if (n_rv++ < 6) { log_text("  frames of that revert:"); dg_dump(g_rv_dump - 200.0); } g_rv_dump = 0; }
        if (g_cfg.enabled) dg_sticks(now_ms, raw_pad);
        int fired_now = g_cfg.enabled && (st->fired != before || st->d_log == 2);
        dg_trick(now_ms, st->fired != before ? st->last_name : "wind-up flick", fired_now, lst,
                 lst == 103 || (lst >= 200 && lst < 300) || (lst >= 400 && lst < 500));
    }
    /* v3.3 STOCK INPUT RECORDER: with Dual Stick switched OFF (L3+R3) while riding, every change of
     * your real sticks / triggers / buttons is written to the log with the game's state, so the
     * standard controls' own motions (prewind, body flips, darkslides...) can be copied exactly. */
    if (!g_user_enabled && !g_menu && !g_paused && !g_walking && g_local >= 0 && g_stock_log) {
        static float p[6]; static WORD pb; static int ps = -1, lines; static double pt;
        float v[6] = {sc_axis(raw_pad.lx), sc_axis(raw_pad.ly), sc_axis(raw_pad.rx), sc_axis(raw_pad.ry), raw_pad.lt / 255.0f, raw_pad.rt / 255.0f};
        int stv = (int)g_slots[g_local].state, ch = stv != ps || raw_pad.buttons != pb;
        for (int i = 0; i < 6; ++i) if (sc_abs(v[i] - p[i]) >= 0.12f || ((v[i] == 0) != (p[i] == 0) && sc_abs(v[i] - p[i]) > 0.02f)) ch = 1;
        if (ch && lines < 20000) {
            char line[160]; unsigned n = 0;
            #define KPUT(str) do { const char* q_ = (str); while (*q_ && n < 150) line[n++] = *q_++; } while (0)
            #define KNUM(fv) do { int iv_ = (int)((fv) * 100.0f); char t_[12]; int k_ = 0; if (iv_ < 0) { line[n++] = '-'; iv_ = -iv_; } \
                do { t_[k_++] = (char)('0' + iv_ % 10); iv_ /= 10; } while (iv_ && k_ < 11); while (k_ && n < 150) line[n++] = t_[--k_]; } while (0)
            KPUT("  stock +"); KNUM((float)((now_ms - pt) / 100.0)); KPUT("ms ["); KNUM((stv + 0.5f) / 100.0f); KPUT("] L(");
            KNUM(v[0]); KPUT(","); KNUM(v[1]); KPUT(") R("); KNUM(v[2]); KPUT(","); KNUM(v[3]); KPUT(") LT"); KNUM(v[4]); KPUT(" RT"); KNUM(v[5]);
            if (raw_pad.buttons) {
                static const char* bn[16] = {"Up","Down","Left","Right","Start","Back","L3","R3","LB","RB","?","?","A","B","X","Y"};
                KPUT(" btn"); for (int b = 0; b < 16; ++b) if (raw_pad.buttons & (1 << b)) { KPUT(" "); KPUT(bn[b]); }
            }
            line[n] = 0; log_text(line); ++lines;
            for (int i = 0; i < 6; ++i) p[i] = v[i];
            pb = raw_pad.buttons; ps = stv; pt = now_ms;
        }
    }
    /* v1.7 landing report: what the sticks did around each landing, to track down manuals */
    {
        static int prev_to = -1, watching; static double land_t; static float mx, my, best;
        static float lx0, ly0, rx0, ry0; static int fc, bc, fi, bi;
        int to = st->takeoff;
        static float amx, amy, abest;   /* biggest air-control push during the flight */
        static float llx, lly, lbest;   /* v2.0: game LEFT stick at / after landing (spin wind-up) */
        static int trg_max; static float spin_max; static int rv_seen;   /* v3.1: spin in the air */
        if (to == 1 && prev_to != 1) { abest = 0; amx = amy = 0; trg_max = 0; spin_max = 0; rv_seen = 0; }
        if (to == 1) { int tg = raw_pad.lt > raw_pad.rt ? raw_pad.lt : raw_pad.rt; if (tg > trg_max) trg_max = tg;
                       float sx = sc_axis(s->Gamepad.lx); if (sc_abs(sx) > sc_abs(spin_max)) spin_max = sx;
                       if (st->rv_start > 0) rv_seen = 1; }
        if (to == 1) { float ax = sc_axis(s->Gamepad.rx), ay = sc_axis(s->Gamepad.ry); if (ax * ax + ay * ay > abest) { abest = ax * ax + ay * ay; amx = ax; amy = ay; } }
        if (g_cfg.enabled && prev_to == 1 && to == 0) {
            watching = 1; land_t = now_ms; best = 0; mx = my = 0; lbest = 0; llx = lly = 0;
            lx0 = sc_axis(raw_pad.lx); ly0 = sc_axis(raw_pad.ly); rx0 = sc_axis(raw_pad.rx); ry0 = sc_axis(raw_pad.ry);
            fc = st->d_front_cign; bc = st->d_back_cign; fi = st->d_front_ignored; bi = st->d_back_ignored;
        }
        prev_to = to;
        if (watching) {
            float ox = sc_axis(s->Gamepad.rx), oy = sc_axis(s->Gamepad.ry), m = ox * ox + oy * oy;
            if (m > best) { best = m; mx = ox; my = oy; }
            { float qx = sc_axis(s->Gamepad.lx), qy = sc_axis(s->Gamepad.ly); if (qy * qy > lbest) { lbest = qy * qy; llx = qx; lly = qy; } }
            if (now_ms - land_t >= 300.0) {
                watching = 0;
                if (g_state_log) {
                    char line[200]; unsigned n = 0;
                    #define LPUT(str) do { const char* q_ = (str); while (*q_ && n < 190) line[n++] = *q_++; } while (0)
                    #define LNUM(v) do { int iv_ = (int)((v) * 100.0f); char t_[12]; int k_ = 0; if (iv_ < 0) { line[n++] = '-'; iv_ = -iv_; } \
                        do { t_[k_++] = (char)('0' + iv_ % 10); iv_ /= 10; } while (iv_ && k_ < 11); while (k_ && n < 190) line[n++] = t_[--k_]; } while (0)
                    LPUT("  landing: sticks L("); LNUM(lx0); LPUT(","); LNUM(ly0); LPUT(") R("); LNUM(rx0); LPUT(","); LNUM(ry0);
                    LPUT(")  ignored F"); LPUT(fi ? (fc ? "=held" : "=yes") : "=no"); LPUT(" B"); LPUT(bi ? (bc ? "=held" : "=yes") : "=no");
                    LPUT("  in the air max ("); LNUM(amx); LPUT(","); LNUM(amy); LPUT(")");
                    LPUT("  after landing max ("); LNUM(mx); LPUT(","); LNUM(my); LPUT(")");
                    LPUT("  left stick ("); LNUM(llx); LPUT(","); LNUM(lly); LPUT(")");
                    LPUT("  spin: trigger "); LNUM(trg_max / 255.0f); LPUT(" -> game left stick "); LNUM(spin_max);
                    if (rv_seen) LPUT(" (revert was running!)");
                    if (trg_max >= 60 && sc_abs(spin_max) < 0.2f) LPUT(" <- NO SPIN");
                    if (my < -0.2f || lly < -0.3f) LPUT(" <- MANUAL side"); else if (my > 0.2f || lly > 0.3f) LPUT(" <- NOSE MANUAL side");
                    line[n] = 0; log_text(line);
                }
            }
        }
    }
    if (g_cfg.enabled) {
        float tx, ty;
        if (test_output(now_ms, &tx, &ty)) { s->Gamepad.rx = sc_short(tx); s->Gamepad.ry = sc_short(ty); }
    }
    if (st->walk_request) {
        st->walk_request = 0;
        sc_state zero = {0}; *st = zero;
        g_cfg.enabled = 0;
        log_text("left stick held like walking: Dual Stick mode OFF (Y or L3+R3 to ride again)");
    }
    if (g_cfg.direct) st->event = 0; /* pop diagnostics belong to draw mode */
    if (g_debug && st->event) {
        char line[160]; unsigned n = 0;
        #define PUT(str) do { const char* q_ = (str); while (*q_ && n < 150) line[n++] = *q_++; } while (0)
        #define NUM(v) do { int iv_ = (int)(v); char t_[12]; int k_ = 0; if (iv_ < 0) { line[n++] = '-'; iv_ = -iv_; } \
            do { t_[k_++] = (char)('0' + iv_ % 10); iv_ /= 10; } while (iv_ && k_ < 11); while (k_ && n < 150) line[n++] = t_[--k_]; } while (0)
        if (st->event == 1) { PUT("  pop "); PUT(st->pop_back ? "back foot" : "front foot"); PUT(", angle "); NUM(st->pop_angle); PUT(", strength "); NUM(st->pop_mag * 100); }
        else if (st->event == 2) { PUT("  pop held "); NUM(now_ms - st->pop_start); PUT(" ms, deepest "); NUM(st->max_mag * 100); PUT(", scoop "); NUM(st->sweep); PUT(" deg"); }
        else if (st->event == 3) { PUT("  NOT a pop: "); PUT(st->ev_foot_back ? "back foot" : "front foot"); PUT(" at angle "); NUM(st->ev_angle); PUT(" (pop needs 300-60, 0 = straight down), strength "); NUM(st->ev_mag * 100); }
        line[n] = 0;
        log_text(line);
        st->event = 0;
    }
    if (st->d_log && g_cfg.enabled) {
        static const char* const dir8[8] = {"down", "down-right", "right", "up-right", "up", "up-left", "left", "down-left"};
        char line[160]; unsigned n = 0;
        #define WPUT(str) do { const char* q_ = (str); while (*q_ && n < 150) line[n++] = *q_++; } while (0)
        #define WNUM(v) do { int iv_ = (int)(v); char t_[12]; int k_ = 0; if (iv_ < 0) { line[n++] = '-'; iv_ = -iv_; } \
            do { t_[k_++] = (char)('0' + iv_ % 10); iv_ /= 10; } while (iv_ && k_ < 11); while (k_ && n < 150) line[n++] = t_[--k_]; } while (0)
        #define WDIR(a) WPUT(dir8[((int)((a) + 22.5f) % 360 + 360) % 360 / 45])
        if (st->d_log == 1) { WPUT("  wind-up: "); WPUT(st->d_log_b > 0.5f ? "back" : "front"); WPUT(" foot started "); WDIR(st->d_log_a); WPUT(" (passed straight through)"); }
        else if (st->d_log == 4) {
            WPUT(st->d_log_b == 0 ? "  revert: both triggers, " : "  revert: double tap "); WPUT(st->d_log_a > 0 ? "RT" : "LT");
            WPUT(" (shape "); WNUM(g_cfg.revert_shape); WPUT(", "); WNUM(g_cfg.revert_sweep_ms); WPUT(" ms, taps "); WNUM(st->d_log_b); WPUT(" ms apart)");
            if (g_rv_watch_t > 0) log_text("  revert: (previous one not judged: next double tap came too soon)");
            g_rv_watch_t = g_now_ms; g_rv_watch_b = read_switch_b(); g_rv_shape = g_cfg.revert_shape; g_rv_ms = (int)g_cfg.revert_sweep_ms;
            if (g_rv_calib) {   /* next double tap tries the next variant */
                g_rv_calib_i = (g_rv_calib_i + 1) % 8;
                g_cfg.revert_shape = k_rv_shapes[g_rv_calib_i / 4]; g_cfg.revert_sweep_ms = (float)k_rv_speeds[g_rv_calib_i % 4];
            }
        }
        else if (st->d_log == 10) { WPUT("  stock grab-air: physical sticks and analogue trigger grabs until landing"); }
        else if (st->d_log == 11) { WPUT("  stock ramp approach: early physical stock controls; until landing or approach expires"); }
        else if (st->d_log == 7) { WPUT("  grind / lip exit: "); WPUT(st->d_log_b < 0 ? "DOWN (tilted stick click)" : st->d_log_b > 0 ? "UP (tilted stick click)" : st->d_log_a > 0 ? "right (R3)" : "left (L3)"); }
        else if (st->d_log == 9) { WPUT(st->d_log_a > 0 ? "  front flip" : "  backflip"); WPUT(st->d_log_a == 2 || st->d_log_a == -2 ? ": waiting for the pop" : ": started in the air"); }
        else if (st->d_log == 8) { WPUT("  revert cancelled: you crouched / popped (the trigger spins you instead)"); }
        else if (st->d_log == 6) { WPUT("  spin wind-up: "); WPUT(st->d_log_a > 0 ? "right (RT)" : "left (LT)"); }
        else if (st->d_log == 5) { WPUT("  powerslide: "); WPUT(st->d_log_a > 0 ? "toe side" : "heel side"); }
        else if (st->d_log == 2) { WPUT("  wind-up flick: game stick "); WDIR(st->d_log_b); WPUT(" -> "); WDIR(st->d_log_a); }
        else { WPUT("  combo: scoop "); WNUM(st->d_log_a); WPUT(" deg, flick x "); WNUM(st->d_log_b * 100); WPUT("%"); }
        line[n] = 0;
        log_text(line);
    }
    st->d_log = 0;
    revert_watch();
    if (st->fired != before) g_probe_ms = g_now_ms;
    if (st->fired != before && st->last_name) {
        char line[128] = "trick: "; unsigned n = 7;
        for (const char* q = st->last_name; *q && n < 60; ++q) line[n++] = *q;
        if (sc_ovr_n[st->trick] > 0) { const char* r = " (your recording)"; while (*r && n < 90) line[n++] = *r++; }
        else {
            /* the game mirrors left/right compared with its trick data: say what it should show */
            static const char* const pairs[][2] = {
                {"kickflip","heelflip"}, {"varialkickflip","varialheelflip"}, {"360flip","laserflip"},
                {"hardflip","inwardheelflip"}, {"360hardflip","360inwardheelflip"}, {"popshuvit","fspopshuvit"},
                {"360popshuvit","fs360popshuvit"}, {"impossible","frontfoot_impossible"},
                {"n_kickflip","n_heelflip"}, {"n_varialkickflip","n_varialheelflip"}, {"n_360flip","n_laserflip"},
                {"n_hardflip","n_inwardheelflip"}, {"n_360hardflip","n_360inwardheelflip"}, {"n_popshuvit","n_fspopshuvit"},
                {"n_360popshuvit","n_fs360popshuvit"}, {"n_impossible","n_backfoot_impossible"}};
            const char* shown = 0;
            for (unsigned i = 0; i < sizeof pairs / sizeof pairs[0]; ++i) {
                if (sc_streq(pairs[i][0], st->last_name)) shown = pairs[i][1];
                else if (sc_streq(pairs[i][1], st->last_name)) shown = pairs[i][0];
            }
            if (shown) { const char* r = "  -> game should show "; while (*r && n < 90) line[n++] = *r++; while (*shown && n < 94) line[n++] = *shown++; }
        }
        line[n] = 0;
        log_text(line);
    }
}
/* Capture at the process boundary, including early returns for menus / walking.
 * The output is the actual translated pad, not a rounded summary or a trick guess. */
static void process(DWORD user, XINPUT_STATE* s) {
    int tracing=g_precise_log && !pt_failed;
    sc_pad raw=s->Gamepad; UINT64 start=0;
    if(tracing) { pt_start(); tracing=!pt_failed; start=pt_time(); }
    UINT64 timing[4]={0};
    process_inner(user,s,tracing?timing:0);
    if(g_ramp_diag || g_remount_diag)rd_capture(GetTickCount64());
    if(tracing) {
        sc_state* st=&g_state[user<4?user:0]; sc_pad out=s->Gamepad;
        pt_record r={0}; r.start_us=start; r.end_us=pt_time();
        int values[PT_FIELDS]={
            (int)user,g_local>=0?(int)g_slots[g_local].state:-1,
            g_cfg.enabled,g_user_enabled,g_paused,g_menu,g_ui_open,g_walking,
            g_cfg.goofy,g_auto_stance,g_cfg.front_back,g_cfg.switch_flip,g_switch_src,
            st->switched,st->feet_map_ok,st->feet_map_left,st->sw_lat,st->takeoff,
            st->air,st->grind,st->hold_foot,st->d_front_on,st->d_back_on,
            st->d_front_spent,st->d_back_spent,st->d_front_ignored,st->d_back_ignored,
            st->d_front_carry,st->d_back_carry,st->d_front_cign,st->d_back_cign,
            raw.lx,raw.ly,raw.rx,raw.ry,raw.lt,raw.rt,raw.buttons,
            out.lx,out.ly,out.rx,out.ry,out.lt,out.rt,out.buttons,
            g_switch_probes[0],g_switch_probes[1],g_switch_probes[2],g_switch_probes[3],st->stock_air_active,g_ramp_diag,
            st->stock_ramp_valid,(int)(st->stock_ramp_vertical*1000),st->stock_ramp_eligible,st->stock_ramp_departing,st->stock_ramp_curve,st->stock_early_active,
            timing[0]?(int)(timing[0]-start):0,
            timing[1]?(int)(timing[1]-timing[0]):0,
            timing[2]?(int)(timing[2]-timing[1]):0,
            timing[3]?(int)(timing[3]-timing[2]):0,
            (int)(r.end_us-(timing[3]?timing[3]:timing[1]?timing[1]:start))};
        typedef char trace_shape_check[(sizeof values/sizeof values[0]==PT_FIELDS)?1:-1];
        for(int i=0;i<PT_FIELDS;i++) r.v[i]=values[i];
        pt_enqueue(&r);
    }
}
/* v0.24b: the game asks XInput for a controller but none answers for 15 s: say why. skate. reads
 * PlayStation pads itself (HID), which this mod cannot see; they need Steam Input or DS4Windows. */
static void no_pad_hint(DWORD r) {
    static UINT64 first; static int said;
    if (r == ERROR_SUCCESS || g_seen_state || said) return;
    UINT64 now = GetTickCount64();
    if (!first) { first = now; return; }
    if (now - first > 15000) {
        said = 1;
        log_text("no controller found: no Xbox-style pad, and no DualSense / DualShock 4 the mod could open.");
        log_text("  See README, PLAYSTATION PADS (PlayStationPads=1 in the ini, or use Steam Input / DS4Windows).");
    }
}
/* v5.6: XInput's packet number tells the game whether ANYTHING changed since its last read. The mod
 * changes the sticks by itself (reverts, spin wind-ups, flips, held crouches...) while your real pad
 * sits still - the packet number then stayed the same and the game was free to ignore the new
 * values. Every time what the game gets changes, the packet number now changes too. */
static void fix_packet(DWORD user, XINPUT_STATE* s) {
    static DWORD seq[4], last_raw[4]; static sc_pad last_out[4]; static int have[4];
    if (user > 3) return;
    const BYTE* a = (const BYTE*)&s->Gamepad; const BYTE* b = (const BYTE*)&last_out[user]; int same = have[user];
    for (unsigned i = 0; same && i < sizeof(sc_pad); ++i) if (a[i] != b[i]) same = 0;
    if (!same || s->dwPacketNumber != last_raw[user]) ++seq[user];
    last_raw[user] = s->dwPacketNumber; last_out[user] = s->Gamepad; have[user] = 1;
    s->dwPacketNumber = seq[user];
}
static DWORD WINAPI wrap_getstate(DWORD user, XINPUT_STATE* s) {
    DWORD r = g_real_getstate ? g_real_getstate(user, s) : ERROR_DEVICE_NOT_CONNECTED;
    if (r != ERROR_SUCCESS && ps_fill(user, s)) r = ERROR_SUCCESS;   /* v0.25: PlayStation pad */
    if (r == ERROR_SUCCESS && s) { DWORD raw = s->dwPacketNumber; process(user, s); s->dwPacketNumber = raw; fix_packet(user, s); }
    else if (user == 0) no_pad_hint(r);
    return r;
}
static DWORD WINAPI wrap_getstateex(DWORD user, XINPUT_STATE* s) {
    DWORD r = g_real_getstateex ? g_real_getstateex(user, s) : ERROR_DEVICE_NOT_CONNECTED;
    if (r != ERROR_SUCCESS && ps_fill(user, s)) r = ERROR_SUCCESS;
    if (r == ERROR_SUCCESS && s) { DWORD raw = s->dwPacketNumber; process(user, s); s->dwPacketNumber = raw; fix_packet(user, s); }
    return r;
}

typedef DWORD (WINAPI* PFN_GetCaps)(DWORD, DWORD, void*);
static PFN_GetCaps g_real_getcaps;
static DWORD WINAPI wrap_getcaps(DWORD user, DWORD flags, void* caps) {
    DWORD r = g_real_getcaps ? g_real_getcaps(user, flags, caps) : ERROR_DEVICE_NOT_CONNECTED;
    if (r != ERROR_SUCCESS && g_ps_enabled && user == 0 && g_ps_connected && caps) {
        BYTE* c = (BYTE*)caps; memset(c, 0, 20);
        c[0] = 1; c[1] = 1;                       /* gamepad */
        *(WORD*)(c + 4) = 0xF3FF; c[6] = c[7] = 255;
        *(short*)(c + 8) = *(short*)(c + 10) = *(short*)(c + 12) = *(short*)(c + 14) = (short)0xFFC0;
        *(WORD*)(c + 16) = *(WORD*)(c + 18) = 0xFFFF;
        r = ERROR_SUCCESS;
    }
    return r;
}
static int is_xinput(HMODULE m) {
    static HMODULE last_yes, last_no;
    if (!m) return 0;
    if (m == last_yes) return 1;
    if (m == last_no) return 0;
    WCHAR path[520] = {0};
    GetModuleFileNameW(m, path, 519);
    if (wistarts(wbase(path), "xinput")) { last_yes = m; return 1; }
    last_no = m;
    return 0;
}

/* The game's GetProcAddress, redirected. */
static FARPROC WINAPI hook_gpa(HMODULE m, const char* name) {
    FARPROC r = g_real_gpa(m, name);
    if (!r || !is_xinput(m)) return r;
    if ((UINT64)name > 0xFFFF) {
        if (streq(name, "XInputGetState")) {
            g_real_getstate = (PFN_GetState)r;
            log_text("caught the game's XInputGetState lookup");
            ps_start();
            return (FARPROC)wrap_getstate;
        }
        if (streq(name, "XInputGetCapabilities")) { g_real_getcaps = (PFN_GetCaps)r; return (FARPROC)wrap_getcaps; }
    } else if ((UINT64)name == 100) {
        g_real_getstateex = (PFN_GetState)r;
        log_text("caught the game's XInputGetStateEx lookup");
        return (FARPROC)wrap_getstateex;
    }
    return r;
}

/* Point every GetProcAddress slot in Skate.exe's import table at hook_gpa. */
static int redirect_imports(void) {
    BYTE* base = (BYTE*)GetModuleHandleW(0);
    if (!base || base[0] != 'M' || base[1] != 'Z') return 0;
    BYTE* nt = base + *(DWORD*)(base + 0x3c);
    if (*(DWORD*)nt != 0x4550) return 0;
    BYTE* opt = nt + 24;
    if (*(WORD*)opt != 0x20b) return 0; /* PE32+ */
    DWORD imp_rva = *(DWORD*)(opt + 112 + 8), imp_size = *(DWORD*)(opt + 112 + 12);
    if (!imp_rva || !imp_size) return 0;

    UINT64 targets[3] = {(UINT64)GetProcAddress, 0, 0};
    g_real_gpa = (PFN_GetProcAddress)GetProcAddress;
    HMODULE kb = GetModuleHandleW(L"kernelbase.dll");
    if (kb) targets[1] = (UINT64)GetProcAddress(kb, "GetProcAddress");
    HMODULE k32 = GetModuleHandleW(L"kernel32.dll");
    if (k32) targets[2] = (UINT64)GetProcAddress(k32, "GetProcAddress");

    int patched = 0;
    for (BYTE* d = base + imp_rva; *(DWORD*)(d + 12) /*Name*/; d += 20) {
        UINT64* thunk = (UINT64*)(base + *(DWORD*)(d + 16) /*FirstThunk*/);
        for (; *thunk; ++thunk) {
            UINT64 v = *thunk;
            if (v && (v == targets[0] || v == targets[1] || v == targets[2])) {
                DWORD old;
                if (VirtualProtect(thunk, 8, PAGE_READWRITE, &old)) {
                    *thunk = (UINT64)hook_gpa;
                    VirtualProtect(thunk, 8, old, &old);
                    ++patched;
                }
            }
        }
    }
    return patched;
}

static void setup(HMODULE self) {
    WCHAR path[520] = {0};
    GetModuleFileNameW(self, path, 519);
    const WCHAR* base = wbase(path);
    unsigned dirlen = (unsigned)(base - path);
    for (unsigned i = 0; i < dirlen && i < 519; ++i) g_dir[i] = path[i];
    g_dir[dirlen] = 0;
    g_ini[0] = 0; wcat(g_ini, 560, g_dir); wcat(g_ini, 560, L"session_controls.ini");
    g_log[0] = 0; wcat(g_log, 560, g_dir); wcat(g_log, 560, L"session_controls.log");

    WCHAR exe[520] = {0};
    GetModuleFileNameW(0, exe, 519);
    if (!wieq(wbase(exe), "skate.exe")) return; /* only the game; stay a plain version.dll elsewhere */

    log_text("---- Dual Stick Controls 5.87 SLIDE PUMP RECOVERY loaded (version.dll route)");
    log_wide("process: ", exe);
    load_config(0);
    if (GetFileAttributesW(g_ini) == 0xFFFFFFFFu) log_text("no session_controls.ini found; using defaults");
    int n = redirect_imports();
    log_num("GetProcAddress import slots redirected: ", (UINT64)n);
    if (g_ps_enabled) ps_redirect_createfile();
    if (g_ps_enabled == 2) ps_native_setup();
    if (!n) { log_text("could not redirect the game's imports; controls unchanged"); return; }
    g_active = 1;
    log_text(g_cfg.enabled ? "Dual Stick mode: ON" : "Dual Stick mode: OFF");
    log_text(g_cfg.goofy ? "stance: goofy" : "stance: regular");
    log_text(g_cfg.direct ? "tricks: direct (feet passed through, v0.3 style)" : "tricks: draw (recognise and draw each trick)");
}

/* ---------------------------------------------------------------- version.dll forwarding */
static HMODULE g_version;
static FARPROC real_version(const char* name) {
    if (!g_version) {
        WCHAR sys[600] = {0};
        GetSystemDirectoryW(sys, 520);
        wcat(sys, 600, L"\\version.dll");
        g_version = LoadLibraryW(sys);
        if (!g_version) return 0;
    }
    return GetProcAddress(g_version, name);
}
#define FWD(ret, fail, name, params, args) \
    DLLEXPORT ret WINAPI name params { \
        static FARPROC f; if (!f) f = real_version(#name); \
        return f ? ((ret (WINAPI*) params)f) args : fail; }

FWD(BOOL, 0, GetFileVersionInfoA, (const char* a, DWORD b, DWORD c, void* d), (a, b, c, d))
FWD(BOOL, 0, GetFileVersionInfoW, (const WCHAR* a, DWORD b, DWORD c, void* d), (a, b, c, d))
FWD(BOOL, 0, GetFileVersionInfoExA, (DWORD a, const char* b, DWORD c, DWORD d, void* e), (a, b, c, d, e))
FWD(BOOL, 0, GetFileVersionInfoExW, (DWORD a, const WCHAR* b, DWORD c, DWORD d, void* e), (a, b, c, d, e))
FWD(DWORD, 0, GetFileVersionInfoSizeA, (const char* a, DWORD* b), (a, b))
FWD(DWORD, 0, GetFileVersionInfoSizeW, (const WCHAR* a, DWORD* b), (a, b))
FWD(DWORD, 0, GetFileVersionInfoSizeExA, (DWORD a, const char* b, DWORD* c), (a, b, c))
FWD(DWORD, 0, GetFileVersionInfoSizeExW, (DWORD a, const WCHAR* b, DWORD* c), (a, b, c))
FWD(BOOL, 0, GetFileVersionInfoByHandle, (void* a, void* b, void* c, void* d), (a, b, c, d))
FWD(BOOL, 0, VerQueryValueA, (const void* a, const char* b, void** c, UINT* d), (a, b, c, d))
FWD(BOOL, 0, VerQueryValueW, (const void* a, const WCHAR* b, void** c, UINT* d), (a, b, c, d))
FWD(DWORD, 0, VerFindFileA, (DWORD a, const char* b, const char* c, const char* d, char* e, UINT* f2, char* g, UINT* h), (a, b, c, d, e, f2, g, h))
FWD(DWORD, 0, VerFindFileW, (DWORD a, const WCHAR* b, const WCHAR* c, const WCHAR* d, WCHAR* e, UINT* f2, WCHAR* g, UINT* h), (a, b, c, d, e, f2, g, h))
FWD(DWORD, 0, VerInstallFileA, (DWORD a, const char* b, const char* c, const char* d, const char* e, const char* f2, char* g, UINT* h), (a, b, c, d, e, f2, g, h))
FWD(DWORD, 0, VerInstallFileW, (DWORD a, const WCHAR* b, const WCHAR* c, const WCHAR* d, const WCHAR* e, const WCHAR* f2, WCHAR* g, UINT* h), (a, b, c, d, e, f2, g, h))
FWD(DWORD, 0, VerLanguageNameA, (DWORD a, char* b, DWORD c), (a, b, c))
FWD(DWORD, 0, VerLanguageNameW, (DWORD a, WCHAR* b, DWORD c), (a, b, c))

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void* reserved) {
    (void)reserved;
    if (reason == 1) {
        g_self = self;
        DisableThreadLibraryCalls(self);
        setup(self); /* no library loading here: only memory writes and file logging */
    }
    return 1;
}
