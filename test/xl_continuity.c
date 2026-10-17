#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/xl_stance_agreement.h"
static sc_config config(int g){sc_config c={0};c.enabled=c.pure=c.xl_layout=c.air_controls=1;c.goofy=g;c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.grind_sensitivity=1.2f;return c;}
int main(void){
 int cases=0;
 for(int g=0;g<2;g++)for(int sw=0;sw<2;sw++)for(int shape=0;shape<4;shape++)for(int rail=0;rail<2;rail++){
  sc_config c=config(g);sc_state s={0};s.switched=sw;s.game_switch_known=1;s.game_switch=sw;s.game_crouch=-1;
  sc_pad p={0};sc_translate(&c,&s,p,100);int left=s.feet_map_left;
  s.takeoff=1;sc_translate(&c,&s,p,110);s.air=1;s.takeoff=2;sc_translate(&c,&s,p,250);
  if(shape==0)p.rx=-32000;
  if(shape==1)p.lx=32767,p.rx=-32768;
  if(shape==2)p.lx=-32768,p.rx=32767;
  if(shape==3)p.lx=22000,p.ly=10000,p.rx=-23000,p.ry=-5000;
  sc_pad before=sc_translate(&c,&s,p,300);assert(sc_abs((float)before.rx)>10000);
  s.switched=!sw;s.game_switch=!sw;
  sc_pad after=sc_translate(&c,&s,p,304);
  assert(s.feet_map_left==left);assert(before.rx==after.rx); /* no leg reversal */
  if(rail){s.air=0;s.takeoff=0;s.grind=1;after=sc_translate(&c,&s,p,320);assert(before.rx* (long)after.rx>0);}
  s.switched=sw;s.game_switch=sw;after=sc_translate(&c,&s,p,340);assert(before.rx*(long)after.rx>0);
  /* Touchdown centres before the next setup. New jobs must then follow stance. */
  s.air=s.takeoff=s.grind=0;s.switched=!sw;s.game_switch=!sw;p=(sc_pad){0};sc_translate(&c,&s,p,500);
  sc_translate(&c,&s,p,510);assert(!s.board_frame_valid);assert(s.feet_map_left!=left);
  cases++;
 }
 /* Literal adjacent polls from the supplied goofy XL trace. The game
  * received opposite X despite unchanged sideways intent. */
 const int recorded[][14]={{1,0,1727,525,-31448,-350,32765,0,0,953,332,-32092,-394,-32766},{0,0,32767,3870,-32768,1758,-32756,1,1,32767,3912,-32768,1758,32754},{0,0,32178,875,-30525,2109,-32763,1,1,32178,875,-30694,2111,32762}};
 for(unsigned i=0;i<sizeof recorded/sizeof recorded[0];i++){
  const int*v=recorded[i];sc_config c=config(1);sc_state t={0};sc_pad p={0};
  t.switched=v[0];t.game_switch_known=1;t.game_switch=v[1];t.game_crouch=-1;
  sc_translate(&c,&t,p,100);t.air=1;t.takeoff=2;sc_translate(&c,&t,p,200);
  p.lx=v[2];p.ly=v[3];p.rx=v[4];p.ry=v[5];sc_pad before=sc_translate(&c,&t,p,300);
  t.switched=v[7];t.game_switch=v[8];p.lx=v[9];p.ly=v[10];p.rx=v[11];p.ry=v[12];
  sc_pad after=sc_translate(&c,&t,p,304);
  assert(before.rx*(long)v[6]>0);assert(before.rx*(long)after.rx>0);
  assert(v[6]*(long)v[13]<0); /* original observed reversal */
 }
 /* Absolute evidence must not become a relative toggle on successive airs.
  * Missing samples and a one-poll blip cannot change the retained stance. */
 for(int normal=0;normal<2;normal++){
  sc_xl_absolute a={0};assert(sc_xl_absolute_step(&a,normal,1-normal,0)==normal);
  assert(sc_xl_absolute_step(&a,1-normal,normal,100)==normal);
  assert(sc_xl_absolute_step(&a,normal,1-normal,104)==normal);
  assert(sc_xl_absolute_step(&a,1-normal,normal,200)==normal);
  assert(sc_xl_absolute_step(&a,1-normal,normal,212)==1-normal);
  assert(sc_xl_absolute_step(&a,-1,normal,300)==1-normal);
  assert(sc_xl_absolute_step(&a,1-normal,normal,400)==1-normal);
  assert(sc_xl_absolute_step(&a,normal,1-normal,500)==1-normal);
  assert(sc_xl_absolute_step(&a,normal,1-normal,512)==normal);
 }
 /* Explicit native mirror compensation applies only to ground tricks. */
 for(int g=0;g<2;g++){
  sc_config c=config(g);c.one_foot_flips=1;sc_state a={0},b={0};sc_pad p={0};
  a.game_crouch=b.game_crouch=-1;b.game_mirror_known=b.game_mirror=1;
  sc_translate(&c,&a,p,1000);sc_translate(&c,&b,p,1000);
  if(g)p.ly=-32768;else p.ry=-32768;
  sc_translate(&c,&a,p,1100);sc_translate(&c,&b,p,1100);
  if(g)p.rx=-23000,p.ry=23000;else p.lx=-23000,p.ly=23000;
  sc_pad x=sc_translate(&c,&a,p,1200),y=sc_translate(&c,&b,p,1200);
  assert(x.rx*(long)y.rx<0 && sc_abs((float)x.rx+y.rx)<=1 && x.ry==y.ry);
 }
 printf("PASS: %d XL air/rail continuity cases, absolute stance transitions, fresh landing jobs and native trick frame compensation\n",cases);
}
