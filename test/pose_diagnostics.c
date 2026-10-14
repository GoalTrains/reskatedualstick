#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef unsigned long long UINT64;typedef long long INT64;
typedef struct {unsigned short buttons;unsigned char lt,rt;short lx,ly,rx,ry;} sc_pad;
static WCHAR g_dir[520]=L"C:\\Games\\Skate\\mine\\reskatedualstick-source\\build";
static int g_ramp_diag,g_remount_diag,g_walking,g_ramp_diag_reset,g_local=-1;
static struct {UINT64 sel;DWORD state;}g_slots[1];
static void wcat(WCHAR*d,unsigned cap,const WCHAR*s){unsigned n=0;while(d[n])n++;while(*s&&n+1<cap)d[n++]=*s++;d[n]=0;}
static void log_text(const char*s){puts(s);}
static int pt_send(HANDLE f,const char*p,unsigned n){while(n){DWORD w=0;if(!WriteFile(f,p,n,&w,0)||!w)return 0;p+=w;n-=w;}return 1;}
static BOOL native_freq(INT64*p){return QueryPerformanceFrequency((LARGE_INTEGER*)p);}
static BOOL native_tick(INT64*p){return QueryPerformanceCounter((LARGE_INTEGER*)p);}
#define QueryPerformanceFrequency native_freq
#define QueryPerformanceCounter native_tick
#include "../src/ramp_diag.h"
#include "../src/pose_diag.h"
int main(void){
 BYTE* mem=VirtualAlloc(0,131072,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(mem);
 *(UINT64*)mem=(UINT64)(mem+4096);*(UINT64*)(mem+4096+0x60)=(UINT64)(mem+8192);
 *(UINT64*)(mem+8)=(UINT64)(mem+8192);mem[8192+0x13d4]=0x80;
 *(UINT64*)(mem+4096+0x68)=~0ull;mem[8192+0xA3]=1;
 *(UINT64*)(mem+16)=(UINT64)(mem+16384);*(UINT64*)(mem+16384+16)=(UINT64)(mem+18432);
 *(UINT64*)(mem+18432+0x3c8)=(UINT64)(mem+20480);
 *(UINT64*)(mem+18432+0x438)=(UINT64)(mem+22528);
 mem[20480+0x12]=1;mem[22528+0x4600+0x38]=9;mem[22528+0x5600+0x3a5]=7;
 *(UINT64*)(mem+18432+0x630)=(UINT64)(mem+65536);
 *(UINT64*)(mem+65536+0x28)=(UINT64)(mem+69632)|4;
 *(UINT64*)(mem+65536+0x38)=(UINT64)(mem+69632)|4;
 *(UINT64*)(mem+18432+0x610)=(UINT64)(mem+73728);
 const unsigned node_offsets[4]={0x60,0x80,0x88,0x68};
 for(int i=0;i<4;i++){*(UINT64*)(mem+69632+node_offsets[i])=(UINT64)(mem+81920+i*512)|4;mem[81920+i*512]=42+i;}
 for(int i=0;i<4;i++){
  *(DWORD*)(mem+81920+i*512+0x28)=0x01000001u+(i<<12);
  *(UINT64*)(mem+81920+i*512+0x20)=(UINT64)GetModuleHandleW(0)+0x61ab030;
  *(UINT64*)(mem+81920+i*512+0x60)=(UINT64)GetModuleHandleW(0)+0x61ab080;
  strcpy((char*)(mem+81920+i*512+0x88),"Animation/Test.Bool_Example");
 }
 *(UINT64*)(mem+73728+0x48+0x18+8)=(UINT64)(mem+90112);
 *(UINT64*)(mem+69632+0x2f8)=(UINT64)(mem+86016)|4;
 *(UINT64*)(mem+86016+0x20)=(UINT64)GetModuleHandleW(0)+0x123456;
 strcpy((char*)(mem+86016+0xa0),"Animation/Test.Bool.Computed");
 mem[90112]=17;mem[90112+0x71c]=15;
 int probes[4]={0,1,0,1};sc_pad raw={0,0,0,0,-1,0,-1},out={0};
 pd_started=1; /* Hold queue for deterministic inspection before starting writer. */
 pd_capture(1000,0,100,0,1,0,probes,&raw,&out);assert(!pd_samples);
 pd_capture(1000,(UINT64)mem,100,0,1,0,probes,&raw,&out);
 assert(pd_samples==1&&pd_queue[0].snapshot.valid==55);
 assert(pd_queue[0].snapshot.bytes[2][0xA3]==1);
 assert(pd_queue[0].context_valid==7 && pd_queue[0].context[0x13d4]==0x80);
 assert(pd_queue[0].upstream_valid==7 && pd_queue[0].upstream[0][0x12]==1 && pd_queue[0].upstream[1][0x38]==9 && pd_queue[0].upstream[2][0x3a5]==7);
 assert(pd_queue[0].model_valid==127 && pd_queue[0].model_address[1]==(UINT64)(mem+69632));
 for(int i=0;i<4;i++)assert(pd_queue[0].model_nodes[i][0]==42+i);
 assert(pd_queue[0].property_valid==15 && pd_queue[0].property_pages[1][0x71c]==15 && pd_queue[0].property_pages[0][0]==17);
 assert(!memcmp(&pd_queue[0].raw,&raw,sizeof raw));
 pd_capture(1050,(UINT64)mem,100,0,1,0,probes,&raw,&out);assert(pd_samples==1);
 raw.buttons=0x2000;
 pd_capture(1060,(UINT64)mem,100,0,1,0,probes,&raw,&out);
 assert(pd_samples==2 && pd_queue[1].events==1);
 raw.buttons=0;probes[1]=0;
 pd_capture(1065,(UINT64)mem,100,0,1,0,probes,&raw,&out);assert(pd_samples==2);
 pd_capture(1075,(UINT64)mem,100,0,1,0,probes,&raw,&out);
 assert(pd_samples==3 && pd_queue[2].events==10);
 pd_capture(1090,(UINT64)mem,200,1,1,1,probes,&raw,&out);
 assert(pd_samples==4 && pd_queue[3].events==4 && pd_queue[3].user==1);
 pd_lock=1;pd_capture(1300,(UINT64)mem,100,0,1,0,probes,&raw,&out);assert(pd_samples==4);pd_lock=0;
 pd_write=PD_CAP;pd_read=0;
 pd_capture(1300,(UINT64)mem,100,0,1,0,probes,&raw,&out);assert(pd_dropped==1);
 pd_samples=PD_LIMIT;pd_capture(1400,(UINT64)mem,100,0,1,0,probes,&raw,&out);assert(pd_samples==PD_LIMIT&&pd_limit_logged);
 /* Actual asynchronous writer and format, including raw/out packet metadata. */
 pd_read=0;pd_write=1;
 DWORD id;HANDLE thread=CreateThread(0,0,pd_worker,0,0,&id);assert(thread);CloseHandle(thread);Sleep(150);
 HANDLE f=CreateFileW(L"C:\\Games\\Skate\\mine\\reskatedualstick-source\\build\\session_controls_pose.bin",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0,OPEN_EXISTING,0,0);assert(f!=INVALID_HANDLE_VALUE);
 BYTE header[16];DWORD n;assert(ReadFile(f,header,16,&n,0)&&n==16&&!memcmp(header,"SCPOSE05",8));
 assert(*(DWORD*)(header+8)==sizeof(pd_record));
 static pd_record r;assert(ReadFile(f,&r,sizeof r,&n,0)&&n==sizeof r);
 assert(r.snapshot.time_us && r.snapshot.state==100 && r.probes[1]==1 && r.raw.ly==-1 && r.out.ly==0);
 assert(sizeof(pd_record)==44336 && r.context_valid==7 && r.context[0x13d4]==0x80 && r.upstream_valid==7 && r.upstream[0][0x12]==1 && r.model_valid==127 && r.property_valid==15);
 CloseHandle(f);puts("PASS: pose candidates, invalid pointer handling, event timing, B edges, controller identity, unchanged packets, nonblocking queue, bounded recording, binary worker output");
 f=CreateFileW(L"C:\\Games\\Skate\\mine\\reskatedualstick-source\\build\\session_controls_pose_properties.csv",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0,OPEN_EXISTING,0,0);assert(f!=INVALID_HANDLE_VALUE);
 char catalogue[8192]={0};assert(ReadFile(f,catalogue,sizeof catalogue-1,&n,0)&&n>0);CloseHandle(f);
 assert(strstr(catalogue,"graph_offset,key,node,page,default_value,name") && strstr(catalogue,"Animation/Test.Bool_Example"));
 assert(strstr(catalogue,"descriptor_hex"));
 assert(strstr(catalogue,"Animation/Test.Bool.Computed"));
 unsigned lines=0;for(unsigned i=0;i<n;i++)if(catalogue[i]=='\n')lines++;assert(lines==11);
 puts("PASS: background read-only boolean property catalogue; descriptor types and graph ownership checked; existing binary format preserved");
}
