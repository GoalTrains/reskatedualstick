#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <setjmp.h>
#define PT_FILE_LIMIT (128u*1024u)
static WCHAR g_dir[512];
static void log_text(const char*s){(void)s;}
static jmp_buf done;
static HANDLE active;
static HANDLE test_open(const WCHAR*p,DWORD a,DWORD s,void*x,DWORD d,DWORD f,HANDLE h){
 HANDLE result=CreateFileW(p,a,s,x,d,f,h); if(a==4)active=result;return result;
}
static BOOL file_size(HANDLE f,INT64*n){LARGE_INTEGER v;BOOL ok=GetFileSizeEx(f,&v);*n=v.QuadPart;return ok;}
static BOOL file_seek(HANDLE f,INT64 n,INT64*out,DWORD method){LARGE_INTEGER v={0},got;v.QuadPart=n;BOOL ok=SetFilePointerEx(f,v,&got,method);if(out)*out=got.QuadPart;return ok;}
static void test_sleep(DWORD ms){if(ms==20){CloseHandle(active);longjmp(done,1);}}
static BOOL counter(INT64*n){LARGE_INTEGER v;BOOL ok=QueryPerformanceCounter(&v);*n=v.QuadPart;return ok;}
static BOOL frequency(INT64*n){LARGE_INTEGER v;BOOL ok=QueryPerformanceFrequency(&v);*n=v.QuadPart;return ok;}
#define QueryPerformanceCounter counter
#define QueryPerformanceFrequency frequency
#define CreateFileW test_open
#define GetFileSizeEx file_size
#define SetFilePointerEx file_seek
#define Sleep test_sleep
#include "../src/precise_trace.h"
#undef CreateFileW
#undef GetFileSizeEx
#undef SetFilePointerEx
#undef Sleep
static UINT64 size(const WCHAR*p){HANDLE f=CreateFileW(p,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);assert(f!=INVALID_HANDLE_VALUE);LARGE_INTEGER n;assert(GetFileSizeEx(f,&n));CloseHandle(f);return n.QuadPart;}
static void valid_csv(const WCHAR*p){
 FILE*f=_wfopen(p,L"rb");assert(f);char line[4096];int headers=0,rows=0;
 while(fgets(line,sizeof line,f)){if(line[0]=='#')continue;if(line[0]=='p'){headers++;continue;}
 int commas=0;for(char*q=line;*q;q++)if(*q==',')commas++;
 assert(commas==PT_FIELDS+3);assert(line[strlen(line)-1]=='\n');rows++;}
 fclose(f);assert(headers>0&&rows>0);
}
int main(void){
 pt_record r={0};r.start_us=1;pt_enqueue(&r);assert(pt_write==1);
 for(int i=57;i<=61;i++){r.v[i]++;r.start_us++;pt_enqueue(&r);}assert(pt_write==1);
 r.v[PT_FIELDS-2]++;pt_enqueue(&r);assert(pt_write==1); /* diagnostic sample age only */
 r.v[62]=1;pt_enqueue(&r);assert(pt_write==2); /* switch-invert setting */
 r.v[31]=200;pt_enqueue(&r);assert(pt_write==3); /* real stick change */
 r.start_us+=100000;pt_enqueue(&r);assert(pt_write==4);
 r.dropped=9;pt_dropped=9;pt_enqueue(&r);assert(pt_write==5);
 WCHAR root[MAX_PATH],path[MAX_PATH],previous[MAX_PATH];GetTempPathW(MAX_PATH,root);
 swprintf(g_dir,512,L"%lsdual-stick-trace-%lu",root,GetCurrentProcessId());assert(CreateDirectoryW(g_dir,0));
 swprintf(path,MAX_PATH,L"%ls\\session_controls_input.csv",g_dir);
 swprintf(previous,MAX_PATH,L"%ls\\session_controls_input.previous.csv",g_dir);
 HANDLE f=CreateFileW(path,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);assert(f!=INVALID_HANDLE_VALUE);
 assert(pt_send(f,pt_header,sizeof pt_header-1));char line[4096];unsigned n=pt_format(&r,line);
 for(int i=0;i<2500;i++)assert(pt_send(f,line,n));CloseHandle(f);
 assert(size(path)>PT_FILE_LIMIT);assert(pt_archive(path,previous));assert(size(previous)<=PT_FILE_LIMIT);valid_csv(previous);
 /* Startup trims legacy oversize; multiple in-session rotations keep whole rows. */
 pt_read=0;pt_write=PT_CAP-1;
 for(unsigned i=0;i<pt_write;i++){pt_queue[i]=r;pt_queue[i].poll=i;pt_queue[i].v[31]=(int)i;}
 if(!setjmp(done))pt_worker(0);
 assert(!pt_failed);assert(size(path)<=PT_FILE_LIMIT);assert(size(previous)<=PT_FILE_LIMIT);
 valid_csv(path);valid_csv(previous);
 /* A new launch keeps the same aggregate bound. */
 pt_read=pt_write=0;if(!setjmp(done))pt_worker(0);assert(!pt_failed);
 assert(size(path)+size(previous)<=2*PT_FILE_LIMIT);
 assert(DeleteFileW(path));assert(DeleteFileW(previous));assert(RemoveDirectoryW(g_dir));
 puts("precise trace: timing suppression, settings/axis changes, heartbeat, oversize migration and repeated rotation pass");return 0;
}
