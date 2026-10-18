/* Read-only local rider samples, not labelled native intent/target values.
 * Controller-poll observations at <=20 Hz, NOT synchronized physics steps.
 * At most 4096 records (~12 MiB), replaces the previous launch's file.
 * No native function calls, target writes or file I/O on the input path. */
#define GD_CAP 16
#define GD_LIMIT 4096
#include "grind_target_read.h"
#include "grind_entry_diag.h"
typedef struct {
    UINT64 time_us,selector,core,context;
    DWORD state,user,enabled,split,valid,dropped;
    sc_pad raw,out;
    float position[3],forward[3],velocity[3],spin[3];
    BYTE candidates[0xa20]; /* context +f00..191f; read-only observations */
    gt_sample native_grind; /* own validity bits, numeric labels from native exporter */
    gd_native_entry native_entry; /* bounded selector observations and read-only candidate inputs */
    DWORD entry_target,entry_rotation; /* supported rail gate and actual compensation, separate from selector acceptance */
} gd_record;
static gd_record gd_queue[GD_CAP];
static volatile int gd_lock,gd_failed;
static unsigned gd_read,gd_write,gd_samples,gd_dropped;
static int gd_started,gd_noted;
static UINT64 gd_last;
static DWORD WINAPI gd_worker(void* unused) {
    (void)unused;WCHAR path[560]={0};wcat(path,560,g_dir);wcat(path,560,L"\\session_controls_grind.bin");
    HANDLE f=CreateFileW(path,0x40000000,1|2|4,0,2,0x80,0);
    if(f==INVALID_HANDLE_VALUE){gd_failed=1;log_text("grind recorder: file open failed");return 0;}
    struct {char magic[8];DWORD size,version,limit,context_offset;} h={{'S','C','G','R','I','N','D','1'},sizeof(gd_record),5156,GD_LIMIT,0xf00};
    if(!pt_send(f,(const char*)&h,sizeof h))goto failed;
    log_text("grind recorder: session_controls_grind.bin; checked local rider only, max 4096 records, 20 Hz; context candidates are unlabelled; native_grind appends exporter numeric types, targeting flag and target direction with separate validity; NOT displayed trick names or synchronized physics steps");
    for(;;){
        static gd_record r;int have=0;
        if(!__sync_lock_test_and_set(&gd_lock,1)){
            if(gd_read!=gd_write){r=gd_queue[gd_read++%GD_CAP];have=1;}
            __sync_lock_release(&gd_lock);
        }
        if(have){if(!pt_send(f,(const char*)&r,sizeof r))goto failed;}else Sleep(20);
    }
failed:gd_failed=1;CloseHandle(f);log_text("grind recorder: write failed; file incomplete");return 0;
}
static void gd_capture(UINT64 now,DWORD user,const sc_pad* raw,const sc_pad* out,const sc_state* entry) {
    if(!g_grind_diag || gd_failed || user>=4 || g_local<0 || gd_samples>=GD_LIMIT ||
       now-gd_last<50 || native_local_slot()!=g_local || g_paused || g_native_menu!=0 || g_ui_open)return;
    DWORD state=g_slots[g_local].state;
    /* Include flat controls, air/approach and rail. Never scan other skaters. */
    if(state<100 || state>=500)return;
    if(__sync_lock_test_and_set(&gd_lock,1))return;
    if(gd_write-gd_read>=GD_CAP){gd_dropped++;__sync_lock_release(&gd_lock);return;}
    UINT64 base=(UINT64)GetModuleHandleW(0),selector=g_slots[g_local].sel;nr_frame frame,after;
    if(!nr_read(selector,base,rd_copy,&frame)){__sync_lock_release(&gd_lock);return;}
    gd_record* r=&gd_queue[gd_write%GD_CAP];memset(r,0,sizeof *r);
    r->time_us=rd_time();r->selector=selector;r->core=frame.core;r->context=frame.context;
    r->state=state;r->user=user;r->enabled=g_cfg.enabled && g_user_enabled;
    r->split=g_cfg.split_grinds;r->raw=*raw;r->out=*out;r->dropped=gd_dropped;
    if(entry && r->enabled){r->entry_target=entry->entry_rail_target;r->entry_rotation=entry->entry_rotation_active;}
    memcpy(r->position,frame.position,12);memcpy(r->forward,frame.forward,12);
    if(rd_copy(frame.context+0xf00,r->candidates,sizeof r->candidates))r->valid|=1;
    else memset(r->candidates,0,sizeof r->candidates);
    UINT64 holder,physics,vt,bodies,owner;DWORD count;
    if(np_pointer(rd_copy,frame.core+0x430,&holder) && np_pointer(rd_copy,holder+0x18,&physics) &&
       rd_copy(physics,&vt,8) && vt==base+0x65e67a0 && np_pointer(rd_copy,physics+0x20,&bodies) &&
       rd_copy(bodies,&count,4) && count==9 && rd_copy(bodies+0x10,&owner,8) && owner==physics &&
       rd_copy(bodies+0x70,r->velocity,12) && rd_copy(bodies+0x90,r->spin,12)) {
        int good=1;for(int i=0;i<3;i++)if(!__builtin_isfinite(r->velocity[i]) || sc_abs(r->velocity[i])>1000 ||
            !__builtin_isfinite(r->spin[i]) || sc_abs(r->spin[i])>1000)good=0;
        if(good)r->valid|=2;
    }
    if(!(r->valid&2)){memset(r->velocity,0,12);memset(r->spin,0,12);}
    gt_read(frame.core,rd_copy,&r->native_grind);
#ifdef SC_NATIVE_GRIND_ENTRY_H
    ne_diagnostic(&r->native_entry);
#endif
    DWORD native_state=0;
    if(!nr_read(selector,base,rd_copy,&after) || !nr_same(&frame,&after) ||
       !rd_copy(frame.context+0x1414,&native_state,4) || native_state!=state){__sync_lock_release(&gd_lock);return;}
    if(!gd_started){gd_started=1;DWORD id;HANDLE t=CreateThread(0,0,gd_worker,0,0,&id);if(t)CloseHandle(t);else gd_failed=1;}
    if(!gd_failed){gd_write++;gd_samples++;gd_last=now;}
    __sync_lock_release(&gd_lock);
    if(gd_samples==GD_LIMIT && !gd_noted){gd_noted=1;log_text("grind recorder: bounded recording complete; input CSV continues");}
}
