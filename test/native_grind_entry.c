#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/native_menu_respawn.h"
typedef struct {void*base;void*alloc;DWORD aprot;WORD part,pad0;UINT64 size;DWORD state,prot,type,pad1;} SC_MBI;
static SIZE_T query_fixture(const void*p,SC_MBI*out,SIZE_T n){
 (void)n;MEMORY_BASIC_INFORMATION m;SIZE_T r=VirtualQuery(p,&m,sizeof m);if(!r)return 0;
 memset(out,0,sizeof *out);out->base=m.BaseAddress;out->size=m.RegionSize;out->state=m.State;out->prot=m.Protect;return sizeof *out;
}
static sc_config g_cfg;static int g_grind_diag=1;static int g_user_enabled=1,g_ui_open,g_native_menu,g_paused,g_menu,g_walking,g_local;
static struct {UINT64 sel;DWORD state;} g_slots[1];
static BYTE context[0x1920],core[0x450],root[0x70],types[0xd0],target[0x200],object[0xb20],foreign[0xb20];
static int owner_ok=1,original_calls,frame_calculations,observed_type,available=1;static float observed_yaw,observed_pitch;
static float fixture_angle=40,alignment_correction;
static int native_local_slot(void){return owner_ok?0:-1;}
static int fixture_read(UINT64 a,void*out,UINT64 n){
 BYTE*blocks[]={context,core,root,types,target,object,foreign};unsigned sizes[]={sizeof context,sizeof core,sizeof root,sizeof types,sizeof target,sizeof object,sizeof foreign};
 for(int i=0;i<7;i++)if(a>=(UINT64)blocks[i] && a+n<=(UINT64)blocks[i]+sizes[i]){memcpy(out,(void*)a,n);return 1;}return 0;
}
static int fixture_frame(UINT64 sel,UINT64 base,np_read_fn read,nr_frame*f){
 (void)base;(void)read;memset(f,0,sizeof *f);if(!owner_ok)return 0;f->selector=sel;f->core=(UINT64)core;f->context=(UINT64)context;return 1;
}
static BYTE*alloc_near(UINT64 a){(void)a;return 0;}
static UINT64 np_base(void){return 0;}
static int bytes_equal(const BYTE*a,const BYTE*b,unsigned n){return !memcmp(a,b,n);}
static void log_text(const char*s){(void)s;}
static void log_num(const char*s,UINT64 n){(void)s;(void)n;}
static UINT64 rd_time(void){return GetTickCount64()*1000;}
#define VirtualQuery query_fixture
#define rd_copy fixture_read
#define nr_read fixture_frame
#include "../src/native_grind_entry.h"
static void ptr(BYTE*b,unsigned off,UINT64 v){memcpy(b+off,&v,8);}
/* Model native candidate validation, not board physics. */
static void original_fixture(UINT64 address,UINT64 output){
 assert(output==123);original_calls++;
 if(address==(UINT64)foreign){observed_type=7;SetLastError(987);return;}
 BYTE*obj=(BYTE*)address;BYTE*ctx=*(BYTE**)(obj+0x98);
 observed_type=*(int*)(ctx+0x17d0);
 observed_yaw=*(float*)(ctx+0x190c);observed_pitch=*(float*)(ctx+0x1910);
 if((*(DWORD*)(ctx+0x13dc)&0x20000) && observed_type<7){
  obj[0xad0]=(BYTE)available;*(int*)(obj+0xad8)=observed_type;frame_calculations++;
  /* Model the verified native rejected-candidate branch 47fc4c1..572.
   * b14 latches b10, so replacing a request without isolating it reuses
   * a different request's signed target. Do not model candidate physics. */
  if(!available){
   if(!obj[0xb14]){
    float desired=observed_type==2?-90.0f:90.0f;
    if(fixture_angle<0)desired=-desired;
    *(float*)(obj+0xb10)=desired;obj[0xb14]=1;
   }
   alignment_correction=*(float*)(obj+0xb10)-fixture_angle;
  }
  *(DWORD*)(ctx+0x13dc)|=0x4000; /* Unrelated native change must survive restore. */
 }
 SetLastError(987);
}
static void original_inputs(void){
 assert(*(int*)(context+0x17d0)==6 && *(int*)(context+0x17d4)==8);
 assert(*(float*)(context+0x190c)==.2f && *(float*)(context+0x1910)==.1f);
 assert(*(float*)(object+0xb10)==30 && object[0xb14]==1);
}
int main(void){
 ptr(core,0x3b8,(UINT64)root);ptr(root,0x20,(UINT64)types);ptr(root,0x10,(UINT64)target);
 ptr(object,0x98,(UINT64)context);ptr(foreign,0x98,(UINT64)root);
 *(DWORD*)(context+0x1414)=201;*(int*)(context+0x17d0)=6;*(int*)(context+0x17d4)=8;
 *(DWORD*)(context+0x13dc)=0x20|0x20000;
 *(float*)(context+0x190c)=.2f;*(float*)(context+0x1910)=.1f;target[0x1fb]=1;
 *(float*)(object+0xb10)=30;object[0xb14]=1;
 g_cfg.enabled=g_cfg.pure=g_cfg.split_grinds=g_cfg.native_grind_entry=g_cfg.air_controls=1;g_cfg.grind_sensitivity=1.2f;
 g_ne_hooked=1;g_ne_previous=original_fixture;
 /* Diagnostics observe owned native selection even without an active mod plan.
  * The native original still runs; no request/cache is supplied by the mod. */
 g_cfg.native_grind_entry=0;ne_target((UINT64)object,123);
 assert(original_calls==1 && g_ne_calls==1 && !g_ne_applied && !g_ne_consumed);
 assert(g_ne_observation.requested==6 && g_ne_observation.selected_type==6 && g_ne_observation.qpc_us);
 original_inputs();assert(GetLastError()==987);
 original_calls=frame_calculations=0;g_ne_calls=0;g_cfg.native_grind_entry=1;

 sc_state s={0};s.air=1;s.entry_back_active=1;s.entry_back_x=1;s.entry_back_y=-.3f;
 g_slots[0].state=201;ne_prepare(&s);assert(s.entry_rail_target);
 target[0x1fb]=0;ne_prepare(&s);assert(!s.entry_rail_target);target[0x1fb]=1;
 g_slots[0].state=400;ne_prepare(&s);assert(!s.entry_rail_target);g_slots[0].state=201;
 owner_ok=0;ne_prepare(&s);assert(!s.entry_rail_target);owner_ok=1;
 g_user_enabled=0;ne_prepare(&s);assert(!s.entry_rail_target);g_user_enabled=1;
 g_ne_bad=1;ne_prepare(&s);assert(!s.entry_rail_target);g_ne_bad=0;
 ne_prepare(&s);assert(s.entry_rail_target);
 ne_publish(&s,GetTickCount64());ne_target((UINT64)object,123);
 assert(original_calls==1 && g_ne_applied==1 && g_ne_consumed==1 && observed_type==2 && frame_calculations==1);
 assert(observed_yaw==.2f && observed_pitch==.1f);original_inputs();assert(GetLastError()==987);
 assert(*(DWORD*)(context+0x13dc)==(0x20|0x20000|0x4000));
 *(DWORD*)(context+0x13dc)=0x20;ne_target((UINT64)object,123);
 assert(g_ne_consumed==2 && *(DWORD*)(context+0x13dc)==(0x20|0x4000));original_inputs();
 gd_native_entry diagnostic;ne_diagnostic(&diagnostic);
 assert(diagnostic.valid && diagnostic.hooked && diagnostic.applied==2 && diagnostic.consumed==2 && diagnostic.requested==2 && diagnostic.accepted==2 && diagnostic.object==(UINT64)object);
 assert(sizeof(gd_native_entry)==136 && diagnostic.candidate_valid==3 && diagnostic.candidate_index==0 && diagnostic.original_type==6 && diagnostic.selected_type==2);
 /* Capture the actual native-validator operands without modifying them. */
 *(int*)(object+2*20+0x10)=1;
 *(float*)(object+2*20)=1.57f;*(float*)(object+2*20+4)=.25f;*(float*)(object+2*20+8)=.7f;
 *(float*)(object+0x9a4)=2;*(float*)(object+0x9b8)=.6f;*(float*)(object+0x9d0)=-1.4f;
 ne_target((UINT64)object,123);ne_diagnostic(&diagnostic);
 assert(diagnostic.candidate_valid==3 && diagnostic.candidate_index==1 && diagnostic.original_type==6);
 assert(diagnostic.candidate_values[1]==2 && diagnostic.candidate_values[2]==.6f && diagnostic.candidate_values[3]==.7f);
 assert(diagnostic.candidate_values[4]==1.57f && diagnostic.candidate_values[5]==.25f && diagnostic.candidate_values[6]==-1.4f);
 assert(*(float*)(object+0x9a4)==2 && *(float*)(object+0x9d0)==-1.4f);
 /* Invalid and nonfinite lookup indices remain bounded and marked incomplete. */
 *(int*)(object+2*20+0x10)=100;ne_candidate_snapshot((UINT64)object,2,6);
 assert(g_ne_observation.candidate_valid==1 && g_ne_observation.candidate_index==100);
 *(int*)(object+2*20+0x10)=1;*(DWORD*)(object+0x9a4)=0x7fc00000;ne_candidate_snapshot((UINT64)object,2,6);
 assert(g_ne_observation.candidate_valid==1);*(float*)(object+0x9a4)=2;
 available=0;ne_target((UINT64)object,123);ne_diagnostic(&diagnostic);
 assert(diagnostic.applied==4 && diagnostic.consumed==3 && diagnostic.accepted==7 && diagnostic.selected_type==7);
 assert(alignment_correction==-130 && diagnostic.cache_valid_before==1 && diagnostic.cache_before==30);
 assert(diagnostic.cache_valid_used==0 && diagnostic.cache_valid_after==1 && diagnostic.cache_after==-90);original_inputs();
 /* Preserve native's sign latch within an approach, but never the game's
  * unrelated cached angle. A new physical side/gesture gets its own latch. */
 fixture_angle=-5;ne_target((UINT64)object,123);ne_diagnostic(&diagnostic);
 assert(alignment_correction==-85 && diagnostic.cache_valid_used==1 && diagnostic.cache_used==-90);original_inputs();
 s.entry_back_x=-1;ne_publish(&s,GetTickCount64());ne_target((UINT64)object,123);ne_diagnostic(&diagnostic);
 assert(alignment_correction==95 && diagnostic.cache_valid_used==0 && diagnostic.cache_after==90);original_inputs();
 s.entry_back_active=0;ne_publish(&s,GetTickCount64());assert(!g_ne_cache.last);
 s.entry_back_active=1;s.entry_back_x=1;fixture_angle=40;ne_publish(&s,GetTickCount64());
 available=1;
 unsigned before=g_ne_applied;g_ne.seen=GetTickCount64()-100;
 ne_target((UINT64)object,123);assert(g_ne_applied==before && observed_type==6);original_inputs();
 ne_publish(&s,GetTickCount64());g_ui_open=1;ne_target((UINT64)object,123);assert(g_ne_applied==before);g_ui_open=0;
 target[0x1fb]=0;ne_target((UINT64)object,123);assert(g_ne_applied==before);target[0x1fb]=1;
 *(float*)(object+0xaec)=5;ne_target((UINT64)object,123);assert(g_ne_applied==before);*(float*)(object+0xaec)=0;
 owner_ok=0;ne_target((UINT64)object,123);assert(g_ne_applied==before);owner_ok=1;
 ne_target((UINT64)foreign,123);assert(g_ne_applied==before);
 *(DWORD*)(context+0x1414)=400;ne_target((UINT64)object,123);assert(g_ne_applied==before);*(DWORD*)(context+0x1414)=201;
 *(int*)(context+0x17d0)=8;ne_target((UINT64)object,123);assert(g_ne_applied==before && *(int*)(context+0x17d0)==8);*(int*)(context+0x17d0)=6;
 g_cfg.enabled=0;ne_target((UINT64)object,123);assert(g_ne_applied==before);g_cfg.enabled=1;
 s.entry_back_active=0;ne_publish(&s,GetTickCount64());ne_target((UINT64)object,123);assert(g_ne_applied==before);
 assert(!g_ne_bad);original_inputs();
 puts("PASS: ordinary rail selector preserves arguments/error, rejected-candidate native alignment uses a private per-approach cache, native cache/type/flags restored, sign latch retained until gesture changes; accepted candidates and geometry untouched; bounded validator snapshots and stale/UI/foreign/non-target/rail/disabled/neutral exclusions");
}
