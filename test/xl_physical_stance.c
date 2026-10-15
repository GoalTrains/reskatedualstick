#include <assert.h>
#include <stdio.h>
#include "../src/stance_track.h"
#include "../src/xl_stance_agreement.h"
static int step(sc_stance_track*t,int profile,int state,int native,int mirror,int baseline,double now){
 return sc_resolve_stance(t,4,profile,3,0,state,sc_xl_physical_raw(native,mirror,baseline),0,0,now);
}
int main(void){
 for(int profile=0;profile<2;profile++)for(int baseline=0;baseline<2;baseline++){
  sc_stance_track t={0};
  assert(step(&t,profile,100,0,baseline,baseline,0)==0);
  assert(step(&t,profile,103,0,baseline,baseline,100)==0);
  assert(step(&t,profile,201,1,baseline,baseline,150)==1);
  assert(step(&t,profile,100,1,baseline,baseline,500)==1);
  /* A paired fakie-timer/mirror change on the ground is not a turn. */
  assert(step(&t,profile,100,0,!baseline,baseline,1000)==1);
  assert(step(&t,profile,103,0,!baseline,baseline,1100)==1);
  assert(step(&t,profile,201,0,!baseline,baseline,1200)==1);
  assert(step(&t,profile,100,0,!baseline,baseline,1500)==1);
  assert(step(&t,profile,103,0,!baseline,baseline,2000)==1);
  assert(step(&t,profile,201,1,!baseline,baseline,2100)==1);
  assert(step(&t,profile,201,1,!baseline,baseline,2120)==0);
  assert(step(&t,profile,100,1,!baseline,baseline,2500)==0);
  assert(step(&t,profile,100,0,baseline,baseline,3000)==0);
 }
 assert(sc_xl_physical_raw(-1,0,0)==-1 && sc_xl_physical_raw(1,2,0)==-1);
 puts("PASS: recorded timer/mirror resets and later 180s retain anatomical foot jobs, both profiles and both animation mirror baselines; native output flag remains separate");
}