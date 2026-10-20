#include "fakie_exit.h"
#include "fakie_input_guard.h"
static fke_state g_fke;
static volatile UINT64 g_fke_input_seen;
static volatile int g_fke_protected;
static int fke_write(np_address address,const void* data,np_address size){
 SC_MBI m;UINT64 done=0;
 if(size!=4||(address&3)||!VirtualQuery((const void*)address,&m,sizeof m)||m.state!=0x1000||
    (m.prot&0xff)!=PAGE_READWRITE||(m.prot&0x100)||address<(UINT64)m.base||address+4>(UINT64)m.base+m.size)return 0;
 return WriteProcessMemory(GetCurrentProcess(),(void*)address,data,4,&done)&&done==4;
}
static void fke_tick(UINT64 base,UINT64 selector,UINT64 now){
 UINT64 input_seen=g_fke_input_seen;
 int enabled=g_user_enabled&&g_cfg.enabled&&g_cfg.pure&&g_cfg.disable_hold_fakie&&!g_paused&&g_native_menu==0&&!g_menu&&!g_ui_open&&
     selector&&g_local>=0&&g_slots[g_local].sel==selector&&now>=input_seen&&now-input_seen<=100;
 int result=fke_step(&g_fke,enabled,g_cfg.fakie_return,g_local>=0?(int)g_slots[g_local].state:0,g_nr_idle,g_fke_protected,(double)now,selector,base,rd_copy,fke_write);
 if(result==1)log_text(g_cfg.fakie_return==2?"fakie return: native timer exit requested (after-landing mode)":"fakie return: native timer exit requested after neutral controls");
}
