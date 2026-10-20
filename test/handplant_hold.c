#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void){
 int cases=0;
 for(int goofy=0;goofy<2;goofy++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)for(int hand=0;hand<2;hand++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.steer_scale=c.steer_curve=1;c.grab_mode=1;c.grab_double_ms=300;c.grab_hold_ms=100;c.goofy=goofy;c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=3;
  sc_state s={0};s.game_crouch=-1;s.switched=sw;sc_pad p={0};sc_translate(&c,&s,p,1000);
  unsigned short bit=hand?SC_RSHOULDER:SC_LSHOULDER;p.buttons=bit;
  sc_pad o=sc_translate(&c,&s,p,1020);assert((o.buttons&bit)&&!o.lt&&!o.rt);
  o=sc_translate(&c,&s,p,1400);assert((o.buttons&bit)&&!o.lt&&!o.rt);
  s.air=s.takeoff=1;o=sc_translate(&c,&s,p,1416);assert((o.buttons&bit)&&!o.lt&&!o.rt); /* Held handplant survives the lip. */
  p.buttons=0;sc_translate(&c,&s,p,1432);p.buttons=bit;o=sc_translate(&c,&s,p,1450);assert(!(o.buttons&bit)&&!o.lt&&!o.rt);
  o=sc_translate(&c,&s,p,1549);assert(!(o.buttons&bit)&&!o.lt&&!o.rt);
  o=sc_translate(&c,&s,p,1550);assert(!(o.buttons&bit)&&(hand?o.rt==255:o.lt==255));
  p.buttons=0;sc_translate(&c,&s,p,1600);p.buttons=bit;sc_translate(&c,&s,p,1700);p.buttons=0;o=sc_translate(&c,&s,p,1730);assert(o.buttons&bit); /* Quick air dark-catch tap remains. */
  cases++;
 }
 printf("PASS: %d single-ground-hold handplants, lip continuity, fresh airborne hold grabs, and quick dark-catch taps across layouts/stances/hands\n",cases);return 0;
}
