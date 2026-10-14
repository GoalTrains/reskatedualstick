#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef unsigned long long UINT64;typedef long long INT64;
static WCHAR g_dir[520]=L"C:\\Games\\Skate\\mine\\reskatedualstick-source\\build";
static int g_ramp_diag=0,g_remount_diag=1,g_walking=0,g_ramp_diag_reset=0,g_local=-1;
static struct {UINT64 sel;DWORD state;}g_slots[1];
static void wcat(WCHAR*d,unsigned cap,const WCHAR*s){unsigned n=0;while(d[n])n++;while(*s&&n+1<cap)d[n++]=*s++;d[n]=0;}
static void log_text(const char*s){puts(s);}
static int readable(UINT64 a,UINT64 n){MEMORY_BASIC_INFORMATION m;return a>=0x10000 && VirtualQuery((void*)a,&m,sizeof m) && m.State==MEM_COMMIT && !(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)) && a+n<=(UINT64)m.BaseAddress+m.RegionSize;}
static int pt_send(HANDLE f,const char*p,unsigned n){while(n){DWORD w=0;if(!WriteFile(f,p,n,&w,0)||!w)return 0;p+=w;n-=w;}return 1;}
static BOOL native_freq(INT64*p){return QueryPerformanceFrequency((LARGE_INTEGER*)p);}
static BOOL native_tick(INT64*p){return QueryPerformanceCounter((LARGE_INTEGER*)p);}
#define QueryPerformanceFrequency native_freq
#define QueryPerformanceCounter native_tick
#include "../src/ramp_diag.h"
int main(void){
 const WCHAR* path=L"C:\\Games\\Skate\\mine\\reskatedualstick-source\\build\\session_controls_remount.bin";DeleteFileW(path);
 assert(sizeof(rd_record)==16480);rd_capture(1000);assert(!rd_samples);
 BYTE* mem=VirtualAlloc(0,4096*4,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(mem);
 g_slots[0].sel=(UINT64)mem;g_slots[0].state=100;g_local=0;
 *(UINT64*)mem=(UINT64)(mem+4096);*(UINT64*)(mem+4096+0x60)=(UINT64)(mem+8192);
 *(float*)(mem+8192+0x100)=.5f;*(UINT64*)(mem+4096+0x68)=0xffffffffffffffffULL;
 rd_capture(1100);assert(rd_samples==1);rd_capture(1150);assert(rd_samples==1);
 g_walking=1;g_slots[0].state=504;rd_capture(1200);assert(rd_samples==2);
 Sleep(150);HANDLE f=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0,OPEN_EXISTING,0,0);assert(f!=INVALID_HANDLE_VALUE);
 BYTE hdr[16];DWORD n;assert(ReadFile(f,hdr,16,&n,0)&&n==16&&!memcmp(hdr,"SCRAMP01",8));assert(*(DWORD*)(hdr+8)==sizeof(rd_record));
 static rd_record r;assert(ReadFile(f,&r,sizeof r,&n,0)&&n==sizeof r);assert(r.label==3&&r.state==100&&r.time_us&&r.valid==7);assert(*(float*)(r.bytes[2]+0x100)==.5f);
 assert(ReadFile(f,&r,sizeof r,&n,0)&&n==sizeof r);assert(r.label==4&&r.state==504);CloseHandle(f);
 rd_samples=2400;rd_capture(1400);assert(rd_samples==2400);
 g_ramp_diag_reset=1;rd_capture(1500);assert(rd_samples==1 && !g_ramp_diag_reset);
 puts("PASS: checked local snapshots, invalid pointer ignored, labelled riding/walking records, nonzero QPC time, 10 Hz limit, binary record shape, background file writer, bounded sample limit");
}
