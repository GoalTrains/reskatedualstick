#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/menu_scheme.h"
int main(void){for(int g=0;g<2;g++)for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2)for(int mirror=0;mirror<2;mirror++){
 sc_config c={0};c.enabled=c.pure=1;c.goofy=g;c.switch_flip=3;c.flip_x=mirror;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
 sc_state s={0};s.switched=sw;s.game_crouch=-1;sc_pad p={0};int lf=sc_menu_left_front(0,g,3,sw,0);
 sc_translate(&c,&s,p,1000);if(lf)p.ry=-32768;else p.ly=-32768;
 sc_translate(&c,&s,p,1100);sc_translate(&c,&s,p,1400);
 if(lf){p.lx=side*23166;p.ly=23166;}else{p.rx=side*23166;p.ry=23166;}
 sc_pad out=sc_translate(&c,&s,p,1450);
 assert(s.feet_map_left==lf);assert(out.rx*(side*(mirror?-1:1))>20000);
 }for(int kind=0;kind<2;kind++){int fb,mode;sc_menu_choose_layout(kind,&fb,&mode);assert(sc_menu_layout_kind(fb,mode)==kind);}
 assert(sc_menu_layout_kind(0,1)==2);
 puts("PASS: True Foot retains kick/heel sideways signs in regular/goofy normal/switch, explicit mirror and complete layout choices");}
