#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad feet(sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout&&sw){fy=-fy;by=-by;}int left=c->front_back?1:(!c->goofy)!=sw;
 sc_pad p={0};p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
int main(void){int checks=0;
 for(int g=0;g<2;g++)for(int l=0;l<3;l++)for(int sw=0;sw<2;sw++)for(int release=0;release<2;release++)for(int direction=-1;direction<=1;direction+=2){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=l==0;c.xl_layout=l==2;c.switch_flip=c.front_back?1:3;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1;c.pop_on_release=release;
  sc_state b={0};b.grind=1;b.switched=sw;b.game_crouch=-1;
  sc_pad p=feet(&c,sw,0,0,0,-1),first=sc_translate(&c,&b,p,1000);assert(first.ry);
  float last=0;int frame=SC_SWFLIP(&c,&b)?-1:1;
  for(int angle=5;angle<=60;angle+=5){
   float x,y;sc_vec_at(1,direction*(float)angle,1,&x,&y);p=feet(&c,sw,0,0,x,y);
   sc_pad out=sc_translate(&c,&b,p,1000+angle*30);assert(!b.grind_pop);
   float a=sc_wrap(sc_angle(sc_axis(out.rx),frame*sc_axis(out.ry)));
   assert(direction*a>direction*last+1);assert(first.ry*(double)out.ry>0);last=a;checks++;
   /* A held sample must not fall back to the reflected absolute position. */
   sc_pad held=sc_translate(&c,&b,p,1001+angle*30);assert(held.rx==out.rx && held.ry==out.ry);
  }
  /* Reversing the circle reverses rotation immediately without crossing ends. */
  float x,y;sc_vec_at(1,direction*45.f,1,&x,&y);p=feet(&c,sw,0,0,x,y);
  sc_pad out=sc_translate(&c,&b,p,3000);float a=sc_wrap(sc_angle(sc_axis(out.rx),frame*sc_axis(out.ry)));assert(direction*a<direction*last-5);
  /* A full slow circle returns to its anchor and stays on the tail end;
   * repeated circles cannot accumulate against a fixed +/-90 degree fence. */
  sc_state circle={0};circle.grind=1;circle.switched=sw;circle.game_crouch=-1;
  p=feet(&c,sw,0,0,0,-1);sc_pad anchor=sc_translate(&c,&circle,p,5000),end=anchor;
  for(int angle=5;angle<=720;angle+=5){
   float cx,cy;sc_vec_at(1,direction*(float)angle,1,&cx,&cy);
   p=feet(&c,sw,0,0,cx,cy);end=sc_translate(&c,&circle,p,5000+angle*30);
   assert(!circle.grind_pop && (double)anchor.ry*end.ry>=0);
   if(angle%360==0){assert(sc_abs(end.rx-anchor.rx)<30 && sc_abs(end.ry-anchor.ry)<30);}
  }
  /* Neutral, front-foot participation, role swap, menus and air end the sweep. */
  sc_translate(&c,&b,(sc_pad){0},3100);assert(!b.split_rear_turn);
  p=feet(&c,sw,0,0,.7f,-.4f);sc_translate(&c,&b,p,3200);assert(b.split_rear_turn);
  p=feet(&c,sw,.3f,.4f,.7f,-.4f);sc_translate(&c,&b,p,3300);assert(!b.split_rear_turn);
  p=feet(&c,sw,0,0,.7f,-.4f);sc_translate(&c,&b,p,3400);assert(b.split_rear_turn);sc_remap_foot_history(&b);assert(!b.split_rear_turn);
  sc_translate(&c,&b,p,3500);p.buttons=SC_DPAD_UP;sc_translate(&c,&b,p,3510);assert(!b.split_rear_turn);
  p.buttons=0;b.grind=0;b.air=1;b.takeoff=2;sc_translate(&c,&b,p,4000);assert(!b.split_rear_turn);
  /* Airborne board setup keeps its existing rear-foot sign. */
  sc_split_position(&c,0,0,.7f,-.4f,1,&x,&y);assert(x<0 && y<0);
 }
 printf("PASS: %d rear circular rail samples: both sweep directions, stable holds, reverse sweeps, tail hemisphere and reset guards across layouts, stance, switch and pop styles\n",checks);
}
