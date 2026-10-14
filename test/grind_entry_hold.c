#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static void foot(sc_pad*p,int lf,int front,int x,int y){if(front==lf){p->lx=x;p->ly=y;}else{p->rx=x;p->ry=y;}}
static sc_config config(int g,int fb){sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.goofy=g;c.front_back=fb;c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.grind_sensitivity=1.2f;return c;}
int main(void){for(int g=0;g<2;g++)for(int fb=0;fb<2;fb++)for(int sw=0;sw<2;sw++)for(int which=0;which<2;which++){
 sc_config c=config(g,fb);sc_state s={0};s.switched=sw;s.game_crouch=-1;sc_pad p={0};int lf=fb?1:((!g)!=sw);
 sc_translate(&c,&s,p,1000);foot(&p,lf,which,31488,which?14593:-14593);
 sc_translate(&c,&s,p,1100);sc_translate(&c,&s,p,1200);
 s.takeoff=1;sc_pad air=sc_translate(&c,&s,p,1300);
 assert(!s.hold_foot);assert(which?air.rx>20000:air.rx<-20000);
 s.takeoff=0;s.grind=1;sc_pad rail=sc_translate(&c,&s,p,1400);
 assert(air.rx* (double)rail.rx>0); /* same input cannot reverse at contact */
 }
 /* A genuine two-foot flick remains holdable; moving the other foot to aim
  * the board must regain pivot control without releasing the flick foot. */
 for(int g=0;g<2;g++)for(int sw=0;sw<2;sw++)for(int which=0;which<2;which++){
 sc_config c=config(g,0);sc_state s={0};s.switched=sw;s.game_crouch=-1;sc_pad p={0};int lf=(!g)!=sw;
 sc_translate(&c,&s,p,2000);foot(&p,lf,!which,0,which?-32768:32767);
 sc_translate(&c,&s,p,2100);sc_translate(&c,&s,p,2400);
 foot(&p,lf,which,-23166,which?23166:-23166);sc_pad flick=sc_translate(&c,&s,p,2450);
 s.takeoff=1;sc_pad held=sc_translate(&c,&s,p,2500);
 assert(s.hold_foot==(which?1:2));assert(held.rx*(double)flick.rx>0);
 held=sc_translate(&c,&s,p,2600);assert(s.hold_foot);
 foot(&p,lf,!which,30000,0);sc_translate(&c,&s,p,2650);assert(!s.hold_foot);
 }
 puts("PASS: setup sticks pivot consistently before/after rail contact, regular/goofy layouts/switch; true held flips survive and new opposite-foot intent cancels hold");}
