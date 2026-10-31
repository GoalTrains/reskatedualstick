#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/translate.h"
static sc_pad point(float a,float m){sc_pad p={0};p.rx=sc_short(m*sc_sin_deg(a));p.ry=sc_short(-m*sc_sin_deg(90-a));return p;}
static sc_config config(void){sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.steer_curve=c.steer_scale=1;c.revert_input=4;c.revert_ms=300;c.revert_shape=5;c.revert_sweep_ms=300;return c;}
int main(void){int cases=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int direction=-1;direction<=1;direction+=2)for(int release=0;release<2;release++){
  sc_config c=config();c.goofy=g;c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=layout?3:1;c.pop_on_release=release;
  sc_state s={0};s.switched=sw;s.game_crouch=0;sc_pad p={0};sc_translate(&c,&s,p,1000);
  int selected_left=layout==0?sw:!((!g)!=sw);
  int selected_y=(layout==0 || layout==2)&&sw?-1:1;
  float start=direction>0?100:260;
  /* Real pad ramp must reserve the candidate before foot activation. */
  for(int i=1;i<=5;i++){p=point(start,i*.18f);p.rx*=selected_y;p.ry*=selected_y;if(selected_left){p.lx=p.rx;p.ly=p.ry;p.rx=p.ry=0;}sc_pad o=sc_translate(&c,&s,p,1000+i*8);assert(!s.rv_start);if(s.top_revert.armed)assert(!o.rx&&!o.ry);}
  assert(s.top_revert.armed && !s.d_front_on && !s.d_back_on);
  for(int i=1;i<=4;i++){p=point(start+direction*i*25,.95f);p.rx*=selected_y;p.ry*=selected_y;if(selected_left){p.lx=p.rx;p.ly=p.ry;p.rx=p.ry=0;}sc_translate(&c,&s,p,1040+i*20);assert(!s.rv_start);}
  p=point(start+direction*125,.95f);p.rx*=selected_y;p.ry*=selected_y;if(selected_left){p.lx=p.rx;p.ly=p.ry;p.rx=p.ry=0;}sc_pad out=sc_translate(&c,&s,p,1140);
  assert(s.rv_start==1140 && s.rv_side==-direction && s.top_revert.fired && !out.rx && !out.ry);
  for(int t=1160;t<1900;t+=20){sc_translate(&c,&s,p,t);assert(!s.rv_start||s.rv_start==1140);}
  assert(!s.rv_start && s.top_revert.blocked);
  sc_translate(&c,&s,(sc_pad){0},1920);assert(!s.top_revert.blocked);cases++;
 }
 /* Ordinary pops begin below, straight up flicks and radial clicks cannot
  * become reverts. Compare their normal native outputs against mode OFF. */
 for(int shape=0;shape<4;shape++){
  sc_config c=config(),plain=c;plain.revert_input=0;sc_state a={0},b={0};
  a.game_crouch=b.game_crouch=0;
  for(int i=0;i<7;i++){
   sc_pad p=point(shape==0?10:shape==1?180:100+i*25,.95f);
   if(shape==2)p.ly=20000;if(shape==3)p.buttons=SC_DPAD_UP;
   sc_pad x=sc_translate(&c,&a,p,1000+i*20),y=sc_translate(&plain,&b,p,1000+i*20);
   assert(!a.rv_start && !memcmp(&x,&y,sizeof x));
  }
 }
 for(int context=0;context<5;context++){
  sc_config c=config();sc_state s={0};s.game_crouch=0;sc_pad p=point(100,.95f);
  if(context==0)s.air=s.takeoff=1;if(context==1)s.grind=1;
  if(context==2)s.game_crouch=1;if(context==3)c.enabled=0;if(context==4)p.buttons=SC_RTHUMB;
  sc_translate(&c,&s,p,1000);assert(!s.top_revert.armed && !s.rv_start);
 }
 {sc_top_revert s={0};sc_top_step(&s,1,1,0,1,100,1000);
  assert(!sc_top_step(&s,1,1,0,1,250,1020));assert(s.blocked);}
 printf("PASS: %d right top-arc revert configurations, analog ramp, both directions, neutral rearm, ordinary foot gestures and context isolation\n",cases);
}
