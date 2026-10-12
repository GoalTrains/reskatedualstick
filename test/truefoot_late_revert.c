#include <assert.h>
#include <stdio.h>
#include "../src/stance_track.h"
#include "../src/translate.h"
static int step(sc_stance_track*t,int g,int st,int raw,double ms){return sc_resolve_stance(t,4,g,3,0,st,raw,0,0,ms);}
static void jobs(int g,int sw){sc_config c={0};c.enabled=c.pure=1;c.goofy=g;c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;sc_state s={0};s.switched=sw;s.game_crouch=-1;sc_pad p={0};sc_translate(&c,&s,p,1000);assert(s.feet_map_left==((!g)!=sw));}
int main(void){for(int g=0;g<2;g++)for(int start=0;start<2;start++)for(int raw=0;raw<2;raw++)for(int delay=0;delay<=400;delay+=100){
 sc_stance_track t={0};assert(step(&t,g,100,start,1000)==start);
 /* An earlier completed turn can leave the ground flag reset to either value. */
 step(&t,g,100,raw,1700);assert(t.stance==start);
 assert(step(&t,g,101,raw,1800)==start);
 assert(step(&t,g,101,raw,2000)==start);
 if(!delay){assert(step(&t,g,100,!raw,2100)==!start);}else{
 assert(step(&t,g,100,raw,2100)==start);
 assert(step(&t,g,100,!raw,2100+delay)==!start);assert(t.late_slide_ms==delay);
 }jobs(g,!start);
 assert(step(&t,g,100,raw,2700)==!start); /* reset must not undo it */
 assert(step(&t,g,101,raw,3000)==!start);
 assert(step(&t,g,100,raw,3300)==!start); /* powerslide/no actual turn */
 assert(step(&t,g,100,raw,3900)==!start);
 assert(step(&t,g,100,!raw,4000)==!start); /* expired late window */
 }for(int g=0;g<2;g++){
 sc_stance_track t={0};step(&t,g,100,0,1000);step(&t,g,101,0,1100);step(&t,g,100,0,1200);
 step(&t,g,200,0,1250);assert(t.pending_reference<0);assert(step(&t,g,201,1,1300)==1);
 assert(step(&t,g,101,1,1400)==1);assert(step(&t,g,100,1,1500)==1); /* landing into slide counted once */
 }puts("PASS: regular/goofy ground reverts both directions: same-frame and delayed flags, foot jobs, no-turn slides, resets, expiry and air transition");}
