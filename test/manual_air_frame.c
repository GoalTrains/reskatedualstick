#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void){
 int cases=0;
 for(int foot=0;foot<2;foot++)for(int initial=0;initial<2;initial++)for(int mode=1;mode<=2;mode++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.front_back=1;c.switch_flip=mode;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
  sc_state s={0};s.game_crouch=-1;s.game_switch_known=1;s.game_switch=s.switched=initial;
  sc_pad p={0};sc_translate(&c,&s,p,1000);s.air=s.takeoff=1;sc_translate(&c,&s,p,1016);
  if(foot)p.ry=-19969;else p.ly=19969;
  sc_pad held=sc_translate(&c,&s,p,1100);assert(sc_abs(sc_axis(held.ry))>.3f);
  int latch=s.sw_lat;
  /* The native fakie edge arrives in the last ~100 ms of the second jump,
   * while the landing foot remains continuously held at partial travel. */
  s.game_switch=s.switched=!initial;
  sc_pad turned=sc_translate(&c,&s,p,1400);assert((long)turned.ry*held.ry<0&&s.sw_lat!=latch);
  s.air=s.takeoff=0;sc_pad landed=sc_translate(&c,&s,p,1500);
  assert((long)landed.ry*turned.ry>0&&s.d_land_hold);
  sc_pad neutral={0};sc_translate(&c,&s,neutral,1600);assert(s.sw_lat!=latch);
  cases++;
 }
 /* A full flip endpoint still follows the existing native airborne frame. */
 {
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.front_back=1;c.switch_flip=1;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
  sc_state s={0};s.game_crouch=-1;s.game_switch_known=1;sc_pad p={0};sc_translate(&c,&s,p,2000);s.air=s.takeoff=1;sc_translate(&c,&s,p,2016);
  p.ly=32767;sc_pad a=sc_translate(&c,&s,p,2100);s.game_switch=s.switched=1;sc_pad b=sc_translate(&c,&s,p,2200);assert(s.sw_lat==1);assert(a.ry*b.ry<0);
 }
 /* Opposing two-foot pressure can also produce a partial vertical output,
  * but must retain two-foot positioning rather than acquire a manual latch. */
 {
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.front_back=1;c.switch_flip=1;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
  sc_state s={0};s.game_crouch=-1;s.game_switch_known=1;sc_pad p={0};sc_translate(&c,&s,p,3000);s.air=s.takeoff=1;sc_translate(&c,&s,p,3016);
  p.ly=24575;p.ry=-13107;sc_translate(&c,&s,p,3100);s.game_switch=s.switched=1;sc_translate(&c,&s,p,3200);assert(s.sw_lat==1);
 }
 printf("PASS: %d partial manual leans follow native airborne frame changes, then retain that frame through landing; full and two-foot inputs preserve tracking\n",cases);return 0;
}
