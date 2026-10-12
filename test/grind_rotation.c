#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void){for(int g=0;g<2;g++)for(int fb=0;fb<2;fb++)for(int sw=0;sw<2;sw++)for(int release=0;release<2;release++){
 sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.goofy=g;c.front_back=fb;c.switch_flip=3;c.pop_on_release=release;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
 sc_state s={0};s.grind=1;s.switched=sw;s.game_crouch=-1;sc_pad p={0};int lf=fb?1:((!g)!=sw);
 sc_translate(&c,&s,p,1000);
 for(int i=0;i<16;i++){float a=i*22.5f;int x=sc_short(sc_sin_deg(a)),y=sc_short(-sc_sin_deg(90-a));if(lf){p.rx=x;p.ry=y;}else{p.lx=x;p.ly=y;}
 sc_pad o=sc_translate(&c,&s,p,1100+i*20);assert(!s.grind_pop);if(x>12000)assert(o.rx< -10000);if(x< -12000)assert(o.rx>10000);
 }
 /* Full vertical setup followed by centring remains a Release exit. */
 if(release){p=(sc_pad){0};if(lf)p.ry=-32768;else p.ly=-32768;sc_translate(&c,&s,p,1500);p=(sc_pad){0};sc_pad o=sc_translate(&c,&s,p,1600);assert(s.grind_pop && sc_abs((float)o.ry)>30000);}
 }puts("PASS: solo back-foot full circles remain rail pivots across both stances/layouts/switch/pop styles; Release rail exits preserved");}

