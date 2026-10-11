/* v3.9 AUTO STANCE: read the skater's stance (regular / goofy) from the game's own settings.
 * reSkate keeps the profile in %LOCALAPPDATA%\ReSkate\profiles\<profile>\reskate.sqlite3, table
 * player_settings, scope "options", name "DefaultStanceIsGoofy" (true / false). It is read with
 * Windows' own SQLite (winsqlite3.dll, part of Windows 10 / 11), read-only, on a background thread
 * every few seconds, so changing your stance in the game's menu switches the mod too.
 * Without winsqlite3 (e.g. Linux / Proton) the Stance setting in the ini is used as before. */
#ifndef SC_GSTANCE_H
#define SC_GSTANCE_H

typedef int (*sq_open_t)(const char*, void**, int, const char*);
typedef int (*sq_prep_t)(void*, const char*, int, void**, const char**);
typedef int (*sq_step_t)(void*);
typedef const unsigned char* (*sq_text_t)(void*, int);
typedef int (*sq_fin_t)(void*);
typedef int (*sq_close_t)(void*);
typedef int (*sq_busy_t)(void*, int);

static void gs_utf8(const WCHAR* w, char* out, int cap) {
    int n = 0;
    for (; *w && n < cap - 4; ++w) {
        unsigned c = *w;
        if (c >= 0xD800 && c <= 0xDBFF && w[1] >= 0xDC00 && w[1] <= 0xDFFF) { c = 0x10000 + ((c - 0xD800) << 10) + (w[1] - 0xDC00); ++w; }
        if (c < 0x80) out[n++] = (char)c;
        else if (c < 0x800) { out[n++] = (char)(0xC0 | (c >> 6)); out[n++] = (char)(0x80 | (c & 63)); }
        else if (c < 0x10000) { out[n++] = (char)(0xE0 | (c >> 12)); out[n++] = (char)(0x80 | ((c >> 6) & 63)); out[n++] = (char)(0x80 | (c & 63)); }
        else { out[n++] = (char)(0xF0 | (c >> 18)); out[n++] = (char)(0x80 | ((c >> 12) & 63)); out[n++] = (char)(0x80 | ((c >> 6) & 63)); out[n++] = (char)(0x80 | (c & 63)); }
    }
    out[n] = 0;
}
/* newest-written profile: profiles\*\reskate.sqlite3 (its -wal counts too) */
static int gs_find_db(const WCHAR* la, WCHAR* out, int cap) {
    WCHAR base[600], pat[600]; dg_wcpy(base, 600, la, L"\\ReSkate\\profiles\\"); dg_wcpy(pat, 600, base, L"*");
    SC_FIND fd; HANDLE h = FindFirstFileW(pat, &fd); UINT64 best = 0; int found = 0;
    if (h == INVALID_HANDLE_VALUE) return 0;
    do {
        if (!(fd.attr & 0x10) || fd.name[0] == '.') continue;
        WCHAR dir[600], f[600]; dg_wcpy(dir, 600, base, fd.name);
        static const WCHAR* const names[2] = {L"\\reskate.sqlite3", L"\\reskate.sqlite3-wal"};
        for (int k = 0; k < 2; ++k) {
            dg_wcpy(f, 600, dir, names[k]);
            SC_FIND f2; HANDLE h2 = FindFirstFileW(f, &f2);
            if (h2 == INVALID_HANDLE_VALUE) continue;
            FindClose(h2);
            UINT64 w = dg_ft(f2.written);
            if (w > best) { best = w; dg_wcpy(out, cap, dir, L"\\reskate.sqlite3"); found = 1; }
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    return found;
}
static DWORD WINAPI gs_thread(void* p) {
    (void)p;
    HMODULE m = LoadLibraryW(L"winsqlite3.dll");
    if (!m) { log_text("auto stance: Windows SQLite not available, using Stance= from the ini"); return 0; }
    sq_open_t op = (sq_open_t)GetProcAddress(m, "sqlite3_open_v2");
    sq_prep_t pr = (sq_prep_t)GetProcAddress(m, "sqlite3_prepare_v2");
    sq_step_t st = (sq_step_t)GetProcAddress(m, "sqlite3_step");
    sq_text_t tx = (sq_text_t)GetProcAddress(m, "sqlite3_column_text");
    sq_fin_t fi = (sq_fin_t)GetProcAddress(m, "sqlite3_finalize");
    sq_close_t cl = (sq_close_t)GetProcAddress(m, "sqlite3_close");
    sq_busy_t bu = (sq_busy_t)GetProcAddress(m, "sqlite3_busy_timeout");
    if (!op || !pr || !st || !tx || !fi || !cl) { log_text("auto stance: Windows SQLite incomplete, using Stance= from the ini"); return 0; }
    WCHAR la[300] = {0}; GetEnvironmentVariableW(L"LOCALAPPDATA", la, 300);
    int logged_missing = 0;
    for (;;) {
        WCHAR db[600]; char db8[1800];
        int got = -1;
        if (la[0] && gs_find_db(la, db, 600)) {
            gs_utf8(db, db8, 1800);
            void* h = 0;
            if (op(db8, &h, 1 /* SQLITE_OPEN_READONLY */, 0) == 0 && h) {
                if (bu) bu(h, 250);
                void* q = 0;
                if (pr(h, "select value_json from player_settings where scope='options' and name='DefaultStanceIsGoofy'", -1, &q, 0) == 0 && q) {
                    if (st(q) == 100 /* SQLITE_ROW */) { const unsigned char* v = tx(q, 0); if (v) got = v[0] == 't' ? 1 : 0; }
                    else got = 0;   /* never set in the game: regular */
                    fi(q);
                }
            }
            if (h) cl(h);
        } else if (!logged_missing) { logged_missing = 1; log_text("auto stance: no reSkate profile found, using Stance= from the ini"); }
        if (got >= 0 && got != g_game_goofy) {
            g_game_goofy = got;
            log_text(got ? "auto stance: the game says GOOFY" : "auto stance: the game says REGULAR");
        }
        Sleep(3000);
    }
    return 0;
}
static void gs_start(void) {
    static int started; if (started) return; started = 1;
    DWORD id; HANDLE th = CreateThread(0, 0, gs_thread, 0, 0, &id);
    if (th) CloseHandle(th);
}
#endif
