/* Settings snapshots use separate INI sections; loading replaces only the
 * active SessionControls section. Static buffers suit the freestanding DLL. */
#define UI_SNAPSHOT_CAP 8192
static WCHAR ui_snapshot[UI_SNAPSHOT_CAP];
static int ui_snapshot_read(const WCHAR* section) {
    DWORD n=GetPrivateProfileSectionW(section,ui_snapshot,UI_SNAPSHOT_CAP,g_ini);
    return n < UI_SNAPSHOT_CAP-2;
}
static int ui_snapshot_put(const WCHAR* key,const WCHAR* val) {
    unsigned used=0,kn=0,vn=0;
    while(key[kn])kn++; while(val[vn])vn++;
    while(ui_snapshot[used]) { while(ui_snapshot[used])used++; used++; }
    for(unsigned i=0;i<used;) {
        unsigned end=i; while(ui_snapshot[end])end++; end++;
        unsigned j=0;
        while(j<kn && wlow(ui_snapshot[i+j])==wlow(key[j]))j++;
        if(j==kn && ui_snapshot[i+j]=='=') {
            unsigned count=end-i;
            for(unsigned q=i;q<=used-count;q++)ui_snapshot[q]=ui_snapshot[q+count];
            used-=count;
        } else i=end;
    }
    if(used+kn+vn+3>UI_SNAPSHOT_CAP)return 0;
    for(unsigned j=0;j<kn;j++)ui_snapshot[used++]=key[j];
    ui_snapshot[used++]='=';
    for(unsigned j=0;j<vn;j++)ui_snapshot[used++]=val[j];
    ui_snapshot[used++]=0;ui_snapshot[used]=0;return 1;
}
static int ui_snapshot_int(const WCHAR* key,int value) {
    WCHAR val[24],rev[20];unsigned n=0,k=0;
    unsigned u=value<0 ? (unsigned)(-(INT64)value) : (unsigned)value;
    if(value<0)val[n++]='-';
    do { rev[k++]=(WCHAR)('0'+u%10);u/=10; } while(u);
    while(k)val[n++]=rev[--k];val[n]=0;
    return ui_snapshot_put(key,val);
}
static int ui_builtin_apply(int xl) {
    if(!ui_snapshot_read(L"SessionControls"))return 0;
    if(!ui_snapshot_int(L"FrontBackFoot",!xl) || !ui_snapshot_int(L"SwitchStance",xl?3:1) ||
       !ui_snapshot_int(L"PopOnRelease",xl) || !ui_snapshot_int(L"StickSteering",xl) ||
       !ui_snapshot_int(L"OneFootFlips",xl) ||
       !ui_snapshot_int(L"SwitchSource",4))return 0;
    if(!WritePrivateProfileSectionW(L"SessionControls",ui_snapshot,g_ini))return 0;
    g_cfg.feet_swapped=0;
    return 1;
}
static int ui_custom_save(void) {
    if(!ui_snapshot_read(L"SessionControls"))return 0;
    /* Save effective values even when they currently came from defaults. */
    if(!ui_snapshot_int(L"FrontBackFoot",g_cfg.front_back) ||
       !ui_snapshot_int(L"SwitchStance",g_cfg.switch_flip) ||
       !ui_snapshot_int(L"PopOnRelease",g_cfg.pop_on_release) ||
       !ui_snapshot_int(L"StickSteering",g_cfg.stick_steer) ||
       !ui_snapshot_int(L"SwitchSource",g_switch_src) ||
       !ui_snapshot_int(L"OneFootFlips",g_cfg.one_foot_flips) ||
       !ui_snapshot_int(L"RevertInput",g_cfg.revert_input) ||
       !ui_snapshot_int(L"SpinWindUp",g_cfg.prewind) ||
       !ui_snapshot_int(L"BodyFlips",g_cfg.body_flips) ||
       !ui_snapshot_int(L"AirControls",g_cfg.air_controls) ||
       !ui_snapshot_int(L"StockGrabAir",g_cfg.stock_grab_air) ||
       !ui_snapshot_int(L"StockTransitionGround",g_cfg.stock_transition_ground) ||
       !ui_snapshot_int(L"Powerslides",g_cfg.powerslide) ||
       !ui_snapshot_int(L"GrindBumpMs",(int)g_cfg.bump_ms) ||
       !ui_snapshot_int(L"AutoStance",g_auto_stance) ||
       !ui_snapshot_put(L"Stance",g_ini_goofy?L"goofy":L"regular") ||
       !ui_snapshot_int(L"MenuScale",g_ui_scale) ||
       !ui_snapshot_int(L"EnabledAtStart",g_user_enabled) ||
       !ui_snapshot_int(L"PresetFeetSwapped",g_cfg.feet_swapped) ||
       !ui_snapshot_int(L"PresetSaved",1))return 0;
    return WritePrivateProfileSectionW(ui_slot_section(),ui_snapshot,g_ini)!=0;
}
static int ui_custom_load(void) {
    if(!ui_slot_filled() || !ui_snapshot_read(ui_slot_section()))return 0;
    /* Startup display is a global menu preference, independent of gameplay
     * presets, including slots saved before this option existed. */
    if(!ui_snapshot_int(L"RampDiagnostics",g_ramp_diag) ||
       !ui_snapshot_int(L"ShowMenuOnStartup",g_show_startup_menu) ||
       !ui_snapshot_put(L"MenuKey",g_menu_key_name))return 0;
    if(!WritePrivateProfileSectionW(L"SessionControls",ui_snapshot,g_ini))return 0;
    g_user_enabled=(int)GetPrivateProfileIntW(ui_slot_section(),L"EnabledAtStart",1,g_ini)!=0;
    g_cfg.feet_swapped=(int)GetPrivateProfileIntW(ui_slot_section(),L"PresetFeetSwapped",0,g_ini)!=0;
    return 1;
}
