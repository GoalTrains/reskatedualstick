#ifndef SC_RESKATE_COMPAT_H
#define SC_RESKATE_COMPAT_H
/* Existing x64 DingoSDKOverlayGetStatus ABI in official ReSkate 2.0.5.
 * Call from the menu worker only: its implementation takes the render mutex,
 * so querying it inside a controller poll on the render thread can deadlock. */
typedef struct {
    unsigned char started, bound, ready, visible, stopping, failed;
    unsigned long long rendered_frames;
} sc_reskate_status;
typedef char sc_reskate_status_size_check[sizeof(sc_reskate_status) == 16 ? 1 : -1];
static int sc_reskate_menu_visible(const sc_reskate_status* status) {
    return status->started && status->visible && !status->stopping && !status->failed;
}
static int sc_allow_menu_graphics_hook(int requested, int reskate_loaded) {
    return requested && !reskate_loaded;
}
#endif
