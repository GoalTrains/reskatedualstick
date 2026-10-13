#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_config cfg(int g,int fb,int sw,int steering){sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.goofy=g;c.front_back=fb;c.switch_flip=3;c.windup_deg=60;c.steer_scale=c.steer_curve=1;c.stick_steer=steering;return c;}
static sc_pad foot(int left,float x,float y){sc_pad p={0};if(left){p.lx=sc_short(x);p.ly=sc_short(y);}else{p.rx=sc_short(x);p.ry=sc_short(y);}return p;}
int main(void){
 for(int g=0;g<2;g++)for(int fb=0;fb<2;fb++)for(int sw=0;sw<2;sw++)for(int steering=1;steering<=2;steering++)for(int side=-1;side<=1;side+=2){
  sc_config c=cfg(g,fb,sw,steering);sc_state s={0};s.switched=sw;
  int lf=fb?1:((!g)!=sw);
  sc_translate(&c,&s,(sc_pad){0},1000);
  sc_translate(&c,&s,foot(lf,0,1),1100);
  sc_translate(&c,&s,foot(lf,side*.6f,.8f),1200);
  sc_pad o=sc_translate(&c,&s,foot(lf,side,0),1300);
  assert(!s.st_latch && s.d_front_on && (o.rx>20000 || o.rx< -20000)); /* Nollie scoop survives steering threshold. */
  s=(sc_state){0};s.switched=sw;
  int sl=steering==2?1:lf;
  o=sc_translate(&c,&s,foot(sl,side,0),2000);assert(s.st_latch && (side>0?o.lx>25000:o.lx< -25000) && o.rx==0);
  sc_translate(&c,&s,(sc_pad){0},2100);assert(!s.st_latch);
 }
 for(int allowed=0;allowed<2;allowed++){
  sc_config c=cfg(0,1,0,0);c.one_foot_flips=allowed;sc_state s={0};
  sc_translate(&c,&s,foot(0,0,-1),1000);sc_translate(&c,&s,foot(0,0,-.4f),1100);
  sc_pad o=sc_translate(&c,&s,foot(0,0,1),1200);
  assert(allowed?o.ry>30000:o.ry<=0); /* Allowed disables the centre-crossing block too. */
 }
 for(int back=0;back<2;back++)for(int dir=-1;dir<=1;dir+=2){
  sc_state s={0};float x=0,y=back?-1:1;sc_lone_start(&s,back,x,y);
  for(int angle=0;angle<=210;angle+=15){sc_vec_at(back,dir*angle,1,&x,&y);sc_lone_clamp(&s,back,&x,&y);assert(!s.lone_unl[back]);assert(back?y<0:y>0);}
  /* A spring rebound in the far quadrant is not a deliberate full circle. */
  sc_vec_at(back,dir*260,.4f,&x,&y);sc_lone_clamp(&s,back,&x,&y);assert(!s.lone_unl[back]);
  s=(sc_state){0};x=0;y=back?-1:1;sc_lone_start(&s,back,x,y);
  for(int angle=0;angle<=300;angle+=15){sc_vec_at(back,dir*angle,1,&x,&y);sc_lone_clamp(&s,back,&x,&y);}
  assert(s.lone_unl[back]); /* Deliberate impossibles remain available. */
 }
 puts("PASS: front scoops retain foot control with front/left steering; fresh lateral input steers, both stances/layouts/switch; half-circle/rebound cannot unlock one-stick flips, deliberate full circles remain");
}
