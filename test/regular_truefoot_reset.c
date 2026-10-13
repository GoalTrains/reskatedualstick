#include <assert.h>
#include <stdio.h>
#include "../src/stance_track.h"
static sc_stance_track t;
static int step(int state,int raw,double ms){return sc_resolve_stance(&t,4,0,3,0,state,raw,0,0,ms);}
int main(void){
 assert(step(100,0,1000)==0);assert(step(200,0,1100)==0);
 assert(step(201,1,1200)==1);assert(step(101,1,1500)==1);assert(step(100,1,1510)==1);
 /* DTG's game clears its flag around 1.4 seconds after touchdown. */
 assert(step(100,0,2950)==1);assert(step(100,0,4000)==1);
 assert(step(103,0,4100)==1);assert(step(200,0,4200)==1);assert(step(100,0,4500)==1);
 assert(step(100,0,5100)==1); /* no-turn hop */
 assert(step(200,0,5200)==1);assert(step(201,1,5300)==0);assert(step(100,1,5600)==0);
 assert(step(100,0,7050)==0); /* second 180 + another reset */
 assert(step(101,0,7100)==0);assert(step(101,1,7200)==0);assert(step(100,1,7300)==1);
 assert(step(100,0,8800)==1); /* revert retained */
 assert(step(504,0,9000)==1);assert(step(100,0,9200)==1);
 assert(step(700,0,9400)==0);assert(step(504,0,9500)==0);assert(step(100,0,9600)==0);
 puts("PASS: regular True Foot retains completed 180/revert through late ground reset; no-turn hop, next turn and normal remount retained and full respawn reset preserved");}
