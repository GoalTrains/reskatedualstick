#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_config config(int g,int sw,int fb,int mode){
 (void)sw;sc_config c={0};c.enabled=c.pure=c.air_controls=c.one_foot_flips=1;c.goofy=g;c.front_back=fb;c.switch_flip=3;c.stick_steer=mode;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.windup_deg=60;c.steer_scale=c.steer_curve=1;return c;
}
static sc_pad stick(int left,float x,float y){sc_pad p={0};if(left){p.lx=sc_short(x);p.ly=sc_short(y);}else{p.rx=sc_short(x);p.ry=sc_short(y);}return p;}
int main(void){
 for(int g=0;g<2;g++)for(int sw=0;sw<2;sw++)for(int fb=0;fb<2;fb++)for(int mode=1;mode<=2;mode++)for(int side=-1;side<=1;side+=2){
  sc_config c=config(g,sw,fb,mode);int lf=fb?1:((!g)!=sw),sl=mode==2?1:lf;
  for(int a=0;a<90;a++){
   sc_pad p=stick(sl,side*.8f*sc_sin_deg(90.f-a),.8f*sc_sin_deg((float)a));sc_state s={0};s.switched=sw;
   sc_translate(&c,&s,p,1000);
   float x=sl?sc_axis(p.lx):sc_axis(p.rx),y=sl?sc_axis(p.ly):sc_axis(p.ry);
   assert(s.st_latch==(sc_abs(x)>=.15f && sc_abs(x)>1.5f*sc_abs(y)));
   sc_state old=s;s=(sc_state){0};s.switched=sw;c.stick_steer_angle=34;sc_translate(&c,&s,p,1000);assert(s.st_latch==old.st_latch);c.stick_steer_angle=0;
  }
  float own=sl==lf?1.f:-1.f;sc_pad diagonal=stick(sl,side*.866f,own*.5f);
  sc_state s={0};s.switched=sw;sc_translate(&c,&s,diagonal,1000);assert(s.st_latch);
  c.stick_steer_angle=20;s=(sc_state){0};s.switched=sw;sc_pad o=sc_translate(&c,&s,diagonal,1000);assert(!s.st_latch);assert(o.rx||o.ry);
  /* Once a diagonal starts a foot gesture, moving to sideways cannot steal
   * it back into steering. Pure sideways still deliberately starts steering. */
  sc_translate(&c,&s,stick(sl,(float)side,0),1020);assert(!s.st_latch);
  s=(sc_state){0};s.switched=sw;sc_translate(&c,&s,stick(sl,(float)side,0),2000);assert(s.st_latch);
  sc_translate(&c,&s,(sc_pad){0},2020);assert(!s.st_latch);
  s=(sc_state){0};s.switched=sw;s.grind=1;sc_translate(&c,&s,diagonal,3000);assert(!s.st_latch);
  s=(sc_state){0};s.switched=sw;s.air=1;sc_translate(&c,&s,diagonal,3000);assert(!s.st_latch);
 }
 sc_config c={0};assert(sc_stick_steer_angle(&c)==34);c.stick_steer_angle=99;assert(sc_stick_steer_angle(&c)==34);
 assert(sc_next_stick_steer_angle(34,-1)==30);assert(sc_next_stick_steer_angle(30,-1)==25);assert(sc_next_stick_steer_angle(25,-1)==20);assert(sc_next_stick_steer_angle(5,-1)==60);assert(sc_next_stick_steer_angle(60,1)==5);
 puts("PASS: legacy steering zone unchanged; 20-degree zone reserves diagonal scoops, retains deliberate steering and started tricks, regular/goofy/layouts/switch/directions; air/rails and menu bounds");return 0;
}
