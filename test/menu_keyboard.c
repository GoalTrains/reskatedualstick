#include <assert.h>
#include <stdio.h>
#include "../src/menu_controls.h"
static int g_ui_bind_mode,g_ui_req_bound_key,g_ui_req_toggle,g_ui_open,g_ui_page,g_ui_sel,g_ui_dirty;
static int g_ui_req_cards,g_ui_controls_page,g_ui_req_nav,g_ui_overwrite,g_ui_req_change;
static const unsigned short *g_ui_preset_status;
#define UI_NCARDS 24
#define UI_CARDS_PER_PAGE 6
static int changes,bound;
static int ui_row_count(void){return 10;}
static int ui_item_for_row(int n){return n;}
static void ui_change(int item,int direction){changes=item*10+direction;}
static void ui_hotkey_apply(int key){bound=key;g_ui_bind_mode=0;}
static void log_text(const char *text){(void)text;}
#include "../src/menu_keyboard.h"
int main(void){
 /* No controller poll: keyboard alone opens, navigates, edits and closes. */
 g_ui_req_toggle=1;ui_keyboard_actions();assert(g_ui_open && !g_ui_req_toggle);
 g_ui_req_nav=1;ui_keyboard_actions();assert(g_ui_sel==1);
 g_ui_req_change=1;ui_keyboard_actions();assert(changes==11);
 g_ui_req_cards=1;ui_keyboard_actions();assert(g_ui_controls_page==1);
 ui_keyboard_actions();assert(g_ui_controls_page==1 && changes==11);
 g_ui_page=2;g_ui_req_toggle=1;ui_keyboard_actions();assert(g_ui_open && g_ui_page==0 && g_ui_sel==9);
 g_ui_req_toggle=1;ui_keyboard_actions();assert(!g_ui_open);
 g_ui_bind_mode=1;g_ui_req_bound_key=0x24;ui_keyboard_actions();assert(bound==0x24 && !g_ui_bind_mode);
 puts("PASS: keyboard opens, navigates, changes settings, pages, backs out and rebinds without controller polls");
}
