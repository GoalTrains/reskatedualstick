#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef unsigned long long UINT64;typedef long long INT64;
static WCHAR g_dir[520]=L"C:\\Games\\Skate\\mine\\reskatedualstick-source\\build";
static void wcat(WCHAR*d,unsigned cap,const WCHAR*s){unsigned n=0;while(d[n])n++;while(*s&&n+1<cap)d[n++]=*s++;d[n]=0;}
static void log_text(const char*s){puts(s);}
static char* pt_num(char*p,UINT64 n){char b[24];int k=0;do{b[k++]=(char)('0'+n%10);n/=10;}while(n);while(k)*p++=b[--k];return p;}
static int pt_send(HANDLE f,const char*p,unsigned n){while(n){DWORD w=0;if(!WriteFile(f,p,n,&w,0)||!w)return 0;p+=w;n-=w;}return 1;}
#include "../src/stance_flight.h"
int main(void){
 sf_record r={0};r.tick_ms=123;r.qpc_us=456;r.owner=789;r.v[0]=100;r.v[5]=1;r.v[16]=4095;r.v[17]=-42000;
 char line[1024];unsigned n=sf_format(&r,line);line[n]=0;
 assert(strstr(line,"123,456,789,0,100,") && strstr(line,"-42000"));
 unsigned columns=1,headers=1;for(unsigned i=0;i<n;i++)columns+=line[i]==',';for(const char*p=sf_header;*p;p++)headers+=*p==',';assert(columns==headers && columns==56);
 sf_started=1;sf_lock=1;sf_capture(&r);assert(sf_dropped==1 && sf_write==0);sf_lock=0;
 sf_write=SF_CAP;sf_capture(&r);assert(sf_dropped==2 && sf_write==SF_CAP);
 sf_read=sf_write=0;sf_capture(&r);assert(sf_write==1 && sf_queue[0].dropped==2);
 DWORD id;HANDLE worker=CreateThread(0,0,sf_worker,0,0,&id);assert(worker);CloseHandle(worker);Sleep(150);
 HANDLE f=CreateFileW(L"C:\\Games\\Skate\\mine\\reskatedualstick-source\\build\\session_controls_stance_0.csv",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0,OPEN_EXISTING,0,0);assert(f!=INVALID_HANDLE_VALUE);
 char data[4096]={0};DWORD size;assert(ReadFile(f,data,sizeof data-1,&size,0)&&size>0);CloseHandle(f);
 assert(strstr(data,"raw_valid_mask") && strstr(data,"xl_candidate,xl_hint") && strstr(data,"123,456,789,2,100,"));
 assert(sf_read==sf_write && !sf_failed);
 puts("PASS: stance CSV field alignment, signed coordinates, timestamps, nonblocking contention/full-queue drops, asynchronous file writer");
}