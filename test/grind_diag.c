#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <setjmp.h>
#include "../src/translate.h"
#include "../src/native_menu_respawn.h"
static WCHAR g_dir[512];static sc_config g_cfg;
static void (*g_grind_step_capture)(UINT64,DWORD,DWORD);
static int g_grind_diag=1,g_local=0,g_paused=0,g_native_menu=0,g_ui_open=0,g_user_enabled=1,owned=0;
static struct {DWORD state;UINT64 sel;} g_slots[1];
static int native_local_slot(void){return owned;}
static BYTE context[0x1920],core[0x500],holder[32],physics[64],body[0x130];
static int reject,changed;static unsigned frame_reads;
static int fixture_read(UINT64 a,void*out,UINT64 n){
 BYTE*blocks[]={context,core,holder,physics,body};unsigned sizes[]={sizeof context,sizeof core,sizeof holder,sizeof physics,sizeof body};
 for(int i=0;i<5;i++)if(a>=(UINT64)blocks[i] && a+n<=(UINT64)blocks[i]+sizes[i]){memcpy(out,(void*)a,n);return 1;}return 0;
}
static int fixture_frame(UINT64 sel,UINT64 base,np_read_fn read,nr_frame*f){
 (void)base;(void)read;frame_reads++;if(reject)return 0;memset(f,0,sizeof *f);
 f->selector=sel;f->core=(UINT64)core;f->context=(UINT64)context;f->source=123;
 if(changed && !(frame_reads&1))f->core++;f->forward[2]=1;return 1;
}
static UINT64 rd_time(void){return 1234567;}
static void log_text(const char*s){(void)s;}
static void wcat(WCHAR*d,unsigned cap,const WCHAR*s){assert(wcslen(d)+wcslen(s)<cap);wcscat(d,s);}
static int pt_send(HANDLE f,const char*p,unsigned n){DWORD done;return WriteFile(f,p,n,&done,0)&&done==n;}
static jmp_buf finished;
static HANDLE no_thread(void*a,SIZE_T b,LPTHREAD_START_ROUTINE c,void*d,DWORD e,LPDWORD f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return (HANDLE)1;}
static BOOL close_test(HANDLE f){return f==(HANDLE)1?TRUE:CloseHandle(f);}
static void stop_worker(DWORD ms){(void)ms;longjmp(finished,1);}
#define rd_copy fixture_read
#define nr_read fixture_frame
#define CreateThread no_thread
#define CloseHandle close_test
#define Sleep stop_worker
#include "../src/grind_diag.h"
#undef CloseHandle
static void ptr(BYTE*b,unsigned off,UINT64 v){memcpy(b+off,&v,8);}
int main(void){
 assert(sizeof(gs_sample)==272 && sizeof(gd_record)==3792);
 ptr(core,0x430,(UINT64)holder);ptr(holder,0x18,(UINT64)physics);ptr(physics,0,(UINT64)GetModuleHandleW(0)+0x65e67a0);ptr(physics,0x20,(UINT64)body);
 *(DWORD*)body=9;ptr(body,0x10,(UINT64)physics);*(float*)(body+0x70)=4;*(DWORD*)(context+0x1414)=401;
 g_slots[0].state=401;g_slots[0].sel=0x123450;sc_pad raw={0},out={0};raw.lx=20000;out.rx=10000;
 reject=1;gd_capture(1000,0,&raw,&out,0);assert(!gd_write);reject=0;
 owned=-1;gd_capture(1000,0,&raw,&out,0);assert(!gd_write);owned=0;
 changed=1;frame_reads=0;gd_capture(1000,0,&raw,&out,0);assert(!gd_write);changed=0;
 gd_capture(1000,0,&raw,&out,0);assert(gd_write==1 && gd_queue[0].valid==3 && gd_queue[0].velocity[0]==4 && gd_queue[0].raw.lx==20000);
 gd_capture(1001,0,&raw,&out,0);assert(gd_write==1);
 for(int i=0;i<16;i++)((float*)(body+0x20))[i]=i%5==0?1.0f:0;
 gd_step_seen=GetTickCount64();SetLastError(0x1234);gd_step_capture(g_slots[0].sel,201,401);
 assert(GetLastError()==0x1234 && gd_step_latest.sequence==1 && gd_step_latest.valid==15);
 assert(gd_step_entry.before==201 && gd_step_entry.selected==401 && gd_step_entry.raw.lx==20000);
 assert(gd_step_entry.poll_us==1234567 && gd_step_entry.board[10]==1);
 gd_step_seen=GetTickCount64()-101;gd_step_capture(g_slots[0].sel,201,401);assert(gd_step_sequence==1);
 gd_step_seen=GetTickCount64();owned=-1;gd_step_capture(g_slots[0].sel,201,401);assert(gd_step_sequence==1);owned=0;
 changed=1;frame_reads=0;gd_step_capture(g_slots[0].sel,201,401);assert(gd_step_sequence==1);changed=0;
 *(float*)(body+0x20)=0.0f/0.0f;gd_step_capture(g_slots[0].sel,401,401);
 assert(gd_step_sequence==2 && gd_step_latest.valid==14 && gd_step_latest.board[0]==0 && gd_step_entry.sequence==1);
 *(float*)(body+0x20)=1;gd_step_capture(g_slots[0].sel,401,500);assert(!gd_step_entry.sequence);

 *(float*)(body+0x90)=0.0f/0.0f;gd_capture(1100,0,&raw,&out,0);assert(gd_write==2 && gd_queue[1].valid==1 && gd_queue[1].velocity[0]==0);
 *(DWORD*)(context+0x1414)=100;gd_capture(1200,0,&raw,&out,0);assert(gd_write==2);*(DWORD*)(context+0x1414)=401;
 g_paused=1;gd_capture(1300,0,&raw,&out,0);assert(gd_write==2);g_paused=0;
 g_slots[0].state=701;*(DWORD*)(context+0x1414)=701;gd_capture(1400,0,&raw,&out,0);assert(gd_write==3 && gd_queue[2].state==701);
 g_slots[0].state=700;*(DWORD*)(context+0x1414)=700;gd_capture(1500,0,&raw,&out,0);assert(gd_write==3);
 g_slots[0].state=701;*(DWORD*)(context+0x1414)=701;gd_samples=GD_LIMIT;gd_capture(1600,0,&raw,&out,0);assert(gd_write==3 && gd_step_limit && !g_grind_step_capture);
 GetTempPathW(512,g_dir);WCHAR prefix[512];GetTempFileNameW(g_dir,L"scg",0,prefix);DeleteFileW(prefix);assert(CreateDirectoryW(prefix,0));wcscpy(g_dir,prefix);
 if(!setjmp(finished))gd_worker(0);
 WCHAR path[560];swprintf(path,560,L"%ls\\session_controls_grind.bin",g_dir);FILE*f=_wfopen(path,L"rb");assert(f);char magic[8];assert(fread(magic,1,8,f)==8 && !memcmp(magic,"SCGRIND1",8));DWORD header[4];assert(fread(header,4,4,f)==4 && header[0]==sizeof(gd_record) && header[1]==5158 && header[2]==GD_LIMIT && header[3]==0xf00);fseek(f,0,SEEK_END);assert(ftell(f)==24+3*sizeof(gd_record));fclose(f);assert(DeleteFileW(path));RemoveDirectoryW(g_dir);
 puts("PASS: grind recorder owner/state validation, failed/racing reads, finite motion, cadence, limit, binary schema and native file writer");
}
