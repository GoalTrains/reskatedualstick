/* Read-only bounded stance trace; game tick enqueues without waiting for disk. */
#define SF_CAP 256
#define SF_FIELDS 52
#define SF_SEGMENT_BYTES (8u*1024u*1024u)
typedef struct { UINT64 tick_ms,qpc_us,owner,dropped; int v[SF_FIELDS]; } sf_record;
static sf_record sf_queue[SF_CAP];
static unsigned sf_read,sf_write;
static UINT64 sf_dropped;
static volatile int sf_lock,sf_failed;
static int sf_started;
static const char sf_header[]="tick_ms,qpc_us,owner,dropped_total,state,native_read_valid,goofy,game_goofy,xl_layout,retained_switch,input_frame_known,input_frame,front_is_left,mapping_valid,source_basis,source_frame,source_mirror,export_frame,export_mirror,context_basis,raw_valid_mask,pos_x_milli,pos_y_milli,pos_z_milli,forward_x_milli,forward_y_milli,forward_z_milli,ground_track,ground_baseline,ground_mirror,ground_suspect,ground_mirror_break,ground_neutral,recovery_trigger,recovery_pending,raw_out_idle,xl_track,xl_valid,xl_baseline,xl_mirror,xl_candidate,xl_hint,xl_feet_idle,paused,native_menu,menu_depth,mod_menu,enabled,profile_source,probe_a,probe_b,probe_c,probe_d,physical_stance_raw,board_backwards_from_basis,baseline_ready\r\n";
static unsigned sf_format(const sf_record* r,char* line) {
 char* p=pt_num(line,r->tick_ms);*p++=',';p=pt_num(p,r->qpc_us);*p++=',';p=pt_num(p,r->owner);*p++=',';p=pt_num(p,r->dropped);
 for(int i=0;i<SF_FIELDS;i++){INT64 n=r->v[i];*p++=',';if(n<0){*p++='-';n=-n;}p=pt_num(p,(UINT64)n);}*p++='\r';*p++='\n';return (unsigned)(p-line);
}
static HANDLE sf_open(unsigned segment) {
 WCHAR path[560]={0};wcat(path,560,g_dir);wcat(path,560,segment?L"\\session_controls_stance_1.csv":L"\\session_controls_stance_0.csv");
 HANDLE f=CreateFileW(path,0x40000000,1|2|4,0,2,0x80,0);
 if(f!=INVALID_HANDLE_VALUE && !pt_send(f,sf_header,sizeof sf_header-1)){CloseHandle(f);return INVALID_HANDLE_VALUE;}return f;
}
static DWORD WINAPI sf_worker(void* unused) {
 (void)unused;unsigned segment=0,bytes=sizeof sf_header-1;HANDLE f=sf_open(segment);if(f==INVALID_HANDLE_VALUE)goto failed;
 log_text("stance flight recorder: session_controls_stance_0/1.csv; 25 Hz native flags, foot jobs and recovery decisions; two rolling 8 MiB segments");
 for(;;){static sf_record batch[32];static char text[32768];unsigned count=0,used=0;
  if(!__sync_lock_test_and_set(&sf_lock,1)){while(sf_read!=sf_write && count<32)batch[count++]=sf_queue[sf_read++%SF_CAP];__sync_lock_release(&sf_lock);}
  for(unsigned i=0;i<count;i++)used+=sf_format(&batch[i],text+used);
  if(used){if(bytes+used>SF_SEGMENT_BYTES){CloseHandle(f);segment^=1;f=sf_open(segment);bytes=sizeof sf_header-1;if(f==INVALID_HANDLE_VALUE)goto failed;}if(!pt_send(f,text,used))goto failed;bytes+=used;}
  if(count<32)Sleep(20);
 }
failed:sf_failed=1;if(f!=INVALID_HANDLE_VALUE)CloseHandle(f);log_text("stance flight recorder: file write failed; recording incomplete");return 0;
}
static void sf_capture(sf_record* record) {
 if(sf_failed)return;if(__sync_lock_test_and_set(&sf_lock,1)){__sync_fetch_and_add(&sf_dropped,1);return;}
 if(!sf_started){sf_started=1;DWORD id;HANDLE th=CreateThread(0,0,sf_worker,0,0,&id);if(th)CloseHandle(th);else sf_failed=1;}
 if(!sf_failed){if(sf_write-sf_read>=SF_CAP)__sync_fetch_and_add(&sf_dropped,1);else {record->dropped=sf_dropped;sf_queue[sf_write++%SF_CAP]=*record;}}
 __sync_lock_release(&sf_lock);
}