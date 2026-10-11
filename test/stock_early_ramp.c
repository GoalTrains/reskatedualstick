#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/translate.h"
static sc_config config(void){sc_config c={0};c.enabled=c.pure=c.stock_grab_air=c.stock_transition_ground=1;c.active=.3f;c.deadzone=.15f;c.steer_scale=c.steer_curve=1;c.powerslide=c.body_flips=c.prewind=1;return c;}
static void climb(sc_config*c,sc_state*s,double start,int sign){
 sc_pad p={0};s->takeoff=0;s->stock_ramp_valid=1;
 for(int k=0;k<=5;k++){s->stock_ramp_vertical=sign*(.05f+.042f*k);sc_translate(c,s,p,start+30*k);}
 assert(s->stock_air_active && s->stock_early_active && s->stock_ramp_eligible);
}
int main(void){
 for(int goofy=0;goofy<2;goofy++)for(int layout=0;layout<2;layout++)for(int sw=0;sw<2;sw++)for(int sign=-1;sign<=1;sign+=2){
  sc_config c=config();c.goofy=goofy;c.front_back=layout;c.switch_flip=3;sc_state s={0};s.switched=sw;
  climb(&c,&s,1000,sign);sc_pad p={0};p.lx=22000;p.ry=-28000;sc_pad o=sc_translate(&c,&s,p,1170);
  assert(!memcmp(&p,&o,sizeof p)); /* stock pop/prewind sticks on ground, no stance remap */
  p.lt=123;p.rt=81;o=sc_translate(&c,&s,p,1200);assert(!memcmp(&p,&o,sizeof p));
  s.takeoff=s.air=1;p.lt=p.rt=0;o=sc_translate(&c,&s,p,1250);assert(s.stock_air_active && !memcmp(&p,&o,sizeof p)); /* no grab required after early entry */
  s.takeoff=s.air=0;s.stock_ramp_vertical=0;o=sc_translate(&c,&s,p,1400);assert(!s.stock_air_active && !s.stock_early_active && !o.rx && !o.ry);
  p=(sc_pad){0};sc_translate(&c,&s,p,1420);p.ry=-32000;sc_translate(&c,&s,p,1440);assert(!s.stock_air_active);
  s=(sc_state){0};climb(&c,&s,2000,sign);p=(sc_pad){0};p.ry=-28000;s.stock_ramp_vertical=0;
  sc_translate(&c,&s,p,2200);assert(s.stock_air_active);o=sc_translate(&c,&s,p,2800);assert(!s.stock_air_active && !o.rx && !o.ry); /* cancelled approach */
  s=(sc_state){0};climb(&c,&s,3000,sign);p=(sc_pad){0};p.buttons=SC_RTHUMB;
  o=sc_translate(&c,&s,p,3170);assert(s.pump_r3 && !(o.buttons&SC_RTHUMB));o=sc_translate(&c,&s,p,3220);assert(o.rt==255);
  p.buttons=0;sc_translate(&c,&s,p,3240);o=sc_translate(&c,&s,p,3300);assert(!o.rt && !s.pump_r3);
  s.takeoff=s.air=1;p.buttons=SC_RTHUMB;o=sc_translate(&c,&s,p,3320);assert(!o.rt && !s.pump_r3); /* pump can't become an airborne grab */
  s=(sc_state){0};climb(&c,&s,4000,sign);c.stock_transition_ground=0;p=(sc_pad){0};o=sc_translate(&c,&s,p,4170);assert(!s.stock_air_active);
 }
 /* Early mode leaves unqualified flat-ground input unchanged. */
 sc_config a=config(),b=a;b.stock_transition_ground=0;sc_state x={0},y={0};x.takeoff=y.takeoff=0;x.stock_ramp_valid=y.stock_ramp_valid=1;
 sc_pad p={0};p.ry=-32000;p.lt=150;sc_pad o=sc_translate(&a,&x,p,5000),q=sc_translate(&b,&y,p,5000);assert(!memcmp(&o,&q,sizeof o));
 puts("PASS: optional early ramp stock pop/prewind, both stances/layouts/directions, no-trigger air continuation, landing/cancelled approach cleanup, R3 pump preserved, flat ground unchanged, air-only option restored");
}
