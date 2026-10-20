#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void){
 int cases=0;
 for(int g=0;g<2;g++)for(int xl=0;xl<2;xl++)for(int foot=0;foot<2;foot++)for(int initial=0;initial<2;initial++)for(int release=0;release<2;release++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.goofy=g;c.xl_layout=xl;c.switch_flip=3;c.pop_on_release=release;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
  sc_state s={0};s.game_crouch=-1;s.switched=initial;s.game_switch_known=1;s.game_switch=initial;
  sc_pad p={0};sc_translate(&c,&s,p,1000);s.air=s.takeoff=1;sc_translate(&c,&s,p,1016);
  int left=s.feet_map_left;if(foot)left=!left;
  /* Reported shape: R(13,-68) / R(13,-76), or the front-foot equivalent. */
  if(left){p.lx=sc_short(.13f);p.ly=sc_short(foot?-.68f:.76f);}else{p.rx=sc_short(.13f);p.ry=sc_short(foot?-.68f:.76f);}
  sc_pad air=sc_translate(&c,&s,p,1100);int frame=s.feet_map_left;assert(sc_abs(sc_axis(air.ry))>.3f);
  /* Stance changes at the first ground poll, BEFORE d_land_hold is set. */
  s.air=s.takeoff=0;s.switched=!initial;s.game_crouch=0;
  sc_pad landed=sc_translate(&c,&s,p,1200);
  assert(s.feet_map_left==frame);assert(s.d_land_hold);
  assert((long)air.ry*landed.ry>0);
  unsigned seq=s.d_seq;
  s.game_switch=!initial;
  for(int i=1;i<20;i++){
   s.game_crouch=i<3?1:0;sc_pad held=sc_translate(&c,&s,p,1200+i*4);
   assert((long)landed.ry*held.ry>0);assert(s.d_seq==seq&&s.d_land_hold&&!s.rp_t);
  }
  p=(sc_pad){0};sc_pad neutral=sc_translate(&c,&s,p,1400);assert(!neutral.ry&&!s.d_land_hold);
  sc_translate(&c,&s,p,1404);assert(s.feet_map_left==!frame);
  cases++;
 }
 /* Full setups, lateral positioning, neutral, and two-foot gestures must
  * not be captured by the new sole-manual first-ground guard. */
 for(int shape=0;shape<4;shape++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
  sc_state s={0};s.game_crouch=-1;sc_pad p={0};sc_translate(&c,&s,p,1000);s.air=s.takeoff=1;sc_translate(&c,&s,p,1016);
  p.ry=sc_short(-.68f);sc_translate(&c,&s,p,1100);int frame=s.feet_map_left;
  p=(sc_pad){0};if(shape==0)p.ry=-32768;
  if(shape==1)p.rx=24000;
  if(shape==2){p.ly=22000;p.ry=-22000;}
  s.air=s.takeoff=0;s.switched=1;sc_translate(&c,&s,p,1200);
  assert(s.feet_map_left!=frame);
 }
 printf("PASS: %d first-ground stance changes preserve manual foot roles and output without a release pop\n",cases);return 0;
}
