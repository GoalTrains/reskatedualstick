#include "native_menu_frame.h"
__declspec(dllimport) BOOL WINAPI WriteProcessMemory(HANDLE,void*,const void*,UINT64,UINT64*);
static np_frame g_native_pose;
static int g_native_pose_valid,g_native_pose_pending;
static DWORD g_native_pose_user;
static int g_native_pose_goofy;
static double g_native_pose_until,g_native_pose_last;
static double g_native_pose_verify;
static int g_native_pose_armed;
static double g_native_pose_arm_time;
static UINT64 np_base(void) {
    static UINT64 base;static int checked,valid;
    if(!checked) {
        checked=1;base=(UINT64)GetModuleHandleW(0);
        const BYTE expected[32]={0x48,0x8b,0x87,0xb8,0x03,0,0,0x48,0x8b,0x97,0xc8,0x03,0,0,0x48,0x8b,0x48,0x60,0x0f,0xb6,0x42,0x12,0x88,0x81,0xa4,0,0,0,0x48,0x8b,0x87,0xb8};
        BYTE bytes[32];valid=base && rd_copy(base+0x47db20e,bytes,32);
        for(int i=0;i<32 && valid;i++)if(bytes[i]!=expected[i])valid=0;
        const BYTE basis_code[32]={0x48,0x8b,0x81,0xc8,0x03,0,0,0x4c,0x8b,0xd1,0x4c,0x8b,0x89,0xc0,0x03,0,0,0x0f,0xb6,0x50,0x10,0x45,0x8b,0x81,0xc4,0x13,0,0,0xc1,0xe2,0x0d,0x41};
        if(valid)valid=rd_copy(base+0x47d2160,bytes,32);
        for(int i=0;i<32 && valid;i++)if(bytes[i]!=basis_code[i])valid=0;
        if(!valid)log_text("native pose restore: build contract differs; no native state writes");
    }
    return valid?base:0;
}
static int np_write(np_address address,const void* data,np_address size) {
    SC_MBI m;UINT64 done=0;
    if(size!=4 || address<0x10000 || address>0x7fffffffffffULL-size ||
       !VirtualQuery((const void*)address,&m,sizeof m) || m.state!=0x1000 ||
       (m.prot&0xff)!=PAGE_READWRITE || (m.prot&0x100) ||
       address+size>(UINT64)m.base+m.size)return 0;
    return WriteProcessMemory(GetCurrentProcess(),(void*)address,data,size,&done) && done==size;
}
static void np_capture_live(double now) {
    /* Start suspends translation before ScreenData reports the native menu.
     * Keep the frame captured before that request, not a UI-mutated frame. */
    if(g_native_pose_armed) {
        if(g_native_pose_restore && now-g_native_pose_arm_time<=1000)return;
        g_native_pose_armed=g_native_pose_valid=0;
    }
    if(!g_native_pose_restore || !g_user_enabled || !g_cfg.enabled || g_paused || g_native_menu!=0 ||
       g_local<0 || g_slots[g_local].state!=100) {g_native_pose_valid=0;return;}
    if(g_native_pose_pending || now-g_native_pose_last<50)return;
    g_native_pose_last=now;
    g_native_pose_valid=np_frame_read(g_slots[g_local].sel,np_base(),rd_copy,&g_native_pose) &&
        g_native_pose.basis==0 && g_native_pose.native_frame==1 && g_native_pose.pose.pair[0]==1 && g_native_pose.pose.pair[1]==0;
    g_native_pose_goofy=g_cfg.goofy;
}
static void np_note_menu_request(double now,DWORD user,const sc_pad* raw) {
    if(!g_native_pose_restore || g_native_pose_armed || g_native_pose_pending ||
       g_native_menu!=0 || !g_native_pose_valid || g_ui_open || now-g_native_pose_last>200)return;
    if(!(raw->buttons&0x30) && !g_menu)return;
    g_native_pose_armed=1;g_native_pose_arm_time=now;g_native_pose_user=user;
}
static void np_begin_menu(double now,DWORD user) {
    if(g_native_pose_armed) {
        if(now-g_native_pose_arm_time>1000 || user!=g_native_pose_user)g_native_pose_valid=0;
    } else if(now-g_native_pose_last>200)g_native_pose_valid=0;
    g_native_pose_armed=0;
    g_native_pose_user=user;
    g_native_pose_pending=0;
}
static void np_end_menu(double now) {
    g_native_pose_pending=g_native_pose_restore && g_native_pose_valid;
    g_native_pose_until=now+1000;
    if(g_native_pose_pending)log_text("experimental menu frame recovery: pre-menu snapshot preserved; waiting for neutral ground input");
}
static void np_finish(double now,DWORD user,const sc_pad* raw,const sc_pad* out) {
    if(!g_native_pose_pending)return;
    if(user!=g_native_pose_user)return;
    if(!g_native_pose_restore || !g_user_enabled || !g_cfg.enabled || g_paused || g_native_menu!=0 || g_local<0 ||
       g_slots[g_local].sel!=g_native_pose.pose.selector || g_slots[g_local].state!=100 ||
       g_cfg.goofy!=g_native_pose_goofy || g_menu || g_ui_open || now>g_native_pose_until) {
        g_native_pose_pending=g_native_pose_valid=0;return;
    }
    if(raw->buttons || raw->lt>16 || raw->rt>16 || sc_abs(sc_axis(raw->lx))>.18f ||
       sc_abs(sc_axis(raw->ly))>.18f || sc_abs(sc_axis(raw->rx))>.18f || sc_abs(sc_axis(raw->ry))>.18f)return;
    if(out->buttons || out->lt>16 || out->rt>16 || sc_abs(sc_axis(out->lx))>.18f ||
       sc_abs(sc_axis(out->ly))>.18f || sc_abs(sc_axis(out->rx))>.18f || sc_abs(sc_axis(out->ry))>.18f)return;
    if(g_native_pose_pending==2) {
        if(now<g_native_pose_verify)return;
        np_frame current;
        int retained=np_frame_read(g_slots[g_local].sel,np_base(),rd_copy,&current) &&
            current.pose.source==g_native_pose.pose.source && current.pose.core==g_native_pose.pose.core &&
            current.pose.context==g_native_pose.pose.context && current.basis==g_native_pose.basis &&
            current.pose.pair[0]==g_native_pose.pose.pair[0] && current.pose.pair[1]==g_native_pose.pose.pair[1];
        log_text(retained?"experimental menu frame recovery: retained after native updates; pose still needs visual confirmation":
            "experimental menu frame recovery: native state overwrote correction; no repeated writes");
        g_native_pose_pending=g_native_pose_valid=0;
        return;
    }
    int result=np_frame_recover(&g_native_pose,g_slots[g_local].sel,np_base(),rd_copy,np_write);
    if(result==1) {
        g_native_pose_pending=2;g_native_pose_verify=now+150;
        log_text("experimental menu frame recovery: recorded reversed-frame signature restored; switch byte and controller input untouched; checking retention");
    } else if(result==0 || result==-2) {
        g_native_pose_pending=g_native_pose_valid=0;
    }
}
