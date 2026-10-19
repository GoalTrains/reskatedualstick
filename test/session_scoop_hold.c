#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad foot(int back,float angle,float strength){
 sc_pad p={0};float x,y;sc_vec_at(back,angle,strength,&x,&y);
 if(back){p.rx=sc_short(x);p.ry=sc_short(y);}else{p.lx=sc_short(x);p.ly=sc_short(y);}return p;
}
int main(void){int cases=0;
 for(int goofy=0;goofy<2;goofy++)for(int sw=0;sw<2;sw++)for(int back=0;back<2;back++)
 for(int side=-1;side<=1;side+=2)for(int full=0;full<2;full++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.front_back=c.split_grinds=1;
  c.goofy=goofy;c.switch_flip=1;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.windup_deg=60;
  sc_state s={0};s.game_crouch=-1;s.switched=sw;
  sc_translate(&c,&s,(sc_pad){0},1000);
  sc_pad p={0},last={0};
  for(int a=full?-90:0;a<=90;a+=15){p=foot(back,a*side,1);last=sc_translate(&c,&s,p,1200+(a+90)*2);}
  assert(sc_abs(s.lone_sw[back])>=35);
  s.air=1;s.takeoff=2;
  sc_pad out=sc_translate(&c,&s,p,1600);
  assert(s.hold_scoop==1 && s.hold_foot==(back?2:1));
  assert(sc_abs(out.rx-last.rx)<=3 && sc_abs(out.ry-last.ry)<=3);
  for(int t=1608;t<1760;t+=8){out=sc_translate(&c,&s,p,t);assert(sc_abs(out.rx-last.rx)<=3 && sc_abs(out.ry-last.ry)<=3);}
  /* Releasing cannot jump across into the opposite board-control direction. */
  out=sc_translate(&c,&s,foot(back,side*90,.6f),1760);assert(!out.rx && !out.ry && s.hold_scoop==2);
  out=sc_translate(&c,&s,foot(back,side*55,.45f),1770);assert(!out.rx && !out.ry);
  out=sc_translate(&c,&s,(sc_pad){0},1780);assert(!out.rx && !out.ry && !s.hold_foot && !s.hold_scoop);
  /* A new airborne foot push remains available for board setup. */
  out=sc_translate(&c,&s,foot(back,side*90,1),1800);assert(out.rx || out.ry);
  cases++;
 }
 /* A static crouch or sideways setup cannot pretend to be a measured scoop. */
 for(int back=0;back<2;back++)for(int angle=0;angle<=90;angle+=90){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.front_back=1;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.windup_deg=60;
  sc_state s={0};s.game_crouch=-1;sc_pad p=foot(back,angle,1);
  sc_translate(&c,&s,(sc_pad){0},1000);sc_translate(&c,&s,p,1100);sc_translate(&c,&s,p,1200);
  s.air=1;s.takeoff=2;sc_translate(&c,&s,p,1300);assert(!s.hold_scoop && !s.hold_foot);
 }
 printf("PASS: %d Session shuv/360 trajectories retain the actual endpoint at takeoff, remain held, centre without a reversed tail pivot, then allow fresh air setup; static inputs never capture\n",cases);
}
