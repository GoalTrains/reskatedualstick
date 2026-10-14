/* Read-only stance/pose investigation. These blocks contain candidate game
 * state, not identified foot-position/animation fields. Correlate them with
 * a visibly stuck pose and its B reset before assigning meanings.
 * 10 Hz plus state/probe/B edges; 6000 records, ~266 MB maximum per launch.
 * File writes run on a worker; poll never waits for the queue lock. */
#define PD_CAP 16
#define PD_LIMIT 6000
typedef struct {
    rd_record snapshot;
    DWORD events, user, enabled, switched;
    int probes[4];
    sc_pad raw, out;
    /* SDK selector+8 is the live rider context; +13d4 contains mount/dismount
     * request bits and +1414 the selected physics state. Animation/foot pose
     * candidates in the rest of this range still need identification. */
    DWORD context_valid, reserved;
    UINT64 context_address;
    BYTE context[6144];
    DWORD upstream_valid, upstream_reserved;
    UINT64 upstream_address[3];
    /* Source state and two bounded animation-controller windows. Read-only
     * candidates: no names or corrective writes are inferred from these. */
    BYTE upstream[3][2048];
    DWORD model_valid, model_reserved;
    UINT64 model_address[7];
    BYTE model_blocks[3][2048];
    BYTE model_nodes[4][256];
    DWORD property_valid, property_reserved;
    UINT64 property_address[4];
    BYTE property_pages[4][2048];
} pd_record;
static pd_record pd_queue[PD_CAP];
static volatile int pd_lock, pd_failed;
static unsigned pd_read,pd_write,pd_samples,pd_dropped;
static int pd_started,pd_seen,pd_limit_logged;
static UINT64 pd_last,pd_selector;
static DWORD pd_state;
static WORD pd_buttons[4];
static int pd_probes[4];
/* Catalogue both graph directions on the background writer. Refresh at
 * selected state/B edges because descriptors can bind keys lazily. */
static void pd_catalogue(const pd_record* r) {
    static HANDLE f;static unsigned passes;static UINT64 last;
    if(!(r->model_valid&7) || !r->model_address[1] || passes>=128)return;
    if(passes && (!(r->events&7) || r->snapshot.time_us-last<100000))return;
    UINT64 graph=0;
    if(!rd_copy(r->model_address[0]+0x28,&graph,8) || (graph&~4ULL)!=r->model_address[1])return;
    if(!f) {
        WCHAR path[560]={0};wcat(path,560,g_dir);wcat(path,560,L"\\session_controls_pose_properties.csv");
        f=CreateFileW(path,0x40000000,1|2|4,0,2,0x80,0);
        if(f==INVALID_HANDLE_VALUE){f=0;return;}
        const char header[]="time_us,graph_kind,graph_offset,key,node,page,default_value,name,descriptor_hex\r\n";
        if(!pt_send(f,header,sizeof header-1)){CloseHandle(f);f=0;return;}
    }
    const char hex[]="0123456789abcdef";unsigned found=0;
    UINT64 base=(UINT64)GetModuleHandleW(0);
    for(unsigned kind=0;kind<2;kind++) {
        BYTE block[2048];
        if(!kind)memcpy(block,r->model_blocks[1],2048);
        else {
            UINT64 output=0;
            if(!rd_copy(r->model_address[0]+0x38,&output,8) || !rd_copy(output&~4ULL,block,2048))continue;
        }
        for(unsigned offset=0x40;offset<0x800;offset+=8) {
            UINT64 node=0,vt=0,page=0;DWORD key=0;BYTE data[256];
            memcpy(&node,block+offset,8);node&=~4ULL;
            if(!rd_copy(node,data,sizeof data))continue;
            memcpy(&vt,data+0x20,8);
            int simple_bool=vt==base+0x61ab030;
            memcpy(&vt,data+0x60,8);simple_bool=simple_bool && vt==base+0x61ab080;
            memcpy(&key,data+0x28,4);
            if(simple_bool && key>>24)rd_copy(r->model_address[2]+0x48+0x18+(UINT64)(key&0xfff)*8,&page,8);
            char line[1024];unsigned len=0;
            UINT64 values[7]={r->snapshot.time_us,kind,offset,key,node,page,data[0x78]};
            for(unsigned field=0;field<7;field++) {
                line[len++]='0';line[len++]='x';
                for(int shift=60;shift>=0;shift-=4)line[len++]=hex[(values[field]>>shift)&15];
                line[len++]=',';
            }
            unsigned name_start=256;
            for(unsigned i=0;i+10<256;i++) {
                if((data[i]=='A' || data[i]=='a') && data[i+1]=='n' && data[i+2]=='i' &&
                   data[i+3]=='m' && data[i+4]=='a' && data[i+5]=='t' && data[i+6]=='i' &&
                   data[i+7]=='o' && data[i+8]=='n' && data[i+9]=='/') {name_start=i;break;}
            }
            for(unsigned i=name_start;i<256 && data[i];i++) {
                char c=(char)data[i];if(c<32 || c>126)break;
                line[len++]=(c==',' || c=='"')?'_':c;
            }
            line[len++]=',';
            for(unsigned i=0;i<256;i++){line[len++]=hex[data[i]>>4];line[len++]=hex[data[i]&15];}
            line[len++]='\r';line[len++]='\n';
            if(!pt_send(f,line,len))return;
            found++;
        }
    }
    passes++;last=r->snapshot.time_us;
    if(found && passes==1)log_text("stance pose diagnostics: full input/output descriptor catalogue saved; computed types retained unlabelled; no native calls or writes");
}
static DWORD WINAPI pd_worker(void* unused) {
    (void)unused; WCHAR path[560]={0};
    wcat(path,560,g_dir);wcat(path,560,L"\\session_controls_pose.bin");
    HANDLE f=CreateFileW(path,0x40000000,1|2|4,0,2,0x80,0);
    if(f==INVALID_HANDLE_VALUE){pd_failed=1;log_text("stance pose diagnostics: file open failed");return 0;}
    struct {char magic[8];DWORD size,version;} header={{'S','C','P','O','S','E','0','5'},sizeof(pd_record),5108};
    if(!pt_send(f,(const char*)&header,sizeof header))goto failed;
    log_text("stance pose diagnostics: session_controls_pose.bin; checked candidate blocks, NOT identified animation fields; 6000-record limit; latest launch replaces file");
    for(;;) {
        static pd_record record;int have=0;
        if(!__sync_lock_test_and_set(&pd_lock,1)) {
            if(pd_read!=pd_write){record=pd_queue[pd_read++%PD_CAP];have=1;}
            __sync_lock_release(&pd_lock);
        }
        if(have){pd_catalogue(&record);if(!pt_send(f,(const char*)&record,sizeof record))goto failed;}
        else Sleep(20);
    }
failed:pd_failed=1;CloseHandle(f);log_text("stance pose diagnostics: write failed; recording incomplete");return 0;
}
static void pd_capture(UINT64 now,UINT64 selector,DWORD state,DWORD user,
                       int enabled,int switched,const int probes[4],
                       const sc_pad* raw,const sc_pad* out) {
    if(pd_failed || !selector || user>=4)return;
    if(pd_samples>=PD_LIMIT){if(!pd_limit_logged){pd_limit_logged=1;log_text("stance pose diagnostics: recording limit reached; normal input logging continues");}return;}
    DWORD events=0;
    if(!pd_seen || selector!=pd_selector)events|=16;
    if(!pd_seen || state!=pd_state)events|=4;
    for(int i=0;i<4;i++)if(!pd_seen || probes[i]!=pd_probes[i])events|=8;
    if((raw->buttons&0x2000) && !(pd_buttons[user]&0x2000))events|=1;
    if(!(raw->buttons&0x2000) && (pd_buttons[user]&0x2000))events|=2;
    if(pd_seen && now-pd_last<100 && (!events || now-pd_last<10))return;
    if(__sync_lock_test_and_set(&pd_lock,1))return;
    if(!pd_started){
        pd_started=1;DWORD id;HANDLE thread=CreateThread(0,0,pd_worker,0,0,&id);
        if(thread)CloseHandle(thread);else pd_failed=1;
    }
    if(pd_failed){__sync_lock_release(&pd_lock);return;}
    if(pd_write-pd_read>=PD_CAP){pd_dropped++;__sync_lock_release(&pd_lock);return;}
    pd_record* r=&pd_queue[pd_write%PD_CAP];memset(r,0,sizeof *r);
    r->events=events;r->user=user;r->enabled=enabled;r->switched=switched;
    r->raw=*raw;r->out=*out;
    rd_record* s=&r->snapshot;
    s->time_us=rd_time();s->selector=selector;s->state=state;s->dropped=pd_dropped;
    UINT64 root=rd_pointer(selector),p=rd_pointer(selector+0x10);
    s->address[0]=selector;s->address[1]=root;
    s->address[2]=rd_pointer(root+0x60);s->address[3]=rd_pointer(root+0x78);
    s->address[4]=p;s->address[5]=rd_pointer(p+0x10);
    s->address[6]=rd_pointer(root+0x68);s->address[7]=rd_pointer(root+0x70);
    for(int i=0;i<RD_BLOCKS;i++)if(rd_copy(s->address[i],s->bytes[i],RD_BYTES))s->valid|=1u<<i;
    r->context_address=rd_pointer(selector+8);
    for(int i=0;i<3;i++)if(rd_copy(r->context_address+(UINT64)i*2048,r->context+i*2048,2048))r->context_valid|=1u<<i;
    UINT64 core=s->address[5],manager=rd_pointer(core+0x438);
    r->upstream_address[0]=rd_pointer(core+0x3c8);
    if(manager){r->upstream_address[1]=manager+0x4600;r->upstream_address[2]=manager+0x5600;}
    for(int i=0;i<3;i++)if(rd_copy(r->upstream_address[i],r->upstream[i],2048))r->upstream_valid|=1u<<i;
    /* Native producer RVA feba50 reads core+630 -> tagged +28 graph,
     * nodes +60/+80/+88/+68 and the per-entity context core+610. Capture
     * descriptor data without executing any virtual getters or setters. */
    r->model_address[0]=rd_pointer(core+0x630);
    UINT64 graph=0;
    if(rd_copy(r->model_address[0]+0x28,&graph,8))graph&=~4ULL;
    r->model_address[1]=graph;r->model_address[2]=rd_pointer(core+0x610);
    for(int i=0;i<3;i++)if(rd_copy(r->model_address[i],r->model_blocks[i],2048))r->model_valid|=1u<<i;
    const unsigned node_offsets[4]={0x60,0x80,0x88,0x68};
    for(int i=0;i<4;i++) {
        UINT64 node=0;
        if(rd_copy(graph+node_offsets[i],&node,8))node&=~4ULL;
        r->model_address[i+3]=node;
        if(rd_copy(node,r->model_nodes[i],256))r->model_valid|=1u<<(i+3);
        /* Boolean getter RVA 11dd4c0 resolves the descriptor's key at +28
         * through context+48. Read its first value page and occupancy bits;
         * never call the getter or allocate/set a property. A missing page
         * is left invalid; the native getter may instead use a parent/default. */
        DWORD key=0;UINT64 page=0;
        if((r->model_valid&(1u<<(i+3))) && rd_copy(node+0x28,&key,4) && (key>>24)) {
            if(rd_copy(r->model_address[2]+0x48+0x18+(UINT64)(key&0xfff)*8,&page,8)) {
                r->property_address[i]=page;
                if(rd_copy(page,r->property_pages[i],2048))r->property_valid|=1u<<i;
            }
        }
    }
    for(int i=0;i<4;i++){r->probes[i]=probes[i];pd_probes[i]=probes[i];}
    pd_seen=1;pd_buttons[user]=raw->buttons;pd_state=state;pd_selector=selector;pd_last=now;
    pd_write++;pd_samples++;__sync_lock_release(&pd_lock);
}
