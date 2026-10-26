#include "native_menu_respawn.h"
#include "native_respawn_board.h"
#include "ground_pose_guard.h"
#include "xl_stance_agreement.h"
#ifndef NR_FUNCTION
#define NR_FUNCTION(base,rva,type) ((type)((base)+(rva)))
#endif
__declspec(dllimport) DWORD WINAPI GetLastError(void);
__declspec(dllimport) void WINAPI SetLastError(DWORD);
typedef void (*nr_tick_fn)(UINT64,UINT64);
static nr_tick_fn g_nr_tick_prev;
static nr_frame g_nr_before,g_nr_last_motion;
static int g_nr_valid,g_nr_armed,g_nr_pending,g_nr_goofy,g_nr_idle;
static int g_nr_ground;
static sc_ground_pose_guard g_ground_pose;
static sc_xl_agreement g_xl_agreement;
static volatile UINT64 g_xl_agreement_owner,g_xl_agreement_seen;
static volatile int g_xl_agreement_hint=-1,g_xl_agreement_profile,g_xl_idle,g_xl_agreement_native;
static nr_frame g_xl_agreement_frame;
static volatile UINT64 g_xl_frame_owner,g_xl_frame_source,g_xl_frame_seen;
static volatile int g_xl_frame_profile,g_xl_baseline_mirror;
static volatile DWORD g_xl_input_user;
static DWORD g_nr_user;
static double g_nr_last,g_nr_arm,g_nr_until;
static UINT64 g_nr_next_hook;
static volatile int g_nr_lock,g_nr_hooked;
static volatile UINT64 g_native_owned_selector,g_native_owner_seen;
static UINT64 g_native_owner_next;
#ifdef SC_STANCE_FLIGHT_RECORDER
#include "stance_flight.h"
#include "stance_flight_native.h"
#endif
static int native_local_slot(void) {
    UINT64 now=GetTickCount64(),seen=g_native_owner_seen,selector=g_native_owned_selector;
    if(!selector || now<seen || now-seen>250)return -1;
    for(int i=0;i<16;i++)if(g_slots[i].sel==selector)return i;
    return -1;
}
static int nr_keep_board(UINT64 base,UINT64 client,const nr_frame* frame) {
    UINT64 address=nr_board_choice(base,client,frame,rd_copy),done=0;BYTE before,value=1;
    SC_MBI memory;
    if(!address || !rd_copy(address,&before,1))return 0;
    if(before==1)return 1;
    if(!VirtualQuery((void*)address,&memory,sizeof memory) || memory.state!=0x1000 ||
       (memory.prot&0xff)!=PAGE_READWRITE || (memory.prot&0x100) || address<(UINT64)memory.base ||
       address>=(UINT64)memory.base+memory.size)return 0;
    if(!WriteProcessMemory(GetCurrentProcess(),(void*)address,&value,1,&done) || done!=1)return 0;
    return rd_copy(address,&before,1) && before==1;
}
static UINT64 nr_base(void) {
    UINT64 base=np_base();BYTE actual[16];
    const UINT64 offsets[4]={0x565010,0x5956f80,0x3dc5e90,0x18b6ae0};
    const BYTE expected[4][16]={
        {0x48,0x89,0x5c,0x24,0x18,0x56,0x57,0x41,0x54,0x41,0x56,0x41,0x57,0x48,0x81,0xec},
        {0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8b,0xd9,0x33,0xc9,0xe8,0xc0,0x9e,0xbb,0xfa},
        {0x48,0x8b,0x01,0x48,0x85,0xc0,0x74,0x1c,0x48,0x8b,0xc8,0x48,0x83,0xe1,0xfb,0x74},
        {0x48,0x89,0x5c,0x24,0x08,0x57,0x48,0x83,0xec,0x20,0x65,0x48,0x8b,0x04,0x25,0x58}};
    if(!base)return 0;
    for(int j=0;j<4;j++) {
        if(!rd_copy(base+offsets[j],actual,16))return 0;
        for(int i=0;i<16;i++)if(actual[i]!=expected[j][i])return 0;
    }
    return base;
}
static int nr_client_ready(UINT64 base,UINT64 client) {
    DWORD state,offset,connection_offset,client_offset;BYTE ready;
    UINT64 context[2]={0},owner,connection,channel;
    if(!rd_copy(client+0xc4,&state,4) || (state!=13 && state!=21))return 0;
    NR_FUNCTION(base,0x18b6ae0,void*(*)(void*))(context);
    if(!context[0] || !rd_copy(base+0x7215038,&offset,4) || offset>0x1000000 ||
       !rd_copy(context[0]+offset+0x100,&ready,1) || ready!=1 ||
       !rd_copy(base+0x6fc25f0,&client_offset,4) || client_offset>0x1000000 ||
       !np_pointer(rd_copy,context[0]+client_offset,&owner) || owner!=client ||
       !rd_copy(base+0x715d5e8,&connection_offset,4) || connection_offset>0x1000000 ||
       !np_pointer(rd_copy,context[0]+connection_offset,&connection) ||
       !np_pointer(rd_copy,connection+0x50,&channel))return 0;
    return 1;
}
static void nr_service(UINT64 client) {
    if(__sync_lock_test_and_set(&g_nr_lock,1))return;
    if(!g_nr_pending){__sync_lock_release(&g_nr_lock);return;}
    UINT64 base=nr_base(),manager=0;DWORD manager_state=~0u;
    if(!g_native_respawn || !g_user_enabled || g_paused || g_native_menu!=0 || g_menu || g_ui_open ||
       g_local<0 || g_slots[g_local].sel!=g_nr_before.selector || g_slots[g_local].state!=100 ||
       g_cfg.goofy!=g_nr_goofy || g_now_ms>g_nr_until) {
        g_nr_pending=g_nr_valid=0;__sync_lock_release(&g_nr_lock);return;
    }
    if(!g_cfg.enabled || !g_nr_idle || !base || !nr_client_ready(base,client) ||
       !np_pointer(rd_copy,base+0x71ee790,&manager) || !rd_copy(manager,&manager_state,4) || manager_state!=0) {
        __sync_lock_release(&g_nr_lock);return;
    }
    nr_frame current;
    if(!nr_read(g_nr_before.selector,base,rd_copy,&current)) {__sync_lock_release(&g_nr_lock);return;}
    /* The timer/export clears roughly 100 ms before the basis changes in the
     * recorded failure. Keep waiting within the bounded exit window. */
    if(g_nr_ground) {
        if(!nr_same(&g_nr_before,&current) || current.basis!=1 || current.native_frame!=1 ||
           current.mirrored!=g_nr_before.mirrored) {
            g_nr_pending=0;__sync_lock_release(&g_nr_lock);return;
        }
    } else if(!nr_mismatch(&g_nr_before,&current)) {__sync_lock_release(&g_nr_lock);return;}
    __declspec(align(16)) float matrix[16];
    if(!nr_transform(&current,g_nr_last_motion.position,g_now_ms-g_nr_last,g_nr_ground?0:g_nr_before.native_frame,matrix)) {
        g_nr_pending=g_nr_valid=0;__sync_lock_release(&g_nr_lock);return;
    }
    if(!nr_keep_board(base,client,&current)){__sync_lock_release(&g_nr_lock);return;}
    g_nr_pending=g_nr_valid=g_nr_armed=0;__sync_lock_release(&g_nr_lock);
    /* Same native API/transition lifetime as ReSkate's position teleport;
     * called from the chained client tick, never a logger/input worker. */
    __declspec(align(16)) UINT64 transition[2]={0};
    NR_FUNCTION(base,0x5956f80,void*(*)(void*))(transition);
    DWORD request=NR_FUNCTION(base,0x565010,DWORD(*)(UINT64,const float*,unsigned,const void*))(manager,matrix,1,transition);
    NR_FUNCTION(base,0x3dc5e90,void(*)(void*))(transition);
    log_text("experimental menu pose recovery: native in-place teleport/reset requested with on-board choice; speed resets; game chooses configured stance");
    log_num("  native recovery request id ",request);
}
#ifdef SC_FAST_FAKIE_EXIT
#include "fakie_exit_native.h"
#endif
static void nr_client_tick(UINT64 client,UINT64 update) {
    g_nr_tick_prev(client,update);
    DWORD error=GetLastError();UINT64 now=GetTickCount64();
    if(now>=g_native_owner_next) {
        g_native_owner_next=now+40;
        UINT64 base=nr_base();nr_local_owner owner;
        UINT64 selector=base && nr_client_ready(base,client) && nr_local_owner_read(base,client,rd_copy,&owner)?owner.selector:0;
        g_native_owned_selector=selector;g_native_owner_seen=now;
#ifdef SC_FAST_FAKIE_EXIT
        fke_tick(base,selector,now);
#endif
        nr_frame frame;
        int track=g_native_respawn && g_native_ground_respawn && g_game_goofy>=0 && g_game_goofy<=1 &&
            g_cfg.goofy==g_game_goofy && g_user_enabled && g_cfg.enabled && !g_paused &&
            g_native_menu==0 && !g_menu && !g_ui_open && selector && g_local>=0 && g_slots[g_local].sel==selector;
        int valid=track && nr_read(selector,base,rd_copy,&frame),triggered=0;
        int had_baseline=g_ground_pose.baseline_valid;
        if(track && !valid)sc_ground_pose_missing(&g_ground_pose,(double)now);
        else triggered=sc_ground_pose_step(&g_ground_pose,valid?&frame:0,valid?(int)g_slots[g_local].state:-1,g_nr_idle,g_cfg.goofy,(double)now);
        if(!had_baseline && g_ground_pose.baseline_valid) {
            log_text(g_cfg.goofy?"native pose recovery: settled GOOFY mirror baseline learned":"native pose recovery: settled REGULAR mirror baseline learned");
            log_num("  learned mirror value ",g_ground_pose.baseline_mirror);
        }
        /* A manual stance override changes physical stick ownership, not the
         * native frame contract. Keep collecting when Skate reports a different
         * configured stance; otherwise XL silently falls back after reverts. */
        int xl_track=sc_physical_foot_layout(g_cfg.front_back,g_cfg.switch_flip) && g_user_enabled && g_cfg.enabled && !g_paused &&
            g_native_menu==0 && !g_menu && !g_ui_open && selector && g_local>=0 && g_slots[g_local].sel==selector;
        int xl_valid=xl_track && (valid || nr_read(selector,base,rd_copy,&frame));
        int hint=-1;
        /* UI/unreadable frames cannot redefine the owned rider's baseline.
         * The next valid frame still resets on owner/core/profile changes. */
        if(!xl_valid)sc_xl_agreement_missing(&g_xl_agreement);
        else hint=sc_xl_agreement_step(&g_xl_agreement,xl_valid?&frame:0,xl_valid?(int)g_slots[g_local].state:-1,g_xl_idle,g_cfg.goofy,(double)now);
#ifdef SC_STANCE_FLIGHT_RECORDER
        if(g_stance_flight)sf_native_capture(base,selector,now,valid,track,triggered,xl_track,xl_valid,hint);
#endif
        g_xl_frame_owner=0;
        if(xl_valid && g_xl_agreement.baseline_valid){
            g_xl_frame_profile=g_cfg.goofy;g_xl_baseline_mirror=g_xl_agreement.mirror;
            g_xl_frame_source=frame.source;g_xl_frame_seen=now;g_xl_frame_owner=selector;
        }
        g_xl_agreement_owner=0;
        if(hint>=0){g_xl_agreement_frame=frame;g_xl_agreement_native=frame.native_frame;g_xl_agreement_profile=g_cfg.goofy;g_xl_agreement_hint=hint;g_xl_agreement_seen=now;g_xl_agreement_owner=selector;}
        if(triggered && !__sync_lock_test_and_set(&g_nr_lock,1)) {
            if(!g_nr_pending) {
                g_nr_before=g_nr_last_motion=frame;g_nr_last=g_now_ms;g_nr_goofy=g_cfg.goofy;
                g_nr_valid=g_nr_pending=g_nr_ground=1;g_nr_until=g_now_ms+1500;
                  log_text(triggered==2 ? "native pose recovery: grounded mirror break persisted into a reversed-basis landing; neutral forward travel confirmed; queuing native reset" :
                      "native pose recovery: recorded grounded frame break followed by regular-direction travel; neutral controls confirmed; queuing native reset");
            }
            __sync_lock_release(&g_nr_lock);
        }
    }
    nr_service(client);SetLastError(error);
}
/* Read a single source flag bundle from this owned rider. The paired native
 * frame must still match the export, so a half-published update is ignored.
 * This changes the physical stance evidence, never Skate's input frame. */
static int nr_xl_physical_raw(UINT64 selector,int profile,int native,int* mirror_delta) {
    UINT64 owner=g_xl_frame_owner,source=g_xl_frame_source,now=GetTickCount64(),seen=g_xl_frame_seen;
    BYTE flags[5];
    if(!owner || owner!=selector || profile!=g_xl_frame_profile || now<seen || now-seen>200 ||
       !source || !rd_copy(source+0x10,flags,5) || flags[0]>1 || flags[2]>1 || flags[4]!=native ||
       owner!=g_xl_frame_owner || source!=g_xl_frame_source)return -1;
    *mirror_delta=flags[2]^g_xl_baseline_mirror;
    return sc_xl_physical_raw(native,flags[2],g_xl_baseline_mirror);
}
static int nr_in_module(UINT64 module,UINT64 address) {
    DWORD pe,size,signature;BYTE sig[2];
    if(!module || !rd_copy(module,sig,2) || sig[0]!='M' || sig[1]!='Z' ||
       !rd_copy(module+0x3c,&pe,4) || pe>0x100000 ||
       !rd_copy(module+pe,&signature,4) || signature!=0x4550 || !rd_copy(module+pe+0x50,&size,4) ||
       size<0x1000 || size>0x30000000)return 0;
    return address>=module && address<module+size;
}
#ifdef SC_HOLD_FAKIE_COMPAT
#include "hold_fakie_hook.h"
#endif
static void nr_try_hook(UINT64 now) {
#ifdef SC_HOLD_FAKIE_COMPAT
    hf_try_hook(now);
#endif
    if(g_nr_hooked || now<g_nr_next_hook)return;
    g_nr_next_hook=now+500;UINT64 base=nr_base(),module=(UINT64)GetModuleHandleW(L"ReSkate.dll");
    if(!base || !module)return;
    BYTE* entry=(BYTE*)(base+0x2e77190);BYTE bytes[16];UINT64 previous=0,slot=0,original;
    if(!rd_copy((UINT64)entry,bytes,16))return;
    memcpy(&original,bytes,8);
    if(bytes[0]==0xe9){int rel;memcpy(&rel,bytes+1,4);previous=(UINT64)entry+5+rel;}
    else if(bytes[0]==0xff && bytes[1]==0x25){int rel;memcpy(&rel,bytes+2,4);slot=(UINT64)entry+6+rel;if(!rd_copy(slot,&previous,8))return;}
    else return; /* Wait for the official runtime to install its tick hook. */
    UINT64 target=previous;
    for(int depth=0;depth<4 && !nr_in_module(module,target);depth++) {
        if(!rd_copy(target,bytes,16))return;
        if(bytes[0]==0xe9){int rel;memcpy(&rel,bytes+1,4);target+=5+rel;}
        else if(bytes[0]==0xff && bytes[1]==0x25){int rel;memcpy(&rel,bytes+2,4);if(!rd_copy(target+6+rel,&target,8))return;}
        else return;
    }
    if(!nr_in_module(module,target))return;
    DWORD old;
    if(slot) {
        if((slot&7) || !VirtualProtect((void*)slot,8,0x40,&old))return;
        g_nr_tick_prev=(nr_tick_fn)previous;
        UINT64 changed=__sync_val_compare_and_swap((volatile UINT64*)slot,previous,(UINT64)nr_client_tick);
        DWORD unused;VirtualProtect((void*)slot,8,old,&unused);
        if(changed!=previous){g_nr_hooked=-1;log_text("native pose recovery: tick hook changed during installation; recovery disabled");return;}
    } else {
        BYTE* stub=alloc_near((UINT64)entry);if(!stub)return;
        stub[0]=0xff;stub[1]=0x25;memset(stub+2,0,4);UINT64 hook=(UINT64)nr_client_tick;memcpy(stub+6,&hook,8);
        FlushInstructionCache(GetCurrentProcess(),stub,14);
        INT64 rel=(INT64)stub-(INT64)(entry+5);if(rel>0x7fffffffLL || rel<-0x7fffffffLL)return;
        UINT64 replacement=original;
        ((BYTE*)&replacement)[0]=0xe9;int rel32=(int)rel;memcpy((BYTE*)&replacement+1,&rel32,4);
        if(!VirtualProtect(entry,8,0x40,&old))return;g_nr_tick_prev=(nr_tick_fn)previous;
        UINT64 changed=__sync_val_compare_and_swap((volatile UINT64*)entry,original,replacement);
        DWORD unused;VirtualProtect(entry,8,old,&unused);FlushInstructionCache(GetCurrentProcess(),entry,8);
        if(changed!=original){g_nr_hooked=-1;log_text("native pose recovery: tick hook changed during installation; recovery disabled");return;}
    }
    g_nr_hooked=1;log_text("local-player tracking: chained official ReSkate client tick; original runtime retained");
}
static void nr_capture_live(double now) {
    if(__sync_lock_test_and_set(&g_nr_lock,1))return;
    if(g_nr_armed && now-g_nr_arm<=1000){__sync_lock_release(&g_nr_lock);return;}
    if(g_nr_armed){g_nr_armed=g_nr_valid=0;}
    if(!g_native_respawn || !g_user_enabled || !g_cfg.enabled || g_paused || g_native_menu!=0 ||
       g_local<0 || g_slots[g_local].state!=100)g_nr_valid=0;
    else if(!g_nr_pending && now-g_nr_last>=50) {
        nr_frame current;
        g_nr_valid=nr_read(g_slots[g_local].sel,nr_base(),rd_copy,&current) && current.native_frame==1;
        if(g_nr_valid){g_nr_before=current;g_nr_last_motion=current;g_nr_goofy=g_cfg.goofy;g_nr_last=now;}
    }
    __sync_lock_release(&g_nr_lock);
}
static int nr_neutral(const sc_pad* pad) {
    return !pad->buttons && pad->lt<=16 && pad->rt<=16 &&
        sc_abs(sc_axis(pad->lx))<.18f && sc_abs(sc_axis(pad->ly))<.18f &&
        sc_abs(sc_axis(pad->rx))<.18f && sc_abs(sc_axis(pad->ry))<.18f;
}
static int nr_xl_feet_neutral(const sc_pad* pad,float deadzone) {
    float zone=deadzone>0 && deadzone<.18f?deadzone:.18f;
    return !pad->buttons && sc_abs(sc_axis(pad->lx))<=zone && sc_abs(sc_axis(pad->ly))<=zone &&
        sc_abs(sc_axis(pad->rx))<=zone && sc_abs(sc_axis(pad->ry))<=zone;
}
static void nr_note_menu(double now,DWORD user,const sc_pad* raw,const sc_pad* out) {
    float xl_deadzone=g_cfg.deadzone>0 && g_cfg.deadzone<.18f?g_cfg.deadzone:.18f;
    g_xl_idle=nr_xl_feet_neutral(raw,xl_deadzone) && !out->buttons &&
        sc_abs(sc_axis(out->rx))<=xl_deadzone && sc_abs(sc_axis(out->ry))<=xl_deadzone;
    g_xl_input_user=user;
    if(__sync_lock_test_and_set(&g_nr_lock,1))return;
    g_nr_idle=nr_neutral(raw) && nr_neutral(out);
#ifdef SC_HOLD_FAKIE_COMPAT
    int hold_state=g_local>=0?(int)g_slots[g_local].state:0;
    int hold_eligible=g_user_enabled&&g_cfg.enabled&&g_cfg.pure&&!g_paused&&!g_menu&&!g_ui_open&&g_native_menu==0&&
        g_local>=0&&g_slots[g_local].sel==g_native_owned_selector&&hold_state>=100&&hold_state<300;
    const sc_state* held_state=&g_state[user<4?user:0];
    int hold_input=held_state->d_land_hold || sc_fakie_vertical_hold(&g_cfg,raw->lx,raw->ly) ||
        sc_fakie_vertical_hold(&g_cfg,raw->rx,raw->ry) || sc_fakie_vertical_hold(&g_cfg,out->rx,out->ry);
    hf_note_hold(&hf_held_stance,g_native_owned_selector,GetTickCount64(),hold_eligible,hold_input);
#endif

#ifdef SC_FAST_FAKIE_EXIT
    g_fke_protected=sc_fakie_protect_input(&g_cfg,&g_state[user<4?user:0],raw,out);
    g_fke_input_seen=GetTickCount64();
#endif
    if(g_native_respawn && !g_nr_armed && !g_nr_pending && g_native_menu==0 &&
       g_nr_valid && !g_ui_open && now-g_nr_last<=200 && ((raw->buttons&0x30) || g_menu)) {
        g_nr_armed=1;g_nr_arm=now;g_nr_user=user;
    }
    if(user!=g_nr_user)g_nr_idle=0;
    __sync_lock_release(&g_nr_lock);
}
static void nr_begin_menu(double now,DWORD user) {
    if(__sync_lock_test_and_set(&g_nr_lock,1))return;
    if(g_nr_armed){if(now-g_nr_arm>1000 || user!=g_nr_user)g_nr_valid=0;}
    else {
        g_nr_valid=0; /* Loading/respawn screens are not deliberate menu entries. */
        if(g_native_respawn)log_text("native pose recovery: screen entered without a Start/Back request; automatic reset skipped");
    }
    g_nr_armed=g_nr_pending=g_nr_ground=0;g_nr_user=user;__sync_lock_release(&g_nr_lock);
}
static void nr_end_menu(double now) {
    if(__sync_lock_test_and_set(&g_nr_lock,1))return;
    g_nr_pending=g_native_respawn && g_nr_valid;g_nr_until=now+1500;
    if(g_nr_pending)log_text("native pose recovery: waiting for neutral grounded controls and an idle native teleport manager");
    __sync_lock_release(&g_nr_lock);
}
