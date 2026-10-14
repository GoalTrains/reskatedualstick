/* v0.25 PlayStation pads (DualSense, DualSense Edge, DualShock 4), no Steam Input / DS4Windows needed.
 *
 * skate. reads PlayStation pads itself through raw HID, so the XInput hook never saw them. Now:
 *  1. The mod reads the pad itself (its own HID handle, its own thread) and turns each report
 *     into an Xbox-style state.
 *  2. When the game asks XInput for controller 0 and no real Xbox pad answers, it gets that
 *     state instead, and the normal Session translation runs on it.
 *  3. The game's own attempts to open the PlayStation pad are refused (its CreateFile import is
 *     redirected), so it does not also read the raw sticks as stock skate. controls.
 * PlayStationPads=0 in the ini turns all of this off. Included by version_proxy.c. */

typedef struct { DWORD size; BYTE guid[16]; DWORD flags; UINT64 reserved; } SC_DEVIFACE;   /* SP_DEVICE_INTERFACE_DATA */
typedef void* (WINAPI* PFN_SetupDiGetClassDevsW)(const void*, const WCHAR*, void*, DWORD);
typedef BOOL (WINAPI* PFN_SetupDiEnumDeviceInterfaces)(void*, void*, const void*, DWORD, SC_DEVIFACE*);
typedef BOOL (WINAPI* PFN_SetupDiGetDeviceInterfaceDetailW)(void*, SC_DEVIFACE*, void*, DWORD, DWORD*, void*);
typedef BOOL (WINAPI* PFN_SetupDiDestroyDeviceInfoList)(void*);
typedef void (WINAPI* PFN_HidD_GetHidGuid)(void*);
typedef BOOL (WINAPI* PFN_HidD_GetFeature)(HANDLE, void*, DWORD);
typedef HANDLE (WINAPI* PFN_CreateFileW)(const WCHAR*, DWORD, DWORD, void*, DWORD, DWORD, HANDLE);
typedef HANDLE (WINAPI* PFN_CreateFileA)(const char*, DWORD, DWORD, void*, DWORD, DWORD, HANDLE);
__declspec(dllimport) BOOL WINAPI ReadFile(HANDLE, void*, DWORD, DWORD*, void*);
__declspec(dllimport) void WINAPI SetLastError(DWORD);

static volatile int g_ps_connected;          /* a pad is open and sending reports */
static volatile int g_ps_kind;               /* 1 DualSense, 2 DualShock 4 */
static volatile DWORD g_ps_seq;
static sc_pad g_ps_state;                    /* latest report, as an Xbox-style pad */
static PFN_CreateFileW g_real_cfw;
static PFN_CreateFileA g_real_cfa;
static int g_ps_started, g_ps_blocked_logged;

static int ps_path_is_sony(const WCHAR* p) {
    /* \\?\hid#vid_054c&pid_0ce6... (case varies) */
    for (; p && p[0]; ++p)
        if ((p[0] == 'v' || p[0] == 'V') && (p[1] == 'i' || p[1] == 'I') && (p[2] == 'd' || p[2] == 'D') && p[3] == '_' &&
            p[4] == '0' && p[5] == '5' && (p[6] == '4') && (p[7] == 'c' || p[7] == 'C')) return 1;
    return 0;
}
static int ps_path_is_sony_a(const char* p) {
    for (; p && p[0]; ++p)
        if ((p[0] | 32) == 'v' && (p[1] | 32) == 'i' && (p[2] | 32) == 'd' && p[3] == '_' && p[4] == '0' && p[5] == '5' && p[6] == '4' && (p[7] | 32) == 'c') return 1;
    return 0;
}
static int ps_pid_kind(const WCHAR* p) {
    static const struct { const char* pid; int kind; } pids[] = {
        {"0ce6", 1}, {"0df2", 1}, {"05c4", 2}, {"09cc", 2}, {"0ba0", 2}};
    for (; p && p[0]; ++p)
        if ((p[0] | 32) == 'p' && (p[1] | 32) == 'i' && (p[2] | 32) == 'd' && p[3] == '_')
            for (unsigned i = 0; i < sizeof pids / sizeof pids[0]; ++i) {
                const char* q = pids[i].pid; int k = 0;
                while (k < 4 && (WCHAR)(p[4 + k] | 32) == (WCHAR)q[k]) ++k;
                if (k == 4) return pids[i].kind;
            }
    return 0;
}

/* The game may not open the pad: it would read the raw sticks as normal skate. controls. */
static void ps_native_track(HANDLE h, int kind);
static HANDLE WINAPI hook_cfw(const WCHAR* name, DWORD acc, DWORD share, void* sa, DWORD disp, DWORD flags, HANDLE tmpl) {
    if (g_ps_enabled == 2 && ps_path_is_sony(name)) {   /* v5.23 NATIVE: the game keeps its own DualSense support */
        HANDLE h = g_real_cfw(name, acc, share, sa, disp, flags, tmpl);
        int kind = ps_pid_kind(name);
        if (h != INVALID_HANDLE_VALUE && kind) ps_native_track(h, kind);
        return h;
    }
    if (g_ps_enabled && ps_path_is_sony(name)) {
        if (!g_ps_blocked_logged) { g_ps_blocked_logged = 1; log_text("PlayStation pad: kept from the game (the mod reads it and passes it on as an Xbox pad)"); }
        SetLastError(5 /*ACCESS_DENIED*/);
        return INVALID_HANDLE_VALUE;
    }
    return g_real_cfw(name, acc, share, sa, disp, flags, tmpl);
}
static HANDLE WINAPI hook_cfa(const char* name, DWORD acc, DWORD share, void* sa, DWORD disp, DWORD flags, HANDLE tmpl) {
    if (g_ps_enabled == 1 && ps_path_is_sony_a(name)) { SetLastError(5); return INVALID_HANDLE_VALUE; }
    return g_real_cfa(name, acc, share, sa, disp, flags, tmpl);
}

#include "ps_reports.h"
static void ps_decode(const BYTE* r,int ds) {
    g_ps_state=sc_ps_decode(r,ds);g_ps_seq++;
}
static void ps_report(const BYTE* b,DWORD n,int kind) {
    sc_ps_frames f=sc_ps_frame(b,n,kind);
    for(unsigned i=0;i<f.count;i++)ps_decode(b+i*f.size+f.offset,f.dualsense);
}

static HANDLE ps_open(int* kind_out) {
    HMODULE sa = LoadLibraryW(L"setupapi.dll"), hid = LoadLibraryW(L"hid.dll");
    if (!sa || !hid) return INVALID_HANDLE_VALUE;
    PFN_SetupDiGetClassDevsW get = (PFN_SetupDiGetClassDevsW)g_real_gpa(sa, "SetupDiGetClassDevsW");
    PFN_SetupDiEnumDeviceInterfaces en = (PFN_SetupDiEnumDeviceInterfaces)g_real_gpa(sa, "SetupDiEnumDeviceInterfaces");
    PFN_SetupDiGetDeviceInterfaceDetailW det = (PFN_SetupDiGetDeviceInterfaceDetailW)g_real_gpa(sa, "SetupDiGetDeviceInterfaceDetailW");
    PFN_SetupDiDestroyDeviceInfoList des = (PFN_SetupDiDestroyDeviceInfoList)g_real_gpa(sa, "SetupDiDestroyDeviceInfoList");
    PFN_HidD_GetHidGuid hg = (PFN_HidD_GetHidGuid)g_real_gpa(hid, "HidD_GetHidGuid");
    PFN_HidD_GetFeature gf = (PFN_HidD_GetFeature)g_real_gpa(hid, "HidD_GetFeature");
    if (!get || !en || !det || !des || !hg || !g_real_cfw) return INVALID_HANDLE_VALUE;
    BYTE guid[16]; hg(guid);
    void* set = get(guid, 0, 0, 0x12 /*PRESENT | DEVICEINTERFACE*/);
    if (set == INVALID_HANDLE_VALUE || !set) return INVALID_HANDLE_VALUE;
    HANDLE found = INVALID_HANDLE_VALUE;
    for (DWORD i = 0; found == INVALID_HANDLE_VALUE; ++i) {
        SC_DEVIFACE di; memset(&di, 0, sizeof di); di.size = sizeof di;
        if (!en(set, 0, guid, i, &di)) break;
        static BYTE detail[1100]; DWORD need = 0;
        *(DWORD*)detail = 8;   /* cbSize of SP_DEVICE_INTERFACE_DETAIL_DATA_W on x64 */
        if (!det(set, &di, detail, sizeof detail, &need, 0)) continue;
        const WCHAR* path = (const WCHAR*)(detail + 4);
        if (!ps_path_is_sony(path)) continue;
        int kind = ps_pid_kind(path);
        if (!kind) continue;
        HANDLE h = g_real_cfw(path, 0x80000000 | 0x40000000, 1 | 2, 0, 3 /*OPEN_EXISTING*/, 0, 0);
        if (h == INVALID_HANDLE_VALUE) h = g_real_cfw(path, 0x80000000, 1 | 2, 0, 3, 0, 0);
        if (h == INVALID_HANDLE_VALUE) continue;
        if (gf) {   /* ask for the calibration report: Bluetooth pads then switch to the full report */
            BYTE f[80]; memset(f, 0, sizeof f); f[0] = kind == 1 ? 0x05 : 0x02; gf(h, f, kind == 1 ? 41 : 37);
        }
        *kind_out = kind; found = h;
    }
    des(set);
    return found;
}

static DWORD WINAPI ps_thread(void* arg) {
    (void)arg;
    BYTE buf[128];
    for (;;) {
        int kind = 0;
        HANDLE h = g_ps_enabled ? ps_open(&kind) : INVALID_HANDLE_VALUE;
        if (h == INVALID_HANDLE_VALUE) { g_ps_connected = 0; Sleep(2000); continue; }
        g_ps_kind = kind;
        log_text(kind == 1 ? "PlayStation pad: DualSense found, reading it" : "PlayStation pad: DualShock 4 found, reading it");
        for (;;) {
            DWORD got = 0;
            if (!ReadFile(h, buf, sizeof buf, &got, 0) || !got) break;
            ps_report(buf, got, kind);
            g_ps_connected = 1;
        }
        g_ps_connected = 0;
        CloseHandle(h);
        log_text("PlayStation pad: disconnected");
        Sleep(1000);
    }
}

/* Point Skate.exe's CreateFileW / CreateFileA imports at the hooks above. */
static void ps_redirect_createfile(void) {
    BYTE* base = (BYTE*)GetModuleHandleW(0);
    if (!base || base[0] != 'M' || base[1] != 'Z') return;
    BYTE* nt = base + *(DWORD*)(base + 0x3c); BYTE* opt = nt + 24;
    DWORD imp_rva = *(DWORD*)(opt + 112 + 8);
    if (!imp_rva) return;
    UINT64 w[2] = {0, 0}, a[2] = {0, 0};
    HMODULE kb = GetModuleHandleW(L"kernelbase.dll"), k32 = GetModuleHandleW(L"kernel32.dll");
    if (kb) { w[0] = (UINT64)GetProcAddress(kb, "CreateFileW"); a[0] = (UINT64)GetProcAddress(kb, "CreateFileA"); }
    if (k32) { w[1] = (UINT64)GetProcAddress(k32, "CreateFileW"); a[1] = (UINT64)GetProcAddress(k32, "CreateFileA"); }
    g_real_cfw = (PFN_CreateFileW)(w[0] ? w[0] : w[1]);
    g_real_cfa = (PFN_CreateFileA)(a[0] ? a[0] : a[1]);
    int n = 0;
    for (BYTE* d = base + imp_rva; *(DWORD*)(d + 12); d += 20) {
        UINT64* thunk = (UINT64*)(base + *(DWORD*)(d + 16));
        for (; *thunk; ++thunk) {
            UINT64 v = *thunk, to = 0;
            if (v && (v == w[0] || v == w[1])) to = (UINT64)hook_cfw;
            else if (v && (v == a[0] || v == a[1])) to = (UINT64)hook_cfa;
            if (!to) continue;
            DWORD old;
            if (VirtualProtect(thunk, 8, PAGE_READWRITE, &old)) { *thunk = to; VirtualProtect(thunk, 8, old, &old); ++n; }
        }
    }
    log_num("PlayStation pad support: CreateFile import slots redirected: ", (UINT64)n);
}

/* ---------------------------------------------------------------- v5.23 NATIVE PlayStation mode
 * PlayStationPads=2: the game opens and reads the DualSense / DualShock 4 itself, so ITS OWN
 * DualSense support stays on (haptics, adaptive triggers, light bar, PS button prompts: the game's
 * output reports are not touched). Each input report the game reads is decoded, run through the
 * same Dual Stick translation as an Xbox pad, and written back into the game's buffer (sticks,
 * triggers, buttons), so the controls are the mod's while everything else is the game's. */
__declspec(dllimport) DWORD WINAPI GetLastError(void);
typedef BOOL (WINAPI* PFN_ReadFile)(HANDLE, void*, DWORD, DWORD*, void*);
typedef BOOL (WINAPI* PFN_GOR)(HANDLE, void*, DWORD*, BOOL);
typedef BOOL (WINAPI* PFN_GORX)(HANDLE, void*, DWORD*, DWORD, BOOL);
static PFN_ReadFile g_real_rf; static PFN_GOR g_real_gor; static PFN_GORX g_real_gorx;
static struct { HANDLE h; int kind; } g_psn_h[8];
static struct { void* ov; HANDLE h; BYTE* buf; } g_psn_pend[16];
static volatile int g_psn_lock;
static unsigned g_psn_reads, g_psn_patched, g_psn_sync, g_psn_async;
static volatile int g_psn_npend;
static DWORD g_psn_seq;
static void process(DWORD user, XINPUT_STATE* s,int route);

static void ps_native_track(HANDLE h, int kind) {
    for (int i = 0; i < 8; ++i) if (!g_psn_h[i].h || g_psn_h[i].h == h) { g_psn_h[i].h = h; g_psn_h[i].kind = kind; break; }
    log_text(kind == 1 ? "PlayStation pad (native): the game opened a DualSense - its haptics / triggers / light bar stay on"
                       : "PlayStation pad (native): the game opened a DualShock 4");
}
static int psn_kind(HANDLE h) { for (int i = 0; i < 8; ++i) if (g_psn_h[i].h == h) return g_psn_h[i].kind; return 0; }

static UINT psn_crc32(const BYTE* p, DWORD n, UINT crc) {
    for (DWORD i = 0; i < n; ++i) { crc ^= p[i]; for (int k = 0; k < 8; ++k) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1))); }
    return crc;
}
#include "ps_native_write.h"
/* Decode, translate, then encode only if the controller state changed. */
static int psn_patch_layout(BYTE* r,int ds){
    ps_decode(r,ds);
    XINPUT_STATE xs;memset(&xs,0,sizeof xs);
    xs.dwPacketNumber=++g_psn_seq;xs.Gamepad=g_ps_state;
    sc_pad original=xs.Gamepad;
    process(1,&xs,4);
    return sc_ps_native_write(r,ds,original,xs.Gamepad);
}

static void psn_patch(HANDLE h, BYTE* b, DWORD n) {
    int kind = psn_kind(h);
    if (!kind || !b || n < 10) return;
    while (__sync_lock_test_and_set(&g_psn_lock, 1)) Sleep(0);
    sc_ps_frames f=sc_ps_frame(b,n,kind);
    if(!f.count) {
        static int skipped;
        __sync_lock_release(&g_psn_lock);
        if(!skipped++) {
            log_text("PlayStation native report: unsupported framing left untouched");
            log_num("  report id: ",b[0]);log_num("  input report bytes: ",n);
        }
        return;
    }
    static unsigned formats_seen;
    unsigned format=kind==1?(f.dualsense?(f.crc?4u:2u):1u):8u;
    if(!(formats_seen&format)) {
        formats_seen|=format;
        log_text(format==1?"PlayStation native report: DualSense Bluetooth simple":format==2?"PlayStation native report: DualSense USB":format==4?"PlayStation native report: DualSense Bluetooth full":"PlayStation native report: DualShock 4");
        log_num("  input report bytes: ",n);
    }
    if(f.count>1) {
        static int batch_logged;
        if(!batch_logged++)log_num("PlayStation native USB reports per read: ",f.count);
    }
    for(unsigned i=0;i<f.count;i++) {
        BYTE* report=b+i*f.size;
        int changed=psn_patch_layout(report+f.offset,f.dualsense);
        if(f.crc && changed) {
            BYTE seed=0xA1;UINT c=psn_crc32(&seed,1,0xFFFFFFFFu);
            c=psn_crc32(report,74,c)^0xFFFFFFFFu;
            report[74]=(BYTE)c;report[75]=(BYTE)(c>>8);
            report[76]=(BYTE)(c>>16);report[77]=(BYTE)(c>>24);
        }
    }
    ++g_psn_patched;
    g_ps_connected = 1;
    __sync_lock_release(&g_psn_lock);
    if (g_psn_patched == 1) log_text("PlayStation pad (native): reading the game's pad reports and applying Dual Stick to them");
    if (g_psn_patched % 5000 == 2) {   /* v5.36: how the game collects its reads, for the log */
        log_num("PlayStation pad (native): reports patched ", (UINT64)g_psn_patched);
        log_num("   read calls ", (UINT64)g_psn_reads); log_num("   finished at once ", (UINT64)g_psn_sync); log_num("   finished later ", (UINT64)g_psn_async);
    }
}
static void psn_pend_add(void* ov, HANDLE h, BYTE* buf) {
    for (int i = 0; i < 16; ++i) if (g_psn_pend[i].ov == ov) { g_psn_pend[i].h = h; g_psn_pend[i].buf = buf; return; }
    for (int i = 0; i < 16; ++i) if (!g_psn_pend[i].ov) { g_psn_pend[i].ov = ov; g_psn_pend[i].h = h; g_psn_pend[i].buf = buf; ++g_psn_npend; return; }
}
static void psn_pend_done(void* ov, DWORD got) {
    for (int i = 0; i < 16; ++i) if (g_psn_pend[i].ov == ov) {
        HANDLE h = g_psn_pend[i].h; BYTE* buf = g_psn_pend[i].buf; g_psn_pend[i].ov = 0; --g_psn_npend;
        ++g_psn_async; psn_patch(h, buf, got);
        return;
    }
}
static BOOL WINAPI hook_rf(HANDLE h, void* buf, DWORD n, DWORD* got, void* ov) {
    BOOL r = g_real_rf(h, buf, n, got, ov);
    if (!psn_kind(h)) return r;
    DWORD err = GetLastError();
    ++g_psn_reads;
    if (r) { DWORD k = got ? *got : (ov ? (DWORD)((UINT64*)ov)[1] : 0); ++g_psn_sync; psn_patch(h, (BYTE*)buf, k); }
    else if (ov && err == 997 /*ERROR_IO_PENDING*/) psn_pend_add(ov, h, (BYTE*)buf);
    SetLastError(err);
    return r;
}
static BOOL WINAPI hook_gor(HANDLE h, void* ov, DWORD* got, BOOL wait) {
    BOOL r = g_real_gor(h, ov, got, wait);
    if (r && psn_kind(h)) { DWORD err = GetLastError(); psn_pend_done(ov, got ? *got : 0); SetLastError(err); }
    return r;
}
static BOOL WINAPI hook_gorx(HANDLE h, void* ov, DWORD* got, DWORD ms, BOOL alert) {
    BOOL r = g_real_gorx(h, ov, got, ms, alert);
    if (r && psn_kind(h)) { DWORD err = GetLastError(); psn_pend_done(ov, got ? *got : 0); SetLastError(err); }
    return r;
}
/* v5.36: the game does not always collect its pad reads with GetOverlappedResult: it can wait on the
 * read's event (WaitForSingleObject / WaitForMultipleObjects) or a completion port and then use the
 * buffer straight away. After any of those waits returns, every pending read whose I/O has finished
 * (OVERLAPPED.Internal no longer STATUS_PENDING) is patched before the game looks at it. */
typedef DWORD (WINAPI* PFN_WFSO)(HANDLE, DWORD);
typedef DWORD (WINAPI* PFN_WFSOX)(HANDLE, DWORD, BOOL);
typedef DWORD (WINAPI* PFN_WFMO)(DWORD, const HANDLE*, BOOL, DWORD);
typedef BOOL (WINAPI* PFN_GQCS)(HANDLE, DWORD*, UINT64*, void**, DWORD);
typedef BOOL (WINAPI* PFN_GQCSX)(HANDLE, void*, UINT, UINT*, DWORD, BOOL);
static PFN_WFSO g_real_wfso; static PFN_WFSOX g_real_wfsox; static PFN_WFMO g_real_wfmo; static PFN_GQCS g_real_gqcs; static PFN_GQCSX g_real_gqcsx;
static void psn_scan(void) {
    if (!g_psn_npend) return;
    DWORD err = GetLastError();
    for (int i = 0; i < 16; ++i) {
        void* ov = g_psn_pend[i].ov;
        if (!ov) continue;
        UINT64 st = ((volatile UINT64*)ov)[0];
        if (st == 0x103 /*STATUS_PENDING*/) continue;
        HANDLE h = g_psn_pend[i].h; BYTE* buf = g_psn_pend[i].buf; g_psn_pend[i].ov = 0; --g_psn_npend;
        if (st == 0) { ++g_psn_async; psn_patch(h, buf, (DWORD)((volatile UINT64*)ov)[1]); }
    }
    SetLastError(err);
}
static DWORD WINAPI hook_wfso(HANDLE h, DWORD ms) { DWORD r = g_real_wfso(h, ms); psn_scan(); return r; }
static DWORD WINAPI hook_wfsox(HANDLE h, DWORD ms, BOOL a) { DWORD r = g_real_wfsox(h, ms, a); psn_scan(); return r; }
static DWORD WINAPI hook_wfmo(DWORD n, const HANDLE* hs, BOOL all, DWORD ms) { DWORD r = g_real_wfmo(n, hs, all, ms); psn_scan(); return r; }
static BOOL WINAPI hook_gqcs(HANDLE p, DWORD* n, UINT64* key, void** ov, DWORD ms) { BOOL r = g_real_gqcs(p, n, key, ov, ms); psn_scan(); return r; }
static BOOL WINAPI hook_gqcsx(HANDLE p, void* e, UINT c, UINT* n, DWORD ms, BOOL a) { BOOL r = g_real_gqcsx(p, e, c, n, ms, a); psn_scan(); return r; }

/* point Skate.exe's import of one kernel32 / kernelbase function at a hook */
static int psn_patch_import(const char* fn, void* hook, void** real) {
    BYTE* base = (BYTE*)GetModuleHandleW(0);
    if (!base || base[0] != 'M' || base[1] != 'Z') return 0;
    BYTE* nt = base + *(DWORD*)(base + 0x3c); BYTE* opt = nt + 24;
    DWORD imp_rva = *(DWORD*)(opt + 112 + 8);
    if (!imp_rva) return 0;
    HMODULE kb = GetModuleHandleW(L"kernelbase.dll"), k32 = GetModuleHandleW(L"kernel32.dll");
    UINT64 f[2] = {kb ? (UINT64)GetProcAddress(kb, fn) : 0, k32 ? (UINT64)GetProcAddress(k32, fn) : 0};
    *real = (void*)(f[0] ? f[0] : f[1]);
    int n = 0;
    for (BYTE* d = base + imp_rva; *(DWORD*)(d + 12); d += 20) {
        UINT64* thunk = (UINT64*)(base + *(DWORD*)(d + 16));
        for (; *thunk; ++thunk) {
            UINT64 v = *thunk;
            if (!v || (v != f[0] && v != f[1])) continue;
            DWORD old;
            if (VirtualProtect(thunk, 8, PAGE_READWRITE, &old)) { *thunk = (UINT64)hook; VirtualProtect(thunk, 8, old, &old); ++n; }
        }
    }
    return n;
}
static void ps_native_setup(void) {
    int a = psn_patch_import("ReadFile", (void*)hook_rf, (void**)&g_real_rf);
    int b = psn_patch_import("GetOverlappedResult", (void*)hook_gor, (void**)&g_real_gor);
    int c = psn_patch_import("GetOverlappedResultEx", (void*)hook_gorx, (void**)&g_real_gorx);
    log_num("PlayStation pad (native): ReadFile slots ", (UINT64)a);
    log_num("PlayStation pad (native): GetOverlappedResult slots ", (UINT64)(b + c));
    int d = psn_patch_import("WaitForSingleObject", (void*)hook_wfso, (void**)&g_real_wfso);
    d += psn_patch_import("WaitForSingleObjectEx", (void*)hook_wfsox, (void**)&g_real_wfsox);
    d += psn_patch_import("WaitForMultipleObjects", (void*)hook_wfmo, (void**)&g_real_wfmo);
    d += psn_patch_import("GetQueuedCompletionStatus", (void*)hook_gqcs, (void**)&g_real_gqcs);
    d += psn_patch_import("GetQueuedCompletionStatusEx", (void*)hook_gqcsx, (void**)&g_real_gqcsx);
    log_num("PlayStation pad (native): wait / completion slots ", (UINT64)d);
}

static void ps_start(void) {
    if (g_ps_started || g_ps_enabled != 1) return;   /* NATIVE (2): the game reads the pad, not us */
    g_ps_started = 1;
    DWORD id; HANDLE th = CreateThread(0, 0, ps_thread, 0, 0, &id);
    if (th) CloseHandle(th);
}

/* Fill an XInput state from the PlayStation pad (controller 0 only). */
static int ps_fill(DWORD user, XINPUT_STATE* s) {
    if (g_ps_enabled != 1 || user != 0 || !g_ps_connected || !s) return 0;
    s->dwPacketNumber = (DWORD)g_ps_seq;
    s->Gamepad = g_ps_state;
    return 1;
}
