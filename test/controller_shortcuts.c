#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/controller_shortcuts.h"
int main(void) {
 const unsigned short pair=SC_LTHUMB|SC_RTHUMB,master=pair|SC_LSHOULDER|SC_RSHOULDER;
 sc_controller_hold h={0};unsigned short consume;
 /* The captured unexpected 192-button hold must neither toggle nor swallow clicks. */
 for(int i=0;i<=1600;i++) {assert(!sc_controller_shortcut(&h,pair,121411798+i*4,3000,0,&consume));assert(!consume);}
 sc_controller_shortcut(&h,0,121418300,3000,0,&consume);
 for(int i=0;i<750;i++){assert(!sc_controller_shortcut(&h,master,121419000+i*4,3000,0,&consume));assert(consume==master);}
 assert(sc_controller_shortcut(&h,master,121422000,3000,0,&consume)==3);
 for(int i=1;i<200;i++)assert(!sc_controller_shortcut(&h,master,121422000+i*4,3000,0,&consume));
 sc_controller_shortcut(&h,0,121423000,3000,0,&consume);
 /* A menu/pause polling gap cannot count toward a held shortcut. */
 sc_controller_shortcut(&h,master,121423010,3000,0,&consume);
 assert(!sc_controller_shortcut(&h,master,121430010,3000,0,&consume));
 sc_controller_hold other={0};assert(!sc_controller_shortcut(&other,master,121430010,3000,0,&consume));
 for(int i=0;i<=99;i++)assert(!sc_controller_shortcut(&other,pair|SC_LSHOULDER,121431000+i*4,3000,0,&consume));
 assert(sc_controller_shortcut(&other,pair|SC_LSHOULDER,121431400,3000,0,&consume)==2);
 other=(sc_controller_hold){0};
 for(int i=0;i<100;i++)assert(!sc_controller_shortcut(&other,pair,121432000+i*4,3000,1,&consume));
 assert(sc_controller_shortcut(&other,pair,121432400,3000,1,&consume)==1);
 /* Adding bumpers late must start a new three-second master hold. */
 other=(sc_controller_hold){0};for(int i=0;i<=1000;i++)assert(!sc_controller_shortcut(&other,pair,121433000+i*4,3000,0,&consume));
 assert(!sc_controller_shortcut(&other,master,121437004,3000,0,&consume));
 sc_state states[4]={0};sc_controller_hold holds[4]={0};
 for(int g=0;g<2;g++) {
  for(int i=0;i<4;i++){states[i].pump_r3=1;states[i].d_front_spent=states[i].d_back_spent=1;states[i].d_windup=1;holds[i].done=1;holds[i].pair_held=1;}
  sc_respawn_input_reset(states,holds,4);
  for(int i=0;i<4;i++) {
   assert(!states[i].pump_r3 && !states[i].d_front_spent && !states[i].d_back_spent && !states[i].d_windup && !holds[i].done && !holds[i].pair_held);
   assert(states[i].d_front_ignored && states[i].d_back_ignored);
   sc_config c={0};c.enabled=1;c.pure=1;c.air_controls=1;c.body_flips=1;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.goofy=g;
   sc_pad p={0};sc_translate(&c,&states[i],p,1000);p.buttons=SC_RTHUMB;
   sc_pad out=sc_translate(&c,&states[i],p,1010);assert(states[i].pump_r3 && out.rt>0 && !states[i].fired);
  }
 }
 puts("PASS: captured unexpected both-stick-click hold cannot toggle or swallow clicks; full bumper/click chord, single activation/release, polling gaps and per-user holds; legacy menu/feet commands retained; respawn clears stale gestures and fresh R3 crouch works regular/goofy");
}
