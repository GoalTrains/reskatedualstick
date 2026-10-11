/* v4.0 IN-GAME MENU: the End menu is drawn straight into the game's own picture, so it no longer
 * needs a window on top of the game (a window on top knocked the game out of fullscreen).
 *
 * How: at start a tiny hidden D3D12 swap chain is made, only to find DXGI's Present / Present1 in
 * its function table; those two entries are pointed at ov_present / ov_present1 (they are shared by
 * every D3D12 swap chain, the game's included). On each Present while the menu is open, the menu
 * picture (drawn with GDI into memory by the menu thread) is copied onto the game's back buffer
 * with one CopyTextureRegion on the game's own command queue, then the real Present runs.
 * If any of this fails (not D3D12, unknown back buffer format...), the old window is used instead.
 * MenuInGame=0 in the ini turns it off. */
#ifndef SC_INGAME_H
#define SC_INGAME_H

typedef long HRESULT;
typedef struct { DWORD a; WORD b, c; BYTE d[8]; } SC_GUID;
#define VT(o) (*(void***)(o))
#define COMREL(o) do { if (o) { ((UINT (*)(void*))VT(o)[2])(o); (o) = 0; } } while (0)

static const SC_GUID IID_Queue   = {0x0ec870a6, 0x5d7e, 0x4c22, {0x8c, 0xfc, 0x5b, 0xaa, 0xe0, 0x76, 0x16, 0xed}};
static const SC_GUID IID_Dev     = {0x189819f1, 0x1db6, 0x4b57, {0xbe, 0x54, 0x18, 0x21, 0x33, 0x9b, 0x85, 0xf7}};
static const SC_GUID IID_Alloc   = {0x6102dee4, 0xaf59, 0x4b09, {0xb9, 0x99, 0xb4, 0x4d, 0x73, 0xf0, 0x9b, 0x24}};
static const SC_GUID IID_GList   = {0x5b160d0f, 0xac1b, 0x4185, {0x8b, 0xa8, 0xb3, 0xae, 0x42, 0xa5, 0xa4, 0x55}};
static const SC_GUID IID_Fence   = {0x0a753dcf, 0xc4d8, 0x4b91, {0xad, 0xf6, 0xbe, 0x5a, 0x60, 0xd9, 0x5a, 0x76}};
static const SC_GUID IID_Res     = {0x696442be, 0xa72e, 0x4059, {0xbc, 0x79, 0x5b, 0x5c, 0x98, 0x04, 0x0f, 0xad}};
static const SC_GUID IID_Fact2   = {0x50c83a1c, 0xe072, 0x4c48, {0x87, 0xb0, 0x36, 0x30, 0xfa, 0x36, 0xa6, 0xd0}};
static const SC_GUID IID_Swap3   = {0x94d99bdb, 0xf1f8, 0x4ab0, {0xb2, 0x36, 0x7d, 0xa0, 0x17, 0x0e, 0xda, 0xb1}};

typedef struct { UINT w, h, rn, rd, fmt, scan, scale; UINT sc, sq, usage, count; void* hwnd; BOOL windowed; UINT effect, flags; } SC_SWAPDESC;
typedef struct { UINT w, h, fmt; BOOL stereo; UINT sc, sq, usage, count, scaling, effect, alpha, flags; } SC_SWAPDESC1;
typedef struct { UINT type, cpu, pool, cnode, vnode; } SC_HEAPPROPS;
typedef struct { UINT dim; UINT64 align, width; UINT height; WORD depth, mips; UINT fmt, sc, sq, layout, flags; } SC_RESDESC;
typedef struct { void* res; UINT type; UINT pad; UINT64 offset; UINT fmt, w, h, d, pitch; UINT pad2; } SC_COPYLOC;  /* placed footprint */
typedef struct { void* res; UINT type; UINT pad; UINT sub; BYTE pad2[28]; } SC_COPYLOC_SUB;
typedef struct { UINT type, flags; void* res; UINT sub, before, after, pad; } SC_BARRIER;
typedef struct { UINT type; int prio; UINT flags, node; } SC_QUEUEDESC;

/* the menu picture, drawn by the menu thread */
static volatile int g_ov_ver;             /* bumped every time the picture changes */
static DWORD* g_ov_pix; static int g_ov_w, g_ov_h, g_ov_cap;
static volatile int g_ov_busy;            /* picture being written */
static volatile int g_ov_ok;              /* the in-game menu has drawn at least once */
static volatile double g_ov_last;         /* when the game last presented (ms) */

static INT64 g_ov_qpf;

static HRESULT (*o_present)(void*, UINT, UINT);
static void (*o_ecl)(void*, UINT, void* const*);
static void* volatile g_ov_cq;            /* v4.2: the game's last DIRECT command queue (seen in ExecuteCommandLists) */
static HRESULT (*o_present1)(void*, UINT, UINT, const void*);

/* render state, owned by the game's render thread */
static void *ov_sc, *ov_queue, *ov_dev, *ov_alloc, *ov_list, *ov_fence, *ov_up;
static UINT64 ov_fval, ov_upsize; static BYTE* ov_upmap; static int ov_upver = -1, ov_upfmt = -1, ov_failed, ov_logged_fmt;
static UINT ov_pitch;
static void ov_ecl(void* q, UINT n, void* const* lists) {
    /* a direct command list (type 0) means a direct queue: remember it (no reference kept) */
    if (n && lists && lists[0] && q != ov_queue && ((UINT (*)(void*))VT(lists[0])[8])(lists[0]) == 0) g_ov_cq = q;
    o_ecl(q, n, lists);
}

static double ov_now(void) { INT64 t = 0; QueryPerformanceCounter(&t); if (!g_ov_qpf) QueryPerformanceFrequency(&g_ov_qpf); return g_ov_qpf ? (double)t * 1000.0 / (double)g_ov_qpf : 0; }

static WORD ov_half(float f) {
    union { float f; UINT u; } v; v.f = f;
    UINT u = v.u, s = (u >> 16) & 0x8000; int e = (int)((u >> 23) & 0xff) - 127 + 15; UINT m = u & 0x7fffff;
    if (e <= 0) return (WORD)s;
    if (e >= 31) return (WORD)(s | 0x7c00);
    return (WORD)(s | (e << 10) | (m >> 13));
}
static float ov_lin(int c) { float x = c / 255.0f; return x <= 0.04045f ? x / 12.92f : x * x * (0.6f + 0.4f * x); } /* close enough to sRGB->linear */

static int ov_bpp(UINT fmt) {
    switch (fmt) { case 28: case 29: case 87: case 91: case 24: return 4; case 10: return 8; default: return 0; }
}
static void ov_release(void) {
    COMREL(ov_up); ov_upmap = 0; ov_upsize = 0; COMREL(ov_list); COMREL(ov_alloc); COMREL(ov_fence); COMREL(ov_dev); COMREL(ov_queue);
    ov_sc = 0; ov_upver = -1; ov_fval = 0;
}
static int ov_step; static HRESULT ov_hr;
static int ov_init(void* sc) {
    ov_release(); ov_step = 0;
    ov_step = 1; if ((ov_hr = ((HRESULT (*)(void*, const SC_GUID*, void**))VT(sc)[7])(sc, &IID_Queue, &ov_queue)) < 0 || !ov_queue) {
        /* v4.2: the swap chain would not say which queue it uses (a layer like NVIDIA Streamline sits in
         * between): use the game's direct queue seen in ExecuteCommandLists instead */
        ov_queue = 0;
        void* q = g_ov_cq;
        if (!q) return 0;
        ((UINT (*)(void*))VT(q)[1])(q); ov_queue = q; ov_hr = 0;
    }
    ov_step = 2; if ((ov_hr = ((HRESULT (*)(void*, const SC_GUID*, void**))VT(ov_queue)[7])(ov_queue, &IID_Dev, &ov_dev)) < 0 || !ov_dev) { ov_dev = 0; return 0; }
    ov_step = 3; if ((ov_hr = ((HRESULT (*)(void*, UINT, const SC_GUID*, void**))VT(ov_dev)[9])(ov_dev, 0, &IID_Alloc, &ov_alloc)) < 0) { ov_alloc = 0; return 0; }
    ov_step = 4; if ((ov_hr = ((HRESULT (*)(void*, UINT, UINT, void*, void*, const SC_GUID*, void**))VT(ov_dev)[12])(ov_dev, 0, 0, ov_alloc, 0, &IID_GList, &ov_list)) < 0) { ov_list = 0; return 0; }
    ((HRESULT (*)(void*))VT(ov_list)[9])(ov_list);   /* Close: it is reset before each use */
    ov_step = 5; if ((ov_hr = ((HRESULT (*)(void*, UINT64, UINT, const SC_GUID*, void**))VT(ov_dev)[36])(ov_dev, 0, 0, &IID_Fence, &ov_fence)) < 0) { ov_fence = 0; return 0; }
    ov_sc = sc;
    return 1;
}
static volatile long g_ov_lock;           /* v4.4: guards g_ov_pix / g_ov_w / g_ov_h / g_ov_ver */
static int ov_sw, ov_sh;                    /* size of the picture that is in the upload buffer now */
static int ov_lock_try(void) { return __sync_lock_test_and_set(&g_ov_lock, 1) == 0; }
static void ov_unlock(void) { __sync_lock_release(&g_ov_lock); }
/* copy the menu picture into the upload buffer (only when it changed). Everything the copy uses
 * afterwards (size, row pitch) comes from this snapshot, so a size change from the menu thread can
 * never make the GPU or this code read past a buffer (v4.3 crashed changing the menu size). */
static int ov_upload(UINT fmt) {
    if (ov_upver == g_ov_ver && ov_upfmt == (int)fmt && ov_up) return 1;
    if (!ov_lock_try()) return ov_up && ov_upver >= 0 && ov_upfmt == (int)fmt;   /* being redrawn: keep the last one */
    int w = g_ov_w, h = g_ov_h, ver = g_ov_ver, ok = 0;
    int bpp = ov_bpp(fmt);
    UINT pitch = ((UINT)(w * bpp) + 255u) & ~255u;
    UINT64 need = (UINT64)pitch * (UINT64)h;
    if (w > 0 && h > 0 && bpp) {
        if (need > ov_upsize) {
            COMREL(ov_up); ov_upmap = 0; ov_upsize = 0; ov_upver = -1;   /* (the previous copy has finished: fence waited) */
            SC_HEAPPROPS hp = {2 /*UPLOAD*/, 0, 0, 1, 1};
            SC_RESDESC rd; memset(&rd, 0, sizeof rd);
            rd.dim = 1 /*BUFFER*/; rd.width = need; rd.height = 1; rd.depth = 1; rd.mips = 1; rd.sc = 1; rd.layout = 1 /*ROW_MAJOR*/;
            void* p = 0;
            if (((HRESULT (*)(void*, const SC_HEAPPROPS*, UINT, const SC_RESDESC*, UINT, const void*, const SC_GUID*, void**))VT(ov_dev)[27])
                    (ov_dev, &hp, 0, &rd, 0x0AC3 /*GENERIC_READ*/, 0, &IID_Res, &ov_up) >= 0 && ov_up &&
                ((HRESULT (*)(void*, UINT, const void*, void**))VT(ov_up)[8])(ov_up, 0, 0, &p) >= 0 && p) {
                ov_upmap = (BYTE*)p; ov_upsize = need;
            } else COMREL(ov_up);
        }
        if (ov_upmap) {
            for (int y = 0; y < h; ++y) {
                const DWORD* src = g_ov_pix + y * w;
                BYTE* row = ov_upmap + (UINT64)y * pitch;
                for (int x = 0; x < w; ++x) {
                    DWORD c = src[x]; int b = c & 255, g = (c >> 8) & 255, r = (c >> 16) & 255;
                    switch (fmt) {
                    case 87: case 91: ((DWORD*)row)[x] = 0xff000000u | (r << 16) | (g << 8) | b; break;
                    case 28: case 29: ((DWORD*)row)[x] = 0xff000000u | (b << 16) | (g << 8) | r; break;
                    case 24: ((DWORD*)row)[x] = 0xC0000000u | ((UINT)(b * 1023 / 255) << 20) | ((UINT)(g * 1023 / 255) << 10) | (UINT)(r * 1023 / 255); break;
                    case 10: { WORD* q = (WORD*)row + x * 4; q[0] = ov_half(ov_lin(r)); q[1] = ov_half(ov_lin(g)); q[2] = ov_half(ov_lin(b)); q[3] = 0x3c00; break; }
                    }
                }
            }
            ov_pitch = pitch; ov_sw = w; ov_sh = h; ov_upver = ver; ov_upfmt = (int)fmt; ok = 1;
        }
    }
    ov_unlock();
    return ok;
}
static void ov_draw(void* sc) {
    if (ov_failed || !g_ov_pix || g_ov_w <= 0) return;
    if (sc != ov_sc && !ov_init(sc)) {
        ov_failed = 1; ov_release();
        char line[160]; unsigned n = 0; const char* a = "in-game menu: could not use the game's renderer, step ";
        while (*a) line[n++] = *a++;
        line[n++] = (char)('0' + ov_step); a = ", code 0x"; while (*a) line[n++] = *a++;
        for (int k = 7; k >= 0; --k) line[n++] = "0123456789abcdef"[((UINT)ov_hr >> (k * 4)) & 15];
        a = " (menu window used instead)"; while (*a) line[n++] = *a++;
        line[n] = 0; log_text(line);
        return;
    }
    SC_SWAPDESC d; memset(&d, 0, sizeof d);
    if (((HRESULT (*)(void*, SC_SWAPDESC*))VT(sc)[12])(sc, &d) < 0) return;
    if (!ov_bpp(d.fmt)) {
        if (!ov_logged_fmt) { ov_logged_fmt = 1; ov_failed = 1; log_text("in-game menu: unknown back buffer format (menu window used instead)"); }
        return;
    }
    void* sc3 = 0;
    if (((HRESULT (*)(void*, const SC_GUID*, void**))VT(sc)[0])(sc, &IID_Swap3, &sc3) < 0 || !sc3) return;
    UINT idx = ((UINT (*)(void*))VT(sc3)[36])(sc3);
    COMREL(sc3);
    void* bb = 0;
    if (((HRESULT (*)(void*, UINT, const SC_GUID*, void**))VT(sc)[9])(sc, idx, &IID_Res, &bb) < 0 || !bb) return;
    /* the previous copy must be finished before its allocator is reused */
    if (ov_fval && ((UINT64 (*)(void*))VT(ov_fence)[8])(ov_fence) < ov_fval)
        ((HRESULT (*)(void*, UINT64, void*))VT(ov_fence)[9])(ov_fence, ov_fval, 0);
    if (!ov_upload(d.fmt)) { COMREL(bb); return; }
    int w = ov_sw, h = ov_sh;
    int x0 = (int)d.w / 40, y0 = (int)d.h / 25;
    if (x0 + w > (int)d.w) w = (int)d.w - x0;
    if (y0 + h > (int)d.h) h = (int)d.h - y0;
    if (w <= 0 || h <= 0) { COMREL(bb); return; }
    if (((HRESULT (*)(void*))VT(ov_alloc)[8])(ov_alloc) < 0) { COMREL(bb); return; }
    if (((HRESULT (*)(void*, void*, void*))VT(ov_list)[10])(ov_list, ov_alloc, 0) < 0) { COMREL(bb); return; }
    SC_BARRIER b; memset(&b, 0, sizeof b);
    b.type = 0; b.res = bb; b.sub = 0xffffffffu; b.before = 0 /*PRESENT*/; b.after = 0x400 /*COPY_DEST*/;
    ((void (*)(void*, UINT, const SC_BARRIER*))VT(ov_list)[26])(ov_list, 1, &b);
    SC_COPYLOC src; memset(&src, 0, sizeof src);
    src.res = ov_up; src.type = 1 /*PLACED_FOOTPRINT*/; src.offset = 0; src.fmt = d.fmt; src.w = (UINT)ov_sw; src.h = (UINT)ov_sh; src.d = 1; src.pitch = ov_pitch;
    SC_COPYLOC_SUB dst; memset(&dst, 0, sizeof dst); dst.res = bb; dst.type = 0 /*SUBRESOURCE_INDEX*/; dst.sub = 0;
    UINT box[6] = {0, 0, 0, (UINT)w, (UINT)h, 1};
    ((void (*)(void*, const void*, UINT, UINT, UINT, const void*, const UINT*))VT(ov_list)[16])(ov_list, &dst, (UINT)x0, (UINT)y0, 0, &src, box);
    b.before = 0x400; b.after = 0;
    ((void (*)(void*, UINT, const SC_BARRIER*))VT(ov_list)[26])(ov_list, 1, &b);
    ((HRESULT (*)(void*))VT(ov_list)[9])(ov_list);
    void* lists[1] = {ov_list};
    ((void (*)(void*, UINT, void* const*))VT(ov_queue)[10])(ov_queue, 1, lists);
    ++ov_fval;
    ((HRESULT (*)(void*, void*, UINT64))VT(ov_queue)[14])(ov_queue, ov_fence, ov_fval);
    COMREL(bb);
    if (!g_ov_ok) { g_ov_ok = 1; log_text("in-game menu: drawing inside the game (fullscreen stays on)"); }
}
static HRESULT ov_present(void* sc, UINT sync, UINT flags) {
    g_ov_last = ov_now();
    if (g_ui_open && g_ov_enabled && !(flags & 1 /*TEST*/)) ov_draw(sc);
    return o_present(sc, sync, flags);
}
static HRESULT ov_present1(void* sc, UINT sync, UINT flags, const void* params) {
    g_ov_last = ov_now();
    if (g_ui_open && g_ov_enabled && !(flags & 1)) ov_draw(sc);
    return o_present1(sc, sync, flags, params);
}

/* find DXGI's Present through a throwaway D3D12 swap chain and hook it */
static int ov_hook(void) {
    /* Never create a probe swapchain or replace shared DXGI vtables while
     * ReSkate owns them, including existing INIs with MenuInGame=1. */
    if (!sc_allow_menu_graphics_hook(1, GetModuleHandleW(L"ReSkate.dll") != 0)) return 0;
    HMODULE d12 = LoadLibraryW(L"d3d12.dll"), dx = LoadLibraryW(L"dxgi.dll");
    if (!d12 || !dx) return 0;
    HRESULT (WINAPI* mkdev)(void*, UINT, const SC_GUID*, void**) = (HRESULT (WINAPI*)(void*, UINT, const SC_GUID*, void**))GetProcAddress(d12, "D3D12CreateDevice");
    HRESULT (WINAPI* mkfac)(const SC_GUID*, void**) = (HRESULT (WINAPI*)(const SC_GUID*, void**))GetProcAddress(dx, "CreateDXGIFactory1");
    if (!mkdev || !mkfac) return 0;
    void *dev = 0, *q = 0, *fac = 0, *sc = 0; int ok = 0;
    SC_WNDCLASS wc; memset(&wc, 0, sizeof wc);
    wc.size = sizeof wc; wc.proc = DefWindowProcW; wc.inst = g_self; wc.name = L"DualStickControlsProbe";
    RegisterClassExW(&wc);
    HWND hw = CreateWindowExW(0, L"DualStickControlsProbe", L"", 0x80000000 /*POPUP*/, 0, 0, 8, 8, 0, 0, g_self, 0);
    if (hw && mkdev(0, 0xb000 /*FL 11_0*/, &IID_Dev, &dev) >= 0 && dev) {
        SC_QUEUEDESC qd = {0, 0, 0, 0};
        if (((HRESULT (*)(void*, const SC_QUEUEDESC*, const SC_GUID*, void**))VT(dev)[8])(dev, &qd, &IID_Queue, &q) >= 0 && q &&
            mkfac(&IID_Fact2, &fac) >= 0 && fac) {
            SC_SWAPDESC1 sd; memset(&sd, 0, sizeof sd);
            sd.w = 8; sd.h = 8; sd.fmt = 87; sd.sc = 1; sd.usage = 0x20; sd.count = 2; sd.effect = 4 /*FLIP_DISCARD*/;
            if (((HRESULT (*)(void*, void*, HWND, const SC_SWAPDESC1*, const void*, void*, void**))VT(fac)[15])(fac, q, hw, &sd, 0, 0, &sc) >= 0 && sc) {
                void** vt = VT(sc); DWORD old;
                /* the command queue's ExecuteCommandLists, to learn the game's queue */
                void** qvt = VT(q); DWORD qold;
                if (VirtualProtect(&qvt[10], sizeof(void*), 0x40, &qold)) {
                    o_ecl = (void (*)(void*, UINT, void* const*))qvt[10];
                    qvt[10] = (void*)ov_ecl;
                    VirtualProtect(&qvt[10], sizeof(void*), qold, &qold);
                }
                if (VirtualProtect(&vt[8], 15 * sizeof(void*), 0x40, &old)) {
                    o_present = (HRESULT (*)(void*, UINT, UINT))vt[8];
                    o_present1 = (HRESULT (*)(void*, UINT, UINT, const void*))vt[22];
                    vt[8] = (void*)ov_present;
                    vt[22] = (void*)ov_present1;
                    VirtualProtect(&vt[8], 15 * sizeof(void*), old, &old);
                    ok = 1;
                }
            }
        }
    }
    COMREL(sc); COMREL(fac); COMREL(q); COMREL(dev);
    /* (the probe window is left hidden: destroying it is not needed) */
    return ok;
}
#endif
