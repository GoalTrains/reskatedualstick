#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
/* Recorded regular front/back gesture: sideways start, scoop through down,
 * then opposite-foot diagonal flick. Previously stolen by slide detection. */
int main(void) {
 for(int goofy=0;goofy<2;goofy++)for(int nollie=0;nollie<2;nollie++)for(int sign=-1;sign<=1;sign+=2) {
  sc_config c={0};c.enabled=c.pure=c.powerslide=c.air_controls=1;
  c.front_back=1;c.goofy=goofy;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;
  c.windup_deg=60; /* Live default: this recorded 61-degree start is a wind-up. */
  c.steer_scale=c.steer_curve=1;sc_state s={0};s.takeoff=0;s.game_known=1;
  const float setup[][2]={{.90f,-.50f},{.80f,-.73f},{0,-1},{-.90f,-.54f},{-.91f,0}};
  const double times[]={1000,1048,1100,1148,1200};
  for(int i=0;i<5;i++) {
   sc_pad in={0};float sx=sign*setup[i][0],sy=setup[i][1];
   if(nollie){in.lx=sc_short(sx);in.ly=sc_short(-sy);}else{in.rx=sc_short(sx);in.ry=sc_short(sy);}
   if(i>=3){if(nollie){in.rx=sc_short(sign*.85f);in.ry=sc_short(-.5f);}else{in.lx=sc_short(sign*.85f);in.ly=sc_short(.5f);}}
   sc_pad out=sc_translate(&c,&s,in,times[i]);
   assert(!s.ps_side);
   if(i>=3){assert(!s.d_front_ignored&&!s.d_back_ignored);assert(out.rx!=0&&out.ry!=0);assert(out.lx==0&&out.ly==0);}
  }
  /* Release both feet and then make a genuine opposing-stick slide. */
  sc_pad neutral={0};sc_translate(&c,&s,neutral,1500);
  neutral.lx=30000;neutral.rx=-30000;
  sc_pad slide=sc_translate(&c,&s,neutral,1550);assert(s.ps_side&&slide.ly<0);
 }
 puts("PASS: recorded sideways-start scoops retain opposite-foot flick, both pop ends/directions and stances; fresh powerslides still activate after release");
}
