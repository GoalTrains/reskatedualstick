/* Lossless changes at the controller-poll boundary. Only the worker writes files.
 * Duplicate polls get a heartbeat at 100 ms; changed values are never throttled. */
#define PT_FIELDS 79
#define PT_CAP 4096
#ifndef PT_FILE_LIMIT
#define PT_FILE_LIMIT (16u * 1024u * 1024u)
#endif
typedef struct { UINT64 poll, start_us, end_us, dropped; int v[PT_FIELDS]; } pt_record;
static pt_record pt_queue[PT_CAP], pt_last[4];
static int pt_seen[4], pt_started;
static volatile int pt_failed;
static volatile int pt_lock;
static unsigned pt_read, pt_write;
static UINT64 pt_poll, pt_dropped, pt_freq;
static UINT64 pt_time(void) {
    INT64 q; if (!pt_freq || !QueryPerformanceCounter(&q)) return 0;
    return (UINT64)q / pt_freq * 1000000 + ((UINT64)q % pt_freq) * 1000000 / pt_freq;
}
static char* pt_num(char* p, UINT64 n) {
    char b[24]; int k=0; do { b[k++]=(char)('0'+n%10); n/=10; } while(n);
    while(k) *p++=b[--k]; return p;
}
static unsigned pt_format(const pt_record* r, char* line) {
    char* p=line; p=pt_num(p,r->poll); *p++=','; p=pt_num(p,r->start_us); *p++=',';
    p=pt_num(p,r->end_us); *p++=','; p=pt_num(p,r->dropped);
    for(int i=0;i<PT_FIELDS;i++) { INT64 n=r->v[i]; *p++=','; if(n<0) { *p++='-'; n=-n; } p=pt_num(p,(UINT64)n); }
    *p++='\r'; *p++='\n'; return (unsigned)(p-line);
}
static int pt_send(HANDLE f, const char* p, unsigned n) {
    while(n) { DWORD done=0; if(!WriteFile(f,p,n,&done,0) || !done) return 0; p+=done; n-=done; }
    return 1;
}
static unsigned pt_take(pt_record* batch, unsigned cap) {
    while(__sync_lock_test_and_set(&pt_lock,1)) Sleep(0);
    unsigned n=0; while(pt_read!=pt_write && n<cap) batch[n++]=pt_queue[pt_read++ % PT_CAP];
    __sync_lock_release(&pt_lock); return n;
}
static const char pt_header[] = "poll,start_qpc_us,return_qpc_us,dropped_total,user,state,mod_enabled,user_enabled,paused,game_menu,mod_menu,walking,goofy,auto_stance,front_back_layout,switch_mode,switch_source,switched,mapping_valid,front_is_left,output_switch_latch,takeoff,air,grind,held_trick_foot,front_on,back_on,front_spent,back_spent,front_ignored,back_ignored,front_carry,back_carry,front_carry_ignored,back_carry_ignored,raw_lx,raw_ly,raw_rx,raw_ry,raw_lt,raw_rt,raw_buttons,out_lx,out_ly,out_rx,out_ry,out_lt,out_rt,out_buttons,switch_probe_a,switch_probe_b,switch_probe_c,switch_probe_d,stock_grab_air_active,ramp_diagnostic_label,ramp_orientation_valid,ramp_vertical_milli,ramp_qualified,ramp_departing,ramp_curve_qualified,stock_early_active,setup_us,menu_us,stance_us,translate_us,tail_us,switch_invert_x,switch_invert_y,pop_on_release,stick_steering,single_stick_flips,native_game_menu,raw_packet_number,controller_route,xl_foot_directions,grind_pop,rail_scoop_foot,rail_scoop_degrees_milli,xl_absolute_raw,native_mirror_known,native_mirror_delta,board_frame_valid,board_frame_left\r\n";
/* Worker-only bounded archive: retain complete recent rows, never preserve a
 * multi-gigabyte legacy file as the rotated copy. Source stays intact on error. */
static int pt_archive(const WCHAR* path, const WCHAR* previous) {
    HANDLE src=CreateFileW(path,0x80000000,1,0,3,0x80,0);
    if(src==INVALID_HANDLE_VALUE)return 0;
    INT64 length=0;
    if(!GetFileSizeEx(src,&length)){CloseHandle(src);return 0;}
    HANDLE dst=CreateFileW(previous,0x40000000,1,0,2,0x80,0);
    if(dst==INVALID_HANDLE_VALUE){CloseHandle(src);return 0;}
    static char chunk[65536]; int ok=1,skip=0;
    const char trimmed[]="# Older rows discarded at log size limit; recent rows follow\r\n";
    unsigned reserve=(unsigned)(sizeof trimmed-1+sizeof pt_header-1);
    if((UINT64)length>PT_FILE_LIMIT) {
        INT64 offset=length-(PT_FILE_LIMIT-reserve);
        ok=SetFilePointerEx(src,offset,0,0) && pt_send(dst,trimmed,sizeof trimmed-1) && pt_send(dst,pt_header,sizeof pt_header-1);
        skip=1;
    }
    while(ok){
        DWORD n=0; if(!ReadFile(src,chunk,sizeof chunk,&n,0)){ok=0;break;}
        if(!n)break;
        unsigned begin=0;
        if(skip){while(begin<n && chunk[begin++]!='\n'){} if(begin && chunk[begin-1]=='\n')skip=0;}
        if(begin<n)ok=pt_send(dst,chunk+begin,n-begin);
    }
    CloseHandle(dst);CloseHandle(src);return ok;
}
static HANDLE pt_roll(const WCHAR* path,const WCHAR* previous) {
    if(!pt_archive(path,previous))return INVALID_HANDLE_VALUE;
    return CreateFileW(path,4,1,0,2,0x80,0);
}
static DWORD WINAPI pt_worker(void* unused) {
    (void)unused; WCHAR path[560],previous[560]; unsigned i=0;
    while(g_dir[i] && i<510) { path[i]=previous[i]=g_dir[i]; i++; }
    unsigned base=i;const WCHAR* tail=L"\\session_controls_input.csv";
    while(*tail)path[i++]=*tail++;path[i]=0;i=base;tail=L"\\session_controls_input.previous.csv";
    while(*tail)previous[i++]=*tail++;previous[i]=0;
    HANDLE f=CreateFileW(path,4,1,0,4,0x80,0);
    if(f==INVALID_HANDLE_VALUE) { pt_failed=1; log_text("precise input log: could not open session_controls_input.csv"); return 0; }
    const char marker[]="# Dual Stick Controls 5.157; routes 1=XInput 2=XInputEx 3=PS fallback 4=native HID; new segment; axes are signed 16-bit; timestamps are absolute QPC microseconds\r\n";
    INT64 length=0;
    if(!GetFileSizeEx(f,&length))goto failed;
    UINT64 bytes=(UINT64)length;
    if(bytes+sizeof marker-1+sizeof pt_header-1>PT_FILE_LIMIT){
        CloseHandle(f);f=pt_roll(path,previous);if(f==INVALID_HANDLE_VALUE)goto failed;
        bytes=0;
    }
    if(!pt_send(f,marker,sizeof marker-1) || !pt_send(f,pt_header,sizeof pt_header-1))goto failed;
    bytes+=sizeof marker-1+sizeof pt_header-1;
    log_text("precise input log: bounded to 16 MiB plus one 16 MiB previous file; unchanged polls heartbeat at 100 ms");
    for(;;) {
        static pt_record batch[64]; static char text[65536]; unsigned count=pt_take(batch,64), used=0;
        for(unsigned j=0;j<count;j++)used+=pt_format(&batch[j],text+used);
        if(used && bytes+used>PT_FILE_LIMIT){
            CloseHandle(f);f=pt_roll(path,previous);if(f==INVALID_HANDLE_VALUE)goto failed;
            if(!pt_send(f,marker,sizeof marker-1) || !pt_send(f,pt_header,sizeof pt_header-1))goto failed;
            bytes=sizeof marker-1+sizeof pt_header-1;
        }
        if(used && !pt_send(f,text,used))goto failed;
        bytes+=used;
        if(count<64)Sleep(20);
    }
failed:
    pt_failed=1; if(f!=INVALID_HANDLE_VALUE)CloseHandle(f);
    log_text("precise input log: stopped after file error; trace incomplete"); return 0;
}
static void pt_start(void) {
    while(__sync_lock_test_and_set(&pt_lock,1)) Sleep(0);
    if(!pt_started) {
        pt_started=1; INT64 f=0;
        if(!QueryPerformanceFrequency(&f) || f<=0) pt_failed=1;
        else { pt_freq=(UINT64)f; DWORD id; HANDLE th=CreateThread(0,0,pt_worker,0,0,&id);
            if(th) CloseHandle(th); else pt_failed=1; }
    }
    __sync_lock_release(&pt_lock);
}
static void pt_enqueue(pt_record* r) {
    unsigned user=(unsigned)r->v[0]; if(user>3) user=0;
    while(__sync_lock_test_and_set(&pt_lock,1)) Sleep(0);
    r->poll=++pt_poll; r->dropped=pt_dropped;
    int changed=!pt_seen[user] || r->dropped!=pt_last[user].dropped;
    /* Profiling durations describe recorded polls, not controller changes.
     * Their microsecond jitter must not defeat duplicate suppression. */
    for(int i=0;i<PT_FIELDS && !changed;i++) {
        if(i>=57 && i<=61)continue; /* setup/menu/stance/translate/tail_us */
        if(r->v[i]!=pt_last[user].v[i])changed=1;
    }
    if(changed || r->start_us-pt_last[user].start_us>=100000) {
        if(pt_write-pt_read>=PT_CAP) pt_dropped++;
        else { pt_queue[pt_write++ % PT_CAP]=*r; pt_last[user]=*r; pt_seen[user]=1; }
    }
    __sync_lock_release(&pt_lock);
}
