#include "stance_flight_read.h"
static void sf_native_capture(UINT64 base,UINT64 selector,UINT64 now,int valid,int track,int triggered,int xl_track,int xl_valid,int hint) {
 sf_record r={0};r.tick_ms=now;r.qpc_us=pt_time();r.owner=selector;for(int i=0;i<SF_FIELDS;i++)r.v[i]=-1;
 int* v=r.v;sc_state* st=&g_state[g_xl_input_user<4?g_xl_input_user:0];
 v[0]=g_local>=0?(int)g_slots[g_local].state:-1;v[1]=valid;v[2]=g_cfg.goofy;v[3]=g_game_goofy;v[4]=g_cfg.xl_layout;
 v[5]=st->switched;v[6]=st->game_switch_known;v[7]=st->game_switch;v[8]=st->feet_map_left;v[9]=st->feet_map_ok;
 v[16]=(int)sf_native_read(base,selector,v);
 v[23]=track;v[24]=g_ground_pose.baseline_valid;v[25]=g_ground_pose.baseline_mirror;v[26]=g_ground_pose.suspect;v[27]=g_ground_pose.mirror_break;v[28]=g_ground_pose.neutral;
 v[29]=triggered;v[30]=g_nr_pending;v[31]=g_nr_idle;v[32]=xl_track;v[33]=xl_valid;v[34]=g_xl_agreement.baseline_valid;v[35]=g_xl_agreement.mirror;v[36]=g_xl_agreement.candidate;v[37]=hint;v[38]=g_xl_idle;
 v[39]=g_paused;v[40]=g_native_menu;v[41]=g_menu;v[42]=g_ui_open;v[43]=g_cfg.enabled;v[44]=g_switch_src;
 for(int i=0;i<4;i++)v[45+i]=g_switch_probes[i];
 v[51]=g_xl_agreement.baseline_valid;
 if(v[51] && v[11]>=0 && v[11]<=1 && v[12]>=0 && v[12]<=1)v[49]=sc_xl_physical_raw(v[11],v[12],g_xl_agreement.mirror);
 if(v[10]>=0 && v[10]<=1 && v[12]>=0 && v[12]<=1)v[50]=v[10]^v[12];
 sf_capture(&r);
}