static const struct { const WCHAR* name; int vk; } k_ui_keys[] = {
    {L"End",0x23},{L"Home",0x24},{L"Insert",0x2D},{L"Delete",0x2E},
    {L"PageUp",0x21},{L"PageDown",0x22},{L"F1",0x70},{L"F2",0x71},
    {L"F3",0x72},{L"F4",0x73},{L"F5",0x74},{L"F6",0x75},{L"F7",0x76},
    {L"F8",0x77},{L"F9",0x78},{L"F10",0x79},{L"F11",0x7A},{L"F12",0x7B},
    {L"Pause",0x13},{L"ScrollLock",0x91},{L"NumLock",0x90}
};
static volatile int g_ui_bind_mode, g_ui_req_bound_key, g_ui_hotkey_sync;
static unsigned char g_ui_bind_down[256];
static int ui_hotkey_name(int vk,WCHAR* out) {
    if((vk>='A' && vk<='Z') || (vk>='0' && vk<='9')) {out[0]=(WCHAR)vk;out[1]=0;return 1;}
    for(unsigned i=0;i<sizeof k_ui_keys/sizeof k_ui_keys[0];i++) if(k_ui_keys[i].vk==vk) {
        unsigned n=0;while(k_ui_keys[i].name[n]) {out[n]=k_ui_keys[i].name[n];n++;}out[n]=0;return 1;
    }
    return 0;
}
static void ui_hotkey_apply(int vk) {
    WCHAR name[24];
    if(vk<0) {g_ui_bind_mode=0;g_ui_hotkey_sync=1;g_ui_preset_status=L"Hotkey change cancelled. Your existing key is unchanged.";g_ui_dirty=1;return;}
    if(!ui_hotkey_name(vk,name)) {
        g_ui_bind_mode=2;
        g_ui_preset_status=L"Use a letter, digit, F1-F12, Home/End, Insert/Delete, PageUp/PageDown, Pause, ScrollLock or NumLock. Escape or controller B cancels.";
        g_ui_dirty=1;return;
    }
    if(!WritePrivateProfileStringW(L"SessionControls",L"MenuKey",name,g_ini)) {
        g_ui_bind_mode=0;g_ui_hotkey_sync=1;
        g_ui_preset_status=L"Could not save the menu key. Your existing key is unchanged. Check that session_controls.ini is writable.";
        g_ui_dirty=1;return;
    }
    unsigned n=0;do {g_menu_key_name[n]=name[n];} while(name[n++]);
    g_ui_key=vk;g_ui_bind_mode=0;g_ui_hotkey_sync=1;
    g_ui_preset_status=L"Menu hotkey saved. The new key is shown below. Release it before using it to close or reopen the menu.";
    g_ui_dirty=1;log_text("menu: opening hotkey changed and saved");
}
/* Only called by the keyboard thread while the user requested a new key.
 * Seed held keys first so the activation key cannot become the new binding. */
static void ui_capture_keys(void) {
    if(g_ui_bind_mode==1) {
        for(int i=1;i<256;i++)g_ui_bind_down[i]=(GetAsyncKeyState(i)&0x8000)!=0;
        g_ui_bind_mode=2;return;
    }
    if(g_ui_bind_mode!=2)return;
    int pressed=0;
    for(int i=1;i<256;i++) {
        int down=(GetAsyncKeyState(i)&0x8000)!=0;
        if(down && !g_ui_bind_down[i] && !pressed && i>6 && i!=0x10 && i!=0x11 && i!=0x12 &&
           i!=0x5B && i!=0x5C && !(i>=0xA0 && i<=0xA5))pressed=i;
        g_ui_bind_down[i]=(unsigned char)down;
    }
    if(pressed) {g_ui_bind_mode=3;g_ui_req_bound_key=pressed==0x1B?-1:pressed;}
}
