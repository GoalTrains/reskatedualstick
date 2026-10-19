#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/manual_catch.h"
int main(void){
 int cases=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.manual_catch=c.manual_catch_replay=1;
  c.goofy=g;c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=c.front_back?1:3;
  c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;
  sc_state s={0};s.air=1;s.takeoff=2;s.d_took_off=1;s.game_crouch=-1;s.switched=sw;
  s.game_mirror_known=1;s.game_mirror=sw;
  sc_manual_catch m={0};m.phase=2;m.was_air=1;m.x=side*.6f;m.y=.8f;
  /* Feed a real translated multi-point flick path into the recorder. */
  float vx[]={0,side*.45f,side*.9f},vy[]={-1,-.4f,.5f};
  for(unsigned i=0;i<3;i++)sc_mc_replay_record(&m,vx[i],vy[i],1000+i*8);
  sc_pad p={0};p.buttons=SC_LSHOULDER;p.lt=73;p.rt=91;
  sc_pad first=sc_manual_catch_translate(&c,&s,&m,p,1024,200);
  assert(m.phase==2 && m.replay_running && !first.rx && !first.ry);
  for(unsigned cycle=0;cycle<4;cycle++)for(unsigned i=0;i<3;i++){
   double now=1024+cycle*40+16+i*8;
   sc_pad out=sc_manual_catch_translate(&c,&s,&m,p,now,200);
   float x=vx[i],y=vy[i];sc_mc_frame(&c,&s,&x,&y);
   assert(sc_abs(out.rx-sc_short(x))<=3 && sc_abs(out.ry-sc_short(y))<=3);
   assert(m.phase==2 && !m.requested);
  }
  /* The catch wins immediately at any point of a replay, then stays stopped. */
  p.buttons|=SC_RTHUMB;
  sc_pad out=sc_manual_catch_translate(&c,&s,&m,p,1201,200);
  assert(m.phase==3 && !m.replay_running && !out.rx && !out.ry && !(out.buttons&SC_RTHUMB));
  p=(sc_pad){0};out=sc_manual_catch_translate(&c,&s,&m,p,1280,200);
  assert(m.phase==3 && !out.rx && !out.ry);
  s.air=0;s.takeoff=0;sc_manual_catch_translate(&c,&s,&m,p,1300,100);
  assert(!m.phase && m.replay_count<=1 && !m.replay_running);
  cases++;
 }
 /* Capture is bounded and expires; a long pause cannot replay an old trick. */
 sc_manual_catch m={0};for(unsigned i=0;i<100;i++)sc_mc_replay_record(&m,.6f,.8f,1000+i*8);
 assert(m.replay_count==32);sc_mc_replay_record(&m,-1,0,3000);assert(m.replay_count==1);
 /* Disabled mode preserves the established constant-endpoint behavior. */
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.manual_catch=1;
 sc_state s={0};s.air=1;s.takeoff=2;s.d_took_off=1;s.game_crouch=-1;
 m=(sc_manual_catch){0};m.phase=2;m.was_air=1;m.x=.6f;m.y=.8f;
 sc_pad out=sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},4000,200);
 assert(out.rx==sc_short(.6f) && out.ry==sc_short(.8f) && !m.replay_running);
 c.manual_catch_replay=1;c.manual_catch=0;
 sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},4010,200);assert(!m.phase && !m.replay_running);
 printf("PASS: %d replay paths preserve direction across stance/layout, repeat until click, stop immediately on catch/landing/disable, bound and expire captured input, and leave the default hold unchanged\n",cases);
}
