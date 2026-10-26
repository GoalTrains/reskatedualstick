#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad feet(sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout&&sw){fy=-fy;by=-by;}int left=c->front_back?1:(!c->goofy)!=sw;
 sc_pad p={0};p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int l=0;l<3;l++)for(int sw=0;sw<2;sw++)for(int foot=0;foot<2;foot++)for(int side=-1;side<=1;side+=2){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=l==0;c.xl_layout=l==2;c.switch_flip=c.front_back?1:3;
  c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;
  sc_state s={0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;
  sc_pad p=feet(&c,sw,foot?0:side,0,foot?side:0,0);sc_pad a=sc_translate(&c,&s,p,1000);
  assert(sc_abs(a.rx)>=32750&&a.ry==0); /* True native flat-side target, not x=.966/y=.259. */
  s.air=s.takeoff=0;s.grind=1;sc_pad b=sc_translate(&c,&s,p,1010);assert(a.rx==b.rx&&b.ry==0&&!s.grind_pop);
  /* A fresh flat-side hold has no remembered end pressure to invent. */
  p=feet(&c,sw,foot?0:side,foot?0:-.2f,foot?side:0,foot?.2f:0);
  b=sc_translate(&c,&s,p,1100);
  if(!foot)assert(sc_abs(b.rx)>=32750&&b.ry==0&&!s.grind_pop);
  else assert(b.rx && (SC_SWFLIP(&c,&s)?-b.ry:b.ry)<=0 && !s.grind_pop); /* Rear angular motion now rotates from its entry anchor. */
  /* Radial selection is intentionally stock. Label it explicitly so a raw
     stick reversal here cannot be misdiagnosed as a stance/mapping change. */
  p.buttons=SC_DPAD_UP;b=sc_translate(&c,&s,p,1200);assert(s.radial_passthrough&&b.rx==p.rx&&b.ry==p.ry);
  p.buttons=0;sc_translate(&c,&s,p,1300);assert(s.radial_passthrough);
  sc_translate(&c,&s,p,1500);assert(!s.radial_passthrough);
  count++;
 }
 printf("PASS: %d fresh flat-side rail profiles: no invented pressure, proper setup contact continuity and radial passthrough\n",count);
}
