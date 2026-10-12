#include <assert.h>
#include <stdio.h>
#include "../src/stance_track.h"
#include "../src/translate.h"
static sc_stance_track t;
static int sw(int state,int raw,double now) {
    return sc_resolve_stance(&t,4,1,3,0,state,raw,0,0,now);
}
static void mapping(int switched) {
    sc_config c={0};sc_state s={0};sc_pad p={0};
    c.enabled=c.pure=c.air_controls=1;c.goofy=1;c.switch_flip=3;
    c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
    s.switched=switched;s.game_crouch=-1;
    sc_translate(&c,&s,p,1000);
    if(switched)p.ry=-32767;else p.ly=-32767;
    sc_pad crouch=sc_translate(&c,&s,p,1020);
    assert(s.feet_map_left==switched && crouch.ry<-30000);
    if(switched)p.ly=32767;else p.ry=32767;
    sc_pad pop=sc_translate(&c,&s,p,1100);
    assert(pop.ry>30000);
}
int main(void) {
    assert(sw(100,0,45340000)==0);
    assert(sw(200,0,45343000)==0);
    assert(sw(201,1,45343690)==1);
    assert(sw(100,1,45343700)==1);
    /* Report's ground reset after several no-turn hops, at 45349.287s. */
    assert(sw(103,1,45345000)==1);
    assert(sw(200,1,45345100)==1);
    assert(sw(100,1,45345500)==1);
    assert(sw(200,1,45346000)==1);
    assert(sw(100,1,45346100)==1);
    assert(sw(200,1,45348000)==1);
    assert(sw(100,1,45348500)==1);
    assert(sw(100,0,45349287)==1);
    mapping(1);
    /* Next 180 returns to normal; a 360 returns to its takeoff stance. */
    assert(sw(200,0,45350000)==1);
    assert(sw(201,1,45350100)==0);
    assert(sw(100,1,45350500)==0);
    assert(sw(100,0,45351000)==0);
    mapping(0);
    assert(sw(200,0,45352000)==0);
    assert(sw(201,1,45352100)==1);
    assert(sw(201,0,45352200)==1);
    assert(sw(201,0,45352216)==0);
    assert(sw(100,0,45352300)==0);
    /* Touchdown blip: 45449.085 -> 45449.089. */
    assert(sw(200,0,45448000)==0);
    assert(sw(201,1,45448853)==1);
    assert(sw(100,0,45449085)==0);
    assert(sw(100,1,45449089)==1);
    assert(sw(100,0,45450569)==1);
    mapping(1);
    /* Goofy revert, and stale state discarded on a real spawn. */
    assert(sw(101,0,45452000)==1);
    assert(sw(101,1,45452100)==1);
    assert(sw(100,1,45452200)==0);
    assert(sc_resolve_stance(&t,4,1,3,0,504,0,1,1,45453000)==0);
    /* Regular / other modes / front-back keep their direct source behavior. */
    assert(sc_resolve_stance(&t,4,0,1,0,100,1,0,0,1)==1);
    assert(sc_resolve_stance(&t,4,0,1,0,100,0,0,1,2)==1);
    assert(sc_resolve_stance(&t,4,0,1,0,100,0,0,1,80)==0);
    assert(sc_resolve_stance(&t,4,1,1,0,100,0,0,1,3)==0);
    assert(sc_resolve_stance(&t,4,1,3,1,100,0,0,1,4)==0);
    puts("PASS: goofy True Foot turn/reset sequences, no-turn hops, second 180, 360, touchdown blip, revert, spawn, physical switch-ollie sticks and unchanged direct routes");
}
