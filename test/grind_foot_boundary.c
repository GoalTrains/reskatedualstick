#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad feet(sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout&&sw){fy=-fy;by=-by;}int left=c->front_back?1:(!c->goofy)!=sw;
 sc_pad p={0};p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int l=0;l<3;l++)for(int sw=0;sw<2;sw++)for(int foot=0;foot<2;foot++)for(int release=0;release<2;release++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=l==0;c.xl_layout=l==2;c.switch_flip=c.front_back?1:3;
  c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;c.pop_on_release=release;
  sc_state s={0};s.grind=1;s.switched=sw;s.game_crouch=-1;
  sc_pad p=feet(&c,sw,0,foot?0:1,0,foot?-1:0);sc_pad initial=sc_translate(&c,&s,p,1000);assert(initial.ry);
  /* Fast/slow reported one-foot semicircles may no longer enter the opposite
     native flip hemisphere. Entry is already held, so this isn't a fresh shove. */
  for(int i=0;i<=180;i+=3){float x,y;sc_vec_at(foot,(float)i,1,&x,&y);
   p=feet(&c,sw,foot?0:x,foot?0:y,foot?x:0,foot?y:0);sc_pad o=sc_translate(&c,&s,p,1020+i*4);
   assert(!s.grind_pop&&!s.rail_scoop_exit);assert((double)initial.ry*o.ry>=0);assert(o.rx||o.ry||i==180);
  }
  /* The wrong vertical end is neutral, not the other truck. */
  s=(sc_state){0};s.grind=1;s.switched=sw;s.game_crouch=-1;p=feet(&c,sw,0,foot?0:-1,0,foot?1:0);
  sc_pad o=sc_translate(&c,&s,p,2500);assert(!o.rx&&!o.ry);
  /* Both feet can flatten the board; neither is silently consumed as a flick. */
  s=(sc_state){0};s.grind=1;s.switched=sw;s.game_crouch=-1;p=feet(&c,sw,.8f,0,-.8f,0);o=sc_translate(&c,&s,p,3000);
  assert(!s.grind_pop&&sc_abs(o.rx)>30000&&o.ry==0);
  p=feet(&c,sw,.4f,.8f,0,-1);o=sc_translate(&c,&s,p,3100);sc_pad a=o;
  p=feet(&c,sw,.2f,.9f,0,-1);o=sc_translate(&c,&s,p,3200);assert(!s.grind_pop&&(o.rx!=a.rx||o.ry!=a.ry));
  a=o;p=feet(&c,sw,.2f,.9f,-.5f,-.5f);o=sc_translate(&c,&s,p,3300);assert(!s.grind_pop&&(o.rx!=a.rx||o.ry!=a.ry));
  /* Geometry is identical on approach and the first rail poll. */
  s=(sc_state){0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;p=feet(&c,sw,foot?0:.8f,0,foot?.8f:0,0);
  a=sc_translate(&c,&s,p,4000);s.air=s.takeoff=0;s.grind=1;o=sc_translate(&c,&s,p,4010);
  assert(sc_abs((float)a.rx-o.rx)<4&&sc_abs((float)a.ry-o.ry)<4);
  /* ALLOWED explicitly retains the old full-range stick behavior. */
  c.one_foot_flips=1;s=(sc_state){0};s.grind=1;s.switched=sw;s.game_crouch=-1;p=feet(&c,sw,0,foot?0:-1,0,foot?1:0);
  o=sc_translate(&c,&s,p,5000);assert(o.ry&&initial.ry*(double)o.ry<0);
  count++;
 }
 printf("PASS: %d foot-boundary profiles: captured sweep hemisphere, own-end loading, both-foot adjustments, contact continuity and allowed-flip opt-out\n",count);
}
