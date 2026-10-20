#include <assert.h>
#include <stdio.h>
#include "../src/fakie_input_guard.h"
int main(void){
 sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.front_back=1;c.switch_flip=1;c.steer_scale=c.steer_curve=1;
 sc_state s={0};s.game_crouch=-1;sc_pad raw={0},out={0};
 assert(!sc_fakie_protect_input(&c,&s,&raw,&out));
 for(int foot=0;foot<2;foot++)for(int sign=-1;sign<=1;sign+=2){
  raw=(sc_pad){0};if(foot)raw.ry=sc_short(sign*.56f);else raw.ly=sc_short(sign*.56f);
  assert(sc_fakie_protect_input(&c,&s,&raw,&out));
  if(foot)raw.ry=sc_short(sign);else raw.ly=sc_short(sign);
  assert(!sc_fakie_protect_input(&c,&s,&raw,&out)); /* Full setup still eligible for optional auto return. */
 }
 raw=(sc_pad){0};raw.lx=24000;assert(!sc_fakie_protect_input(&c,&s,&raw,&out));raw=(sc_pad){0};raw.ly=1000;assert(!sc_fakie_protect_input(&c,&s,&raw,&out));
 out.ry=sc_short(-.72f);assert(sc_fakie_protect_input(&c,&s,&raw,&out));out=(sc_pad){0};
 for(int hand=0;hand<2;hand++){raw.buttons=hand?SC_RSHOULDER:SC_LSHOULDER;assert(sc_fakie_protect_input(&c,&s,&raw,&out));}
 raw=(sc_pad){0};s.d_land_hold=1;assert(sc_fakie_protect_input(&c,&s,&raw,&out));s.d_land_hold=0;
 /* Recorded Session nose-manual shape across a 180 and later native stance
  * correction remains a manual; the native exit must defer throughout. */
 sc_translate(&c,&s,raw,1000);s.air=s.takeoff=1;raw.ly=sc_short(.56f);
 out=sc_translate(&c,&s,raw,1100);assert(sc_fakie_protect_input(&c,&s,&raw,&out));
 s.switched=1;s.air=s.takeoff=0;out=sc_translate(&c,&s,raw,1200);short lean=out.ry;
 assert(s.d_land_hold&&lean>10000&&sc_fakie_protect_input(&c,&s,&raw,&out));
 s.switched=0;out=sc_translate(&c,&s,raw,1240);assert(out.ry==lean&&sc_fakie_protect_input(&c,&s,&raw,&out));
 raw=(sc_pad){0};out=sc_translate(&c,&s,raw,1300);assert(!sc_fakie_protect_input(&c,&s,&raw,&out));
 puts("PASS: manual leans and handplant holds block fakie normalization; neutral/steering/full setup stay available; recorded 180 manual retained until release");return 0;
}
