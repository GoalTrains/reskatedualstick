#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_config config(int goofy,int layout,int sw){
 (void)sw;sc_config c={0};c.enabled=c.pure=c.air_controls=c.powerslide=1;
 c.goofy=goofy;c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=layout?3:1;
 c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.steer_scale=c.steer_curve=1;
 c.revert_input=3;c.revert_ms=300;c.revert_shape=5;c.revert_sweep_ms=300;return c;
}
static sc_pad pair(int inward,float degrees){
 sc_pad p={0};float a=(inward?90:270)+degrees;
 p.lx=sc_short(.95f*sc_sin_deg(a));p.ly=sc_short(-.95f*sc_sin_deg(90-a));
 p.rx=-p.lx;p.ry=-p.ly;return p;
}
int main(void){
 int count=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int inward=0;inward<2;inward++)for(int dir=-1;dir<=1;dir+=2){
  sc_config c=config(g,layout,sw);sc_state s={0};s.switched=sw;s.game_crouch=-1;
  sc_pad p=pair(inward,0);sc_pad slide=sc_translate(&c,&s,p,1000);
  assert(s.ps_side && s.slide_revert.armed && !s.rv_start && slide.ly<0);
  /* Merely holding the slide never reverts, even after a long hold. */
  for(int t=1020;t<=2000;t+=20)sc_translate(&c,&s,p,t);
  assert(!s.rv_start);
  for(int i=1;i<=2;i++){p=pair(inward,dir*i*12);sc_translate(&c,&s,p,2000+i*20);assert(!s.rv_start);}
  p=pair(inward,dir*36);sc_translate(&c,&s,p,2060);
  assert(s.rv_start==2060 && s.rv_side==-dir && s.slide_revert.blocked);
  double started=s.rv_start;
  for(int t=2080;t<2800;t+=20){sc_translate(&c,&s,p,t);assert(s.rv_start==started || !s.rv_start);}
  assert(!s.rv_start && s.slide_revert.blocked);
  p=(sc_pad){0};sc_translate(&c,&s,p,2820);assert(!s.slide_revert.blocked);
  p=pair(inward,0);sc_translate(&c,&s,p,2840);assert(s.slide_revert.armed);
  count++;
 }
 /* Both-stick fore/aft slide adjustment is not matching circular rotation. */
 for(int mode=0;mode<=3;mode++){
  sc_config c=config(0,0,0);c.revert_input=mode;sc_state s={0};s.game_crouch=-1;
  sc_pad p=pair(1,0);sc_translate(&c,&s,p,1000);
  p.ly=p.ry=15000;sc_translate(&c,&s,p,1020);assert(!s.rv_start);
  p=pair(1,36);sc_translate(&c,&s,p,1040);if(mode!=3)assert(!s.rv_start);
 }
 /* Trigger taps/holds cannot request a revert in stick mode. */
 {sc_config c=config(0,0,0);sc_state s={0};s.game_crouch=-1;sc_pad p={0};
  p.lt=p.rt=255;sc_translate(&c,&s,p,1000);assert(!s.rv_start);
  p.lt=p.rt=0;sc_translate(&c,&s,p,1030);p.rt=255;sc_translate(&c,&s,p,1070);assert(!s.rv_start);}
 /* Cancelled contexts cannot carry a partly completed rotation. */
 for(int context=0;context<6;context++){
  sc_config c=config(0,0,0);sc_state s={0};s.game_crouch=-1;
  sc_pad p=pair(1,0);sc_translate(&c,&s,p,1000);p=pair(1,20);sc_translate(&c,&s,p,1020);
  if(context==0)s.air=s.takeoff=1;if(context==1)s.grind=1;if(context==2)s.game_crouch=1;
  if(context==3)c.enabled=0;if(context==4)p.buttons=1;if(context==5)c.powerslide=0;
  p=pair(1,36);if(context==4)p.buttons=1;sc_translate(&c,&s,p,1040);
  assert(!s.rv_start && !s.slide_revert.armed);
 }
 /* Helper edge cases: jitter, reversed arcs, one-stick movement, stale polls,
  * snap jumps and slow rotations do not falsely fire. */
 {sc_slide_revert s={0};sc_sr_begin(&s,90,270,1000);
  assert(!sc_sr_step(&s,1,1,1,1,1,92,268,1020));
  assert(!sc_sr_step(&s,1,1,1,1,1,125,270,1040));
  assert(!sc_sr_step(&s,1,1,1,1,1,90,270,1060));
  assert(!sc_sr_step(&s,1,1,1,1,1,91,271,1200));assert(s.blocked);
  sc_sr_cancel(&s);sc_sr_begin(&s,90,270,1000);
  assert(!sc_sr_step(&s,1,1,1,1,1,170,350,1020));assert(s.blocked);
  sc_sr_cancel(&s);sc_sr_begin(&s,90,270,1000);
  for(int i=1;i<=9;i++)assert(!sc_sr_step(&s,1,1,1,1,1,90+i*3,270+i*3,1000+i*90));
  assert(!sc_sr_step(&s,1,1,1,1,1,120,300,1900));assert(s.blocked);
 }
 printf("PASS: %d slide-rotate combinations, both directions and slide types; held/re-arm, trigger isolation, context cancellation, slide-angle adjustment, jitter, slow arcs and jumps\n",count);
}
