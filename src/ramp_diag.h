/* Checked skater snapshots, labelled by the user; candidates are not treated
 * as angle/velocity until correlated against ramp and flat-ground recordings.
 * At most 2400 samples per launch, 10 Hz. File I/O is on a background thread. */
__declspec(dllimport) BOOL WINAPI ReadProcessMemory(HANDLE,const void*,void*,UINT64,UINT64*);
#define RD_CAP 16
#define RD_BYTES 1024
#define RD_BLOCKS 8
typedef struct {
    UINT64 time_us, selector;
    DWORD state, label, dropped, valid;
    UINT64 address[RD_BLOCKS]; BYTE bytes[RD_BLOCKS][RD_BYTES];
} rd_record;
static rd_record rd_queue[RD_CAP];
static volatile int rd_lock,rd_started,rd_failed;
static unsigned rd_read,rd_write,rd_dropped,rd_samples;
static UINT64 rd_last;
static UINT64 rd_time(void) {
    static INT64 frequency;INT64 tick=0;
    if(!frequency && (!QueryPerformanceFrequency(&frequency) || frequency<=0))return 0;
    if(!QueryPerformanceCounter(&tick))return 0;
    return (UINT64)tick/(UINT64)frequency*1000000+(UINT64)tick%(UINT64)frequency*1000000/(UINT64)frequency;
}
static int rd_copy(UINT64 address,void* out,UINT64 size) {
    UINT64 got=0;
    if(address<0x10000 || address>0x7FFFFFFFFFFFull || !readable(address,size))return 0;
    return ReadProcessMemory(GetCurrentProcess(),(const void*)address,out,size,&got) && got==size;
}
static UINT64 rd_pointer(UINT64 address) {UINT64 value=0;rd_copy(address,&value,8);return value;}
static DWORD WINAPI rd_worker(void* unused) {
    (void)unused;WCHAR path[560];path[0]=0;wcat(path,560,g_dir);wcat(path,560,L"\\session_controls_ramp.bin");
    HANDLE f=CreateFileW(path,4,1|2|4,0,4,0x80,0);
    if(f==INVALID_HANDLE_VALUE){rd_failed=1;log_text("ramp diagnostics: could not open snapshot file");return 0;}
    struct {char magic[8];DWORD size,version;} header={{'S','C','R','A','M','P','0','1'},sizeof(rd_record),571};
    if(!pt_send(f,(const char*)&header,sizeof header))goto failed;
    log_text("ramp diagnostics: labelled checked snapshots in session_controls_ramp.bin; no ramp values inferred yet");
    for(;;) {
        static rd_record record;int have=0;
        if(!__sync_lock_test_and_set(&rd_lock,1)) {
            if(rd_read!=rd_write){record=rd_queue[rd_read++%RD_CAP];have=1;}
            __sync_lock_release(&rd_lock);
        }
        if(have){if(!pt_send(f,(const char*)&record,sizeof record))goto failed;}
        else Sleep(20);
    }
failed:rd_failed=1;CloseHandle(f);log_text("ramp diagnostics: write failed; recording incomplete");return 0;
}
static void rd_capture(UINT64 now) {
    if(!g_ramp_diag || rd_failed || (rd_samples>=2400 && !g_ramp_diag_reset) || now-rd_last<100 || g_local<0 || !g_slots[g_local].sel)return;
    if(__sync_lock_test_and_set(&rd_lock,1))return;
    if(g_ramp_diag_reset){rd_samples=0;g_ramp_diag_reset=0;}
    rd_last=now;
    if(!rd_started) {
        rd_started=1;DWORD id;HANDLE thread=CreateThread(0,0,rd_worker,0,0,&id);
        if(thread)CloseHandle(thread);else rd_failed=1;
    }
    if(rd_failed){__sync_lock_release(&rd_lock);return;}
    if(rd_write-rd_read>=RD_CAP){rd_dropped++;__sync_lock_release(&rd_lock);return;}
    rd_record* r=&rd_queue[rd_write%RD_CAP];memset(r,0,sizeof *r);
    r->time_us=rd_time();r->selector=g_slots[g_local].sel;r->state=g_slots[g_local].state;r->label=g_ramp_diag;r->dropped=rd_dropped;
    UINT64 root=rd_pointer(r->selector),p=rd_pointer(r->selector+0x10);
    r->address[0]=r->selector;r->address[1]=root;
    r->address[2]=rd_pointer(root+0x60);r->address[3]=rd_pointer(root+0x78);
    r->address[4]=p;r->address[5]=rd_pointer(p+0x10);
    r->address[6]=rd_pointer(root+0x68);r->address[7]=rd_pointer(root+0x70);
    for(int i=0;i<RD_BLOCKS;i++)if(rd_copy(r->address[i],r->bytes[i],RD_BYTES))r->valid|=1u<<i;
    rd_write++;rd_samples++;
    __sync_lock_release(&rd_lock);
}
