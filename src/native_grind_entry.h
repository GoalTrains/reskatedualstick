/* Ordinary air-to-rail target selector, supported 20260929 binary only.
 * 47fc590 consumes context+17d0 with +13dc bit17 enabled; 47fc450
 * validates that explicit GrindTargetType against the available candidates.
 * Object+98 owns the context; ad0/ad8 expose a selected valid candidate.
 * Temporary type/enable and isolated native alignment cache. Native geometry
 * always runs; stick output and native accepted-candidate thresholds stay intact.
 */
#ifndef SC_NATIVE_GRIND_ENTRY_H
#define SC_NATIVE_GRIND_ENTRY_H
#include "grind_entry_plan.h"
#include "grind_target_read.h"
#include "grind_entry_diag.h"
typedef void (*ne_fn)(UINT64,UINT64);
static ne_fn g_ne_previous;
static int g_ne_hooked,g_ne_bad;
static volatile int g_ne_lock;
static unsigned g_ne_calls,g_ne_applied,g_ne_consumed;
static gd_native_entry g_ne_observation;
static UINT64 g_ne_next;
static struct {UINT64 core,context,seen;sc_entry_plan plan;} g_ne;
/* 47fc4c1..572 caches the SIGNED alignment target at object+b10/b14.
 * A temporary type must not inherit a cached target from another request.
 * Let native code calculate our cache, then restore the game's five bytes.
 * Keep native's sign latch through this one continuous approach only. */
static struct {UINT64 object,context,last;int type,side;BYTE bytes[5];} g_ne_cache;
static int ne_writable(UINT64 a,UINT64 n){
 SC_MBI m;
 return VirtualQuery((void*)a,&m,sizeof m) && m.state==0x1000 &&
        (m.prot&0xff)==PAGE_READWRITE && !(m.prot&0x100) &&
        a>=(UINT64)m.base && a+n>=a && a+n<=(UINT64)m.base+m.size;
}
static int ne_store(UINT64 a,const void*p,unsigned n){
 UINT64 done=0;return WriteProcessMemory(GetCurrentProcess(),(void*)a,p,n,&done)&&done==n;
}
/* Poll-time, read-only qualification for polar input compensation. Fail closed
 * when this supported hook is unavailable; no curve changes in ordinary air. */
static void ne_prepare(sc_state*s){
 s->entry_rail_target=0;
 if(!g_ne_hooked || g_ne_bad || !g_cfg.enabled || !g_user_enabled || !g_cfg.split_grinds || !g_cfg.native_grind_entry ||
    g_cfg.one_foot_flips || !g_cfg.air_controls || g_ui_open || g_native_menu || g_paused || g_menu || g_walking ||
    g_local<0 || native_local_slot()!=g_local || g_slots[g_local].state!=201)return;
 nr_frame f;gt_sample target;DWORD state=0;
 if(nr_read(g_slots[g_local].sel,(UINT64)GetModuleHandleW(0),rd_copy,&f) &&
    rd_copy(f.context+0x1414,&state,4) && state==201 &&
    (gt_read(f.core,rd_copy,&target)&2) && target.targeting_grind)s->entry_rail_target=1;
}
static void ne_candidate_snapshot(UINT64 object,int type,int original){
 gd_native_entry*d=&g_ne_observation;
 d->candidate_valid=0;d->candidate_index=-1;d->original_type=original;d->selected_type=7;
 memset(d->candidate_values,0,sizeof d->candidate_values);
 if(type<0 || type>6)return;
 UINT64 row=object+(UINT64)type*20;int index=-1;
 if(!rd_copy(row+0x10,&index,4) || !rd_copy(object+0xaec,&d->candidate_values[0],4) ||
    !rd_copy(row+8,&d->candidate_values[3],4) || !rd_copy(row,&d->candidate_values[4],4) ||
    !rd_copy(row+4,&d->candidate_values[5],4))return;
 d->candidate_index=index;d->candidate_valid|=1;
 /* Native arrays occupy five float slots each; do not follow a corrupt index. */
 if(index>=0 && index<5 && rd_copy(object+0x9a0+index*4,&d->candidate_values[1],4) &&
    rd_copy(object+0x9b4+index*4,&d->candidate_values[2],4)){
  float distance=d->candidate_values[1];
  if(__builtin_isfinite(distance) && distance>=-.5f && distance<65.5f){
   int angle_index=(int)(distance+.5f);
   if(rd_copy(object+0x9c8+angle_index*4,&d->candidate_values[6],4))d->candidate_valid|=2;
  }
 }
 UINT64 base=np_base();
 if(base && rd_copy(base+0x607acc8,&d->candidate_values[7],4))d->candidate_valid|=4;
}
static void ne_target(UINT64 object,UINT64 output){
 if(__sync_lock_test_and_set(&g_ne_lock,1)){g_ne_previous(object,output);return;}
 DWORD saved_error=GetLastError();UINT64 now=GetTickCount64(),context=0;nr_frame f;
 int slot=native_local_slot();DWORD state=0;gt_sample target;float native_request_gate=5;
 int owned=slot>=0 && slot==g_local &&
   np_pointer(rd_copy,object+0x98,&context) &&
   nr_read(g_slots[slot].sel,(UINT64)GetModuleHandleW(0),rd_copy,&f) &&
   f.context==context;
 if(owned)g_ne_calls++;
 int eligible=owned && f.core==g_ne.core && context==g_ne.context && !g_ne_bad && g_cfg.enabled && g_user_enabled && g_cfg.split_grinds && g_cfg.native_grind_entry &&
   !g_cfg.one_foot_flips && !g_ui_open && !g_native_menu && !g_paused && !g_menu && !g_walking &&
   g_ne.plan.active && rd_copy(context+0x1414,&state,4) &&
   sc_entry_fresh(now,g_ne.seen,state) &&
   /* Match the selector's own explicit-request branch (47fc5c7..5d7). */
   rd_copy(object+0xaec,&native_request_gate,4) && __builtin_isfinite(native_request_gate) && native_request_gate<5 &&
   (gt_read(f.core,rd_copy,&target)&2) && target.targeting_grind;
 int observed_native_type=7;
 if(owned)rd_copy(context+0x17d0,&observed_native_type,4);
 int old_type=7,applied=0;DWORD old_flags=0;BYTE old_cache[5];
 if(owned && !eligible)g_ne_cache.last=0;
 if(eligible && (g_ne.plan.type==1 || g_ne.plan.type==2) &&
    ne_writable(context+0x17d0,4) && ne_writable(context+0x13dc,4) &&
    ne_writable(object+0xb10,5) && rd_copy(object+0xb10,old_cache,5) && old_cache[4]<=1 &&
    rd_copy(context+0x17d0,&old_type,4) && old_type>=0 && old_type<=7 &&
    rd_copy(context+0x13dc,&old_flags,4)){
   DWORD flags=old_flags|0x20000;
   ne_candidate_snapshot(object,g_ne.plan.type,old_type);
   int side=g_ne.plan.yaw<0;
   if(!g_ne_cache.last || now<g_ne_cache.last || now-g_ne_cache.last>60 ||
      g_ne_cache.object!=object || g_ne_cache.context!=context ||
      g_ne_cache.type!=g_ne.plan.type || g_ne_cache.side!=side)memset(g_ne_cache.bytes,0,5);
   g_ne_cache.object=object;g_ne_cache.context=context;g_ne_cache.type=g_ne.plan.type;g_ne_cache.side=side;g_ne_cache.last=now;
   memcpy(&g_ne_observation.cache_before,old_cache,4);g_ne_observation.cache_valid_before=old_cache[4];
   memcpy(&g_ne_observation.cache_used,g_ne_cache.bytes,4);g_ne_observation.cache_valid_used=g_ne_cache.bytes[4];
   if(ne_store(object+0xb10,g_ne_cache.bytes,5) && ne_store(context+0x17d0,&g_ne.plan.type,4) && ne_store(context+0x13dc,&flags,4)){
    applied=1;g_ne_applied++;
   }else {
    ne_store(object+0xb10,old_cache,5);g_ne_cache.last=0;
    ne_store(context+0x17d0,&old_type,4);ne_store(context+0x13dc,&old_flags,4);g_ne_bad=1;
   }
 }
 SetLastError(saved_error);g_ne_previous(object,output);DWORD original_error=GetLastError();
 if(applied){
  BYTE calculated_cache[5];float angle=0;
  if(rd_copy(object+0xb10,calculated_cache,5)){
   memcpy(&angle,calculated_cache,4);
   g_ne_observation.cache_after=angle;g_ne_observation.cache_valid_after=calculated_cache[4];
   if(calculated_cache[4]<=1 && __builtin_isfinite(angle) && sc_abs(angle)<=180)memcpy(g_ne_cache.bytes,calculated_cache,5);
   else {g_ne_cache.last=0;g_ne_bad=1;}
  }else {g_ne_cache.last=0;g_ne_bad=1;}
  if(!ne_store(object+0xb10,old_cache,5))g_ne_bad=1;
  int consumed=7,after_type=7;BYTE active=0;DWORD after_flags;
  if(rd_copy(object+0xad0,&active,1) && active==1 && rd_copy(object+0xad8,&consumed,4) && consumed==g_ne.plan.type)g_ne_consumed++;
  else consumed=7;
  if(active==1){int selected=7;if(rd_copy(object+0xad8,&selected,4))g_ne_observation.selected_type=selected;}
  g_ne_observation.qpc_us=rd_time();g_ne_observation.object=object;g_ne_observation.context=context;
  g_ne_observation.requested=(DWORD)g_ne.plan.type;g_ne_observation.accepted=(DWORD)consumed;
  /* These angles describe physical input intent, not values written to physics. */
  g_ne_observation.yaw=g_ne.plan.yaw;g_ne_observation.pitch=g_ne.plan.pitch;
  /* Restore only fields which still contain our request. Never overwrite a
   * newer native writer. The calculated target matrices remain game's work. */
  if(rd_copy(context+0x17d0,&after_type,4) && after_type==g_ne.plan.type){
   if(!ne_store(context+0x17d0,&old_type,4))g_ne_bad=1;
   if(rd_copy(context+0x13dc,&after_flags,4) && (after_flags&0x20000)){
    after_flags=(after_flags&~0x20000u)|(old_flags&0x20000);
    if(!ne_store(context+0x13dc,&after_flags,4))g_ne_bad=1;
   }
  }
 }
 if(owned && !applied && g_grind_diag){
  BYTE active=0;int selected=7;
  ne_candidate_snapshot(object,observed_native_type,observed_native_type);
  if(rd_copy(object+0xad0,&active,1) && active==1)rd_copy(object+0xad8,&selected,4);
  g_ne_observation.qpc_us=rd_time();g_ne_observation.object=object;g_ne_observation.context=context;
  g_ne_observation.requested=(DWORD)observed_native_type;g_ne_observation.accepted=(DWORD)selected;
  g_ne_observation.selected_type=selected;g_ne_observation.yaw=g_ne_observation.pitch=0;
 }
 __sync_lock_release(&g_ne_lock);SetLastError(original_error);
}
static void ne_try_hook(UINT64 now){
 if(g_ne_hooked || g_ne_bad || now<g_ne_next || (!g_grind_diag && (!g_cfg.split_grinds || !g_cfg.native_grind_entry)))return;
 g_ne_next=now+1000;UINT64 base=np_base();BYTE actual[16];
 static const BYTE prefix[16]={0x40,0x53,0x48,0x83,0xec,0x40,0x48,0x8b,0x81,0x98,0,0,0,0x48,0x8b,0xd9};
 if(!base || !rd_copy(base+0x47fc590,actual,16) || !bytes_equal(actual,prefix,16)){
  g_ne_bad=1;log_text("native back-foot entry: unsupported/changed ordinary air selector; stick fallback retained");return;
 }
 BYTE* stub=alloc_near(base+0x47fc590);if(!stub){g_ne_bad=1;return;}
 /* First 13 bytes comprise three complete non-relative instructions. */
 memcpy(stub,actual,13);stub[13]=0xff;stub[14]=0x25;memset(stub+15,0,4);
 UINT64 continuation=base+0x47fc59d;memcpy(stub+19,&continuation,8);
 stub[64]=0xff;stub[65]=0x25;memset(stub+66,0,4);UINT64 callback=(UINT64)ne_target;memcpy(stub+70,&callback,8);
 INT64 delta=(INT64)(UINT64)(stub+64)-(INT64)(base+0x47fc595);
 if(delta<-2147483647LL || delta>2147483647LL){g_ne_bad=1;return;}
 DWORD old,unused;if(!VirtualProtect(stub,0x1000,0x20,&old)){g_ne_bad=1;return;}
 FlushInstructionCache(GetCurrentProcess(),stub,78);
 BYTE branch[8]={0xe9,0,0,0,0,0x90,0x90,0x90};int displacement=(int)delta;memcpy(branch+1,&displacement,4);
 UINT64 expected,replacement;memcpy(&expected,actual,8);memcpy(&replacement,branch,8);
 if(!VirtualProtect((void*)(base+0x47fc590),8,0x40,&old)){g_ne_bad=1;return;}
 g_ne_previous=(ne_fn)stub;
 UINT64 before=__sync_val_compare_and_swap((volatile UINT64*)(base+0x47fc590),expected,replacement);
 VirtualProtect((void*)(base+0x47fc590),8,old,&unused);FlushInstructionCache(GetCurrentProcess(),(void*)(base+0x47fc590),8);
 if(before!=expected){g_ne_bad=1;log_text("native back-foot entry: competing hook; stick fallback retained");return;}
 g_ne_hooked=1;log_text("native air selector: owned observations enabled; experimental requests still require NativeGrindEntry");
}
static void ne_publish(const sc_state*s,UINT64 now){
 if(__sync_lock_test_and_set(&g_ne_lock,1))return;
 g_ne.plan.active=0;
 if(g_ne_hooked && !g_ne_bad && g_local>=0 && native_local_slot()==g_local){
  sc_entry_plan p=sc_back_entry_plan(&g_cfg,s);nr_frame f;
  if(p.active && nr_read(g_slots[g_local].sel,(UINT64)GetModuleHandleW(0),rd_copy,&f)){
   g_ne.core=f.core;g_ne.context=f.context;g_ne.seen=now;g_ne.plan=p;
  }
 }
 if(!g_ne.plan.active)g_ne_cache.last=0;
 static UINT64 logged;static unsigned last;
 if(now-logged>=1000 && g_ne_calls!=last){
  logged=now;last=g_ne_calls;
  log_num("native back-foot entry: owned air-selector calls ",g_ne_calls);
  log_num("native back-foot entry: requests applied ",g_ne_applied);
  log_num("native back-foot entry: candidates accepted ",g_ne_consumed);
 }
 __sync_lock_release(&g_ne_lock);
}
static void ne_diagnostic(gd_native_entry*out){
 memset(out,0,sizeof *out);
 if(__sync_lock_test_and_set(&g_ne_lock,1))return;
 *out=g_ne_observation;out->valid=1;out->hooked=g_ne_hooked;out->bad=g_ne_bad;
 out->calls=g_ne_calls;out->applied=g_ne_applied;out->consumed=g_ne_consumed;
 __sync_lock_release(&g_ne_lock);
}
#endif
