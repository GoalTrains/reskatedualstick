#include <assert.h>
#include <stdio.h>
#include "../src/stance_track.h"
#include "../src/translate.h"
static int step(sc_stance_track*t,int g,int state,int raw,double now){return sc_resolve_stance(t,4,g,3,0,state,raw,0,0,now);}
int main(void){for(int g=0;g<2;g++)for(int start=0;start<2;start++){
 sc_stance_track t={0};assert(step(&t,g,100,start,1000)==start);
 step(&t,g,200,start,1100);step(&t,g,201,!start,1180);assert(step(&t,g,201,!start,1200)==!start);
 assert(step(&t,g,100,!start,1300)==!start);
 assert(step(&t,g,100,0,2500)==!start); /* Wait until native fakie flag resets. */
 assert(step(&t,g,103,0,2600)==!start);
 assert(step(&t,g,504,0,2700)==!start);assert(step(&t,g,504,0,2800)==!start);
 assert(step(&t,g,103,0,2900)==!start);assert(step(&t,g,100,0,3000)==!start);
 sc_config c={0};c.enabled=c.pure=1;c.goofy=g;c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
 sc_state s={0};s.switched=t.stance;sc_translate(&c,&s,(sc_pad){0},3100);assert(s.feet_map_left==((!g)!=(!start)));
 step(&t,g,200,0,3200);assert(step(&t,g,201,1,3300)==start);assert(step(&t,g,100,1,3400)==start);
 step(&t,g,700,0,3500);assert(step(&t,g,504,0,3600)==0);assert(step(&t,g,100,0,3700)==0); /* Real respawn still resets. */
 }puts("PASS: regular/goofy completed 180 retained through fakie flag reset, walking and 103 remount; physical foot jobs, next turn, and full respawn reset");}
