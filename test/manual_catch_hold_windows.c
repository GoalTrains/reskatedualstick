#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/manual_catch.h"
int main(void){
 sc_config c={0};c.enabled=c.pure=c.manual_catch=1;
 static const int ids[]={-1,1,2,3,4,5,7,9,10,11,15,17,18,25,31,36,39,64};
 for(unsigned kind=1;kind<=2;kind++)for(unsigned i=0;i<sizeof ids/sizeof ids[0];i++){
  sc_state s={0};s.air=1;s.game_crouch=-1;sc_manual_catch m={0};m.phase=2;m.candidate_kind=(int)kind;m.x=.6f;m.y=-.8f;
  sc_pad out=sc_manual_catch_translate_ex(&c,&s,&m,(sc_pad){0},1000,200,ids[i]);
  assert(!m.hold_window_id && out.rx==sc_short(.6f) && out.ry==sc_short(-.8f));
 }
 puts("PASS: manual catch preserves the translated flick/scoop endpoint for all native trick IDs; no unsafe trick-specific angle substitution");
}
