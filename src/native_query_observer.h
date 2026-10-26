/* Observe native approach construction; always chain the original exactly once.
 * Only the checked local rider's pointers are retained. No gameplay data writes. */
#ifndef SC_NATIVE_QUERY_OBSERVER_H
#define SC_NATIVE_QUERY_OBSERVER_H
#include "query_observation.h"
typedef void (*qo_fn)(UINT64);
static qo_fn g_qo_previous;
static int g_qo_hooked,g_qo_bad;static volatile int g_qo_lock;
static UINT64 g_qo_next,g_qo_air,g_qo_context,g_qo_seen;static unsigned g_qo_calls;
static void qo_construct(UINT64 air){
 g_qo_previous(air);DWORD error=GetLastError();
 if(g_grind_diag && !g_paused && !g_native_menu && !g_menu && !g_ui_open &&
    g_local>=0 && native_local_slot()==g_local && !__sync_lock_test_and_set(&g_qo_lock,1)){
  nr_frame f;qa_snapshot s;
  if(nr_read(g_slots[g_local].sel,np_base(),rd_copy,&f) && qa_read(air,f.context,rd_copy,&s)){
   g_qo_air=air;g_qo_context=f.context;g_qo_seen=GetTickCount64();g_qo_calls++;
  }
  __sync_lock_release(&g_qo_lock);
 }
 SetLastError(error);
}
static void qo_try_hook(UINT64 now){
 if(!g_grind_diag || g_qo_hooked || g_qo_bad || now<g_qo_next)return;
 g_qo_next=now+1000;UINT64 base=np_base();BYTE actual[16];
 static const BYTE prefix[16]={0x48,0x8b,0xc4,0x48,0x89,0x58,0x10,0x57,0x48,0x81,0xec,0,2,0,0,0x48};
 if(!base || !rd_copy(base+0x481ca80,actual,16) || !bytes_equal(actual,prefix,16)){
  g_qo_bad=1;log_text("entry query observer: unsupported or competing native construction hook; capture skipped");return;
 }
 BYTE* stub=alloc_near(base+0x481ca80);if(!stub){g_qo_bad=1;return;}
 /* Four complete position-independent instructions, ending after SUB RSP. */
 memcpy(stub,actual,15);stub[15]=0xff;stub[16]=0x25;memset(stub+17,0,4);
 UINT64 continuation=base+0x481ca8f;memcpy(stub+21,&continuation,8);
 stub[64]=0xff;stub[65]=0x25;memset(stub+66,0,4);UINT64 callback=(UINT64)qo_construct;memcpy(stub+70,&callback,8);
 INT64 delta=(INT64)(UINT64)(stub+64)-(INT64)(base+0x481ca85);
 if(delta<-2147483647LL || delta>2147483647LL){g_qo_bad=1;return;}
 DWORD old,unused;if(!VirtualProtect(stub,0x1000,0x20,&old)){g_qo_bad=1;return;}
 FlushInstructionCache(GetCurrentProcess(),stub,78);
 BYTE branch[8]={0xe9,0,0,0,0,0x90,0x90,0x90};int displacement=(int)delta;memcpy(branch+1,&displacement,4);
 UINT64 expected,replacement;memcpy(&expected,actual,8);memcpy(&replacement,branch,8);
 if(!VirtualProtect((void*)(base+0x481ca80),8,0x40,&old)){g_qo_bad=1;return;}
 g_qo_previous=(qo_fn)stub;
 UINT64 before=__sync_val_compare_and_swap((volatile UINT64*)(base+0x481ca80),expected,replacement);
 VirtualProtect((void*)(base+0x481ca80),8,old,&unused);FlushInstructionCache(GetCurrentProcess(),(void*)(base+0x481ca80),8);
 if(before!=expected){g_qo_bad=1;log_text("entry query observer: concurrent hook change; capture skipped");return;}
 g_qo_hooked=1;log_text("entry query observer: read-only owned approach construction capture enabled");
}
static void qo_diagnostic(UINT64 context,UINT64 now,qa_snapshot*out,unsigned*hooked,unsigned*bad,unsigned*calls){
 memset(out,0,sizeof *out);*hooked=g_qo_hooked;*bad=g_qo_bad;*calls=0;
 if(__sync_lock_test_and_set(&g_qo_lock,1))return;
 *calls=g_qo_calls;
 if(g_qo_hooked && g_qo_air && g_qo_context==context && now>=g_qo_seen && now-g_qo_seen<=5000)
  qa_read(g_qo_air,context,rd_copy,out);
 __sync_lock_release(&g_qo_lock);
}
#endif
