#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void){
 for(int side=-1;side<=1;side+=2)for(int sw=0;sw<2;sw++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.xl_layout=c.goofy=c.one_foot_flips=c.pop_on_release=c.split_grinds=1;
  c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.windup_deg=60;c.manual_max=.85f;c.grind_sensitivity=1.2f;
  sc_state s={0};s.grind=1;s.game_crouch=-1;s.switched=sw;s.game_switch_known=1;s.game_switch=sw;
  sc_pad p={0};sc_translate(&c,&s,p,1000);
  /* A quick start, a 60ms pause, then a slower arc reproduces the
   * recorded routing dropout. Once recognised it must remain continuous. */
  const float angles[]={0,4,8,12,16,16,17,18,20,23,26,30,34,38,45,60,80};
  const int times[]={0,4,8,12,16,76,80,84,88,92,96,100,104,108,112,120,130};
  int active=0,confirmed=0;
  for(unsigned i=0;i<sizeof angles/sizeof angles[0];i++){
   float x,y;sc_vec_at(1,angles[i]*side,1,&x,&y);if(sw)y=-y;
   if(!sw){p.lx=sc_short(x);p.ly=sc_short(y);}else{p.rx=sc_short(x);p.ry=sc_short(y);}
   sc_pad o=sc_translate(&c,&s,p,1100+times[i]);
   if(s.rail_scoop_candidate==2||s.rail_scoop_exit==2)active=1;
   if(active){assert(s.rail_scoop_candidate==2||s.rail_scoop_exit==2);assert(o.rx*(long)sc_short(x)>0);}
   if(s.rail_scoop_exit==2)confirmed=1;
  }
  assert(active&&confirmed);
  p=(sc_pad){0};sc_translate(&c,&s,p,1400);assert(!s.rail_scoop_candidate);
 }
 puts("PASS: quick rail scoops keep their output direction through pauses and speed changes, both directions and switch states");
 return 0;
}
