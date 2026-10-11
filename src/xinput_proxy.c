/* Session Controls for reSkate: an XInput proxy.
 * Built freestanding (no CRT): clang --target=x86_64-pc-windows-msvc + lld-link.
 * Placed beside Skate.exe as xinput1_4.dll. Loads the real XInput from System32,
 * forwards every call, and rewrites controller state in XInputGetState(Ex).
 */
#include "translate.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef int BOOL;
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

typedef struct { DWORD dwPacketNumber; sc_pad Gamepad; } XINPUT_STATE;

__declspec(dllimport) HMODULE WINAPI LoadLibraryW(const WCHAR*);
__declspec(dllimport) FARPROC WINAPI GetProcAddress(HMODULE, const char*);
__declspec(dllimport) DWORD WINAPI GetSystemDirectoryW(WCHAR*, DWORD);
__declspec(dllimport) DWORD WINAPI GetModuleFileNameW(HMODULE, WCHAR*, DWORD);
__declspec(dllimport) HANDLE WINAPI CreateFileW(const WCHAR*, DWORD, DWORD, void*, DWORD, DWORD, HANDLE);
__declspec(dllimport) BOOL WINAPI WriteFile(HANDLE, const void*, DWORD, DWORD*, void*);
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE);
__declspec(dllimport) DWORD WINAPI GetPrivateProfileIntW(const WCHAR*, const WCHAR*, int, const WCHAR*);
__declspec(dllimport) DWORD WINAPI GetPrivateProfileStringW(const WCHAR*, const WCHAR*, const WCHAR*, WCHAR*, DWORD, const WCHAR*);
__declspec(dllimport) BOOL WINAPI DisableThreadLibraryCalls(HMODULE);
__declspec(dllimport) UINT64 WINAPI GetTickCount64(void);
__declspec(dllimport) DWORD WINAPI GetFileAttributesW(const WCHAR*);

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

/* ---------------------------------------------------------------- state */
static HMODULE g_self, g_real;
static WCHAR g_dir[520], g_ini[560], g_log[560], g_name[64];
static sc_config g_cfg;
static int g_ready;
static WORD g_prev_buttons[4];
static UINT64 g_last_ini_check;

typedef DWORD (WINAPI* PFN_GetState)(DWORD, XINPUT_STATE*);
typedef DWORD (WINAPI* PFN_SetState)(DWORD, void*);
typedef DWORD (WINAPI* PFN_GetCaps)(DWORD, DWORD, void*);
typedef void  (WINAPI* PFN_Enable)(BOOL);
typedef DWORD (WINAPI* PFN_Battery)(DWORD, BYTE, void*);
typedef DWORD (WINAPI* PFN_Keystroke)(DWORD, DWORD, void*);
typedef DWORD (WINAPI* PFN_AudioIds)(DWORD, WCHAR*, unsigned*, WCHAR*, unsigned*);
typedef DWORD (WINAPI* PFN_DSound)(DWORD, void*, void*);
typedef DWORD (WINAPI* PFN_U)(DWORD);
typedef DWORD (WINAPI* PFN_UP)(DWORD, void*);
typedef DWORD (WINAPI* PFN_CapsEx)(DWORD, DWORD, DWORD, void*);

static PFN_GetState p_GetState, p_GetStateEx;
static PFN_SetState p_SetState;
static PFN_GetCaps p_GetCaps;
static PFN_Enable p_Enable;
static PFN_Battery p_Battery;
static PFN_Keystroke p_Keystroke;
static PFN_AudioIds p_AudioIds;
static PFN_DSound p_DSound;
static PFN_UP p_WaitGuide, p_BusInfo;
static PFN_U p_CancelGuide, p_PowerOff;
static PFN_CapsEx p_CapsEx;

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

/* ---------------------------------------------------------------- config */
static int ini_int(const WCHAR* key, int def) { return (int)GetPrivateProfileIntW(L"SessionControls", key, def, g_ini); }
static float ini_percent(const WCHAR* key, int def) { return (float)ini_int(key, def) / 100.0f; }
static void load_config(int keep_runtime) {
    int enabled = g_cfg.enabled, swapped = g_cfg.feet_swapped;
    WCHAR stance[16] = {0};
    GetPrivateProfileStringW(L"SessionControls", L"Stance", L"regular", stance, 16, g_ini);
    g_cfg.goofy = (stance[0] == 'g' || stance[0] == 'G');
    g_cfg.flip_x = ini_int(L"FlipSideways", 0) != 0;
    g_cfg.grab_mode = ini_int(L"GrabMode", 1);
    g_cfg.grab_threshold = ini_int(L"GrabTriggerThreshold", 250);
    g_cfg.deadzone = ini_percent(L"FootDeadzonePercent", 15);
    g_cfg.steer_scale = ini_percent(L"SteerStrengthPercent", 100);
    g_cfg.steer_curve = ini_percent(L"SteerCurvePercent", 150);
    if (g_cfg.deadzone < 0 || g_cfg.deadzone > 0.9f) g_cfg.deadzone = 0.15f;
    if (g_cfg.steer_curve < 1.0f) g_cfg.steer_curve = 1.0f;
    if (g_cfg.steer_curve > 3.0f) g_cfg.steer_curve = 3.0f;
    if (keep_runtime) { g_cfg.enabled = enabled; g_cfg.feet_swapped = swapped; }
    else { g_cfg.enabled = ini_int(L"EnabledAtStart", 1) != 0; g_cfg.feet_swapped = 0; }
}

static void init(void) {
    if (g_ready) return;
    WCHAR path[520] = {0};
    GetModuleFileNameW(g_self, path, 519);
    const WCHAR* base = wbase(path);
    unsigned dirlen = (unsigned)(base - path);
    for (unsigned i = 0; i < dirlen && i < 519; ++i) g_dir[i] = path[i];
    g_dir[dirlen] = 0;
    g_name[0] = 0; wcat(g_name, 64, base);
    g_ini[0] = 0; wcat(g_ini, 560, g_dir); wcat(g_ini, 560, L"session_controls.ini");
    g_log[0] = 0; wcat(g_log, 560, g_dir); wcat(g_log, 560, L"session_controls.log");

    log_text("---- Session Controls 0.1 loaded");
    log_wide("proxy: ", path);
    WCHAR exe[520] = {0}; GetModuleFileNameW(0, exe, 519);
    log_wide("process: ", exe);

    WCHAR sys[600] = {0};
    GetSystemDirectoryW(sys, 520);
    wcat(sys, 600, L"\\");
    wcat(sys, 600, g_name);
    g_real = LoadLibraryW(sys);
    if (!g_real) { log_wide("FAILED to load real XInput: ", sys); }
    else {
        log_wide("real XInput: ", sys);
        p_GetState = (PFN_GetState)GetProcAddress(g_real, "XInputGetState");
        p_GetStateEx = (PFN_GetState)GetProcAddress(g_real, (const char*)(INT64)100);
        p_SetState = (PFN_SetState)GetProcAddress(g_real, "XInputSetState");
        p_GetCaps = (PFN_GetCaps)GetProcAddress(g_real, "XInputGetCapabilities");
        p_Enable = (PFN_Enable)GetProcAddress(g_real, "XInputEnable");
        p_Battery = (PFN_Battery)GetProcAddress(g_real, "XInputGetBatteryInformation");
        p_Keystroke = (PFN_Keystroke)GetProcAddress(g_real, "XInputGetKeystroke");
        p_AudioIds = (PFN_AudioIds)GetProcAddress(g_real, "XInputGetAudioDeviceIds");
        p_DSound = (PFN_DSound)GetProcAddress(g_real, "XInputGetDSoundAudioDeviceGuids");
        p_WaitGuide = (PFN_UP)GetProcAddress(g_real, (const char*)(INT64)101);
        p_CancelGuide = (PFN_U)GetProcAddress(g_real, (const char*)(INT64)102);
        p_PowerOff = (PFN_U)GetProcAddress(g_real, (const char*)(INT64)103);
        p_BusInfo = (PFN_UP)GetProcAddress(g_real, (const char*)(INT64)104);
        p_CapsEx = (PFN_CapsEx)GetProcAddress(g_real, (const char*)(INT64)108);
    }
    load_config(0);
    if (GetFileAttributesW(g_ini) == 0xFFFFFFFFu) log_text("no session_controls.ini found; using defaults");
    log_text(g_cfg.enabled ? "Session mode: ON" : "Session mode: OFF");
    log_text(g_cfg.goofy ? "stance: goofy" : "stance: regular");
    g_ready = 1;
}

/* L3+R3 together toggles Session mode; LB+L3+R3 swaps feet (for switch). */
static void handle_combos(DWORD user, XINPUT_STATE* s) {
    if (user > 3) return;
    WORD b = s->Gamepad.buttons, prev = g_prev_buttons[user];
    WORD both = SC_LTHUMB | SC_RTHUMB;
    if ((b & both) == both && (prev & both) != both) {
        if (b & SC_LSHOULDER) {
            g_cfg.feet_swapped = !g_cfg.feet_swapped;
            log_text(g_cfg.feet_swapped ? "feet swapped (switch)" : "feet normal");
        } else {
            g_cfg.enabled = !g_cfg.enabled;
            log_text(g_cfg.enabled ? "Session mode: ON" : "Session mode: OFF");
        }
    }
    g_prev_buttons[user] = b;
    if ((b & both) == both) s->Gamepad.buttons &= (WORD)~(both | SC_LSHOULDER); /* the combo is ours */
}

static void process(DWORD user, XINPUT_STATE* s) {
    UINT64 now = GetTickCount64();
    if (now - g_last_ini_check > 2000) { g_last_ini_check = now; load_config(1); } /* live-edit the ini */
    handle_combos(user, s);
    s->Gamepad = sc_translate(&g_cfg, s->Gamepad);
}

/* ---------------------------------------------------------------- exports */
DLLEXPORT DWORD WINAPI XInputGetState(DWORD user, XINPUT_STATE* s) {
    init();
    if (!p_GetState) return ERROR_DEVICE_NOT_CONNECTED;
    DWORD r = p_GetState(user, s);
    if (r == ERROR_SUCCESS && s) process(user, s);
    return r;
}
DWORD WINAPI XInputGetStateEx(DWORD user, XINPUT_STATE* s) {
    init();
    PFN_GetState f = p_GetStateEx ? p_GetStateEx : p_GetState;
    if (!f) return ERROR_DEVICE_NOT_CONNECTED;
    DWORD r = f(user, s);
    if (r == ERROR_SUCCESS && s) process(user, s);
    return r;
}
DLLEXPORT DWORD WINAPI XInputSetState(DWORD u, void* v) { init(); return p_SetState ? p_SetState(u, v) : ERROR_DEVICE_NOT_CONNECTED; }
DLLEXPORT DWORD WINAPI XInputGetCapabilities(DWORD u, DWORD f, void* c) { init(); return p_GetCaps ? p_GetCaps(u, f, c) : ERROR_DEVICE_NOT_CONNECTED; }
DLLEXPORT void WINAPI XInputEnable(BOOL e) { init(); if (p_Enable) p_Enable(e); }
DLLEXPORT DWORD WINAPI XInputGetBatteryInformation(DWORD u, BYTE t, void* b) { init(); return p_Battery ? p_Battery(u, t, b) : ERROR_DEVICE_NOT_CONNECTED; }
DLLEXPORT DWORD WINAPI XInputGetKeystroke(DWORD u, DWORD r, void* k) { init(); return p_Keystroke ? p_Keystroke(u, r, k) : ERROR_DEVICE_NOT_CONNECTED; }
DLLEXPORT DWORD WINAPI XInputGetAudioDeviceIds(DWORD u, WCHAR* a, unsigned* b, WCHAR* c, unsigned* d) { init(); return p_AudioIds ? p_AudioIds(u, a, b, c, d) : ERROR_DEVICE_NOT_CONNECTED; }
DLLEXPORT DWORD WINAPI XInputGetDSoundAudioDeviceGuids(DWORD u, void* a, void* b) { init(); return p_DSound ? p_DSound(u, a, b) : ERROR_DEVICE_NOT_CONNECTED; }
DWORD WINAPI XInputWaitForGuideButton(DWORD u, void* l) { init(); return p_WaitGuide ? p_WaitGuide(u, l) : ERROR_DEVICE_NOT_CONNECTED; }
DWORD WINAPI XInputCancelGuideButtonWait(DWORD u) { init(); return p_CancelGuide ? p_CancelGuide(u) : ERROR_DEVICE_NOT_CONNECTED; }
DWORD WINAPI XInputPowerOffController(DWORD u) { init(); return p_PowerOff ? p_PowerOff(u) : ERROR_DEVICE_NOT_CONNECTED; }
DWORD WINAPI XInputGetBaseBusInformation(DWORD u, void* i) { init(); return p_BusInfo ? p_BusInfo(u, i) : ERROR_DEVICE_NOT_CONNECTED; }
DWORD WINAPI XInputGetCapabilitiesEx(DWORD a, DWORD u, DWORD f, void* c) {
    init(); return p_CapsEx ? p_CapsEx(a, u, f, c) : ERROR_DEVICE_NOT_CONNECTED;
}

BOOL WINAPI DllMain(HMODULE self, DWORD reason, void* reserved) {
    (void)reserved;
    if (reason == 1) { g_self = self; DisableThreadLibraryCalls(self); }
    return 1;
}
