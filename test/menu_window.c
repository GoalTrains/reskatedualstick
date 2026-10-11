#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include "../src/reskate_compat.h"
int main(void) {
    HWND before=GetForegroundWindow();
    HWND owner=CreateWindowExW(0,L"STATIC",L"Hidden test owner",WS_POPUP,0,0,100,100,0,0,0,0);
    assert(owner);
    HWND menu=CreateWindowExW(SC_MENU_EX_STYLE,L"STATIC",L"Hidden test menu",WS_POPUP,0,0,100,100,owner,0,0,0);
    assert(menu);
    assert(GetWindow(menu,GW_OWNER)==owner);
    LONG_PTR style=GetWindowLongPtrW(menu,GWL_EXSTYLE);
    assert(style&WS_EX_NOACTIVATE);
    assert(!(style&WS_EX_TOPMOST));
    assert((SC_MENU_POSITION_FLAGS&(SWP_NOACTIVATE|SWP_NOZORDER))==(SWP_NOACTIVATE|SWP_NOZORDER));
    assert(GetForegroundWindow()==before);
    HWND real_game=CreateWindowExW(0,L"STATIC",L"Hidden replacement owner",WS_POPUP,0,0,100,100,0,0,0,0);
    assert(real_game);
    SetWindowLongPtrW(menu,GWLP_HWNDPARENT,(LONG_PTR)real_game);
    assert(GetWindow(menu,GW_OWNER)==real_game);
    DestroyWindow(owner);assert(IsWindow(menu));
    DestroyWindow(real_game);assert(!IsWindow(menu));
    puts("PASS: native game-owned non-topmost popup preserves focus; startup owner replacement and owned-window lifetime verified");
}
