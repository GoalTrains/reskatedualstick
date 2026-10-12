/* v0.24 in-game settings menu.
 * Open / close with the MenuKey (End by default, rebindable in session_controls.ini).
 * A game-owned window that never takes focus away from the game. While it is open the
 * controller drives it (D-pad up / down = choose, D-pad left / right or A = change, B = close)
 * and the game gets no controller input. Arrow keys / Enter work too. Every change is written to
 * session_controls.ini, so it sticks. Included by version_proxy.c after the config code. */

typedef void* HWND; typedef void* HDC; typedef void* HGDIOBJ; typedef void* HBRUSH;
typedef struct { int left, top, right, bottom; } SC_RECT;
typedef struct { HDC hdc; BOOL erase; SC_RECT rc; BOOL restore, incupdate; BYTE reserved[32]; } SC_PAINT;
typedef struct { HWND hwnd; UINT message; UINT64 wparam; INT64 lparam; DWORD time; int x, y; DWORD priv; } SC_MSG;
typedef INT64 (WINAPI* SC_WNDPROC)(HWND, UINT, UINT64, INT64);
typedef struct { UINT size, style; SC_WNDPROC proc; int cls_extra, wnd_extra; HMODULE inst; void* icon; void* cursor; HBRUSH bg;
                 const WCHAR* menu; const WCHAR* name; void* icon_sm; } SC_WNDCLASS;

__declspec(dllimport) WORD WINAPI RegisterClassExW(const SC_WNDCLASS*);
__declspec(dllimport) HWND WINAPI CreateWindowExW(DWORD, const WCHAR*, const WCHAR*, DWORD, int, int, int, int, HWND, void*, HMODULE, void*);
__declspec(dllimport) INT64 WINAPI DefWindowProcW(HWND, UINT, UINT64, INT64);
__declspec(dllimport) BOOL WINAPI ShowWindow(HWND, int);
__declspec(dllimport) BOOL WINAPI PeekMessageW(SC_MSG*, HWND, UINT, UINT, UINT);
__declspec(dllimport) BOOL WINAPI TranslateMessage(const SC_MSG*);
__declspec(dllimport) INT64 WINAPI DispatchMessageW(const SC_MSG*);
__declspec(dllimport) BOOL WINAPI InvalidateRect(HWND, const SC_RECT*, BOOL);
__declspec(dllimport) HDC WINAPI BeginPaint(HWND, SC_PAINT*);
__declspec(dllimport) BOOL WINAPI EndPaint(HWND, const SC_PAINT*);
__declspec(dllimport) int WINAPI FillRect(HDC, const SC_RECT*, HBRUSH);
__declspec(dllimport) int WINAPI DrawTextW(HDC, const WCHAR*, int, SC_RECT*, UINT);
__declspec(dllimport) short WINAPI GetAsyncKeyState(int);
__declspec(dllimport) BOOL WINAPI SetLayeredWindowAttributes(HWND, DWORD, BYTE, DWORD);
__declspec(dllimport) BOOL WINAPI SetWindowPos(HWND, HWND, int, int, int, int, UINT);
__declspec(dllimport) HWND WINAPI GetForegroundWindow(void);
__declspec(dllimport) DWORD WINAPI GetWindowThreadProcessId(HWND,DWORD*);
__declspec(dllimport) DWORD WINAPI GetCurrentProcessId(void);
__declspec(dllimport) BOOL WINAPI IsWindow(HWND);
__declspec(dllimport) BOOL WINAPI GetWindowRect(HWND,SC_RECT*);
__declspec(dllimport) INT64 WINAPI SetWindowLongPtrW(HWND,int,INT64);
__declspec(dllimport) HGDIOBJ WINAPI CreateFontW(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, const WCHAR*);
__declspec(dllimport) HGDIOBJ WINAPI SelectObject(HDC, HGDIOBJ);
__declspec(dllimport) DWORD WINAPI SetTextColor(HDC, DWORD);
__declspec(dllimport) int WINAPI SetBkMode(HDC, int);
__declspec(dllimport) HBRUSH WINAPI CreateSolidBrush(DWORD);
__declspec(dllimport) BOOL WINAPI DeleteObject(HGDIOBJ);
__declspec(dllimport) HANDLE WINAPI CreateThread(void*, UINT64, DWORD (WINAPI*)(void*), void*, DWORD, DWORD*);
__declspec(dllimport) void WINAPI Sleep(DWORD);
__declspec(dllimport) HDC WINAPI CreateCompatibleDC(HDC);
__declspec(dllimport) HGDIOBJ WINAPI CreateDIBSection(HDC, const void*, UINT, void**, HANDLE, DWORD);
__declspec(dllimport) BOOL WINAPI DeleteDC(HDC);
__declspec(dllimport) BOOL WINAPI GdiFlush(void);
__declspec(dllimport) BOOL WINAPI Ellipse(HDC, int, int, int, int);
__declspec(dllimport) BOOL WINAPI MoveToEx(HDC, int, int, void*);
__declspec(dllimport) BOOL WINAPI LineTo(HDC, int, int);
typedef struct { int x, y; } SC_PT;
__declspec(dllimport) BOOL WINAPI Polygon(HDC, const SC_PT*, int);
__declspec(dllimport) HGDIOBJ WINAPI CreatePen(int, int, DWORD);
__declspec(dllimport) HGDIOBJ WINAPI GetStockObject(int);

#define RGB3(r, g, b) ((DWORD)(r) | ((DWORD)(g) << 8) | ((DWORD)(b) << 16))

static volatile int g_ui_open, g_ui_sel, g_ui_dirty = 1;
/* g_ui_scale (percent) lives in version_proxy.c: MenuScale in the ini */
static volatile int g_ui_req_toggle, g_ui_req_nav, g_ui_req_change, g_ui_req_cards; /* from the keyboard thread */
static int g_ui_key = 0x23;                /* VK_END */
static HWND g_ui_hwnd;
static int g_ui_started;
#include "reskate_compat.h"
#include "ingame.h"
static volatile int g_reskate_menu_visible;
static HWND ui_foreground_game(void) {
    HWND foreground=GetForegroundWindow();DWORD pid=0;
    if(foreground)GetWindowThreadProcessId(foreground,&pid);
    return sc_menu_has_focus(pid,GetCurrentProcessId()) ? foreground : 0;
}


/* v5.12: PURE only. Trick input and Kick / heel sides rows removed; Menu size moved to the bottom. */
enum { UI_PRESET_SESSION, UI_PRESET_XL, UI_SESSION, UI_STANCE, UI_LAYOUT, UI_SWITCH, UI_POPSTYLE, UI_ONEFOOT, UI_STEER, UI_REVERTS, UI_WINDUP, UI_FLIPS, UI_AIR, UI_SLIDES, UI_BUMP, UI_SIZE, UI_CLOSE, UI_ADVANCED, UI_BACK, UI_CUSTOM, UI_SLOT, UI_SAVE, UI_LOAD, UI_STARTUP, UI_HOTKEY, UI_STOCKAIR, UI_RAMPDIAG, UI_COUNT };
/* v5.8: wider menu, an explanation box for the highlighted line, and a controls page */
#define UI_SETTINGS_W 700
#define UI_CONTROLS_W 700
#define UI_CONTROLS_X (UI_SETTINGS_W + 8)
#define UI_W (UI_CONTROLS_X + UI_CONTROLS_W)
#define UI_INFO 104
#define UI_HS (48 + 14 * 30 + 10 + UI_INFO + 34)
#define UI_CARDS_PER_PAGE 6
#define UI_HC (66 + 3 * 232 + 54)
#define UI_H (UI_HS > UI_HC ? UI_HS : UI_HC)
static volatile int g_ui_page;   /* 0 = presets/settings, 1 = controls, 2 = advanced, 3 = custom presets */
static int ui_height(void) { return UI_HC; }
static volatile int g_ui_draw_scale=100;
#include "menu_controls.h"
static int g_ui_controls_page;   /* six large cards per page */

static const int k_ui_main[] = {UI_PRESET_SESSION, UI_PRESET_XL, UI_CUSTOM, UI_SESSION, UI_STANCE, UI_ADVANCED, UI_STARTUP, UI_HOTKEY, UI_SIZE, UI_CLOSE};
static const int k_ui_advanced[] = {UI_LAYOUT, UI_SWITCH, UI_POPSTYLE, UI_ONEFOOT, UI_STEER, UI_REVERTS, UI_WINDUP, UI_FLIPS, UI_AIR, UI_SLIDES, UI_BUMP, UI_STOCKAIR, UI_RAMPDIAG, UI_BACK};
static int g_ui_slot = 1, g_ui_overwrite;
static int g_ui_startup_input_sync;
static const WCHAR* g_ui_preset_status = L"";
#include "menu_hotkey.h"
static const WCHAR* ui_slot_section(void) {
    static const WCHAR* const names[] = {L"CustomPreset1",L"CustomPreset2",L"CustomPreset3",L"CustomPreset4",L"CustomPreset5"};
    return names[g_ui_slot-1];
}
static int ui_slot_filled(void) { return GetPrivateProfileIntW(ui_slot_section(),L"PresetSaved",0,g_ini)==1; }
static const int k_ui_custom[] = {UI_SLOT, UI_SAVE, UI_LOAD, UI_BACK};
static int ui_row_count(void) { return g_ui_page == 3 ? 4 : g_ui_page == 2 ? (g_cfg.front_back?13:14) : (int)(sizeof k_ui_main/sizeof k_ui_main[0]); }
static const WCHAR* ui_reopen_text(int welcome) {
    static WCHAR texts[2][360];WCHAR* text=texts[welcome?1:0];unsigned n=0;
    const WCHAR* a=welcome?L"Welcome! Choose a preset, or customize in Advanced settings and save a custom preset. Use arrows / D-pad and A / Enter. B closes this menu. Press ":L"A / Enter: apply or open    B: close    Press ";
    while(*a && n<330)text[n++]=*a++;
    a=g_menu_key_name;while(*a && n<330)text[n++]=*a++;
    a=welcome?L" to reopen it anytime. Turn Show menu on startup OFF to skip it next launch.":L" to reopen";
    while(*a && n<359)text[n++]=*a++;text[n]=0;return text;
}
static void ui_startup_open(void) {
    if(!g_show_startup_menu)return;
    g_ui_open=1;g_ui_page=0;g_ui_sel=0;g_ui_dirty=1;
    g_ui_startup_input_sync=1;
    g_ui_preset_status=ui_reopen_text(1);
    log_text("menu: opened on startup; disable Show menu on startup to skip future launches");
}
static int ui_item_for_row(int row) {
    if (row < 0 || row >= ui_row_count()) row = 0;
    if(g_ui_page==2) return k_ui_advanced[row+(g_cfg.front_back && row>=1)];
    return g_ui_page == 3 ? k_ui_custom[row] : k_ui_main[row];
}
static int ui_preset_active(int xl) {
    return g_cfg.front_back == !xl && g_cfg.switch_flip == (xl ? 3 : 1) &&
           g_cfg.pop_on_release == xl && g_cfg.stick_steer == xl && g_cfg.one_foot_flips == xl &&
           g_switch_src == 4 && !g_cfg.feet_swapped;
}
static void ui_value(int item, WCHAR* out) {
    const char* v = "";
    switch (item) {
    case UI_SIZE: { int sc = g_ui_scale, n = 0; char t[8]; if (sc >= 100) t[n++] = (char)('0' + sc / 100); t[n++] = (char)('0' + sc / 10 % 10); t[n++] = (char)('0' + sc % 10); t[n++] = '%'; t[n] = 0;
                    for (int i = 0; i <= n; ++i) out[i] = (WCHAR)t[i]; return; }
    case UI_PRESET_SESSION: v = ui_preset_active(0) ? "ACTIVE" : "APPLY"; break;
    case UI_PRESET_XL: v = ui_preset_active(1) ? "ACTIVE" : "APPLY"; break;
    case UI_CUSTOM: v = "OPEN"; break;
    case UI_HOTKEY: {
        const WCHAR* name=g_ui_bind_mode?L"PRESS KEY...":g_menu_key_name;
        int i=0;while(name[i] && i<15) {out[i]=name[i];i++;}out[i]=0;return;
    }
    case UI_STOCKAIR: v = !g_cfg.stock_grab_air ? "OFF" : g_cfg.stock_transition_ground ? "EARLY ON RAMP" : "AIR ONLY"; break;
    case UI_RAMPDIAG: v = g_ramp_diag==1 ? "RAMP" : g_ramp_diag==2 ? "FLAT" : "OFF"; break;
    case UI_STARTUP: v = g_show_startup_menu ? "ON" : "OFF"; break;
    case UI_SLOT: { const char* slots[]={"1 / 5","2 / 5","3 / 5","4 / 5","5 / 5"}; v=slots[g_ui_slot-1]; break; }
    case UI_SAVE: v = g_ui_overwrite==g_ui_slot ? "CONFIRM SAVE" : ui_slot_filled() ? "OVERWRITE" : "SAVE"; break;
    case UI_LOAD: v = ui_slot_filled() ? "LOAD" : "EMPTY"; break;
    case UI_ADVANCED: v = "OPEN"; break;
    case UI_BACK: v = "BACK"; break;
    case UI_SESSION: v = g_user_enabled ? "ON" : "OFF"; break;
    case UI_STANCE: v = g_auto_stance ? (g_game_goofy < 0 ? (g_cfg.goofy ? "AUTO (GOOFY?)" : "AUTO (REG?)") : g_cfg.goofy ? "AUTO: GOOFY" : "AUTO: REGULAR") : g_cfg.goofy ? "GOOFY" : "REGULAR"; break;
    case UI_LAYOUT: v = g_cfg.front_back ? "FRONT/BACK FOOT" : "LEFT/RIGHT FOOT"; break;
    case UI_SWITCH: v = g_cfg.front_back && g_cfg.switch_flip == 3 ? "ON" : g_cfg.switch_flip == 1 ? "ON" : g_cfg.switch_flip == 2 ? "ON + MIRROR" : g_cfg.switch_flip == 3 ? "TRUE FOOT" : "OFF"; break;
    case UI_POPSTYLE: v = g_cfg.pop_on_release ? "RELEASE" : "FLICK"; break;
    case UI_ONEFOOT: v = g_cfg.one_foot_flips ? "ALLOWED" : "BLOCKED"; break;
    case UI_STEER: v = g_cfg.stick_steer ? "ON" : "OFF"; break;
    case UI_REVERTS: v = g_cfg.revert_input == 2 ? "BOTH TRIGGERS" : g_cfg.revert_input == 1 ? "DOUBLE TAP" : "OFF"; break;
    case UI_WINDUP: v = g_cfg.prewind ? "ON" : "OFF"; break;
    case UI_AIR: v = g_cfg.air_controls ? "ON" : "OFF"; break;
    case UI_SLIDES: v = g_cfg.powerslide ? "ON" : "OFF"; break;
    case UI_BUMP: v = g_cfg.bump_ms > 0 ? "ON" : "OFF"; break;
    case UI_FLIPS: v = g_cfg.body_flips ? "ON" : "OFF"; break;
    }
    int n = 0; while (v[n] && n < 15) { out[n] = (WCHAR)v[n]; ++n; } out[n] = 0;
}
static const WCHAR* const k_ui_labels[UI_COUNT] = {
    L"Session-style preset", L"XL-style preset", L"Dual Stick controls", L"Stance", L"Stick layout", L"Switch stance", L"Pop style", L"One-stick flips", L"Stick steering", L"Reverts", L"Spin wind-up",
    L"Body flips", L"Air controls", L"Powerslides", L"Grind bump-out", L"Menu size", L"Close", L"Advanced settings", L"Back to presets", L"Custom presets", L"Custom slot", L"Save current settings", L"Load saved settings", L"Show menu on startup", L"Menu hotkey", L"Stock ramp controls", L"Ramp diagnostic recording"};
static const WCHAR* const k_ui_help[UI_COUNT] = {
    L"Applies FRONT/BACK FOOT, FLICK pop, steering OFF and one-stick flips BLOCKED. Left = front, right = back in every stance. These translate Skate controls across two sticks; inputs differ from Session.",
    L"Applies LEFT/RIGHT FOOT, TRUE FOOT, RELEASE pop, steering ON and one-stick flips ALLOWED. Sticks stay with physical feet; jobs swap in switch. These translate Skate controls across two sticks; inputs differ from XL.",
    L"Dual Stick on / off. Off = the game's normal controls. Holding L3 + R3 for 3 seconds does the same.",
    L"Which foot is in front. AUTO follows the stance you picked in the game. REGULAR / GOOFY force one.",
    L"LEFT / RIGHT FOOT: each stick is one of your feet - goofy puts the left stick on your back foot, and riding switch with TRUE FOOT swaps them. FRONT / BACK FOOT: the left stick is always the front foot (goofy and switch too).",
    L"Riding switch. ON: the front-foot stick still pops the end in front of you. TRUE FOOT: the sticks swap jobs (physical feet). OFF: nothing changes.",
    L"FLICK: crouch, then flick the other foot to pop. RELEASE: crouch fully with one foot and just let go to pop. Both are translated into Skate flick-stick input.",
    L"BLOCKED: flip tricks need both feet - one stick on its own can still scoop shove-its, 360 shove-its and impossibles (a full circle over the top), but not sweep into a heelflip, kickflip or tre. ALLOWED: no limit.",
    L"Skater XL style: while rolling with the back foot let go, push the front-foot stick sideways to turn (triggers still work). Let it back to the middle before you pop or flick with it.",
    L"How to revert. BOTH TRIGGERS: squeeze LT + RT together (reverts towards the one pressed first). DOUBLE TAP: double-tap LT or RT. OFF.",
    L"Crouch, then hold LT or RT before you pop: the skater winds up a spin that way (the game's prewind).",
    L"Click the front-foot stick for a frontflip or the back-foot stick for a backflip while foot-crouched or airborne. This option enables body flips. Centred R3 pumping and rail/lip exits are separate controls.",
    L"In the air and on rails, each foot moves its board end. On a rail or lip stall, pull front UP or back DOWN firmly, then flick the other foot. A pop foot already held in the stall can also prepare the trick; flick the opposite foot.",
    L"Push both sticks sideways: towards each other = toe-side powerslide, apart = heel-side. No activation delay. Tilt both sticks up/down while held to adjust the angle for a coping approach.",
    L"Rail / lip exit: tilt a stick DOWN or UP and click it. Centred L3 = left, R3 = right.",
    L"Preferred menu size (75-250 %). The two panels automatically shrink to fit the game window.",
    L"Close this menu (your menu hotkey or B also closes it).",
    L"Customize individual options. Changing layout, switch handling, pop or steering can take you outside a preset. Both presets translate two-stick inputs into Skate's controls.",
    L"Return to the two presets and main settings. Your custom options are kept.",
    L"Save your own settings in one of five slots, then load them whenever you want. Slots are stored in session_controls.ini.",
    L"Choose slot 1-5. Each slot stores your current settings, including custom options and stance selection.",
    L"Save current settings in this slot. For a filled slot, press again to confirm replacement; moving to another row cancels.",
    L"Restore this slot's saved settings. Empty slots do nothing. Your other saved presets are kept.",
    L"ON opens this menu when the mod first receives controller input each launch. OFF skips it on future launches. You can always reopen the menu using the key shown below.",
    L"A / Enter or Right: press a new keyboard key to open this menu. Left restores End. Escape or controller B cancels. Use a letter, digit, function key or navigation key such as Home.",
    L"OFF: foot controls. AIR ONLY: the current trigger-grab air experiment. EARLY ON RAMP: switch to stock controls during a qualifying ramp climb, for stock pop/prewind, then keep them until landing. A cancelled approach returns to foot controls. R3 still pumps. Experimental tilt cue.",
    L"OFF / RAMP / FLAT: record checked skater-object snapshots for finding ramp angle or velocity. Label ramp runs RAMP, then flat-ground ollies FLAT. No automatic ramp detection yet. Bounded recording; OFF stops capture."};
/* v5.9 controls page as Flick-it style pictures: each card shows the two sticks (L = front foot,
 * R = back foot, regular stance) with the moves drawn on them. kind 0 = line from (a,b) to (c,d),
 * 1 = arc of radius a from angle b to c (0 = up, clockwise), 2 = stick click. end: 0 arrow (flick),
 * 1 dot (hold), 2 dot, thin (light lean). Coordinates in hundredths, y up. */
typedef struct { BYTE stick, kind; short a, b, c, d; BYTE step, end; } UI_STROKE;
typedef struct { const WCHAR* title; const WCHAR* note; int n; UI_STROKE s[4]; } UI_CARD;
enum { UI_CARD_OLLIE=0, UI_CARD_NOLLIE=1, UI_CARD_PUMP=11, UI_CARD_REVERT=13, UI_CARD_GRAB=16, UI_CARD_SLIDE=14, UI_CARD_EXIT=17, UI_CARD_TOGGLE=18 };
static const UI_CARD k_cards[] = {
    {L"OLLIE", L"Hold back down; flick front up", 2, {{1,0, 0,0,0,-85, 1,1}, {0,0, 0,0,0,85, 2,0}}},
    {L"NOLLIE", L"Hold front up; flick back down", 2, {{0,0, 0,0,0,85, 1,1}, {1,0, 0,0,0,-85, 2,0}}},
    {L"KICKFLIP", L"Hold back down; flick front diagonally", 2, {{1,0, 0,0,0,-85, 1,1}, {0,0, 0,0,-62,62, 2,0}}},
    {L"HEELFLIP", L"Hold back down; flick front diagonally", 2, {{1,0, 0,0,0,-85, 1,1}, {0,0, 0,0,62,62, 2,0}}},
    {L"SHOVE-IT", L"further round = 360", 2, {{1,0, 0,0,0,-85, 1,1}, {1,1, 85,180,90,0, 2,0}}},
    {L"VARIAL KICKFLIP", L"Back: diagonal to down; hold\nThen flick front up-diagonal", 3, {{1,0, 0,0,60,-60, 1,1}, {1,1, 85,135,180,0, 2,1}, {0,0, 0,0,-62,62, 3,0}}},
    {L"HARDFLIP", L"Back: diagonal to down; hold\nThen flick front up-diagonal", 3, {{1,0, 0,0,-60,-60, 1,1}, {1,1, 85,225,180,0, 2,1}, {0,0, 0,0,-62,62, 3,0}}},
    {L"360 FLIP", L"Back: side, round bottom across\nThen flick front diagonally", 3, {{1,0, 0,0,85,0, 1,1}, {1,1, 85,90,270,0, 2,1}, {0,0, 0,0,-62,62, 3,0}}},
    {L"LASER FLIP", L"Back: side, round bottom across\nThen flick front diagonally", 3, {{1,0, 0,0,-85,0, 1,1}, {1,1, 85,270,90,0, 2,1}, {0,0, 0,0,62,62, 3,0}}},
    {L"360 HARDFLIP", L"Back: side, round bottom across\nThen flick front up-diagonal", 3, {{1,0, 0,0,-85,0, 1,1}, {1,1, 85,270,90,0, 2,1}, {0,0, 0,0,-62,62, 3,0}}},
    {L"MANUALS", L"Back down: manual\nFront up: nose manual", 2, {{1,0, 0,0,0,-45, 0,2}, {0,0, 0,0,0,45, 0,2}}},
    {L"PUMP CROUCH", L"hold R3, release to rise", 1, {{1,2, 0,0,0,0, 0,0}}},
    {L"SPIN WIND-UP", L"crouch + hold LT / RT", 1, {{1,0, 0,0,0,-85, 1,1}}},
    {L"REVERT", L"squeeze LT + RT", 0, {{0}}},
    {L"POWERSLIDE", L"in = toe; out = heel\nTilt both to adjust angle", 2, {{0,0, 0,0,85,0, 0,1}, {1,0, 0,0,-85,0, 0,1}}},
    {L"FRONT / BACKFLIP", L"click front / back foot", 2, {{0,2, 0,0,0,0, 0,0}, {1,2, 0,0,0,0, 0,0}}},
    {L"GRAB / DARKSLIDE", L"hold LB / RB: rolling or in air = grab, while flicking = darkslide", 0, {{0}}},
    {L"GRIND / LIP EXIT", L"L3 / R3; lip: DOWN + click", 2, {{0,2, 0,0,0,0, 0,0}, {1,2, 0,0,0,0, 0,0}}},
    {L"DUAL STICK ON / OFF", L"hold L3 + R3 3 s", 2, {{0,2, 0,0,0,0, 0,0}, {1,2, 0,0,0,0, 0,0}}}};
#define UI_NCARDS ((int)(sizeof k_cards / sizeof k_cards[0]))
static void ui_stick_pt(int cx, int cy, int r, float x, float y, int* px, int* py) { *px = cx + (int)(x * r); *py = cy - (int)(y * r); }
static void ui_draw_card(HDC dc, const UI_CARD* cd, int x0, int y0, int w, int h, int k, HGDIOBJ small, HGDIOBJ bold, int goofy) {
    int physical = cd==&k_cards[UI_CARD_PUMP] || cd==&k_cards[UI_CARD_SLIDE] || cd==&k_cards[UI_CARD_EXIT] || cd==&k_cards[UI_CARD_TOGGLE];
    int swap_feet = !physical && (goofy != !!g_cfg.feet_swapped);
    int mirror_x = !physical && goofy;
    UI_CARD option;
    if(cd==&k_cards[UI_CARD_REVERT]) {
        option=*cd;option.note=g_cfg.revert_input==2?L"squeeze LT + RT":g_cfg.revert_input==1?L"double-tap LT or RT":L"Reverts disabled in settings";cd=&option;
    } else if(cd==&k_cards[UI_CARD_GRAB] && g_cfg.stock_grab_air) {
        option=*cd;option.note=L"LB / RB normally; stock ramp mode: LT / RT grab";cd=&option;
    }
    UI_CARD release;
    if(g_cfg.pop_on_release && (cd==&k_cards[UI_CARD_OLLIE] || cd==&k_cards[UI_CARD_NOLLIE])) {
        release=*cd;release.note=L"hold, then release";release.n=2;
        release.s[1]=release.s[0];release.s[1].a=release.s[0].c;release.s[1].b=release.s[0].d;
        release.s[1].c=release.s[1].d=0;release.s[1].step=2;release.s[1].end=0;
        cd=&release;
    }
    #define SS(v) ((int)(v) * k / 100)
    static HBRUSH cbg; if (!cbg) cbg = CreateSolidBrush(RGB3(34, 36, 40));
    SC_RECT cr = {x0, y0, x0 + w, y0 + h}; FillRect(dc, &cr, cbg);
    SelectObject(dc, bold); SetTextColor(dc, RGB3(242, 242, 240));
    SC_RECT tr = {x0 + SS(10), y0 + SS(8), x0 + w - SS(10), y0 + SS(32)}; DrawTextW(dc, cd->title, -1, &tr, 0x20 | 0x01 /*CENTER*/);
    int r = SS(44), cy = y0 + SS(90), cxs[2] = {x0 + w / 4, x0 + 3 * w / 4};
    HGDIOBJ ring = CreatePen(0, SS(2) > 0 ? SS(2) : 1, RGB3(170, 178, 192));
    HGDIOBJ blue = CreatePen(0, SS(3) > 0 ? SS(3) : 1, RGB3(88, 178, 255));
    HGDIOBJ thin = CreatePen(0, SS(2) > 0 ? SS(2) : 1, RGB3(88, 178, 255));
    HBRUSH fill = CreateSolidBrush(RGB3(88, 178, 255));
    HGDIOBJ op = SelectObject(dc, ring), ob = SelectObject(dc, GetStockObject(5 /*NULL_BRUSH*/));
    int has_sticks = cd->n > 0;
    if (has_sticks) for (int i = 0; i < 2; ++i) {
        Ellipse(dc, cxs[i] - r, cy - r, cxs[i] + r, cy + r);
        SC_RECT lr = {cxs[i] - r - SS(20), cy + r + SS(5), cxs[i] + r + SS(20), cy + r + SS(27)};
        /* v5.10: front / back foot follow the stance: regular L = front, goofy L = back */
        const WCHAR* cap = physical ? (i ? L"R stick" : L"L stick") : i ? (swap_feet ? L"R front" : L"R back") : (swap_feet ? L"L back" : L"L front");
        SelectObject(dc, small); SetTextColor(dc, RGB3(225, 230, 238)); DrawTextW(dc, cap, -1, &lr, 0x20 | 0x01);
    }
    for (int i = 0; i < cd->n; ++i) {
        UI_STROKE sv = cd->s[i]; const UI_STROKE* st = &sv;
        if (swap_feet) sv.stick = (BYTE)!sv.stick;
        if (mirror_x) {   /* Preserve the existing normal-stance sideways mapping. */
            if (sv.kind == 0) { sv.a = (short)-sv.a; sv.c = (short)-sv.c; }
            else if (sv.kind == 1) { sv.b = (short)(360 - sv.b); sv.c = (short)(360 - sv.c); }
        }
        int cx = cxs[st->stick ? 1 : 0];
        SelectObject(dc, st->end == 2 ? thin : blue);
        int ex, ey, px, py; float lx = 0, ly = 0, dx = 0, dy = 1;
        if (st->kind == 2) {   /* click */
            SelectObject(dc, fill); Ellipse(dc, cx - SS(7), cy - SS(7), cx + SS(7), cy + SS(7)); SelectObject(dc, GetStockObject(5));
            continue;
        } else if (st->kind == 0) {
            ui_stick_pt(cx, cy, r, st->a / 100.0f, st->b / 100.0f, &px, &py); MoveToEx(dc, px, py, 0);
            ui_stick_pt(cx, cy, r, st->c / 100.0f, st->d / 100.0f, &ex, &ey); LineTo(dc, ex, ey);
            lx = st->c / 100.0f; ly = st->d / 100.0f; dx = (st->c - st->a) / 100.0f; dy = (st->d - st->b) / 100.0f;
        } else {
            float rad = st->a / 100.0f; int steps = 32;
            for (int j = 0; j <= steps; ++j) {
                float ang = st->b + (st->c - st->b) * (float)j / steps;
                float x = sc_sin_deg(ang) * rad, y = sc_sin_deg(90.0f - ang) * rad;
                ui_stick_pt(cx, cy, r, x, y, &px, &py);
                if (j == 0) MoveToEx(dc, px, py, 0); else LineTo(dc, px, py);
                if (j == steps) { dx = x - lx; dy = y - ly; }
                lx = x; ly = y;
            }
            ui_stick_pt(cx, cy, r, lx, ly, &ex, &ey);
        }
        SelectObject(dc, fill);
        if (st->end == 0) {   /* arrowhead */
            float m = sc_sqrt(dx * dx + dy * dy); if (m < 0.0001f) m = 1; dx /= m; dy /= m;
            float hx = -dx, hy = dy;   /* screen space, back along the line */
            float L = (float)SS(9), W = (float)SS(5);
            SC_PT tri[3] = {{ex + (int)(dx * SS(3)), ey - (int)(dy * SS(3))},
                            {ex + (int)(hx * L + -hy * W), ey + (int)(hy * L + hx * W)},
                            {ex + (int)(hx * L + hy * W), ey + (int)(hy * L - hx * W)}};
            Polygon(dc, tri, 3);
        } else {
            int d = st->end == 2 ? SS(3) : SS(5);
            Ellipse(dc, ex - d, ey - d, ex + d, ey + d);
        }
        SelectObject(dc, GetStockObject(5));
        if (st->step) {
            WCHAR num[2] = {(WCHAR)('0' + st->step), 0};
            SC_RECT nr = {ex + SS(8), ey - SS(13), ex + SS(28), ey + SS(11)};
            SelectObject(dc, small); SetTextColor(dc, RGB3(255, 210, 90)); DrawTextW(dc, num, -1, &nr, 0x20);
        }
    }
    SelectObject(dc, op); SelectObject(dc, ob);
    DeleteObject(ring); DeleteObject(blue); DeleteObject(thin); DeleteObject(fill);
    SelectObject(dc, small); SetTextColor(dc, RGB3(200, 205, 212));
    SC_RECT nr2 = {x0 + SS(12), has_sticks ? y0 + SS(166) : y0 + SS(66), x0 + w - SS(12), y0 + h - SS(6)};
    DrawTextW(dc, cd->note, -1, &nr2, (0x10 | 0x01));
    #undef SS
}

static void ui_set(const WCHAR* key, const WCHAR* val) { WritePrivateProfileStringW(L"SessionControls", key, val, g_ini); }
#include "presets.h"

/* Runs on the game's input thread (process), so it never races the translation. */
static void ui_change(int item, int dir) {
    if (item != UI_SAVE) g_ui_overwrite=0;
    switch (item) {
    case UI_HOTKEY:
        if(dir<0) {ui_hotkey_apply(0x23);return;}
        g_ui_req_cards=g_ui_req_toggle=g_ui_req_nav=g_ui_req_change=0;g_ui_req_bound_key=0;
        g_ui_bind_mode=1;g_ui_overwrite=0;
        g_ui_preset_status=L"Release any held keys, then press the new menu key. Use a letter, digit, F1-F12 or a navigation key such as Home. Escape or controller B cancels.";
        g_ui_dirty=1;return;
    case UI_STOCKAIR: {
        int mode=!g_cfg.stock_grab_air?0:g_cfg.stock_transition_ground?2:1;
        mode=(mode+(dir<0?2:1))%3;
        if(!WritePrivateProfileStringW(L"SessionControls",L"StockTransitionGround",mode==2?L"1":L"0",g_ini) ||
           !WritePrivateProfileStringW(L"SessionControls",L"StockGrabAir",mode?L"1":L"0",g_ini)) {
            g_ui_preset_status=L"Could not save the ramp control mode. Check that the settings file is writable.";
        }
        break;
    }
    case UI_RAMPDIAG: {int v=(g_ramp_diag+(dir<0?2:1))%3;g_ramp_diag_reset=1;ui_set(L"RampDiagnostics",v==1?L"1":v==2?L"2":L"0");break;}
    case UI_STARTUP:
        if(!WritePrivateProfileStringW(L"SessionControls",L"ShowMenuOnStartup",g_show_startup_menu?L"0":L"1",g_ini)) {
            g_ui_preset_status=L"Could not save the startup preference. Check that session_controls.ini is writable.";g_ui_dirty=1;return;
        }
        load_config(1);g_ui_preset_status=L"Startup preference saved. The menu stays open; use B to close. The reopen key is shown below.";g_ui_dirty=1;return;
    case UI_CUSTOM: g_ui_page=3;g_ui_sel=0;g_ui_preset_status=L"";g_ui_dirty=1;return;
    case UI_SLOT:
        g_ui_slot=(g_ui_slot-1+(dir<0?4:1))%5+1;g_ui_preset_status=L"";g_ui_dirty=1;return;
    case UI_SAVE:
        if(ui_slot_filled() && g_ui_overwrite!=g_ui_slot) {
            g_ui_overwrite=g_ui_slot;
            g_ui_preset_status=L"This slot already contains a preset. Press Save again to replace it. Moving to another row cancels.";
        } else {
            g_ui_preset_status=ui_custom_save()?L"Saved your current settings in this custom slot.":L"Could not save this preset. Check that session_controls.ini is writable and the settings section is not too large.";
            g_ui_overwrite=0;
        }
        g_ui_dirty=1;return;
    case UI_LOAD:
        if(!ui_slot_filled()) {g_ui_preset_status=L"This slot is empty. Save your settings here first.";g_ui_dirty=1;return;}
        if(!ui_custom_load()) {g_ui_preset_status=L"Could not load this preset. Check that session_controls.ini is writable and the preset is not too large.";g_ui_dirty=1;return;}
        load_config(1);g_ui_preset_status=L"Custom preset loaded.";g_ui_dirty=1;
        log_text("menu: custom preset loaded");return;
    case UI_PRESET_SESSION:
    case UI_PRESET_XL: {
        int xl = item == UI_PRESET_XL;
        if(!ui_builtin_apply(xl)) {g_ui_preset_status=L"Could not apply the preset. Check that session_controls.ini is writable.";g_ui_dirty=1;return;}
        g_ui_preset_status=xl?L"XL-style applied: physical feet, release pop, stick steering. These inputs translate into Skate's controls and differ from XL.":L"Session-style applied: front/back feet, flick pop, trigger steering. These inputs translate into Skate's controls and differ from Session.";
        log_text(xl ? "menu: XL-style preset applied (Skate input translation)" : "menu: Session-style preset applied (Skate input translation)");
        break;
    }
    case UI_ADVANCED: g_ui_page = 2; g_ui_sel = 0; g_ui_dirty = 1; return;
    case UI_BACK: g_ui_sel = g_ui_page == 3 ? 2 : 5; g_ui_page = 0; g_ui_dirty = 1; return;
    case UI_SIZE: {
        int sc = g_ui_scale + (dir < 0 ? -25 : 25);
        if (sc > 250) sc = 75; if (sc < 75) sc = 250;
        g_ui_scale = sc; WCHAR t[8]; int n = 0; if (sc >= 100) t[n++] = (WCHAR)('0' + sc / 100); t[n++] = (WCHAR)('0' + sc / 10 % 10); t[n++] = (WCHAR)('0' + sc % 10); t[n] = 0;
        ui_set(L"MenuScale", t); g_ui_dirty = 1; return;
    }
    case UI_SESSION: g_user_enabled = !g_user_enabled; log_text(g_user_enabled ? "menu: Dual Stick controls ON" : "menu: Dual Stick controls OFF"); break;
    case UI_STANCE: {   /* v3.9: AUTO (from the game) -> REGULAR -> GOOFY -> AUTO */
        int cur = g_auto_stance ? 0 : g_ini_goofy ? 2 : 1;
        int v = (cur + (dir < 0 ? 2 : 1)) % 3;
        ui_set(L"AutoStance", v == 0 ? L"1" : L"0");
        if (v) ui_set(L"Stance", v == 2 ? L"goofy" : L"regular");
        break; }
    case UI_LAYOUT: ui_set(L"FrontBackFoot", g_cfg.front_back ? L"0" : L"1"); break;
    case UI_SWITCH: {   /* OFF -> ON -> TRUE FOOT (ON + MIRROR, 2, is ini only) */
        static const int order[3] = {0, 1, 3};
        int cur = g_cfg.switch_flip == 3 ? 2 : g_cfg.switch_flip == 0 ? 0 : 1;
        int v = order[(cur + (dir < 0 ? 2 : 1)) % 3];
        ui_set(L"SwitchStance", v == 1 ? L"1" : v == 3 ? L"3" : L"0"); break; }
    case UI_POPSTYLE: ui_set(L"PopOnRelease", g_cfg.pop_on_release ? L"0" : L"1"); break;
    case UI_ONEFOOT: ui_set(L"OneFootFlips", g_cfg.one_foot_flips ? L"0" : L"1"); break;
    case UI_STEER: ui_set(L"StickSteering", g_cfg.stick_steer ? L"0" : L"1"); break;
    case UI_REVERTS: { int v = (g_cfg.revert_input + (dir < 0 ? 2 : 1)) % 3; ui_set(L"RevertInput", v == 2 ? L"2" : v == 1 ? L"1" : L"0"); break; }
    case UI_WINDUP: ui_set(L"SpinWindUp", g_cfg.prewind ? L"0" : L"1"); break;
    case UI_AIR: ui_set(L"AirControls", g_cfg.air_controls ? L"0" : L"1"); break;
    case UI_SLIDES: ui_set(L"Powerslides", g_cfg.powerslide ? L"0" : L"1"); break;
    case UI_BUMP: ui_set(L"GrindBumpMs", g_cfg.bump_ms > 0 ? L"0" : L"250"); break;
    case UI_FLIPS: ui_set(L"BodyFlips", g_cfg.body_flips ? L"0" : L"1"); break;
    case UI_CLOSE: g_ui_open = 0; break;
    }
    if (item != UI_SESSION && item != UI_CLOSE) { load_config(1); log_text("menu: setting changed and saved"); }
    g_ui_dirty = 1;
}

static void ui_draw_controls(HDC dc,int k) {
    #define S(v) ((int)(v)*k/100)
    SC_RECT title={S(UI_CONTROLS_X+16),S(12),S(UI_W-16),S(40)};
    static HGDIOBJ title_font;static int title_scale;
    if(!title_font || title_scale!=k) {if(title_font)DeleteObject(title_font);title_font=CreateFontW(-S(19),0,0,0,800,0,0,0,0,0,0,5,0,L"Segoe UI");title_scale=k;}
    SelectObject(dc,title_font);SetTextColor(dc,RGB3(242,242,240));
    DrawTextW(dc,g_cfg.front_back?L"CONTROLS - FRONT / BACK FOOT":g_cfg.goofy?L"CONTROLS - GOOFY":L"CONTROLS - REGULAR",-1,&title,0x20);
            static HGDIOBJ small, cbold; static int sk;
            if (!small || sk != k) {
                if (small) { DeleteObject(small); DeleteObject(cbold); }
                small = CreateFontW(-S(18), 0, 0, 0, 500, 0, 0, 0, 0, 0, 0, 5, 0, L"Segoe UI");
                cbold = CreateFontW(-S(18), 0, 0, 0, 800, 0, 0, 0, 0, 0, 0, 5, 0, L"Segoe UI"); sk = k;
            }
            SelectObject(dc,small);
            SC_RECT sub = {S(UI_CONTROLS_X+16), S(40), S(UI_W-16), S(64)};
            SetTextColor(dc, RGB3(215, 220, 228));
            DrawTextW(dc, L"Normal stance shown. L / R = physical sticks; front / back = foot jobs.", -1, &sub, 0x20);
            int cols = 2, gap = S(8), cw = (S(UI_CONTROLS_W) - S(16) - gap * (cols - 1)) / cols, ch = S(224);
            int first = g_ui_controls_page * UI_CARDS_PER_PAGE;
            for (int i = first; i < UI_NCARDS && i < first + UI_CARDS_PER_PAGE; ++i) {
                int slot = i - first;
                int cx0 = S(UI_CONTROLS_X+8) + (slot % cols) * (cw + gap), cy0 = S(66) + (slot / cols) * (ch + gap);
                ui_draw_card(dc, &k_cards[i], cx0, cy0, cw, ch, k, small, cbold, g_cfg.goofy && !g_cfg.front_back);
            }

    WCHAR legend[]=L"PAGE 1 / 4    LB: previous    RB: next    PgUp / PgDn\n1, 2, 3 = order    Arrow = flick    End dot = hold    Centre dot = click";
    legend[5]=(WCHAR)(L'1'+g_ui_controls_page);
    legend[9]=(WCHAR)(L'0'+(UI_NCARDS+UI_CARDS_PER_PAGE-1)/UI_CARDS_PER_PAGE);
    SC_RECT footer={S(UI_CONTROLS_X+16),S(UI_HC-52),S(UI_W-16),S(UI_HC-4)};
    SetTextColor(dc,RGB3(215,220,228));DrawTextW(dc,legend,-1,&footer,0x10);
    #undef S
}

static void ui_paint(HDC dc) {
    {
        static HBRUSH bg, hi; static HGDIOBJ font, bold; static int font_scale;
        int k = g_ui_draw_scale;
        #define S(v) ((int)(v) * k / 100)
        if (!bg) { bg = CreateSolidBrush(RGB3(22, 23, 25)); hi = CreateSolidBrush(RGB3(26, 124, 255)); }
        if (!font || font_scale != k) {
            if (font) { DeleteObject(font); DeleteObject(bold); }
            font = CreateFontW(-S(17), 0, 0, 0, 500, 0, 0, 0, 0, 0, 0, 5, 0, L"Segoe UI"); bold = CreateFontW(-S(19), 0, 0, 0, 800, 0, 0, 0, 0, 0, 0, 5, 0, L"Segoe UI");
            font_scale = k;
        }
        SC_RECT all = {0, 0, S(UI_W), S(ui_height())};
        FillRect(dc, &all, bg);
        SetBkMode(dc, 1 /*TRANSPARENT*/);
        SelectObject(dc, bold); SetTextColor(dc, RGB3(242, 242, 240));
        SC_RECT t = {S(16), S(12), S(UI_SETTINGS_W - 16), S(40)};
        DrawTextW(dc, g_ui_page == 3 ? L"DUAL STICK CONTROLS  -  CUSTOM PRESETS" : g_ui_page == 2 ? L"DUAL STICK CONTROLS  -  ADVANCED" : g_ui_page == 1 ? (g_cfg.front_back ? L"DUAL STICK CONTROLS  -  CONTROLS (FRONT / BACK FOOT)" : g_cfg.goofy ? L"DUAL STICK CONTROLS  -  CONTROLS (GOOFY)" : L"DUAL STICK CONTROLS  -  CONTROLS (REGULAR)") : L"DUAL STICK CONTROLS", -1, &t, 0x20 /*SINGLELINE*/);
        SelectObject(dc, font);
        {
            int rows = ui_row_count();
            for (int i = 0; i < rows; ++i) {
                int item = ui_item_for_row(i);
                SC_RECT r = {S(8), S(48 + i * 30), S(UI_SETTINGS_W - 8), S(48 + i * 30 + 28)};
                if (i == g_ui_sel) FillRect(dc, &r, hi);
                SetTextColor(dc, RGB3(242, 242, 240));
                SC_RECT l = {S(18), r.top, S(UI_SETTINGS_W - 190), r.bottom}; DrawTextW(dc, k_ui_labels[item], -1, &l, 0x20 | 0x04 /*VCENTER*/);
                WCHAR v[16]; ui_value(item, v);
                SC_RECT vr = {S(UI_SETTINGS_W - 200), r.top, S(UI_SETTINGS_W - 18), r.bottom}; DrawTextW(dc, v, -1, &vr, 0x20 | 0x04 | 0x02 /*RIGHT*/);
            }
            /* explanation of the highlighted line */
            static HBRUSH box; if (!box) box = CreateSolidBrush(RGB3(34, 36, 40));
            SC_RECT b = {S(8), S(48 + rows * 30 + 10), S(UI_SETTINGS_W - 8), S(48 + rows * 30 + 10 + UI_INFO)};
            FillRect(dc, &b, box);
            SC_RECT bt = {b.left + S(12), b.top + S(8), b.right - S(12), b.bottom - S(6)};
            SetTextColor(dc, RGB3(215, 220, 228));
            int sel = ui_item_for_row(g_ui_sel);
            DrawTextW(dc, g_ui_preset_status[0] ? g_ui_preset_status : k_ui_help[sel], -1, &bt, 0x10 /*WORDBREAK*/);
        }
        SetTextColor(dc, RGB3(150, 155, 160));
        SC_RECT f = {S(16), S(ui_height() - 32), S(UI_SETTINGS_W - 16), S(ui_height() - 4)};
        DrawTextW(dc, g_ui_page >= 2 ? L"D-pad / arrows: settings    A: change    B: back" : ui_reopen_text(0), -1, &f, 0x20 | 0x04);
        ui_draw_controls(dc,k);
        #undef S
    }
}
/* v4.0: draw the menu into memory for the in-game copy */
static void ui_render_mem(int w, int h) {
    static HDC mdc; static HGDIOBJ bmp; static DWORD* bits; static int bw, bh;
    if (!g_ov_pix) {
        g_ov_cap = 1800 * 2000;
        g_ov_pix = (DWORD*)VirtualAlloc(0, (UINT64)g_ov_cap * 4, 0x3000, 0x04);
        if (!g_ov_pix) return;
    }
    if (w * h > g_ov_cap) return;
    if (!mdc) mdc = CreateCompatibleDC(0);
    if (!mdc) return;
    if (!bmp || bw != w || bh != h) {
        if (bmp) DeleteObject(bmp);
        struct { UINT size; int w, h; WORD planes, bpp; UINT comp, isize; int xp, yp; UINT used, imp; } bi = {40, w, -h, 1, 32, 0, 0, 0, 0, 0, 0};
        void* p = 0; bmp = CreateDIBSection(mdc, &bi, 0, &p, 0, 0); bits = (DWORD*)p; bw = w; bh = h;
        if (!bmp || !bits) { bmp = 0; return; }
        SelectObject(mdc, bmp);
    }
    ui_paint(mdc); GdiFlush();
    while (__sync_lock_test_and_set(&g_ov_lock, 1)) Sleep(0);   /* v4.4: the game's render thread may be reading it */
    for (int i = 0; i < w * h; ++i) g_ov_pix[i] = bits[i];
    g_ov_w = w; g_ov_h = h;
    g_ov_ver++;
    __sync_lock_release(&g_ov_lock);
}

static INT64 WINAPI ui_proc(HWND h, UINT msg, UINT64 wp, INT64 lp) {
    if (msg == 0x000F /*WM_PAINT*/) {
        SC_PAINT ps; HDC dc = BeginPaint(h, &ps);
        ui_paint(dc);
        EndPaint(h, &ps);
        return 0;
    }
    if (msg == 0x0021 /*WM_MOUSEACTIVATE*/) return 3; /* MA_NOACTIVATE */
    return DefWindowProcW(h, msg, wp, lp);
}

static int key_edge(int vk, int* prev) { int d = (GetAsyncKeyState(vk) & 0x8000) != 0; int e = d && !*prev; *prev = d; return e; }

static void ui_reskate_status_poll(void) {
    typedef void (*GetStatus)(sc_reskate_status*);
    static GetStatus get_status;
    static double next_lookup;
    if (!get_status && ov_now() >= next_lookup) {
        next_lookup = ov_now() + 1000.0;
        HMODULE reskate = GetModuleHandleW(L"ReSkate.dll");
        if (reskate) get_status = (GetStatus)GetProcAddress(reskate, "DingoSDKOverlayGetStatus");
        if (get_status) log_text("reSkate compatibility: official menu status detected; controller input yields while Insert is open");
    }
    sc_reskate_status status = {0};
    if (get_status) get_status(&status);
    g_reskate_menu_visible = sc_reskate_menu_visible(&status);
    if (g_reskate_menu_visible) {
        g_ui_open = g_ui_bind_mode = 0;
        g_ui_req_cards = g_ui_req_toggle = g_ui_req_nav = g_ui_req_change = g_ui_req_bound_key = 0;
    }
}

static void ui_window_bounds(HWND owner,int* x,int* y,int* w,int* h,int* scale) {
    SC_RECT bounds={0};*x=*y=40;
    int k=g_ui_scale;
    if(owner && GetWindowRect(owner,&bounds)) {
        *x=bounds.left+40;*y=bounds.top+40;
        k=sc_menu_fit_scale(k,bounds.right-bounds.left,bounds.bottom-bounds.top,UI_W,UI_HC);
    }
    *scale=k;*w=UI_W*k/100;*h=UI_HC*k/100;
}
static HWND ui_create_owned_window(HWND owner,int w,int h) {
    HWND window=CreateWindowExW(SC_MENU_EX_STYLE,L"DualStickControlsMenu",L"Dual Stick Controls",
                               0x80000000 /*WS_POPUP*/,40,40,w,h,owner,0,g_self,0);
    if(window)SetLayeredWindowAttributes(window,0,235,2 /*LWA_ALPHA*/);
    return window;
}
#include "menu_keyboard.h"
static DWORD WINAPI ui_thread(void* arg) {
    (void)arg;
    ui_startup_open();
    SC_WNDCLASS wc; memset(&wc, 0, sizeof wc);
    wc.size = sizeof wc; wc.proc = ui_proc; wc.inst = g_self; wc.name = L"DualStickControlsMenu";
    RegisterClassExW(&wc);
    /* v5.22: the game's Present is hooked the first time you open the menu, not at start-up. Hooking
     * it before reSkate's own overlay set up made reSkate ignore the swap chain ("Swapchain hook setup
     * incomplete; candidate ignored" in ReSkate.log), so its mod manager (Insert) never opened. */
    int hooked = 0, hook_tried = 0; double hook_t = 0;
    log_text("menu compatibility: game-owned menu window; no graphics hooks regardless of ReSkate load order");
    int w=0,h=0,x=40,y=40,cur_scale=0;
    /* Wait for the game to be foreground before creating its owned popup.
     * An independent topmost window could cover another app or disturb an
     * exclusive-fullscreen launch even when SW_SHOWNOACTIVATE is used. */
    HWND owner=0;
    while(!(owner=ui_foreground_game()))Sleep(15);
    ui_window_bounds(owner,&x,&y,&w,&h,&cur_scale);g_ui_draw_scale=cur_scale;
    g_ui_hwnd = ui_create_owned_window(owner,w,h);
    if (!g_ui_hwnd) { log_text("menu: could not create the menu window"); return 0; }

    int shown = 0, pk = 0, pu = 0, pd = 0, pl = 0, pr = 0, pe = 0, pprev=0, pnext=0, was_focused=0;
    for (;;) {
        SC_MSG m;
        while (PeekMessageW(&m, 0, 0, 0, 1 /*PM_REMOVE*/)) { TranslateMessage(&m); DispatchMessageW(&m); }
        HWND foreground_game=ui_foreground_game();
        int focused=foreground_game!=0;
        if(!focused) {
            g_ui_req_cards=g_ui_req_toggle=g_ui_req_nav=g_ui_req_change=g_ui_req_bound_key=0;
            if(shown){ShowWindow(g_ui_hwnd,0);shown=0;}
            if(g_ui_bind_mode)g_ui_bind_mode=1;
            was_focused=0;
            Sleep(15);continue;
        }
        /* A startup splash can be replaced by the real game window. Windows
         * destroys owned popups with their owner; recreate rather than keeping
         * a dead handle. Reattach when the foreground game window changes. */
        if(!IsWindow(g_ui_hwnd)) {
            owner=foreground_game;g_ui_hwnd=ui_create_owned_window(owner,w,h);
            shown=0;g_ui_dirty=1;
            if(!g_ui_hwnd){Sleep(15);continue;}
        } else if(foreground_game!=owner && foreground_game!=g_ui_hwnd) {
            SetWindowLongPtrW(g_ui_hwnd,-8 /*GWLP_HWNDPARENT*/,(INT64)foreground_game);
            owner=foreground_game;
        }
        if(!was_focused) {
            /* Keys held while returning from another app are not new presses. */
            pk=(GetAsyncKeyState(g_ui_key)&0x8000)!=0;
            pu=(GetAsyncKeyState(0x26)&0x8000)!=0;pd=(GetAsyncKeyState(0x28)&0x8000)!=0;
            pl=(GetAsyncKeyState(0x25)&0x8000)!=0;pr=(GetAsyncKeyState(0x27)&0x8000)!=0;
            pe=(GetAsyncKeyState(0x0D)&0x8000)!=0;
            pprev=(GetAsyncKeyState(0x21)&0x8000)!=0;pnext=(GetAsyncKeyState(0x22)&0x8000)!=0;was_focused=1;
        }
        ui_reskate_status_poll();
        if(g_ui_hotkey_sync) {pk=(GetAsyncKeyState(g_ui_key)&0x8000)!=0;g_ui_hotkey_sync=0;}
        if(g_ui_bind_mode) ui_capture_keys();
        else {
        if (key_edge(g_ui_key, &pk)) g_ui_req_toggle = 1;
        if (g_ui_open) {
            if (key_edge(0x21,&pprev))g_ui_req_cards=-1;
            if (key_edge(0x22,&pnext))g_ui_req_cards=1;
            if (key_edge(0x26, &pu)) g_ui_req_nav = -1;
            if (key_edge(0x28, &pd)) g_ui_req_nav = 1;
            if (key_edge(0x25, &pl)) g_ui_req_change = -1;
            if (key_edge(0x27, &pr) || key_edge(0x0D, &pe)) g_ui_req_change = 1;
            /* the game may not be polling the controller (paused): apply keyboard actions here too */
        }
        }
        if(!g_reskate_menu_visible)ui_keyboard_actions();
        /* v4.0: drawn inside the game when the game is presenting through our hook; the window is
         * only the fallback (it knocked exclusive fullscreen out) */
        static double open_t; static int was_open;
        if (g_ui_open && !was_open) open_t = ov_now();
        was_open = g_ui_open;
        if (g_ui_open && !hook_tried && sc_allow_menu_graphics_hook(g_ov_enabled, GetModuleHandleW(L"ReSkate.dll") != 0)) {
            hook_tried = 1; hooked = ov_hook(); hook_t = ov_now();
            log_text(hooked ? "in-game menu: ready (drawn inside the game)" : "in-game menu: not available, the menu uses its own window");
        }
        int ingame = hooked && g_ov_enabled && !ov_failed && (ov_now() - hook_t < 400.0 || ov_now() - g_ov_last < 500.0);
        /* safety: open for a second and still nothing drawn in the game -> use the window */
        if (ingame && g_ui_open && !g_ov_ok && ov_now() - open_t > 1000.0) { ov_failed = 1; ingame = 0; log_text("in-game menu: nothing drawn, using the menu window"); }
        int want = g_ui_open && !ingame;
        if (g_ui_open && ingame && (g_ui_dirty || g_ov_w != w || g_ov_h != h)) { g_ui_dirty = 0; ui_render_mem(w, h); }
        if (want != shown) {
            shown = want;
            ShowWindow(g_ui_hwnd, shown ? 4 /*SW_SHOWNOACTIVATE*/ : 0);
            if (shown) { SetWindowPos(g_ui_hwnd, 0, x, y, w, h, SC_MENU_POSITION_FLAGS); g_ui_dirty = 1; }
        }
        int nx,ny,nw,nh,nk;ui_window_bounds(owner,&nx,&ny,&nw,&nh,&nk);
        if(cur_scale!=nk || w!=nw || h!=nh || x!=nx || y!=ny) {
            cur_scale=nk;g_ui_draw_scale=nk;w=nw;h=nh;x=nx;y=ny;
            SetWindowPos(g_ui_hwnd,0,x,y,w,h,SC_MENU_POSITION_FLAGS);g_ui_dirty=1;
        }
        if (g_ui_dirty && shown) { g_ui_dirty = 0; InvalidateRect(g_ui_hwnd, 0, 0); }
        Sleep(15);
    }
}

static void ui_start(void) {
    if (g_ui_started) return;
    g_ui_started = 1;
    DWORD id; HANDLE th = CreateThread(0, 0, ui_thread, 0, 0, &id);
    if (th) CloseHandle(th);
    else {g_ui_started=0;log_text("menu: window thread creation failed; retry on controller poll");}
}

/* Called from process() on every controller poll. Returns 1 while the menu has the controller. */
static int ui_poll(XINPUT_STATE* s) {
    static WORD prev;
    WORD b = s->Gamepad.buttons, dn = (WORD)(b & ~prev);
    prev = b;
    if(!ui_foreground_game()) {
        g_ui_req_cards=g_ui_req_toggle=g_ui_req_nav=g_ui_req_change=g_ui_req_bound_key=0;
        g_ui_startup_input_sync=1;
        return 0;
    }
    if(g_ui_startup_input_sync) {dn=0;g_ui_startup_input_sync=0;}
    if(g_ui_bind_mode) {
        if(dn&0x2000)ui_hotkey_apply(-1);
        memset(&s->Gamepad,0,sizeof s->Gamepad);return 1;
    }
    if (!g_ui_open) return 0;
    if(g_ui_page!=1 && (g_ui_sel<0 || g_ui_sel>=ui_row_count())) {g_ui_sel=0;g_ui_dirty=1;}
    int cards=0;
    int bumpers=sc_controls_bumper_direction(dn);if(bumpers)cards=bumpers;
    if(cards) {
        g_ui_controls_page=sc_controls_next_page(g_ui_controls_page,cards,UI_NCARDS,UI_CARDS_PER_PAGE);
        g_ui_dirty=1;
    }
    int nav = 0;
    if (dn & 0x0001) nav = -1;            /* D-pad up */
    if (dn & 0x0002) nav = 1;             /* D-pad down */
    if (nav) { g_ui_overwrite = 0; g_ui_preset_status=L""; g_ui_sel = (g_ui_sel + nav + ui_row_count()) % ui_row_count(); g_ui_dirty = 1; }
    int ch = 0;
    if (dn & 0x0004) ch = -1;                 /* D-pad left */
    if (dn & (0x0008 | 0x1000)) ch = 1;       /* D-pad right / A */
    if (ch) ui_change(ui_item_for_row(g_ui_sel), ch);
    if (dn & 0x2000) { if (g_ui_page >= 2) { g_ui_sel = g_ui_page == 3 ? 2 : 5; g_ui_page = 0; } else g_ui_open = 0; g_ui_dirty = 1; }   /* B closes */
    /* the game sees an idle controller while the menu is open */
    memset(&s->Gamepad, 0, sizeof s->Gamepad);
    return 1;
}

static int key_from_name(const WCHAR* n) {
    static const struct { const char* name; int vk; } keys[] = {
        {"end", 0x23}, {"home", 0x24}, {"insert", 0x2D}, {"delete", 0x2E}, {"pageup", 0x21}, {"pagedown", 0x22},
        {"f1", 0x70}, {"f2", 0x71}, {"f3", 0x72}, {"f4", 0x73}, {"f5", 0x74}, {"f6", 0x75}, {"f7", 0x76}, {"f8", 0x77},
        {"f9", 0x78}, {"f10", 0x79}, {"f11", 0x7A}, {"f12", 0x7B}, {"pause", 0x13}, {"scrolllock", 0x91}, {"numlock", 0x90}};
    for (unsigned i = 0; i < sizeof keys / sizeof keys[0]; ++i) if (wieq(n, keys[i].name)) return keys[i].vk;
    if (n[0] && !n[1]) { WCHAR c = n[0]; if (c >= 'a' && c <= 'z') c -= 32; if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) return c; }
    if (n[0] == '0' && (n[1] == 'x' || n[1] == 'X')) {   /* a raw virtual-key code, e.g. 0x23 */
        int v = 0; for (int i = 2; n[i]; ++i) { WCHAR c = n[i]; v = v * 16 + (c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : 0); }
        return v;
    }
    return 0x23;
}
