#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad feet(const sc_config*c,int sw,int foot,int x,int y){
 float fx=foot==1?sc_axis((short)x):0,fy=foot==1?sc_axis((short)y):0;
 float bx=foot==2?sc_axis((short)x):0,by=foot==2?sc_axis((short)y):0;
 if(c->xl_layout&&sw){fy=-fy;by=-by;}
 int left=c->front_back?1:(!c->goofy)!=sw;sc_pad p={0};
 p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);
 p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
static void equal(sc_pad a,sc_pad b){assert(sc_abs((float)a.rx-b.rx)<=4&&sc_abs((float)a.ry-b.ry)<=4);}
int main(void){unsigned checks=0;
 /* Raw back-foot poses from the reported session, including the observed
  * air -> brief ground -> rail handoff. Mirror to exercise the nose too. */
 const int points[][2]={{15616,-30721},{27392,-20993},{-32768,-8961},{32767,-1}};
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int foot=1;foot<=2;foot++)for(int mirror=0;mirror<2;mirror++)for(unsigned k=0;k<sizeof points/sizeof points[0];k++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=layout==0;c.xl_layout=layout==2;
  c.switch_flip=c.front_back?1:3;c.active=.3f;c.deadzone=.15f;c.grind_sensitivity=1.2f;c.manual_max=.85f;
  sc_state s={0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;s.game_mirror_known=1;s.game_mirror=mirror;
  int y=foot==2?points[k][1]:-points[k][1];sc_pad p=feet(&c,sw,foot,points[k][0],y);
  sc_pad air=sc_translate(&c,&s,p,1000);assert(s.split_air_setup);
  s.air=s.takeoff=0;
  for(int i=0;i<24;i++){sc_pad ground=sc_translate(&c,&s,p,1010+i*4);equal(air,ground);assert(s.split_landing_setup&&!s.grind_pop);checks++;}
  s.grind=1;sc_pad rail=sc_translate(&c,&s,p,1120);
  if(!(c.xl_layout&&mirror))equal(air,rail);
  assert(!s.split_landing_setup&&!s.grind_pop);
  /* A held strong ground pose stays a continuation until neutral. */
  s=(sc_state){0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;s.game_mirror_known=1;s.game_mirror=mirror;
  sc_translate(&c,&s,p,2000);s.air=s.takeoff=0;sc_translate(&c,&s,p,2010);
  p=(sc_pad){0};sc_pad out=sc_translate(&c,&s,p,2100);assert(!s.split_landing_setup&&!s.d_land_hold&&!out.rx&&!out.ry);
  /* A foot first pressed on the ground did not carry from neutral air. */
  s=(sc_state){0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;
  sc_translate(&c,&s,(sc_pad){0},2150);assert(!s.split_air_setup);
  s.air=s.takeoff=0;p=feet(&c,sw,foot,points[k][0],y);
  sc_translate(&c,&s,p,2160);assert(!s.split_landing_setup);
  /* Fresh ground input is not classified as an airborne continuation. */
  p=feet(&c,sw,foot,0,foot==2?-32768:32767);sc_translate(&c,&s,p,2200);assert(!s.split_landing_setup);
 }
 printf("PASS: %u recorded-pose handoff checks: held lateral setup survives ground contact without reversal; nose/back, layouts, stance and native mirrors; neutral and fresh input rearm\n",checks);
}
