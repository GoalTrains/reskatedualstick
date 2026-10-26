#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad feet(const sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->flip_x){fx=-fx;bx=-bx;}
 if(sw&&c->switch_invert_x){fx=-fx;bx=-bx;}
 if(sw&&c->switch_invert_y){fy=-fy;by=-by;}
 if(c->xl_layout&&!c->front_back&&c->switch_flip==3&&sw){fy=-fy;by=-by;}
 int left=c->front_back?1:!c->goofy;left^=c->feet_swapped;
 if(c->switch_flip==3&&sw&&!c->front_back)left^=1;
 sc_pad p={0};p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int mode=0;mode<4;mode++)for(int sw=0;sw<2;sw++)for(int swap=0;swap<2;swap++)for(int axes=0;axes<8;axes++)for(int release=0;release<2;release++)for(int pop=1;pop<=2;pop++)for(int steer=0;steer<3;steer++)for(int foot=1;foot<=2;foot++)for(int side=-1;side<=1;side+=2){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=mode;c.feet_swapped=swap;c.flip_x=axes&1;c.switch_invert_x=(axes>>1)&1;c.switch_invert_y=(axes>>2)&1;c.windup_deg=60;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;c.pop_on_release=release;c.stick_steer=steer;c.steer_scale=c.steer_curve=1;
 sc_state s={0};s.game_known=1;s.switched=sw;
 sc_translate(&c,&s,feet(&c,sw,0,pop==1?1:0,0,pop==2?-1:0),1000);
 sc_translate(&c,&s,feet(&c,sw,0,pop==1?1:0,0,pop==2?-1:0),1100);
 if(release){
  sc_pad pulse=sc_translate(&c,&s,(sc_pad){0},1110);s.game_crouch=1;
  sc_pad early=feet(&c,sw,pop==1?side:0,0,pop==2?side:0,0);
  sc_pad held=sc_translate(&c,&s,early,1140);assert(!held.rx && held.ry==pulse.ry);
  s.game_crouch=0;
 }
 else sc_translate(&c,&s,feet(&c,sw,0,1,0,-1),1110);
 sc_translate(&c,&s,(sc_pad){0},1185);s.game_crouch=1;
 sc_pad in=feet(&c,sw,foot==1?side:0,0,foot==2?side:0,0);
 for(int t=1200;t<1300;t+=10){sc_pad o=sc_translate(&c,&s,in,t);if(o.rx||o.ry||o.lx){printf("FAIL g=%d layout=%d mode=%d switch=%d swapped=%d axes=%d release=%d foot=%d side=%d pop=%d steer=%d t=%d out=%d,%d,%d\n",g,layout,mode,sw,swap,axes,release,foot,side,pop,steer,t,o.rx,o.ry,o.lx);return 1;}}
 s.takeoff=1;s.game_crouch=0;sc_pad o=sc_translate(&c,&s,in,1300);assert(o.rx && s.split_grind_active);
 sc_state ref={0};ref.game_known=1;ref.switched=sw;ref.takeoff=1;sc_pad expected=sc_translate(&c,&ref,in,1300);assert(o.rx==expected.rx&&o.ry==expected.ry);count++;
 }
 printf("PASS: %d setup/pop combinations: three layouts, four switch modes, both stances, normal/switch, swapped feet, mirrored flicks and custom switch axes, ollie/nollie flick/release pops, all stick steering modes, either foot and side; existing airborne endpoints preserved\n",count);
}
