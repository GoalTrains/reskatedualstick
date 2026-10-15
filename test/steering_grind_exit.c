#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_config cfg(void){sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.steer_scale=c.steer_curve=1;c.bump_ms=250;return c;}
int main(void){
 for(int g=0;g<2;g++)for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2){
  sc_config c=cfg();c.stick_steer=2;c.goofy=g;sc_state s={0};s.switched=sw;sc_pad p={0};
  p.lx=sc_short(.20f*side);sc_pad out=sc_translate(&c,&s,p,1000);
  assert(s.st_latch && out.lx*side>6000 && out.rx==0);
  p.lx=sc_short(.12f*side);out=sc_translate(&c,&s,p,1010);assert(s.st_latch && out.lx*side>3500);
  p.lx=sc_short(.05f*side);sc_translate(&c,&s,p,1020);assert(!s.st_latch);
  s=(sc_state){0};s.switched=sw;p=(sc_pad){0};p.lt=51;out=sc_translate(&c,&s,p,1100);assert(out.lx< -6500 && out.lx> -6600);
 }
 for(int dir=-1;dir<=1;dir+=2){
  sc_config c=cfg();sc_state s={0};s.grind=1;sc_pad p={0};p.ly=dir*30000;p.buttons=SC_LTHUMB;
  /* Recorded repeated clicks while the previous request is still full UP. */
  s.bump_output_time=990;s.bump_output_y=(float)dir;
  sc_pad out=sc_translate(&c,&s,p,1000);assert(out.lx==0 && out.ly==0 && out.rx==0 && out.ry==0);
  out=sc_translate(&c,&s,p,1010);assert(out.ly==0);
  out=sc_translate(&c,&s,p,1021);assert(out.ly*dir>32000 && out.rx==0 && out.ry==0);
  assert(!s.fired);p.buttons=0;sc_translate(&c,&s,p,1030);p.buttons=SC_LTHUMB;
  out=sc_translate(&c,&s,p,1040);assert(out.ly==0);out=sc_translate(&c,&s,p,1061);assert(out.ly*dir>32000);
  s=(sc_state){0};s.grind=1;out=sc_translate(&c,&s,p,2000);assert(out.ly*dir>32000 && out.rx==0 && out.ry==0); /* No gap for a fresh direction. */
 }
 puts("PASS: lower steering onset/hysteresis both profiles/directions, linear small trigger input, fresh immediate exits, held-direction re-arm, board-lock release, repeated clicks without synthetic pop");
}