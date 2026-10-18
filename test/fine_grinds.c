#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_config config(int g,int layout,int release){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.fine_grinds=1;c.goofy=g;
 c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=c.front_back?1:3;
 c.pop_on_release=release;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;return c;
}
static sc_pad feet(sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout && sw){fy=-fy;by=-by;}
 int left=c->front_back?1:(!c->goofy)!=sw;
 sc_pad p={0};p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);
 p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
static void close_value(int a,int b){assert(sc_abs((float)a-b)<=3);}
int main(void){
 int count=0;
 for(int g=0;g<2;g++)for(int l=0;l<3;l++)for(int sw=0;sw<2;sw++)for(int release=0;release<2;release++)for(int sign=-1;sign<=1;sign+=2){
  sc_config c=config(g,l,release);sc_state s={0};s.grind=1;s.switched=sw;s.game_crouch=-1;
  /* Reported smith/feeble intent held into contact, not a trick-out. */
  sc_pad p=feet(&c,sw,sign*.65f,.70f,0,-1);
  sc_pad first=sc_translate(&c,&s,p,1000),held=sc_translate(&c,&s,p,1200);
  assert(!s.grind_pop);assert(sc_abs((float)held.ry)>sc_abs((float)held.rx));
  close_value(first.rx,held.rx);close_value(first.ry,held.ry);
  /* The reported smaller front-foot sideways tweak also keeps a dip. */
  sc_pad small=feet(&c,sw,sign*.20f,.98f,0,-1);
  sc_pad small_out=sc_translate(&c,&s,small,1250);
  assert(!s.grind_pop && sc_abs((float)small_out.ry)>sc_abs((float)small_out.rx));
  /* Fine forward/backward adjustments remain distinct (old abs() lost them). */
  sc_pad q=feet(&c,sw,sign*.65f,-.70f,0,-1);sc_pad reverse=sc_translate(&c,&s,q,1300);
  assert(!s.grind_pop);assert(sc_abs((float)reverse.ry)>sc_abs((float)held.ry));
  /* Releasing an entry pose must not fire Release pop. */
  p=(sc_pad){0};sc_translate(&c,&s,p,1400);assert(!s.grind_pop && !s.fired);
  /* Deliberate fresh pop setup after neutral, then a fast opposite foot. */
  p=feet(&c,sw,0,0,0,-1);sc_translate(&c,&s,p,1500);sc_translate(&c,&s,p,1600);
  if(release){p=(sc_pad){0};sc_translate(&c,&s,p,1610);assert(s.grind_pop && s.fired);}
  else {p=feet(&c,sw,sign*.65f,.70f,0,-1);sc_translate(&c,&s,p,1610);assert(s.grind_pop);}
  /* Same setup with a slow front-foot tweak must stay positioning. */
  s=(sc_state){0};s.grind=1;s.switched=sw;s.game_crouch=-1;p=(sc_pad){0};sc_translate(&c,&s,p,2000);
  p=feet(&c,sw,0,0,0,-1);sc_translate(&c,&s,p,2100);
  for(int j=1;j<=5;j++){float m=j*.19f;p=feet(&c,sw,sign*.68f*m,.73f*m,0,-1);sc_translate(&c,&s,p,2200+j*60);assert(!s.grind_pop);}
  /* Match positioning immediately before and after rail contact. */
  s=(sc_state){0};s.switched=sw;s.game_crouch=-1;p=(sc_pad){0};sc_translate(&c,&s,p,3000);
  s.air=1;s.takeoff=2;sc_translate(&c,&s,p,3200);p=feet(&c,sw,sign*.65f,.70f,0,-1);
  sc_pad air=sc_translate(&c,&s,p,3300);s.air=s.takeoff=0;s.grind=1;
  sc_pad rail=sc_translate(&c,&s,p,3310);close_value(air.rx,rail.rx);close_value(air.ry,rail.ry);assert(!s.grind_pop);
  /* Front-foot/nollie exit and quick solo scoops remain available too. */
  s=(sc_state){0};s.grind=1;s.switched=sw;s.game_crouch=-1;p=(sc_pad){0};sc_translate(&c,&s,p,4000);
  p=feet(&c,sw,0,1,0,0);sc_translate(&c,&s,p,4100);sc_translate(&c,&s,p,4200);
  if(release){p=(sc_pad){0};sc_translate(&c,&s,p,4210);assert(s.grind_pop && s.fired);}
  else {p=feet(&c,sw,0,1,sign*.65f,-.70f);sc_translate(&c,&s,p,4210);assert(s.grind_pop);}
  for(int back=0;back<2;back++){
   s=(sc_state){0};s.grind=1;s.switched=sw;s.game_crouch=-1;p=(sc_pad){0};sc_translate(&c,&s,p,5000);
   p=feet(&c,sw,0,back?0:1,0,back?-1:0);sc_translate(&c,&s,p,5100);
   p=feet(&c,sw,back?0:sign*.38f,back?0:.92f,back?sign*.38f:0,back?-.92f:0);sc_translate(&c,&s,p,5110);
   p=feet(&c,sw,back?0:sign*.71f,back?0:.71f,back?sign*.71f:0,back?-.71f:0);sc_translate(&c,&s,p,5120);
   assert(s.grind_pop && s.rail_scoop_exit==(back?2:1));
  }
  count++;
 }
 /* Small movements are proportional, not a full-strength snapped direction. */
 float x1,y1,x2,y2;sc_fine_board(.1f,0,0,0,1,&x1,&y1);sc_fine_board(.2f,0,0,0,1,&x2,&y2);
 assert(x1>0 && x1<x2 && x2<.3f && y1==0 && y2==0);
 sc_fine_board(.7f,.7f,0,-1,1,&x1,&y1);assert(x1>0 && x1<.3f && y1<-.5f);
 printf("PASS: %d fine grind configurations: reported two-foot pose, signed tweaks, slow adjustments, deliberate Flick/Release exits, air/rail continuity and fine response\n",count);
}
