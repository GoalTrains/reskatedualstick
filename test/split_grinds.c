#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad feet(sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout && sw){fy=-fy;by=-by;}
 int left=c->front_back?1:(!c->goofy)!=sw;
 sc_pad p={0};p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);
 p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)for(int release=0;release<2;release++){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.front_back=layout==0;c.xl_layout=layout==2;c.goofy=g;
 c.switch_flip=c.front_back?1:3;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1;c.pop_on_release=release;c.steer_curve=c.steer_scale=1;c.bump_ms=180;
 sc_state s={0};s.grind=1;s.switched=sw;s.game_crouch=-1;
 sc_pad p=feet(&c,sw,.8f,0,.8f,0),o=sc_translate(&c,&s,p,1000);
 assert(s.split_grind_active && o.lx==0 && o.rx==0 && o.ry==0 && !s.grind_pop);
 /* Matching sideways inputs balance; opposing feet rotate the board. */
 p=feet(&c,sw,-.8f,0,-.8f,0);o=sc_translate(&c,&s,p,1100);assert(o.lx==0 && o.rx==0);
 p=feet(&c,sw,.8f,0,-.8f,0);o=sc_translate(&c,&s,p,1200);assert(o.lx==0 && o.rx>20000);
 /* Either foot can pivot; their contributions retain opposite signs. */
 p=feet(&c,sw,.8f,0,0,0);o=sc_translate(&c,&s,p,1300);int yaw=o.rx;assert(o.lx==0 && sc_abs(yaw)>24000);
 p=feet(&c,sw,0,0,.8f,0);o=sc_translate(&c,&s,p,1400);assert(o.lx==0 && o.rx*yaw<0);
 /* Neutral immediately clears lean; bump overrides it and keeps exit buttons masked. */
 p=(sc_pad){0};o=sc_translate(&c,&s,p,1500);assert(!o.lx && !o.rx && !o.ry);
 p=feet(&c,sw,.8f,0,.8f,0);p.buttons=SC_LTHUMB;o=sc_translate(&c,&s,p,1600);assert(o.lx<0 && !(o.buttons&SC_LTHUMB));
 /* No injected lean in flight or stock/radial passthrough. */
 s=(sc_state){0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;p=feet(&c,sw,.8f,0,.8f,0);o=sc_translate(&c,&s,p,2000);assert(s.split_grind_active && o.lx==0);
 c.enabled=0;p=(sc_pad){0};p.lx=12345;p.rx=-21000;o=sc_translate(&c,&s,p,2100);assert(o.lx==p.lx && o.rx==p.rx && !s.split_grind_active);
 c.enabled=1;s=(sc_state){0};s.grind=1;p.buttons=1;o=sc_translate(&c,&s,p,2200);assert(o.lx==p.lx && o.rx==p.rx && !s.split_grind_active);
 /* Existing settings preserve their previous output when the mode is off. */
 c.split_grinds=0;s=(sc_state){0};s.grind=1;s.switched=sw;s.game_crouch=-1;p=feet(&c,sw,.8f,0,.8f,0);o=sc_translate(&c,&s,p,2400);assert(!s.split_grind_active && o.lx==0 && o.rx==0);
 count++;
 }
 printf("PASS: %d split grind layouts: no automatic lean, full foot pivot, both feet, bump priority and passthrough\n",count);
}
