#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_config config(void) {
 sc_config c={0};c.enabled=1;c.pure=1;c.air_controls=1;c.powerslide=1;
 c.ps_delay_ms=120;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.steer_scale=1;c.steer_curve=1;
 return c;
}
static sc_pad inward(float v){sc_pad p={0};p.lx=sc_short(v);p.rx=sc_short(-v);return p;}
int main(void) {
 sc_config c=config();sc_state s={0};sc_pad p=inward(1);
 sc_translate(&c,&s,p,1000);sc_translate(&c,&s,p,1150);assert(s.ps_side);
 s.grind=1;p=(sc_pad){0};sc_pad o=sc_translate(&c,&s,p,1160);
 assert(!s.ps_side); /* Old build stays latched, never reaches grind handling. */
 p.ry=sc_short(-1);o=sc_translate(&c,&s,p,1170);assert(s.grind_pop && o.ry< -30000);
 p.lx=sc_short(-.7f);p.ly=sc_short(.7f);o=sc_translate(&c,&s,p,1260);assert(o.rx< -18000 && o.ry>18000);
 s=(sc_state){0};p=inward(1);o=sc_translate(&c,&s,p,2000);assert(o.lx>20000 && o.ly< -20000); /* No delay, even legacy config. */
 s.air=s.takeoff=1;p=(sc_pad){0};sc_translate(&c,&s,p,2010);assert(!s.ps_side);
 s=(sc_state){0};p=(sc_pad){0};p.lx=sc_short(.8f);sc_translate(&c,&s,p,3000);
 p.rx=sc_short(-.8f);o=sc_translate(&c,&s,p,3140);assert(s.ps_side && o.ly< -20000); /* thumbs needn't cross threshold together */
 s=(sc_state){0};p=(sc_pad){0};p.ry=sc_short(-1);sc_translate(&c,&s,p,4000);
 p.lx=sc_short(1);p.rx=sc_short(-1);p.ry=0;sc_translate(&c,&s,p,4140);assert(!s.ps_side); /* real crouch then flick stays a trick */

 s=(sc_state){0};p=inward(.55f);o=sc_translate(&c,&s,p,5000);assert(s.ps_side && o.ly< -20000);
 p=inward(.42f);o=sc_translate(&c,&s,p,5010);assert(s.ps_side); /* Hold hysteresis. */
 p=(sc_pad){0};sc_translate(&c,&s,p,5020);assert(!s.ps_side);
 s=(sc_state){0};p=inward(1);p.ly=p.ry=sc_short(-.3f);o=sc_translate(&c,&s,p,5100);short down=o.ly;
 p.ly=p.ry=sc_short(.3f);o=sc_translate(&c,&s,p,5110);assert(o.ly>down && o.ly<0);
 for(int goofy=0;goofy<2;goofy++)for(int layout=0;layout<2;layout++)for(int mode=0;mode<4;mode++)for(int sw=0;sw<2;sw++)for(int release=0;release<2;release++) {
  c=config();c.goofy=goofy;c.front_back=layout;c.switch_flip=mode;c.pop_on_release=release;
  s=(sc_state){0};s.switched=sw;p=inward(1);sc_translate(&c,&s,p,6000);assert(s.ps_side);
  s.grind=1;p=(sc_pad){0};sc_translate(&c,&s,p,6010);assert(!s.ps_side);
  int lf=layout?1:!goofy;if(mode==3 && sw && !layout)lf=!lf;
  if(lf)p.ry=sc_short(-.78f);else p.ly=sc_short(-.78f);
  o=sc_translate(&c,&s,p,6030);assert(s.grind_pop && sc_abs(sc_axis(o.ry))>.65f);
  if(lf){p.lx=sc_short(.7f);p.ly=sc_short(.7f);}else{p.rx=sc_short(.7f);p.ry=sc_short(.7f);}
  o=sc_translate(&c,&s,p,6130);assert(sc_abs(sc_axis(o.rx))>.4f && sc_abs(sc_axis(o.ry))>.4f);
 }

 for(int goofy=0;goofy<2;goofy++)for(int layout=0;layout<2;layout++)for(int mode=0;mode<4;mode++)for(int sw=0;sw<2;sw++)for(int nollie=0;nollie<2;nollie++)for(int release=0;release<2;release++) {
  c=config();c.goofy=goofy;c.front_back=layout;c.switch_flip=mode;
  c.pop_on_release=release;
  s=(sc_state){0};s.switched=sw;int lf=layout?1:!goofy;if(mode==3 && sw && !layout)lf=!lf;
  p=(sc_pad){0};int pop_left=nollie?lf:!lf;
  if(pop_left)p.ly=sc_short(nollie?.95f:-.95f);else p.ry=sc_short(nollie?.95f:-.95f);
  sc_translate(&c,&s,p,7000);s.grind=1;
  sc_translate(&c,&s,p,7010);assert(!s.grind_pop);sc_translate(&c,&s,p,7100);assert(!s.grind_pop);
  if(!pop_left){p.lx=sc_short(.7f);p.ly=sc_short(nollie?-.7f:.7f);}else{p.rx=sc_short(.7f);p.ry=sc_short(nollie?-.7f:.7f);}
  o=sc_translate(&c,&s,p,7120);assert(s.grind_pop && sc_abs(sc_axis(o.rx))>.4f && sc_abs(sc_axis(o.ry))>.4f);
 }
 puts("PASS: held stall pop plus fresh opposite-foot flick; ollie and nollie, both stances/layouts and switch modes; held board position alone stays an adjustment");
 puts("PASS: both stances, layouts, switch modes and pop styles; softer slide activation, hold hysteresis, continuous angle, softer fresh stall pop");
 puts("PASS: powerslide-to-stall latch cleanup and flip-out; zero-delay activation despite legacy setting; release in air; asynchronous sideways thumbs; crouch/flick protected");
}
