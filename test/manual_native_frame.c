#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.front_back=1;c.switch_flip=1;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.steer_scale=c.steer_curve=1;
 int cases=0;
 for(int goofy=0;goofy<2;goofy++)for(int layout=0;layout<3;layout++)for(int mode=1;mode<=3;mode++)for(int foot=0;foot<2;foot++)for(int initial=0;initial<2;initial++)for(int delay=20;delay<=300;delay+=70){
  sc_config cfg=c;cfg.goofy=goofy;cfg.front_back=layout==0;cfg.xl_layout=layout==2;cfg.switch_flip=mode;
  sc_state s={0};s.game_crouch=-1;s.game_switch_known=1;s.game_switch=initial;s.switched=initial;
  sc_pad p={0};sc_translate(&cfg,&s,p,1000);s.air=s.takeoff=1;sc_translate(&cfg,&s,p,1016);
  int left=cfg.front_back?1:!goofy;if(foot)left=!left;
  if(left)p.ly=sc_short(foot?-.58f:.58f);else p.ry=sc_short(foot?-.58f:.58f);
  sc_translate(&cfg,&s,p,1100);s.air=s.takeoff=0;
  sc_pad old=sc_translate(&cfg,&s,p,1200);assert(s.d_land_hold&&sc_abs(sc_axis(old.ry))>.3f);
  unsigned seq=s.d_seq;int latch=s.sw_lat;
  /* Recorded regression: raw lean stays held while native flags reverse
   * 120-150 ms after touchdown. Reversing output here triggers a native pop,
   * even though the translator itself emits no new trick event. */
  s.game_switch=!initial;s.switched=!initial;
  sc_pad changed=sc_translate(&cfg,&s,p,1200+delay);
  assert(old.ry==changed.ry&&old.rx==changed.rx);
  assert(s.d_seq==seq&&s.d_land_hold&&s.sw_lat==latch);
  /* A later return or a brief flag disagreement cannot reverse the lean. */
  s.game_switch=initial;sc_pad returned=sc_translate(&cfg,&s,p,1600);
  assert(returned.ry==old.ry&&s.d_land_hold&&s.d_seq==seq);
  s.game_switch=!initial;returned=sc_translate(&cfg,&s,p,1700);assert(returned.ry==old.ry);
  /* Only physical release hands the next gesture to the new frame. */
  sc_pad neutral={0};sc_pad released=sc_translate(&cfg,&s,neutral,1800);assert(!released.ry&&!s.d_land_hold);
  sc_pad fresh=sc_translate(&cfg,&s,p,1900);
  sc_state ref={0};ref.game_crouch=-1;ref.game_switch_known=1;ref.game_switch=!initial;ref.switched=!initial;
  sc_translate(&cfg,&ref,neutral,1800);sc_pad expected=sc_translate(&cfg,&ref,p,1900);
  assert(fresh.ry==expected.ry&&fresh.rx==expected.rx);
  cases++;
 }
 /* Full held pop setups also retain their pre-existing latch. */
 {
  sc_state s={0};s.game_crouch=-1;s.game_switch_known=1;sc_pad p={0};
  sc_translate(&c,&s,p,3000);p.ry=-32768;sc_pad a=sc_translate(&c,&s,p,3100);
  s.game_switch=s.switched=1;sc_pad b=sc_translate(&c,&s,p,3220);assert(!s.d_land_hold&&a.ry==b.ry&&s.sw_lat==0);
 }
 printf("PASS: %d delayed 180 frame changes preserve held manual output until physical release\n",cases);return 0;
}
