#include "catch_read.h"
static sc_catch_sample g_catch_sample=SC_CATCH_UNKNOWN;
static int g_catch_manual_poll;
static UINT64 g_catch_next,g_catch_selector,g_catch_sample_time;
static int g_catch_sample_state=-1;
static void sc_catch_poll_force(UINT64 now,int force){
 int state=g_local>=0 && g_local<16?(int)g_slots[g_local].state:-1;
 if((!g_catch_diag && !g_catch_manual_poll) || g_local<0 || g_local>=16 || g_native_menu!=0 || g_menu || g_paused || g_ui_open ||
    (g_walking && !(state>=500 && state<600))){
  g_catch_sample=(sc_catch_sample)SC_CATCH_UNKNOWN;g_catch_selector=0;g_catch_next=g_catch_sample_time=0;g_catch_sample_state=-1;return;
 }
 UINT64 selector=g_slots[g_local].sel;
 if(!force && state==g_catch_sample_state && selector==g_catch_selector && now<g_catch_next)return;
 g_catch_selector=selector;g_catch_next=now+20;
 g_catch_sample_time=now;g_catch_sample_state=state;
 g_catch_sample=sc_catch_read(selector,np_base(),rd_copy);
}
static void sc_catch_poll(UINT64 now){sc_catch_poll_force(now,0);}
