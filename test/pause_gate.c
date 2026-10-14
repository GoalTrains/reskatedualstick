#include <assert.h>
#include <stdio.h>
#include "../src/pause_gate.h"
int main(void) {
 /* Recorded 1.0–1.4-second hitches falsely reset foot state in gameplay. */
 const double gaps[]={0,199,201,1056,1256,1430,10000};
 for(unsigned i=0;i<sizeof gaps/sizeof gaps[0];i++) {
  assert(!sc_pause_gate(0,1,gaps[i]));
  assert(!sc_pause_gate(0,0,gaps[i]));
  assert(sc_pause_gate(1,1,gaps[i]));
  assert(sc_pause_gate(1,0,gaps[i]));
  assert(sc_pause_gate(-1,1,gaps[i])==(gaps[i]>200));
  assert(!sc_pause_gate(-1,0,gaps[i]));
 }
 assert(sc_pause_gate(-1,1,300));
 assert(!sc_pause_gate(0,1,300)); /* Native closure restores gameplay immediately. */
 puts("PASS: recorded gameplay hitches cannot become menu pauses; explicit native menus still yield input; unavailable-model fallback preserves its 200-ms threshold and unknown-actor handling");
}
