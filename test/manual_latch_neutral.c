#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void){
 int cases=0;
 for(int foot=0;foot<2;foot++)for(int initial=0;initial<2;initial++)for(int mode=1;mode<=2;mode++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.front_back=1;c.switch_flip=mode;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
  sc_state s={0};s.game_crouch=-1;s.game_switch_known=1;s.game_switch=s.switched=initial;
  sc_pad p={0};sc_translate(&c,&s,p,1000);s.air=s.takeoff=1;sc_translate(&c,&s,p,1016);
  if(foot)p.ry=-20000;else p.ly=20000;
  sc_translate(&c,&s,p,1100);s.air=s.takeoff=0;
  sc_pad landed=sc_translate(&c,&s,p,1200);assert(s.d_land_hold);
  int latch=s.sw_lat;s.game_switch=s.switched=!initial;
  /* Exact local trace: front 13311 -> 13055 (back -13313 -> -13057),
   * crossing 30% TRANSLATED travel with only 256 raw units of motion. */
  const int front[]={13311,13055,13311,13055,12000,10000,7000,5000};
  for(unsigned i=0;i<sizeof(front)/sizeof(front[0]);i++){
   p=(sc_pad){0};if(foot)p.ry=-(front[i]+2);else p.ly=front[i];
   sc_pad o=sc_translate(&c,&s,p,1220+i*4);
   assert(s.sw_lat==latch);assert((long)landed.ry*o.ry>0);
   cases++;
  }
  /* Physical centring enters the radial deadzone. The pending frame is
   * applied there, at zero output, so the next gesture uses that frame. */
  p=(sc_pad){0};if(foot)p.ry=-4000;else p.ly=4000;
  sc_pad released=sc_translate(&c,&s,p,1400);assert(!released.rx&&!released.ry);
  assert(s.sw_lat!=latch);
  p=(sc_pad){0};if(foot)p.ry=-20000;else p.ly=20000;
  sc_pad fresh=sc_translate(&c,&s,p,1500);assert((long)landed.ry*fresh.ry<0);
 }
 puts("PASS: 64 manual-strength dips preserve the held frame through 30 percent; true deadzone release applies pending stance at zero output");return 0;
}
