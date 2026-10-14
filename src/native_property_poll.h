#include "native_pose_property.h"
static npp_pose g_npp;
static int g_npp_valid,g_npp_pending;
static DWORD g_npp_user;
static int g_npp_goofy;
static double g_npp_until,g_npp_last;
static double g_npp_verify;
static int g_npp_armed;
static double g_npp_arm_time;
static UINT64 npp_base(void) {
    UINT64 base=np_base();BYTE code[16];
    const BYTE getter[16]={0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8d,0x59,0xa0,0x48,0x8b,0xcb,0xe8,0xfe,0xf8};
    const BYTE setter[16]={0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8b,0xc2,0x41,0x0f,0xb6,0xd8,0x48,0x8d,0x51};
    if(!base || !rd_copy(base+0x11dd4c0,code,16))return 0;
    for(int i=0;i<16;i++)if(code[i]!=getter[i])return 0;
    if(!rd_copy(base+0x11dd7a0,code,16))return 0;
    for(int i=0;i<16;i++)if(code[i]!=setter[i])return 0;
    return base;
}
static int npp_write(np_address address,const void* data,np_address size) {
    SC_MBI m;UINT64 done=0;
    if(size!=1 || address<0x10000 || address>=0x7fffffffffffULL ||
       !VirtualQuery((const void*)address,&m,sizeof m) || m.state!=0x1000 ||
       (m.prot&0xff)!=PAGE_READWRITE || (m.prot&0x100) || address+size>(UINT64)m.base+m.size)return 0;
    return WriteProcessMemory(GetCurrentProcess(),(void*)address,data,size,&done) && done==size;
}
static void npp_capture_live(double now) {
    /* Start suspends translation before ScreenData reports the native menu.
     * Keep the frame captured before that request, not a UI-mutated frame. */
    if(g_npp_armed) {
        if(g_native_property_restore && now-g_npp_arm_time<=1000)return;
        g_npp_armed=g_npp_valid=0;
    }
    if(!g_native_property_restore || !g_user_enabled || !g_cfg.enabled || g_paused || g_native_menu!=0 ||
       g_local<0 || g_slots[g_local].state!=100) {g_npp_valid=0;return;}
    if(g_npp_pending || now-g_npp_last<50)return;
    g_npp_last=now;
    g_npp_valid=npp_read(g_slots[g_local].sel,npp_base(),rd_copy,&g_npp) &&
        g_npp.frame.basis==0 && g_npp.frame.native_frame==1 && g_npp.frame.pose.pair[0]==1 && g_npp.frame.pose.pair[1]==0;
    g_npp_goofy=g_cfg.goofy;
}
static void npp_note_menu_request(double now,DWORD user,const sc_pad* raw) {
    if(!g_native_property_restore || g_npp_armed || g_npp_pending ||
       g_native_menu!=0 || !g_npp_valid || g_ui_open || now-g_npp_last>200)return;
    if(!(raw->buttons&0x30) && !g_menu)return;
    g_npp_armed=1;g_npp_arm_time=now;g_npp_user=user;
}
static void npp_begin_menu(double now,DWORD user) {
    if(g_npp_armed) {
        if(now-g_npp_arm_time>1000 || user!=g_npp_user)g_npp_valid=0;
    } else if(now-g_npp_last>200)g_npp_valid=0;
    g_npp_armed=0;
    g_npp_user=user;
    g_npp_pending=0;
}
static void npp_end_menu(double now) {
    g_npp_pending=g_native_property_restore && g_npp_valid;
    g_npp_until=now+1000;
    if(g_npp_pending)log_text("experimental animation property recovery: pre-menu snapshot preserved; waiting for neutral ground input");
}
static void npp_finish(double now,DWORD user,const sc_pad* raw,const sc_pad* out) {
    if(!g_npp_pending)return;
    if(user!=g_npp_user)return;
    if(!g_native_property_restore || !g_user_enabled || !g_cfg.enabled || g_paused || g_native_menu!=0 || g_local<0 ||
       g_slots[g_local].sel!=g_npp.frame.pose.selector || g_slots[g_local].state!=100 ||
       g_cfg.goofy!=g_npp_goofy || g_menu || g_ui_open || now>g_npp_until) {
        g_npp_pending=g_npp_valid=0;return;
    }
    if(raw->buttons || raw->lt>16 || raw->rt>16 || sc_abs(sc_axis(raw->lx))>.18f ||
       sc_abs(sc_axis(raw->ly))>.18f || sc_abs(sc_axis(raw->rx))>.18f || sc_abs(sc_axis(raw->ry))>.18f)return;
    if(out->buttons || out->lt>16 || out->rt>16 || sc_abs(sc_axis(out->lx))>.18f ||
       sc_abs(sc_axis(out->ly))>.18f || sc_abs(sc_axis(out->rx))>.18f || sc_abs(sc_axis(out->ry))>.18f)return;
    if(g_npp_pending==2) {
        if(now<g_npp_verify)return;
        npp_pose current;
        int retained=npp_read(g_slots[g_local].sel,npp_base(),rd_copy,&current) &&
            npp_same(&g_npp,&current) && current.mirrored==g_npp.mirrored;
        log_text(retained?"experimental animation property recovery: retained after native updates; pose still needs visual confirmation":
            "experimental animation property recovery: native state overwrote correction; no repeated writes");
        g_npp_pending=g_npp_valid=0;
        return;
    }
    int result=npp_restore(&g_npp,g_slots[g_local].sel,npp_base(),rd_copy,npp_write);
    if(result==1) {
        g_npp_pending=2;g_npp_verify=now+150;
        log_text("experimental animation property recovery: Anim.Mirrored property restored once; exported flags and input untouched; checking retention");
    } else if(result==0 || result==-2) {
        g_npp_pending=g_npp_valid=0;
    }
}
