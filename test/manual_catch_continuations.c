#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/manual_catch.h"
int main(void){int paths=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.manual_catch=1;c.goofy=g;
  c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=c.front_back?1:3;
  c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;
  sc_state s={0};s.air=1;s.takeoff=2;s.d_took_off=1;s.game_crouch=-1;s.switched=sw;
  sc_manual_catch m={0};m.phase=2;m.x=.6f;m.y=-.8f;m.was_air=1;
  sc_pad out=sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1000,200);
  assert(m.held_recentred && m.phase==2 && out.rx && out.ry);
  sc_state normal=s;int emitted=0;
  for(int a=0;a<=360;a+=15){
   float x,y;sc_vec_at(0,a*(float)side,1,&x,&y);if(c.xl_layout&&sw)y=-y;
   sc_pad p={0};int left=c.front_back?1:(!c.goofy)!=sw;
   if(left){p.lx=sc_short(x);p.ly=sc_short(y);}else{p.rx=sc_short(x);p.ry=sc_short(y);}
   sc_pad expected=sc_translate(&c,&normal,p,1100+a*2);
   out=sc_manual_catch_translate(&c,&s,&m,p,1100+a*2,200);
   float ex=sc_axis(expected.rx),ey=sc_axis(expected.ry),mag=ex*ex+ey*ey;
   if(mag>=.85f*.85f){float scale=sc_sqrt(mag);
    assert(sc_abs((float)out.rx-sc_short(ex/scale))<=3 && sc_abs((float)out.ry-sc_short(ey/scale))<=3);emitted++;
   }
   assert(m.phase==2 && !m.requested && (out.rx||out.ry));
  }
  assert(emitted>5);
  out=sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1900,200);assert(m.phase==2 && (out.rx||out.ry));
  sc_pad p={0};p.buttons=SC_LTHUMB|SC_RTHUMB;
  out=sc_manual_catch_translate(&c,&s,&m,p,1920,200);assert(m.phase==3 && m.requested==3 && !out.rx && !out.ry);
  paths++;
 }
 printf("PASS: %d airborne continuation circles retain actual strong translated input after physical centring, never release on neutral, and keep the click-catch requirement\n",paths);
}
