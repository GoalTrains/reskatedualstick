#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/gesture_speed.h"
static float stroke(int duration,int side,int foot,int scoop){
 sc_gesture_speed s={0};sc_pad p={0};float f=1;
 if(foot)p.ry=-32767;else p.ly=-32767;
 sc_gs_step(&s,p,1000,100,1);
 for(int t=4;t<=duration;t+=4){
  float a=(float)t/duration;
  short x=sc_short(side*a),y=sc_short(scoop?-(1-a*.5f):-1.f);
  if(foot){p.rx=x;p.ry=y;}else{p.lx=x;p.ly=y;}
  f=sc_gs_step(&s,p,1000+t,101,1);
 }
 float frozen=sc_gs_step(&s,(sc_pad){0},1004+duration,200,1);
 assert(frozen==f);
 assert(sc_gs_step(&s,(sc_pad){0},1008+duration,200,1)==f);
 assert(sc_gs_step(&s,p,1008+duration,200,1)==f); /* duplicate route */
 assert(sc_gs_step(&s,p,1012+duration,200,0)==1.f);
 return f;
}
int main(void){int n=0;
 for(int side=-1;side<=1;side+=2)for(int foot=0;foot<2;foot++)for(int scoop=0;scoop<2;scoop++){
  float slow=stroke(240,side,foot,scoop),fast=stroke(32,side,foot,scoop);
  assert(slow>=.25f && slow<.6f);assert(fast>1.4f && fast<=1.75f);assert(fast>slow);n++;
 }
 assert(sc_gs_factor(0)==.25f && sc_gs_factor(3)==.25f && sc_gs_factor(24)==1.75f);
 /* Stopping the stroke is not a new slower stroke. Hold/release delay before
  * takeoff must retain the speed of the completed motion. */
 sc_gesture_speed tail={0};sc_pad held={0};held.ry=-32767;
 sc_gs_step(&tail,held,2000,100,1);float finished=1.f;
 for(int t=4;t<=64;t+=4){held.rx=sc_short((float)t/64);finished=sc_gs_step(&tail,held,2000+t,100,1);}
 assert(finished>.9f);
 for(int t=4;t<=40;t+=4)assert(sc_gs_step(&tail,held,2064+t,100,1)==finished);
 held=(sc_pad){0};assert(sc_gs_step(&tail,held,2108,100,1)==finished);
 assert(sc_gs_step(&tail,held,2112,200,1)==finished);
 sc_gesture_speed s={0};sc_pad p={0};p.rx=32767;
 sc_gs_step(&s,p,1000,100,1);
 assert(sc_gs_step(&s,(sc_pad){0},1020,100,1)==1.f); /* recenter is not a flick */
 assert(sc_gs_step(&s,p,1040,400,1)==1.f);
 puts("gesture speed: slow/fast flips and scoops, both feet/directions, release, disable and duplicate polls passed");return n!=8;
}
