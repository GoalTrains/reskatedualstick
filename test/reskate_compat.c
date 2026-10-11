#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include "../src/reskate_compat.h"
int main(void) {
    assert(sizeof(sc_reskate_status) == 16);
    assert(offsetof(sc_reskate_status, rendered_frames) == 8);
    /* Old saved settings must not re-enable the conflicting graphics hook. */
    assert(!sc_allow_menu_graphics_hook(1,1));
    assert(!sc_allow_menu_graphics_hook(0,1));
    assert(!sc_allow_menu_graphics_hook(0,0));
    assert(sc_allow_menu_graphics_hook(1,0));
    sc_reskate_status s = {0};
    assert(!sc_reskate_menu_visible(&s));
    s.started = s.bound = s.ready = 1;
    assert(!sc_reskate_menu_visible(&s));
    s.visible = 1; assert(sc_reskate_menu_visible(&s));
    s.visible = 0; assert(!sc_reskate_menu_visible(&s));
    s.visible = 1; s.stopping = 1; assert(!sc_reskate_menu_visible(&s));
    s.stopping = 0; s.failed = 1; assert(!sc_reskate_menu_visible(&s));
    puts("PASS: official ReSkate status ABI, menu open/close/failure and graphics-hook compatibility");
}
