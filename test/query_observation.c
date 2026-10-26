#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/native_menu_respawn.h"
static BYTE air[0x300],context[0x1800],source[0xbb90],query[0xb20],foreign[0x100];
static _Alignas(16) BYTE code[32],stub[128];
static int owner=0,race,missing,mutate,allow_alloc=1,original_calls;
static UINT64 clock_ms=1000;static int pointer_reads;
static int g_grind_diag=1,g_paused,g_native_menu,g_menu,g_ui_open,g_local;
static struct {UINT64 sel;} g_slots[1];
static int native_local_slot(void){return owner;}
static UINT64 fixture_base(void){return (UINT64)code-0x481ca80;}
static int read_fixture(UINT64 a,void*out,UINT64 n){
 if(missing && a==(UINT64)source+0xbb84)return 0;
 if(mutate && a==(UINT64)air+0x2c0 && ++pointer_reads==2){UINT64 changed=(UINT64)foreign;memcpy(out,&changed,8);return 1;}
 BYTE*blocks[]={air,context,source,query,foreign,code};unsigned sizes[]={sizeof air,sizeof context,sizeof source,sizeof query,sizeof foreign,sizeof code};
 for(int i=0;i<6;i++)if(a>=(UINT64)blocks[i] && a+n<=(UINT64)blocks[i]+sizes[i]){memcpy(out,(void*)a,n);return 1;}
 return 0;
}
static int frame_fixture(UINT64 selector,UINT64 base,np_read_fn read,nr_frame*f){(void)base;(void)read;memset(f,0,sizeof *f);f->selector=selector;f->context=(UINT64)context;return owner==0;}
static UINT64 clock_fixture(void){return clock_ms;}
static BYTE* alloc_near(UINT64 a){assert(a==(UINT64)code);return allow_alloc?stub:0;}
static int bytes_equal(const BYTE*a,const BYTE*b,unsigned n){return !memcmp(a,b,n);}
static void log_text(const char*s){(void)s;}
static BOOL protect_fixture(void*p,SIZE_T n,DWORD prot,DWORD*old){(void)n;*old=4;if(race && p==code && prot==0x40)code[0]=0xcc;return 1;}
static BOOL flush_fixture(HANDLE h,const void*p,SIZE_T n){(void)h;(void)p;(void)n;return 1;}
#define np_base fixture_base
#define rd_copy read_fixture
#define nr_read frame_fixture
#define GetTickCount64 clock_fixture
#define VirtualProtect protect_fixture
#define FlushInstructionCache flush_fixture
#include "../src/native_query_observer.h"
static void ptr(BYTE*b,unsigned off,UINT64 v){memcpy(b+off,&v,8);}
static void previous(UINT64 p){assert(p==(UINT64)air);original_calls++;SetLastError(987);}
static void reset_hook(void){
 const BYTE prefix[16]={0x48,0x8b,0xc4,0x48,0x89,0x58,0x10,0x57,0x48,0x81,0xec,0,2,0,0,0x48};
 memcpy(code,prefix,16);memset(stub,0,sizeof stub);g_qo_hooked=g_qo_bad=0;g_qo_next=0;race=0;allow_alloc=1;
}
int main(void){
 ptr(air,8,(UINT64)context);ptr(air,0x2c0,(UINT64)query);ptr(air,0x30,(UINT64)source);ptr(query,0x98,(UINT64)context);
 source[0xbb84]=1;query[0x180]=1;query[0x181]=0;query[0x182]=1;
 *(unsigned*)(context+0x17d0)=4;*(unsigned*)(context+0x13dc)=0x20000;*(unsigned*)(query+0x170)=2;
 query[0xad0]=1;*(unsigned*)(query+0xad8)=3;*(float*)(query+0xb10)=-90;query[0xb14]=1;*(float*)(query+0xaec)=.4f;
 for(int k=0;k<7;k++){*(int*)(query+k*20+16)=k%5;*(float*)(query+k*20)=.1f*k;}
 for(int i=0;i<5;i++){*(float*)(query+0x9a0+i*4)=2;*(float*)(query+0x9b4+i*4)=.3f;}
 *(float*)(query+0x9c8+8)=-1.4f;
 qa_snapshot s;assert(qa_read((UINT64)air,(UINT64)context,read_fixture,&s));
 assert(s.valid==255 && s.armed==1 && !s.active && s.contact==1 && s.init_enabled==1 && s.request_type==4 && s.profile_type==2);
 assert(s.selected_type==3 && s.alignment==-90 && s.alignment_valid==1);
 for(int k=0;k<7;k++)assert(s.row_valid[k]==7 && s.row[k][2]==2 && s.row[k][7]==-1.4f);
 mutate=1;pointer_reads=0;assert(!qa_read((UINT64)air,(UINT64)context,read_fixture,&s) && !s.valid);mutate=0;
 missing=1;assert(qa_read((UINT64)air,(UINT64)context,read_fixture,&s) && !(s.valid&4));missing=0;
 ptr(query,0x98,(UINT64)foreign);assert(!qa_read((UINT64)air,(UINT64)context,read_fixture,&s) && !s.valid);ptr(query,0x98,(UINT64)context);
 *(int*)(query+16)=100;assert(qa_read((UINT64)air,(UINT64)context,read_fixture,&s) && s.row_valid[0]==1);
 *(int*)(query+16)=0;*(DWORD*)(query+0x9a0)=0x7fc00000;assert(qa_read((UINT64)air,(UINT64)context,read_fixture,&s) && s.row_valid[0]==3);*(float*)(query+0x9a0)=2;
 BYTE before_query[sizeof query],before_context[sizeof context],before_source[sizeof source];memcpy(before_query,query,sizeof query);memcpy(before_context,context,sizeof context);memcpy(before_source,source,sizeof source);
 g_qo_previous=previous;g_qo_hooked=1;qo_construct((UINT64)air);assert(original_calls==1 && g_qo_calls==1 && GetLastError()==987);
 unsigned hooked,bad,calls;qo_diagnostic((UINT64)context,1001,&s,&hooked,&bad,&calls);assert(hooked && !bad && calls==1 && s.valid==255);
 qo_diagnostic((UINT64)context,6001,&s,&hooked,&bad,&calls);assert(!s.valid);
 qo_diagnostic((UINT64)foreign,1001,&s,&hooked,&bad,&calls);assert(!s.valid);
 owner=-1;qo_construct((UINT64)air);assert(original_calls==2 && g_qo_calls==1);owner=0;
 g_paused=1;qo_construct((UINT64)air);assert(original_calls==3 && g_qo_calls==1);g_paused=0;
 g_grind_diag=0;qo_construct((UINT64)air);assert(original_calls==4 && g_qo_calls==1);g_grind_diag=1;
 g_qo_lock=1;qo_construct((UINT64)air);g_qo_lock=0;assert(original_calls==5 && g_qo_calls==1);
 assert(!memcmp(query,before_query,sizeof query) && !memcmp(context,before_context,sizeof context) && !memcmp(source,before_source,sizeof source));
 reset_hook();qo_try_hook(1000);assert(g_qo_hooked && !g_qo_bad && code[0]==0xe9 && code[5]==0x90 && code[7]==0x90);
 assert(!memcmp(code+8,(BYTE[]){0x48,0x81,0xec,0,2,0,0,0x48},8));
 assert(stub[15]==0xff && stub[16]==0x25 && *(UINT64*)(stub+21)==(UINT64)code+15);
 reset_hook();code[0]=0xcc;qo_try_hook(1000);assert(g_qo_bad && !g_qo_hooked && code[0]==0xcc);
 reset_hook();race=1;qo_try_hook(1000);assert(g_qo_bad && !g_qo_hooked && code[0]==0xcc);
 reset_hook();allow_alloc=0;qo_try_hook(1000);assert(g_qo_bad && !g_qo_hooked && code[0]==0x48);
 reset_hook();g_grind_diag=0;qo_try_hook(1000);assert(!g_qo_bad && !g_qo_hooked && code[0]==0x48);
 puts("PASS: query activation, requested/selected types and all candidate rows captured read-only; owner, partial-read, stale/foreign and finite-index guards; native construction chained once with error preserved; exact prefix/trampoline/tail and concurrent-hook rejection checked");
}
