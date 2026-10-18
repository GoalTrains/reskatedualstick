#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad feet(sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout && sw){fy=-fy;by=-by;}
 int left=c->front_back?1:(!c->goofy)!=sw;
 sc_pad p={0};p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
static sc_state held(int sw,int foot,float fx,float fy,float bx,float by){
 sc_state s={0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;
 s.d_took_off=s.d_was_air=s.d_front_on=s.d_back_on=s.d_front_spent=s.d_back_spent=1;
 s.hold_foot=foot;s.tk_fa=sc_angle(fx,fy);s.tk_ba=sc_angle(bx,by);
 s.hold_x=foot==2?.5f:-.5f;s.hold_y=foot==2?-.866f:.866f;return s;
}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int l=0;l<3;l++)for(int sw=0;sw<2;sw++)for(int foot=1;foot<=2;foot++){
 sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.goofy=g;c.front_back=l==0;c.xl_layout=l==2;c.switch_flip=c.front_back?1:3;
 c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;
 float fx=foot==2?-1:.6f,fy=foot==2?0:.8f,bx=foot==2?.6f:-1,by=foot==2?-.8f:0;
 sc_pad p=feet(&c,sw,fx,fy,bx,by);sc_state old=held(sw,foot,fx,fy,bx,by),s=old;
 sc_pad stale=sc_translate(&c,&old,p,1000);assert(old.hold_foot==foot);
 c.split_grinds=1;sc_pad setup=sc_translate(&c,&s,p,1000);
 assert(!s.hold_foot && s.grind_setup_override && s.split_grind_active && setup.rx*stale.rx<0 && setup.lx==0);
 /* Literal failure: holding front sideways froze an opposite flick, then
    rail contact reversed output unexpectedly. The corrected rail channel
    now has a pre-reversal X/Y mapping. */
 s.air=0;s.takeoff=0;s.grind=1;sc_pad contact=sc_translate(&c,&s,p,1100);
 assert(contact.lx==0 && !s.grind_pop);
 assert(sc_abs((float)setup.rx-contact.rx)<3 && sc_abs((float)setup.ry-contact.ry)<3);
 /* Contact retains the established X/Y mapping; Y/exit behavior is
    checked by the dedicated rail-direction reference cases. */
 /* A real held flip with its other foot still vertically set stays held. */
 fx=.6f;fy=.8f;bx=0;by=-1;p=feet(&c,sw,fx,fy,bx,by);s=held(sw,1,fx,fy,bx,by);
 setup=sc_translate(&c,&s,p,1200);assert(s.hold_foot==1 && !s.grind_setup_override && !s.split_grind_active);
 count++;
 }
 /* A small deliberate change used to fall inside the 20-degree ignore cone. */
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=c.front_back=1;c.active=.3f;c.deadzone=.15f;c.grind_sensitivity=1;
 sc_state s=held(0,1,.6f,.8f,0,-1);sc_pad p=feet(&c,0,.6f,.8f,.208f,-.978f);
 sc_translate(&c,&s,p,2000);assert(!s.hold_foot && s.grind_setup_override);
 puts("PASS: 24 held-entry cases across both stances, all layouts and both flick feet: stale opposite flick reproduced, setup wins before contact, restored rail continuity, held flip preserved; small adjustment responds");
}
