#include <assert.h>
#include <stdio.h>
#include "../src/stance_track.h"
static int step(sc_stance_track* t,int state,int raw,double time){
 return sc_resolve_stance(t,4,0,1,1,state,raw,0,0,time);
}
int main(void){
 for(int initial=0;initial<2;initial++){
  sc_stance_track t={0};t.direct_neutral=1;
  assert(step(&t,100,initial,0)==initial);
  assert(step(&t,100,!initial,10)==initial);
  assert(step(&t,100,!initial,29)==initial);
  assert(step(&t,100,!initial,30)==!initial);
  /* A transient native flag does not swap neutral foot jobs. */
  assert(step(&t,100,initial,40)==!initial);
  assert(step(&t,100,!initial,44)==!initial);
  assert(step(&t,100,initial,50)==!initial);
  assert(step(&t,100,initial,69)==!initial);
  assert(step(&t,100,initial,70)==initial);
  /* Active input retains its original confirmation time. */
  t.direct_neutral=0;
  assert(step(&t,100,!initial,80)==initial);
  assert(step(&t,100,!initial,100)==initial);
  assert(step(&t,100,!initial,154)==initial);
  assert(step(&t,100,!initial,155)==!initial);
  /* Beginning input while waiting removes the shorter threshold. */
  t.direct_neutral=1;assert(step(&t,100,initial,200)==!initial);
  t.direct_neutral=0;assert(step(&t,100,initial,220)==!initial);
  assert(step(&t,100,initial,275)==initial);
  /* Takeoff/slide state handling is unchanged. */
  assert(step(&t,200,!initial,280)==!initial);
  assert(step(&t,101,initial,284)==initial);
 }
 puts("PASS: neutral stance confirms in 20 ms; transient flags rejected; held controls retain 75 ms; air and slide unchanged");
 return 0;
}
