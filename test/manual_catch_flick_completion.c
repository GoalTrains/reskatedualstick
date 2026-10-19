#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/manual_catch.h"
int main(void){int cases=0;
 for(int g=0;g<2;g++)for(int sw=0;sw<2;sw++)for(int nollie=0;nollie<2;nollie++)for(int side=-1;side<=1;side+=2){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.manual_catch=c.front_back=1;
  c.goofy=g;c.switch_flip=1;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;
  sc_state s={0},normal={0};s.game_crouch=normal.game_crouch=-1;s.switched=normal.switched=sw;
  sc_manual_catch m={0};sc_pad p={0},out,last={0};
  sc_manual_catch_translate(&c,&s,&m,p,1000,100);sc_translate(&c,&normal,p,1000);
  if(nollie)p.ly=32767;else p.ry=-32768;
  sc_manual_catch_translate(&c,&s,&m,p,1100,100);sc_translate(&c,&normal,p,1100);
  sc_manual_catch_translate(&c,&s,&m,p,1200,100);sc_translate(&c,&normal,p,1200);
  if(nollie){p.rx=sc_short(side*.3f);p.ry=sc_short(-.95f);}else{p.lx=sc_short(side*.3f);p.ly=sc_short(.95f);}
  sc_manual_catch_translate(&c,&s,&m,p,1300,100);sc_translate(&c,&normal,p,1300);
  out=sc_manual_catch_translate(&c,&s,&m,p,1308,103);sc_translate(&c,&normal,p,1308);
  assert(m.phase==2 && m.candidate_kind==1);
  short initial_x=out.rx,initial_y=out.ry;
  for(int i=0;i<5;i++){
   float x=side*(.4f+i*.1f),y=(nollie?-1:1)*(.9f-i*.075f);
   if(nollie){p.rx=sc_short(x);p.ry=sc_short(y);}else{p.lx=sc_short(x);p.ly=sc_short(y);}
   last=sc_translate(&c,&normal,p,1316+i*8);out=sc_manual_catch_translate(&c,&s,&m,p,1316+i*8,103);
   float ex=sc_axis(last.rx),ey=sc_axis(last.ry),mag=sc_sqrt(ex*ex+ey*ey);
   assert(mag>=.85f);
   assert(sc_abs(out.rx-sc_short(ex/mag))<=3 && sc_abs(out.ry-sc_short(ey/mag))<=3);
  }
  assert(out.rx!=initial_x || out.ry!=initial_y);last=out;
  out=sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1360,103);assert(out.rx==last.rx && out.ry==last.ry);
  s.air=1;s.takeoff=2;out=sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1380,200);
  assert(out.rx==last.rx && out.ry==last.ry && m.phase==2);
  p=(sc_pad){0};p.buttons=SC_LTHUMB;
  out=sc_manual_catch_translate(&c,&s,&m,p,1400,200);assert(m.phase==3 && !out.rx && !out.ry);
  cases++;
 }
 printf("PASS: %d Session flicks finish their strong translated gesture through pop instead of freezing early, retain the final endpoint through release/takeoff, then release on catch\n",cases);
}
