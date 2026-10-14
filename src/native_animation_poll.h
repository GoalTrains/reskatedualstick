#include "native_animation_reset.h"
static nar_pose g_nar;
static int g_nar_valid,g_nar_pending;
static DWORD g_nar_user;
static int g_nar_goofy;
static double g_nar_until,g_nar_last;
static double g_nar_verify;
static int g_nar_armed;
static double g_nar_arm_time;
static UINT64 nar_base(void) {
    UINT64 base=npp_base();BYTE code[16];
    const BYTE expected[16]={0x49,0x8b,0x48,0x60,0x48,0x83,0xe1,0xfb,0x74,0x24,0x48,0x8b,0x41,0x20,0x48,0x83};
    if(!base || !rd_copy(base+0xffd28d,code,16))return 0;
    for(int i=0;i<16;i++)if(code[i]!=expected[i])return 0;
    return base;
}
static void nar_capture_live(double now) {
    /* Start suspends translation before ScreenData reports the native menu.
     * Keep the frame captured before that request, not a UI-mutated frame. */
    if(g_nar_armed) {
        if(g_native_animation_reset && now-g_nar_arm_time<=1000)return;
        g_nar_armed=g_nar_valid=0;
    }
    if(!g_native_animation_reset || !g_user_enabled || !g_cfg.enabled || g_paused || g_native_menu!=0 ||
       g_local<0 || g_slots[g_local].state!=100) {g_nar_valid=0;return;}
    if(g_nar_pending || now-g_nar_last<50)return;
    g_nar_last=now;
    g_nar_valid=nar_read(g_slots[g_local].sel,nar_base(),rd_copy,&g_nar) &&
        g_nar.reset==0 && g_cfg.goofy==0 && g_nar.frame.basis==0 && g_nar.frame.native_frame==1 && g_nar.frame.pose.pair[0]==1 && g_nar.frame.pose.pair[1]==0;
    g_nar_goofy=g_cfg.goofy;
}
static void nar_note_menu_request(double now,DWORD user,const sc_pad* raw) {
    if(!g_native_animation_reset || g_nar_armed || g_nar_pending ||
       g_native_menu!=0 || !g_nar_valid || g_ui_open || now-g_nar_last>200)return;
    if(!(raw->buttons&0x30) && !g_menu)return;
    g_nar_armed=1;g_nar_arm_time=now;g_nar_user=user;
}
static void nar_begin_menu(double now,DWORD user) {
    if(g_nar_armed) {
        if(now-g_nar_arm_time>1000 || user!=g_nar_user)g_nar_valid=0;
    } else if(now-g_nar_last>200)g_nar_valid=0;
    g_nar_armed=0;
    g_nar_user=user;
    g_nar_pending=0;
}
static void nar_end_menu(double now) {
    g_nar_pending=g_native_animation_reset && g_nar_valid;
    g_nar_until=now+1000;
    if(g_nar_pending)log_text("experimental animation reset: pre-menu snapshot preserved; waiting for neutral ground input");
}
static void nar_finish(double now,DWORD user,const sc_pad* raw,const sc_pad* out) {
    if(g_nar_pending==2) {
        if(now<g_nar_verify)return;
        int result=nar_release(&g_nar,nar_base(),rd_copy,npp_write);
        if(result<0 && now<g_nar_verify+500)return;
        log_text(result<0?"experimental animation reset: release could not validate ownership; no further writes":
            "experimental animation reset: request released; visible pose still needs confirmation");
        g_nar_pending=g_nar_valid=0;return;
    }
    if(!g_nar_pending)return;
    if(user!=g_nar_user)return;
    if(!g_native_animation_reset || !g_user_enabled || !g_cfg.enabled || g_paused || g_native_menu!=0 || g_local<0 ||
       g_slots[g_local].sel!=g_nar.frame.pose.selector || g_slots[g_local].state!=100 ||
       g_cfg.goofy!=g_nar_goofy || g_menu || g_ui_open || now>g_nar_until) {
        g_nar_pending=g_nar_valid=0;return;
    }
    if(raw->buttons || raw->lt>16 || raw->rt>16 || sc_abs(sc_axis(raw->lx))>.18f ||
       sc_abs(sc_axis(raw->ly))>.18f || sc_abs(sc_axis(raw->rx))>.18f || sc_abs(sc_axis(raw->ry))>.18f)return;
    if(out->buttons || out->lt>16 || out->rt>16 || sc_abs(sc_axis(out->lx))>.18f ||
       sc_abs(sc_axis(out->ly))>.18f || sc_abs(sc_axis(out->rx))>.18f || sc_abs(sc_axis(out->ry))>.18f)return;
    int result=nar_request(&g_nar,g_slots[g_local].sel,nar_base(),rd_copy,npp_write);
    if(result==1) {
        g_nar_pending=2;g_nar_verify=now+24;
        log_text("experimental animation reset: Skater.ResetAnimation requested briefly; board/mirror properties, transforms and input untouched");
    } else if(result==0 || result==-2) {
        g_nar_pending=g_nar_valid=0;
    }
}
