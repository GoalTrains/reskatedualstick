#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad feet(sc_config*c,int sw,float f,float b){
 if(c->xl_layout && sw){f=-f;b=-b;}int left=c->front_back?1:(!c->goofy)!=sw;
 sc_pad p={0};p.ly=sc_short(left?f:b);p.ry=sc_short(left?b:f);return p;
}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int l=0;l<3;l++)for(int sw=0;sw<2;sw++)for(int foot=0;foot<2;foot++)for(int sign=-1;sign<=1;sign+=2){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=l==0;c.xl_layout=l==2;c.switch_flip=c.front_back?1:3;
 c.deadzone=.15f;c.active=.3f;c.grind_sensitivity=1.2f;c.one_foot_flips=1;float previous=0;
 for(int j=0;j<6;j++){
  float raw=.2f+j*.1f;sc_state s={0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;
  sc_pad p=feet(&c,sw,foot?0:sign*raw,foot?sign*raw:0);sc_pad o=sc_translate(&c,&s,p,1000+j*50);
  float y=sc_abs(sc_axis(o.ry));assert(o.rx==0 && o.lx==0 && s.split_grind_active && y>previous);previous=y;
  if(j==2)assert(y>.39f && y<.41f);
 }
 sc_state s={0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;sc_pad p=feet(&c,sw,1,-1),o=sc_translate(&c,&s,p,2000);assert(!o.rx && !o.ry && !o.lx);
 p=(sc_pad){0};o=sc_translate(&c,&s,p,2100);assert(!o.rx && !o.ry);
 count++;
 }
 puts("PASS: 48 setup-response cases: both feet/directions, all layouts/stances, proportional small tilt, no added spin, balanced full pressure and neutral");
}
