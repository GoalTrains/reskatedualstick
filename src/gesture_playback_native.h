#include "gesture_playback.h"
typedef float (*gsp_get_fn)(UINT64,UINT64);
static sc_gp_binding gsp_binding;
static UINT64 gsp_table[7],gsp_next,gsp_check,gsp_bank,gsp_publish_at,gsp_seen_bank;
static unsigned gsp_domain;
static int gsp_installed,gsp_busy,gsp_enabled,gsp_reported;
static float gsp_factor=1.f,gsp_last_native,gsp_last_result;
static volatile unsigned gsp_reads,gsp_local_reads;
static float gsp_get(UINT64 reference,UINT64 bank){
 float original=((gsp_get_fn)gsp_binding.functions[4])(reference,bank);
 DWORD error=GetLastError();
 __sync_fetch_and_add(&gsp_reads,1);
 gsp_seen_bank=bank;
 int active=gsp_installed==1 && gsp_enabled && GetTickCount64()-gsp_publish_at<100 &&
   g_user_enabled && g_gesture_speed_option && !g_native_menu && !g_menu && !g_ui_open && !g_paused && !g_walking;
 float result=sc_gp_value(original,gsp_factor,active,reference,gsp_binding.asset,bank,gsp_bank);
 if(active && reference==gsp_binding.asset+0x60 && bank==gsp_bank){
  gsp_last_native=original;gsp_last_result=result;__sync_fetch_and_add(&gsp_local_reads,1);
 }
 SetLastError(error);return result;
}
static int gsp_swap(UINT64 address,UINT64 expected,UINT64 replacement){
 SC_MBI m;
 if((address&7) || !VirtualQuery((const void*)address,&m,sizeof m) || m.state!=0x1000 ||
    (m.prot&0xff)!=PAGE_READWRITE || (m.prot&0x100) || address+8>(UINT64)m.base+m.size)return 0;
 return __sync_val_compare_and_swap((volatile UINT64*)address,expected,replacement)==expected;
}
static int gsp_same_asset(UINT64 base){
 UINT64 vt,type,name;static const char expected[]="Animation/Dingo/Float.Anim.TrickFlipSpeed";
 char text[sizeof expected];
 if(!np_pointer(rd_copy,gsp_binding.asset,&vt) || vt!=base+0x61ab4a8 ||
    !np_pointer(rd_copy,gsp_binding.asset+8,&type) || type!=base+0x72afbb0 ||
    !np_pointer(rd_copy,gsp_binding.asset+0x18,&name) || !rd_copy(name,text,sizeof text))return 0;
 for(unsigned i=0;i<sizeof expected;i++)if(text[i]!=expected[i])return 0;return 1;
}
static void gsp_sync(UINT64 now,int option){
 if(__sync_lock_test_and_set(&gsp_busy,1))return;
 UINT64 base=np_base();if(!base)goto done;
 if(gsp_installed){
  if(now>=gsp_check || !option){
   gsp_check=now+1000;UINT64 vt=0;
   if(!gsp_same_asset(base) || !rd_copy(gsp_binding.asset+0x60,&vt,8)){
    gsp_enabled=0;gsp_installed=0;gsp_next=now+2000;
   }else if(vt!=(UINT64)(gsp_table+1)){
    gsp_enabled=0;gsp_installed=-1;
    log_text("gesture speed: playback reader changed externally; override suspended");
   }else if(!option && gsp_swap(gsp_binding.asset+0x60,vt,gsp_binding.vtable)){
    gsp_enabled=0;gsp_installed=0;gsp_next=0;
    log_text("gesture speed: original playback reader restored");
   }
  }
  goto done;
 }
 if(!option || now<gsp_next || !cha_contract(base))goto done;
 for(unsigned skipped=0;skipped<0xbbf;skipped++){
  UINT64 owner=0;unsigned domain=gsp_domain++;
  if(gsp_domain>=0xbbf){gsp_domain=0;gsp_next=now+10000;}
  if(rd_copy(base+0x7726d10+domain*8ULL,&owner,8) && owner){
   UINT64 asset=((cha_find_fn)(base+0x41b5800))((WORD)domain,"animation/dingo/float.anim.trickflipspeed");
   sc_gp_binding b={0};
   if(asset && sc_gp_bind(asset,base,rd_copy,&b)){
    gsp_binding=b;memcpy(gsp_table,b.functions,sizeof gsp_table);
    gsp_table[4]=(UINT64)gsp_get;
    if(gsp_swap(b.asset+0x60,b.vtable,(UINT64)(gsp_table+1))){
     gsp_installed=1;gsp_check=now+1000;
     log_text("gesture speed: native TrickFlipSpeed reader installed; local property-bank scope only");
    }
   }
   break;
  }
  if(!gsp_domain)break;
 }
 done:__sync_lock_release(&gsp_busy);
}
static void gsp_publish(UINT64 now,float factor,int enabled){
 gsp_enabled=0;
 if(!enabled || gsp_installed!=1 || g_local<0 || g_local>=16 || g_slots[g_local].state<100 || g_slots[g_local].state>=300)return;
 UINT64 selector=g_slots[g_local].sel,holder,core,vt,model;
 if(!np_pointer(rd_copy,selector+16,&holder) || !np_pointer(rd_copy,holder+16,&core) ||
    !np_pointer(rd_copy,core,&vt) || vt!=np_base()+0x6176d30 ||
    !np_pointer(rd_copy,core+0x610,&model))return;
 gsp_bank=model+0x48;gsp_factor=factor;gsp_publish_at=now;
 __sync_synchronize();gsp_enabled=1;
}
