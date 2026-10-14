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
#include "../src/native_pose_property.h"
static unsigned char provider[256],graph[256],model[2048],nodes[3][256],pages[2][2048];
static int read_all(np_address a,void* out,np_address n){
 if(read_mem(a,out,n))return 1;
 unsigned char* b[]={provider,graph,model,nodes[0],nodes[1],nodes[2],pages[0],pages[1]};
 unsigned size[]={256,256,2048,256,256,256,2048,2048};
 for(int i=0;i<8;i++)if(a>=(np_address)b[i] && a+n<=(np_address)b[i]+size[i]){memcpy(out,(void*)a,n);return 1;}
 return 0;
}
static int write_value(np_address a,const void* b,np_address n){assert(a==(np_address)pages[0]+533 && n==1);memcpy((void*)a,b,n);writes++;return 1;}
static void frame(int mirror,int native){flags(mirror,native);source[0x10]=!mirror;unsigned bits=mirror?0:0x2000;memcpy(context+0x13c4,&bits,4);pages[0][602]=mirror;}
static int g_native_property_restore=1;
static int property_rd(UINT64 a,void* out,UINT64 n){
 if(a==0x140ffd28dULL && n==16){const BYTE v[16]={0x49,0x8b,0x48,0x60,0x48,0x83,0xe1,0xfb,0x74,0x24,0x48,0x8b,0x41,0x20,0x48,0x83};memcpy(out,v,16);return 1;}

 if(a==0x1411dd4c0ULL && n==16){const BYTE v[16]={0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8d,0x59,0xa0,0x48,0x8b,0xcb,0xe8,0xfe,0xf8};memcpy(out,v,16);return 1;}
 if(a==0x1411dd7a0ULL && n==16){const BYTE v[16]={0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8b,0xc2,0x41,0x0f,0xb6,0xd8,0x48,0x8d,0x51};memcpy(out,v,16);return 1;}
 return read_all(a,out,n)||rd_copy(a,out,n);
}
static UINT64 property_query(const void* p,SC_MBI* m,UINT64 n){(void)p;(void)n;m->base=pages[0];m->size=2048;m->state=0x1000;m->prot=4;return sizeof *m;}
static BOOL property_write(HANDLE h,void* a,const void* b,UINT64 n,UINT64* done){(void)h;*done=n;return write_value((UINT64)a,b,n);}
#define rd_copy property_rd
#define VirtualQuery property_query
#define WriteProcessMemory property_write
#include "../src/native_property_poll.h"
#undef rd_copy
#undef VirtualQuery
#undef WriteProcessMemory
static int g_native_animation_reset=1;
#define rd_copy property_rd
#define VirtualQuery property_query
#define WriteProcessMemory property_write
#include "../src/native_animation_poll.h"
#undef rd_copy
#undef VirtualQuery
#undef WriteProcessMemory
static void reset_begin(void){
 g_native_animation_reset=1;g_cfg.enabled=1;g_cfg.goofy=0;g_native_menu=0;g_menu=0;g_local=0;g_slots[0].sel=(UINT64)selector;g_slots[0].state=100;pages[0][533]=0;
 frame(1,1);clock_ms+=4000;nar_capture_live(clock_ms);assert(g_nar_valid);
 sc_pad start={0x10};g_menu=1;g_cfg.enabled=0;nar_note_menu_request(clock_ms+10,1,&start);assert(g_nar_armed);
 nar_capture_live(clock_ms+100);assert(g_nar_valid);nar_begin_menu(clock_ms+254,1);
 g_native_menu=1;frame(0,0);nar_end_menu(clock_ms+2000);g_native_menu=0;g_menu=0;g_cfg.enabled=1;assert(g_nar_pending);
}
int main(void){
 pose_fixture_main();np_address base=0x140000000ULL;
 ptr(core,0x630,(np_address)provider);ptr(provider,0x28,(np_address)graph|4);ptr(core,0x610,(np_address)model);
 unsigned offsets[]={0x60,0x80,0x68},keys[]={0x01259001,0x0125a001,0x010a3002};
 for(int i=0;i<3;i++){ptr(graph,offsets[i],(np_address)nodes[i]|4);ptr(nodes[i],0x20,base+0x61ab030);ptr(nodes[i],0x60,base+0x61ab080);memcpy(nodes[i]+0x28,&keys[i],4);}
 strcpy((char*)nodes[1]+0x88,"Animation/Dingo/Bool.Anim.Mirrored");
 ptr(model,0x48+0x18+8,(np_address)pages[0]);ptr(model,0x48+0x18+16,(np_address)pages[1]);
 unsigned bits=(1u<<25)|(1u<<26);memcpy(pages[0]+0x71c+18*4,&bits,4);bits=1u<<3;memcpy(pages[1]+0x71c+5*4,&bits,4);
 pages[0][601]=1;pages[1][163]=0;frame(1,1);

 ptr(provider,0x38,(np_address)graph);unsigned reset_key=0x01215001;memcpy(nodes[0]+0x28,&reset_key,4);
 strcpy((char*)nodes[0]+0x88,"Animation/Dingo/Bool.Skater.ResetAnimation");bits=1u<<21;memcpy(pages[0]+0x71c+16*4,&bits,4);pages[0][533]=0;

 sc_pad raw={0},out={0};unsigned before;
 reset_begin();before=writes;raw.lx=20000;nar_finish(clock_ms+2100,1,&raw,&out);assert(writes==before);
 raw.lx=0;nar_finish(clock_ms+2110,0,&raw,&out);assert(writes==before);
 nar_finish(clock_ms+2120,1,&raw,&out);assert(writes==before+1 && pages[0][533]==1 && source[0x12]==0 && g_nar_pending==2);
 nar_finish(clock_ms+2130,1,&raw,&out);assert(writes==before+1);
 raw.lx=20000;g_cfg.enabled=0;g_native_menu=1;nar_finish(clock_ms+2145,1,&raw,&out);assert(writes==before+2 && pages[0][533]==0 && !g_nar_pending);
 raw.lx=0;reset_begin();before=writes;g_slots[0].state=200;nar_finish(clock_ms+2120,1,&raw,&out);assert(!g_nar_pending && writes==before);
 reset_begin();nar_finish(clock_ms+3101,1,&raw,&out);assert(!g_nar_pending && writes==before);
 reset_begin();g_native_animation_reset=0;nar_finish(clock_ms+2120,1,&raw,&out);assert(!g_nar_pending && writes==before);
 puts("PASS: confirmed menu-only reset request, delayed Start handoff, neutral/same-user ground gate, 24-ms bounded pulse, release despite active input/UI/disabled controls, airborne/expiry/disabled cancellation");
}
