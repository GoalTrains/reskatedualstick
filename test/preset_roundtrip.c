#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static WCHAR g_ini[MAX_PATH],g_menu_key_name[24]=L"End";
static sc_config g_cfg;
static int g_ps_enabled=2,g_board_detect=1,g_toggle_ms=3000,g_air_ms=120,g_switch_src=4,g_auto_stance=1,g_ini_goofy=1,g_ui_scale=100,g_user_enabled=1,g_ramp_diag=0,g_show_startup_menu=0;
static WCHAR wlow(WCHAR c){return c>='A'&&c<='Z'?c+32:c;}
static const WCHAR *ui_slot_section(void){return L"CustomPreset1";}
static int ui_slot_filled(void){return GetPrivateProfileIntW(ui_slot_section(),L"PresetSaved",0,g_ini);}
#include "../src/presets.h"
int main(void){WCHAR dir[MAX_PATH];GetTempPathW(MAX_PATH,dir);GetTempFileNameW(dir,L"scp",0,g_ini);
 g_cfg.front_back=0;g_cfg.switch_flip=3;g_cfg.pop_on_release=1;g_cfg.feet_swapped=1;
 g_cfg.deadzone=.15f;g_cfg.active=.3f;g_cfg.manual_max=.85f;g_cfg.steer_scale=1;g_cfg.steer_curve=1.5f;g_cfg.grind_sensitivity=1.2f;
 g_cfg.prewind_steer=1;g_cfg.prewind_percent=80;g_cfg.revert_ms=450;
 WritePrivateProfileStringW(L"SessionControls",L"UnrelatedSetting",L"42",g_ini);
 assert(ui_custom_save());
 assert(GetPrivateProfileIntW(ui_slot_section(),L"SpinWindUpSteer",-1,g_ini)==1);
 assert(GetPrivateProfileIntW(ui_slot_section(),L"SpinWindUpTriggerPercent",-1,g_ini)==80);
 assert(GetPrivateProfileIntW(ui_slot_section(),L"RevertDoubleTapMs",-1,g_ini)==450);
 assert(GetPrivateProfileIntW(ui_slot_section(),L"FootDeadzonePercent",-1,g_ini)==15);
 assert(GetPrivateProfileIntW(ui_slot_section(),L"PlayStationPads",-1,g_ini)==2);
 assert(GetPrivateProfileIntW(ui_slot_section(),L"UnrelatedSetting",-1,g_ini)==42);
 assert(ui_builtin_apply(0));assert(!g_cfg.feet_swapped);
 assert(GetPrivateProfileIntW(L"SessionControls",L"FrontBackFoot",-1,g_ini)==1);
 assert(GetPrivateProfileIntW(L"SessionControls",L"SwitchStance",-1,g_ini)==1);
 assert(ui_custom_load());assert(g_cfg.feet_swapped);
 assert(GetPrivateProfileIntW(L"SessionControls",L"SwitchStance",-1,g_ini)==3);
 assert(GetPrivateProfileIntW(L"SessionControls",L"PresetFeetSwapped",-1,g_ini)==1);
 assert(GetPrivateProfileIntW(L"SessionControls",L"ShowMenuOnStartup",-1,g_ini)==0);
 assert(ui_builtin_apply(1));
 assert(GetPrivateProfileIntW(L"SessionControls",L"OneFootFlips",-1,g_ini)==1);
 assert(GetPrivateProfileIntW(L"SessionControls",L"SwitchStance",-1,g_ini)==3);
 for(int physical=0;physical<2;physical++)for(int release=0;release<2;release++){
 assert(ui_builtin_apply_style(physical,release));
 assert(GetPrivateProfileIntW(L"SessionControls",L"FrontBackFoot",-1,g_ini)==!physical);
 assert(GetPrivateProfileIntW(L"SessionControls",L"SwitchStance",-1,g_ini)==(physical?3:1));
 assert(GetPrivateProfileIntW(L"SessionControls",L"PopOnRelease",-1,g_ini)==release);
 assert(GetPrivateProfileIntW(L"SessionControls",L"StickSteering",-1,g_ini)==physical);
 assert(GetPrivateProfileIntW(L"SessionControls",L"OneFootFlips",-1,g_ini)==physical);
 }
 DeleteFileW(g_ini);puts("PASS: actual INI presets round-trip defaults, explicit settings and foot swap; complete builtins and global startup preference");}
