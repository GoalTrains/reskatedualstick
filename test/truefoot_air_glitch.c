#include <assert.h>
#include <stdio.h>
#include "../src/stance_track.h"
int main(void){for(int goofy=0;goofy<2;goofy++){
 sc_stance_track t={0};
 #define STEP(st,raw,ms) sc_resolve_stance(&t,4,goofy,3,0,st,raw,0,0,ms)
 assert(STEP(100,0,1000)==0);assert(STEP(200,0,1100)==0);
 assert(STEP(201,1,1200)==1);assert(STEP(100,1,1400)==1);
 assert(STEP(200,1,2000)==1);assert(STEP(201,0,2090)==1);
 assert(STEP(201,1,2094)==1);assert(STEP(100,1,2300)==1);
 /* A real 180 back to normal is confirmed, not discarded. */
 assert(STEP(200,1,2400)==1);assert(STEP(201,0,2490)==1);
 assert(STEP(201,0,2506)==0);assert(STEP(100,0,2700)==0);
 assert(STEP(100,1,3400)==0); /* unrelated ground reset */
 #undef STEP
 }puts("PASS: both stances ignore 4-ms airborne reset; sustained turn, landing and ground reset preserved");}
