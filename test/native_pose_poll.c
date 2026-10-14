#define main pose_fixture_main
#include "native_pose_restore.c"
#undef main
#include "../src/translate.h"
#undef __declspec
#define __declspec(x)
#define WINAPI
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef unsigned long long UINT64;
typedef int BOOL;
typedef void* HANDLE;
typedef struct {void* base;UINT64 size;DWORD state,prot;} SC_MBI;
#define PAGE_READWRITE 4
static int g_native_pose_restore=1,g_user_enabled=1,g_native_menu=0,g_local=0,g_paused,g_menu,g_ui_open;
static struct {UINT64 sel;int state;} g_slots[1];
static sc_config g_cfg;
static unsigned logs;
static void log_text(const char* s){(void)s;logs++;}
static void* GetModuleHandleW(void* p){(void)p;return (void*)0x140000000ULL;}
static void* GetCurrentProcess(void){return (void*)1;}
static UINT64 VirtualQuery(const void* p,SC_MBI* m,UINT64 size){(void)p;(void)size;m->base=source;m->size=sizeof source;m->state=0x1000;m->prot=4;return sizeof *m;}
static int rd_copy(UINT64 a,void* out,UINT64 n){
 if(a==0x1447db20eULL && n==32){const BYTE code[32]={0x48,0x8b,0x87,0xb8,0x03,0,0,0x48,0x8b,0x97,0xc8,0x03,0,0,0x48,0x8b,0x48,0x60,0x0f,0xb6,0x42,0x12,0x88,0x81,0xa4,0,0,0,0x48,0x8b,0x87,0xb8};memcpy(out,code,32);return 1;}
 if(a==0x1447d2160ULL && n==32){const BYTE code[32]={0x48,0x8b,0x81,0xc8,0x03,0,0,0x4c,0x8b,0xd1,0x4c,0x8b,0x89,0xc0,0x03,0,0,0x0f,0xb6,0x50,0x10,0x45,0x8b,0x81,0xc4,0x13,0,0,0xc1,0xe2,0x0d,0x41};memcpy(out,code,32);return 1;}
 return read_mem(a,out,n);
}
BOOL WriteProcessMemory(HANDLE p,void* a,const void* data,UINT64 n,UINT64* done){(void)p;assert(a==source+0x10 && n==4);memcpy(a,data,n);writes++;*done=n;return 1;}
#include "../src/native_pose_poll.h"
static double clock_ms=1000;
static void basis(int value){source[0x10]=value;unsigned bits=value?0x2000:0;memcpy(context+0x13c4,&bits,4);}
static void begin(void){
 flags(1,1);basis(0);g_native_menu=0;g_cfg.enabled=1;g_cfg.goofy=0;g_slots[0].state=100;
 clock_ms+=2000;np_capture_live(clock_ms);assert(g_native_pose_valid);
 np_begin_menu(clock_ms+20,1);g_native_menu=1;flags(0,0);basis(1);
 np_end_menu(clock_ms+100);g_native_menu=0;
 assert(g_native_pose_pending);
}
int main(void){
 pose_fixture_main();g_slots[0].sel=(UINT64)selector;sc_pad raw={0},out={0};unsigned before;
 begin();before=writes;np_finish(clock_ms+110,0,&raw,&out);assert(writes==before && g_native_pose_pending);
 raw.lx=20000;np_finish(clock_ms+120,1,&raw,&out);assert(writes==before && g_native_pose_pending);
 raw.lx=0;out.rx=20000;np_finish(clock_ms+130,1,&raw,&out);assert(writes==before && g_native_pose_pending);
 out.rx=0;np_finish(clock_ms+140,1,&raw,&out);assert(writes==before+1 && g_native_pose_pending==2 && source[0x10]==0 && source[0x12]==1 && source[0x14]==0);
 np_finish(clock_ms+150,1,&raw,&out);assert(writes==before+1);
 flags(1,0);basis(0);np_finish(clock_ms+300,1,&raw,&out);assert(!g_native_pose_pending && writes==before+1);
 begin();before=writes;np_finish(clock_ms+140,1,&raw,&out);flags(0,0);basis(1);np_finish(clock_ms+300,1,&raw,&out);assert(writes==before+1 && !g_native_pose_pending);
 begin();before=writes;g_cfg.goofy=1;np_finish(clock_ms+140,1,&raw,&out);assert(!g_native_pose_pending && writes==before);
 begin();g_slots[0].state=200;np_finish(clock_ms+140,1,&raw,&out);assert(!g_native_pose_pending && writes==before);
 begin();np_finish(clock_ms+1101,1,&raw,&out);assert(!g_native_pose_pending && writes==before);
 /* Real trace: Start disables translation immediately; native detection
  * arrives 244 ms later. Preserve the clean frame across that handoff. */
 flags(1,1);basis(0);g_cfg.enabled=1;g_slots[0].state=100;g_native_menu=0;clock_ms+=2000;
 np_capture_live(clock_ms);raw.buttons=0x10;g_menu=1;g_cfg.enabled=0;
 np_note_menu_request(clock_ms+10,1,&raw);assert(g_native_pose_armed);
 np_capture_live(clock_ms+100);assert(g_native_pose_valid);
 np_begin_menu(clock_ms+254,1);assert(g_native_pose_valid && !g_native_pose_armed);
 flags(0,0);basis(1);g_native_menu=1;np_end_menu(clock_ms+3000);
 assert(g_native_pose_pending);g_native_menu=0;g_menu=0;g_cfg.enabled=1;raw.buttons=0;
 before=writes;np_finish(clock_ms+3100,1,&raw,&out);assert(writes==before+1);
 flags(1,0);basis(0);np_finish(clock_ms+3260,1,&raw,&out);assert(!g_native_pose_pending);
 flags(1,1);basis(0);clock_ms+=5000;np_capture_live(clock_ms);raw.buttons=0x10;
 np_note_menu_request(clock_ms+10,1,&raw);assert(g_native_pose_armed);
 g_cfg.enabled=0;np_capture_live(clock_ms+1100);assert(!g_native_pose_valid && !g_native_pose_armed);
 raw.buttons=0;
 begin();before=writes;g_native_pose_restore=0;np_finish(clock_ms+140,1,&raw,&out);assert(!g_native_pose_pending && writes==before);
 puts("PASS: recorded menu frame signature, owner and neutral raw/output gates; switch byte preserved; one write with retention/overwrite verification; stance changes/air/timeout/disabled option rejected");
}
