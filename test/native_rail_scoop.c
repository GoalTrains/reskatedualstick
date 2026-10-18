#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void){
 for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2)for(int split=0;split<2;split++)for(int mirror=0;mirror<2;mirror++)for(int native=0;native<2;native++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.xl_layout=c.goofy=c.one_foot_flips=c.pop_on_release=1;c.split_grinds=split;c.flip_x=mirror;c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.windup_deg=60;c.manual_max=.85f;c.grind_sensitivity=1.2f;
  sc_state s={0};s.grind=1;s.game_crouch=-1;s.switched=sw;s.game_switch_known=1;s.game_switch=sw;s.game_mirror_known=1;s.game_mirror=native;
  sc_pad p={0};sc_translate(&c,&s,p,1000);
  for(int i=0;i<=12;i++){
   float x,y;sc_vec_at(1,i*15.f*side,1.12f,&x,&y);p=(sc_pad){0};if(sw){p.rx=sc_short(x);p.ry=sc_short(-y);}else{p.lx=sc_short(x);p.ly=sc_short(y);}
   sc_pad o=sc_translate(&c,&s,p,1100+i*16);
   if(i>=3){int px=sw?p.rx:p.lx,py=sw?p.ry:p.ly;int expected=mirror!=native?-px:px;
    assert(s.rail_scoop_exit==2);assert(sc_abs((float)o.rx-expected)<=1);assert(o.ry==py);}
  }
  /* Preserve a gradual low-strength release, below the position deadzone. */
  const int release[]={7000,5000,3000,1500};
  for(int i=0;i<4;i++){p=(sc_pad){0};if(sw)p.ry=-release[i];else p.ly=release[i];sc_pad o=sc_translate(&c,&s,p,1320+i*4);assert(o.ry==(sw?p.ry:p.ly));assert(!s.rp_t);}
  p=(sc_pad){0};sc_pad o=sc_translate(&c,&s,p,1340);assert(!s.rail_scoop_exit);assert(!o.rx&&!o.ry);
 }
 puts("PASS: native goofy XL rear-foot rail scoops retain square-axis strength, full half-circle, direction and low-strength release; switch/custom/native mirrors and both grind options");return 0;
}
