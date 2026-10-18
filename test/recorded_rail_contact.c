#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad physical(sc_config*c,int sw,const short*v){
 float fx=sc_axis(v[0]),fy=sc_axis(v[1]),bx=sc_axis(v[2]),by=sc_axis(v[3]);
 if(c->xl_layout&&sw){fy=-fy;by=-by;}
 int left=c->front_back?1:(!c->goofy)!=sw;sc_pad p={0};
 p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
/* Actual held inputs from the six v5.143 entries that were dropped to zero. */
static const short captured[][4]={
 {0,-1,-32768,-14081},
 {0,-1,-32768,255},
 {0,-1,-32768,2559},
 {0,-1,-32512,-17665},
 {12544,32255,-29952,-1},
 {0,-1,-32768,-1025}
};
int main(void){int cases=0;
 for(int g=0;g<2;g++)for(int l=0;l<3;l++)for(int sw=0;sw<2;sw++)for(int k=0;k<6;k++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=l==0;c.xl_layout=l==2;c.switch_flip=c.front_back?1:3;
  c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;
  sc_state s={0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;
  sc_pad p=physical(&c,sw,captured[k]),a=sc_translate(&c,&s,p,1000);
  s.air=s.takeoff=0;s.grind=1;sc_pad b=sc_translate(&c,&s,p,1010);
  assert(!s.grind_pop && s.split_grind_active);
  assert((int)b.rx*b.rx+(double)b.ry*b.ry>10000.0*10000);
  assert(sc_abs((float)a.rx-b.rx)<4&&sc_abs((float)a.ry-b.ry)<4);
  /* Repeated held and subsequent movement continue producing board input. */
  b=sc_translate(&c,&s,p,1030);assert(b.rx||b.ry);cases++;
 }
 printf("PASS: %d recorded rail-contact cases: all six lost setups retain input across contact in regular/goofy, every layout and both riding directions\n",cases);
}
