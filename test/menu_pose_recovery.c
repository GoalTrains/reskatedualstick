#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/menu_pose_recovery.h"
int main(void) {
 sc_menu_pose_recovery p={0};sc_pad raw={0};
 for(int flag=0;flag<2;flag++) {
  sc_menu_pose_enter(&p,123,100,flag,1);
  assert(!sc_menu_pose_exit(&p,123,100,flag,1000));
  sc_menu_pose_enter(&p,123,100,flag,1);
  assert(sc_menu_pose_exit(&p,123,100,!flag,1000));
  raw.buttons=0x2000;assert(!sc_menu_pose_step(&p,123,100,1,0,1001,&raw));
  raw.buttons=0;assert(sc_menu_pose_step(&p,123,100,1,0,1050,&raw));
  assert(sc_menu_pose_step(&p,123,100,1,0,1169,&raw));
  assert(!sc_menu_pose_step(&p,123,100,1,0,1170,&raw));
  assert(!sc_menu_pose_step(&p,123,100,1,0,1180,&raw));
 }
 const int badstates[]={103,200,201,400,403,504,300,700};
 for(unsigned i=0;i<sizeof badstates/sizeof badstates[0];i++) {
  sc_menu_pose_enter(&p,123,100,1,1);assert(sc_menu_pose_exit(&p,123,100,0,0));
  assert(!sc_menu_pose_step(&p,123,badstates[i],1,0,100,&raw));assert(!p.pending);
 }
 sc_menu_pose_enter(&p,123,100,1,0);assert(!sc_menu_pose_exit(&p,123,100,0,0));
 sc_menu_pose_enter(&p,123,100,-1,1);assert(!sc_menu_pose_exit(&p,123,100,0,0));
 sc_menu_pose_enter(&p,123,100,1,1);assert(!sc_menu_pose_exit(&p,456,100,0,0));
 for(int cancel=0;cancel<4;cancel++) {
  sc_menu_pose_enter(&p,123,100,1,1);assert(sc_menu_pose_exit(&p,123,100,0,0));
  assert(!sc_menu_pose_step(&p,cancel==0?456:123,100,cancel!=1,cancel==2,cancel==3?1500:100,&raw));
  assert(!p.pending);
 }
 sc_menu_pose_enter(&p,123,100,1,1);assert(sc_menu_pose_exit(&p,123,100,0,0));
 raw.rx=8000;assert(!sc_menu_pose_step(&p,123,100,1,0,100,&raw));
 raw.rx=0;raw.lt=30;assert(!sc_menu_pose_step(&p,123,100,1,0,200,&raw));
 raw.lt=0;assert(sc_menu_pose_step(&p,123,100,1,0,300,&raw));
 raw.buttons=4096;assert(!sc_menu_pose_step(&p,123,100,1,0,320,&raw));
 assert(!p.pulsing && raw.buttons==4096);
 puts("PASS: menu-frame mismatch only, held exit button release, one bounded brake pulse, grounded idle gate, immediate input cancellation, same skater, timeout, disabled/unknown/menu/air/grind/walking excluded");
}
