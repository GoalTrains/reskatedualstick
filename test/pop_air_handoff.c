#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad feet(sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout&&sw){fy=-fy;by=-by;}int left=c->front_back?1:(!c->goofy)!=sw;
 sc_pad p={0};p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
static sc_state popped(sc_config*c,int sw){
 sc_state s={0};s.game_known=1;s.game_crouch=0;s.switched=sw;
 sc_translate(c,&s,feet(c,sw,0,0,0,-1),1000);
 sc_translate(c,&s,feet(c,sw,0,1,0,-1),1100);
 assert(s.split_pop_t==1100);
 sc_translate(c,&s,(sc_pad){0},1170);s.game_crouch=1;return s;
}
int main(void){int checks=0;
 for(int g=0;g<2;g++)for(int l=0;l<3;l++)for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=l==0;c.xl_layout=l==2;c.switch_flip=c.front_back?1:3;c.windup_deg=60;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;
 sc_state s=popped(&c,sw);sc_pad in=feet(&c,sw,0,0,side,0);
 for(int t=1200;t<=1300;t+=4){sc_pad o=sc_translate(&c,&s,in,t);assert(!o.rx&&!o.ry&&!s.d_windup);checks++;}
 /* Confirmed takeoff immediately gets the existing full rear setup. */
 s.takeoff=1;s.game_crouch=0;sc_pad o=sc_translate(&c,&s,in,1304);assert(o.rx && !s.split_pop_t && s.split_grind_active);
 sc_state reference={0};reference.game_known=1;reference.game_crouch=0;reference.switched=sw;reference.takeoff=1;
 sc_pad expected=sc_translate(&c,&reference,in,1304);assert(o.rx==expected.rx&&o.ry==expected.ry);
 /* Crouching alone never arms this handoff; normal one-foot ground input stays live. */
 sc_state plain={0};plain.game_known=plain.game_crouch=1;plain.switched=sw;
 sc_translate(&c,&plain,in,1200);o=sc_translate(&c,&plain,in,1260);assert(o.rx&&!plain.split_pop_t);
 /* An unaccepted pop cannot indefinitely block a fresh ground gesture. */
 s=popped(&c,sw);sc_translate(&c,&s,in,1200);o=sc_translate(&c,&s,in,1452);assert(!s.split_pop_t);o=sc_translate(&c,&s,in,1512);assert(o.rx);
 /* Other-foot participation, vertical setups and mode changes use normal handling. */
 s=popped(&c,sw);o=sc_translate(&c,&s,feet(&c,sw,0,1,side,0),1200);assert(o.rx||o.ry);
 s=popped(&c,sw);o=sc_translate(&c,&s,feet(&c,sw,0,0,0,-1),1200);assert(o.ry);
 s=popped(&c,sw);c.one_foot_flips=1;sc_translate(&c,&s,in,1200);assert(!s.split_pop_t);c.one_foot_flips=0;
 s=popped(&c,sw);c.enabled=0;sc_translate(&c,&s,in,1200);assert(!s.split_pop_t);c.enabled=1;
 s=popped(&c,sw);sc_remap_foot_history(&s);assert(!s.split_pop_t);
 s=popped(&c,sw);in.buttons=SC_DPAD_UP;sc_translate(&c,&s,in,1200);assert(!s.split_pop_t);
 }
 printf("PASS: %d crouch-to-air samples, existing air endpoints, ordinary crouches, expiry, opposite-foot/vertical gestures and lifecycle guards across layouts and stances\n",checks);
}
