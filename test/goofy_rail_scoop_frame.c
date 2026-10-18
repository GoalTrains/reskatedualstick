#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void){
 for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2)for(int split=0;split<2;split++)for(int native_mirror=0;native_mirror<2;native_mirror++)for(int custom_mirror=0;custom_mirror<2;custom_mirror++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.xl_layout=c.goofy=c.one_foot_flips=1;
  c.flip_x=custom_mirror;c.switch_flip=3;c.pop_on_release=1;c.split_grinds=split;c.deadzone=.15f;c.active=.3f;c.windup_deg=60;c.manual_max=.85f;c.grind_sensitivity=1.2f;
  sc_state ground={0};ground.game_crouch=-1;ground.switched=sw;ground.game_switch_known=1;ground.game_switch=sw;
  sc_state s={0};s.grind=1;s.game_crouch=-1;s.switched=sw;s.game_switch_known=1;s.game_switch=sw;
  s.game_mirror_known=ground.game_mirror_known=1;s.game_mirror=ground.game_mirror=native_mirror;
  sc_pad p={0};sc_translate(&c,&s,p,1000);sc_translate(&c,&ground,p,1000);int back_left=!sw;sc_pad before={0},after={0};
  for(int i=0;i<6;i++){
   float x,y;sc_vec_at(1,(float)(i*15*side),1,&x,&y);if(sw)y=-y;
   if(back_left){p.lx=sc_short(x);p.ly=sc_short(y);}else{p.rx=sc_short(x);p.ry=sc_short(y);}
   sc_pad out=sc_translate(&c,&s,p,1100+i*20), reference=sc_translate(&c,&ground,p,1100+i*20);
   if(i>=1){assert(out.rx*(long)reference.rx>0);assert(sc_abs(sc_wrap(sc_angle((float)out.rx,(float)out.ry)-sc_angle((float)reference.rx,(float)reference.ry)))<.1f);}
   if(i==1)before=out;
   if(i>=3){assert(s.rail_scoop_exit==2);assert(before.rx*(long)out.rx>0);after=out;}
  }
  s.grind=0;s.air=1;s.takeoff=2;sc_pad airborne=sc_translate(&c,&s,p,1240);
  (void)airborne;
  p=(sc_pad){0};sc_translate(&c,&s,p,1300);
 }
 /* A foot already held on contact must remain a board adjustment until
  * it has been released and deliberately set up again. */
 {sc_config c={0};c.enabled=c.pure=c.air_controls=c.xl_layout=c.goofy=c.one_foot_flips=c.pop_on_release=c.split_grinds=1;c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.windup_deg=60;c.manual_max=.85f;sc_state s={0};s.game_crouch=-1;s.grind=1;
  for(int i=0;i<6;i++){sc_pad p={0};float x,y;sc_vec_at(1,i*15.f,1,&x,&y);p.lx=sc_short(x);p.ly=sc_short(y);sc_translate(&c,&s,p,2000+i*20);assert(!s.rail_scoop_candidate && !s.rail_scoop_exit && !s.grind_pop);}}
 puts("PASS: goofy XL back-foot scoop matches ground input from initial quick motion through exit recognition, both directions/switch states and grind options");
 return 0;
}
