#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_config cfg(void){sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.steer_scale=c.steer_curve=1;c.prewind=1;c.prewind_ms=60;c.prewind_percent=70;c.revert_ms=300;c.revert_input=1;c.revert_sweep_ms=300;c.revert_shape=5;return c;}
int main(void){
 for(int goofy=0;goofy<2;goofy++)for(int side=0;side<2;side++){
  sc_config c=cfg();c.goofy=goofy;c.front_back=1;c.prewind_steer=1;
  sc_state s={0};sc_pad p={0};p.ry=sc_short(-1);sc_translate(&c,&s,p,1000);
  if(side)p.rt=130;else p.lt=130;
  sc_pad o=sc_translate(&c,&s,p,1100);assert(!s.pw_side && (side?o.lx>15000:o.lx< -15000) && o.ly==0);
  if(side)p.rt=220;else p.lt=220;
  sc_translate(&c,&s,p,1200);assert(s.pw_side==(side?1:-1));
  o=sc_translate(&c,&s,p,1300);assert(s.pw_side && (side?o.lx>30000:o.lx< -30000));
  if(side)p.rt=130;else p.lt=130;
  o=sc_translate(&c,&s,p,1400);assert(!s.pw_side && (side?o.lx>15000:o.lx< -15000));
  if(side)p.rt=220;else p.lt=220;
  sc_translate(&c,&s,p,1500);assert(s.pw_side);
  s.air=s.takeoff=1;if(side)p.rt=130;else p.lt=130;
  sc_translate(&c,&s,p,1600);assert(s.pw_side); /* Lower pull continues air spin. */
  p.lt=p.rt=0;sc_translate(&c,&s,p,1700);assert(!s.pw_side);
  s=(sc_state){0};c.prewind_steer=0;p=(sc_pad){0};p.ry=sc_short(-1);sc_translate(&c,&s,p,2000);
  if(side)p.rt=130;else p.lt=130;sc_translate(&c,&s,p,2100);assert(s.pw_side); /* Historical default. */
 }
 for(int goofy=0;goofy<2;goofy++)for(int side=0;side<2;side++)for(int window=150;window<=450;window+=300){
  sc_config c=cfg();c.goofy=goofy;c.revert_ms=window;sc_state s={0};sc_pad p={0};
  if(side)p.rt=255;else p.lt=255;sc_translate(&c,&s,p,1000);
  p.lt=p.rt=0;sc_translate(&c,&s,p,1050);
  if(side)p.rt=255;else p.lt=255;sc_translate(&c,&s,p,1350);assert((s.rv_start>0)==(window==450));
  s=(sc_state){0};c.revert_input=2;p=(sc_pad){0};p.lt=p.rt=255;sc_translate(&c,&s,p,2000);assert(s.rv_start>0); /* Both-trigger mode independent. */
 }
 puts("PASS: light crouched steering, strong pre-wind, cancellation/restart and air spin; old default; strict/forgiving tap windows and both-trigger mode, regular/goofy both directions");
}
