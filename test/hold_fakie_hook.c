#define NR_FIXTURE_MAIN native_respawn_fixture_main
#include "native_respawn_poll.c"
static BYTE settings_mem[1024];
static int setting_read(UINT64 a,void*out,UINT64 n){if(a>=(UINT64)settings_mem && a+n<=(UINT64)settings_mem+sizeof settings_mem){memcpy(out,(void*)a,n);return 1;}return respawn_read(a,out,n);}
#define rd_copy setting_read
#define np_base test_base
#define GetModuleHandleW test_module
static int g_camera_dpad_enabled;
#define GetTickCount64 mock_tick_time
#include "../src/hold_fakie_hook.h"
#undef GetTickCount64
#undef rd_copy
#undef np_base
#undef GetModuleHandleW
static unsigned bool_calls;
static BYTE bool_result=1;
static BYTE original_bool(UINT64 cache,const void* reference){assert(cache==7 && reference==settings_mem);bool_calls++;last_error=77;return bool_result;}
static void setup_setting_hook(int indirect){
 prepare_hook(indirect);hf_hooked=0;hf_next=0;BYTE*entry=hook_code+0x804f60;memset(entry,0x90,16);
 if(indirect){entry[0]=0xff;entry[1]=0x25;int rel=(int)((hook_code+0x100)-(entry+6));memcpy(entry+2,&rel,4);UINT64 dest=(UINT64)official_module+0x200;memcpy(hook_code+0x100,&dest,8);}
 else {entry[0]=0xe9;int rel=(int)((official_module+0x200)-(entry+5));memcpy(entry+1,&rel,4);}
}
int main(void){
 ptr(settings_mem,0,((UINT64)settings_mem+64)|4);ptr(settings_mem,64+0x60,(UINT64)settings_mem+512);strcpy((char*)settings_mem+512,"egf_holdfakiestance");
 hf_previous=original_bool;g_user_enabled=1;g_cfg.disable_hold_fakie=1;
 assert(hf_get_bool(7,settings_mem)==0 && bool_calls==1 && last_error==77);
 strcpy((char*)settings_mem+512,"egf_holdfakiepush");assert(hf_get_bool(7,settings_mem)==0 && bool_calls==2 && last_error==77);
 strcpy((char*)settings_mem+512,"another_setting");assert(hf_get_bool(7,settings_mem)==1 && bool_calls==3 && last_error==77);
 strcpy((char*)settings_mem+512,"egf_holdfakiepush");g_user_enabled=0;assert(hf_get_bool(7,settings_mem)==1);g_user_enabled=1;g_cfg.disable_hold_fakie=0;assert(hf_get_bool(7,settings_mem)==1);g_cfg.disable_hold_fakie=1;
 /* Held manual/setup input must outrank the compatibility OFF override.
  * The input has to be fresh, eligible and owned by the current skater. */
 g_cfg.enabled=g_cfg.pure=1;g_native_owned_selector=0x20000;native_clock=1000;
 strcpy((char*)settings_mem+512,"egf_holdfakiestance");bool_result=0;
 hf_note_hold(&hf_held_stance,0x20000,1000,1,1);assert(hf_get_bool(7,settings_mem)==1&&last_error==77);
 strcpy((char*)settings_mem+512,"egf_holdfakiepush");assert(hf_get_bool(7,settings_mem)==0);
 strcpy((char*)settings_mem+512,"egf_holdfakiestance");native_clock=1101;assert(hf_get_bool(7,settings_mem)==0);
 native_clock=1000;g_native_owned_selector=0x30000;assert(hf_get_bool(7,settings_mem)==0);g_native_owned_selector=0x20000;
 hf_note_hold(&hf_held_stance,0x20000,1000,0,1);assert(hf_get_bool(7,settings_mem)==0);
 hf_note_hold(&hf_held_stance,0x20000,1000,1,0);assert(hf_get_bool(7,settings_mem)==0);
 hf_note_hold(&hf_held_stance,0x20000,1000,1,1);g_cfg.enabled=0;assert(hf_get_bool(7,settings_mem)==0);g_cfg.enabled=1;
 g_cfg.pure=0;assert(hf_get_bool(7,settings_mem)==0);g_cfg.pure=1;
 g_paused=1;assert(hf_get_bool(7,settings_mem)==0);g_paused=0;
 g_menu=1;assert(hf_get_bool(7,settings_mem)==0);g_menu=0;
 g_ui_open=1;assert(hf_get_bool(7,settings_mem)==0);g_ui_open=0;
 g_native_menu=1;assert(hf_get_bool(7,settings_mem)==0);g_native_menu=0;
 hf_note_hold(&hf_held_stance,0,1000,0,0);g_native_owned_selector=0;
 /* Manual catch supplies the native held-trick prerequisite independently of
  * hold-fakie compatibility. Its saved value and unrelated keys are untouched. */
 strcpy((char*)settings_mem+512,"HeldFlipTricks");assert(hf_reference((UINT64)settings_mem,setting_read)==3);
 bool_result=0;g_cfg.enabled=1;g_cfg.pure=1;g_cfg.manual_catch=1;g_cfg.disable_hold_fakie=0;
 assert(hf_get_bool(7,settings_mem)==1 && last_error==77);
 g_cfg.manual_catch=0;assert(hf_get_bool(7,settings_mem)==0);g_cfg.manual_catch=1;
 g_user_enabled=0;assert(hf_get_bool(7,settings_mem)==0);g_user_enabled=1;
 g_cfg.enabled=0;assert(hf_get_bool(7,settings_mem)==0);g_cfg.enabled=1;
 g_cfg.pure=0;assert(hf_get_bool(7,settings_mem)==0);g_cfg.pure=1;
 strcpy((char*)settings_mem+512,"HeldFlipTricksExtra");assert(hf_get_bool(7,settings_mem)==0);
 strcpy((char*)settings_mem+512,"another_setting");assert(hf_get_bool(7,settings_mem)==0);
 /* Both camera prerequisites override conflicting saved values, including
  * while a menu temporarily suspends foot translation. Off restores originals. */
 g_cfg.manual_catch=0;g_cfg.disable_hold_fakie=0;
 for(int original=0;original<2;original++)for(int enabled=0;enabled<2;enabled++)for(int option=0;option<2;option++) {
  bool_result=original;g_user_enabled=enabled;g_camera_dpad_enabled=option;g_cfg.enabled=0;
  strcpy((char*)settings_mem+512,"egf_cameraenablemirrorinput");
  assert(hf_get_bool(7,settings_mem)==(enabled && option?1:original) && last_error==77);
  strcpy((char*)settings_mem+512,"egf_cameraallowautomirror");
  assert(hf_get_bool(7,settings_mem)==(enabled && option?0:original) && last_error==77);
  strcpy((char*)settings_mem+512,"egf_cameraallowautomirror_extra");assert(hf_get_bool(7,settings_mem)==original);
 }
 g_camera_dpad_enabled=0;g_user_enabled=1;g_cfg.enabled=1;
 bool_result=1;g_cfg.manual_catch=0;
 for(int indirect=0;indirect<2;indirect++){
  setup_setting_hook(indirect);hf_try_hook(1000);assert(hf_hooked==1 && (UINT64)hf_previous==(UINT64)official_module+0x200);
  BYTE*entry=hook_code+0x804f60;assert(entry[6]==(indirect?0x90:0x90) && entry[7]==0x90);
 }
 setup_setting_hook(0);hook_race=1;hf_try_hook(1000);assert(hf_hooked==-1 && hook_code[0x804f60]==0xcc);
 setup_setting_hook(0);official_module[0x80]=0;hf_try_hook(1000);assert(!hf_hooked);
 puts("PASS: settings hook calls original once and preserves native error; Hold Fakie overrides and Manual Catch HeldFlipTricks prerequisite are scoped correctly; disabled pass-through, E9/FF25 chaining and race/foreign rejection");
}
