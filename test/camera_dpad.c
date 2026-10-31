#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "../src/camera_dpad.h"
#include "../src/camera_side_read.h"
static unsigned short step(sc_camera_dpad* s,unsigned short b,double t,int* r){return sc_camera_dpad_step(s,b,t,1,r);}
int main(void){
 sc_camera_dpad s={0};int r;
 step(&s,0,1,&r);assert(step(&s,4,10,&r)==0 && !r);assert(step(&s,0,90,&r)==0 && r==1);
 assert(step(&s,8,92,&r)==0 && !r);assert(step(&s,0,98,&r)==0 && r==-1);
 step(&s,8,100,&r);for(int t=200;t<600;t+=100)assert(step(&s,8,t,&r)==0 && !r);
 assert(step(&s,8,599,&r)==0 && !r);assert(step(&s,8,600,&r)==8 && !r);
 assert(step(&s,8,650,&r)==8 && !r);step(&s,0,700,&r);assert(!r);
 step(&s,4,800,&r);for(int t=900;t<1300;t+=100)step(&s,4,t,&r);step(&s,0,1300,&r);assert(!r);
 step(&s,4,1400,&r);step(&s,12,1410,&r);step(&s,0,1420,&r);assert(!r);
 step(&s,4,1500,&r);step(&s,0x104,1510,&r);step(&s,0,1520,&r);assert(!r);
 step(&s,4,1600,&r);sc_camera_dpad_suspend(&s,4);assert(step(&s,4,1700,&r)==4);step(&s,0,1800,&r);assert(!r);
 step(&s,4,1900,&r);assert(step(&s,4,2900,&r)==4 && !r);step(&s,0,3000,&r);assert(!r);
 assert(sc_camera_dpad_step(&s,8,3100,0,&r)==8 && !r);step(&s,0,3200,&r);assert(!r);
 step(&s,1,3300,&r);assert(step(&s,2,3310,&r)==2 && !r);
 /* Push remains held and reaches Skate while either camera side is tapped. */
 const unsigned pushes[]={0x1000,0x4000,0x5000};
 for(unsigned pi=0;pi<3;pi++){
  unsigned push=pushes[pi];
  for(unsigned side=4;side<=8;side+=4){
   sc_camera_dpad p={0};step(&p,push,4000,&r);
   assert(step(&p,push|side,4010,&r)==push && !r);
   assert(step(&p,push,4100,&r)==push && r==(side==4?1:-1));
  }
 }
 {sc_camera_dpad p={0};step(&p,0,4000,&r);
  step(&p,0x4004,4010,&r);step(&p,0x4004,4510,&r);
  assert(step(&p,0x4000,4520,&r)==0x4000 && !r);
  step(&p,0x4104,4600,&r);step(&p,0x4000,4650,&r);assert(!r);}
 assert(sc_camera_dpad_toggle(&s,-1,1,6500)==1);assert(sc_camera_dpad_toggle(&s,-1,1,6600)==0);
 assert(sc_camera_dpad_toggle(&s,1,1,6700)==1);assert(sc_camera_dpad_toggle(&s,1,1,8000)==0);
 assert(sc_camera_dpad_toggle(&s,-1,0,10000)==0);
 /* Physical L3 cannot toggle the camera while remapped, including sustained
  * holds and alongside R3/other buttons. D-pad pulses still reach native L3;
  * disabling/removing gameplay ownership returns the translated pad intact. */
 for(unsigned buttons=0;buttons<65536;buttons++) {
  assert(sc_camera_dpad_output_buttons(buttons,0,0)==buttons);
  assert(sc_camera_dpad_output_buttons(buttons,1,0)==(buttons&~0x0040u));
  assert(sc_camera_dpad_output_buttons(buttons,1,1)==(buttons|0x0040u));
 }
 float p[3]={0,0,0},f[3]={0,0,1},c[3]={1,1,-3};
 assert(sc_camera_side_geometry(p,f,c)==1);c[0]=-1;assert(sc_camera_side_geometry(p,f,c)==-1);
 f[2]=-1;assert(sc_camera_side_geometry(p,f,c)==-1);f[2]=1;
 c[0]=0;assert(sc_camera_side_geometry(p,f,c)==0);c[0]=100;assert(sc_camera_side_geometry(p,f,c)==0);
 c[0]=NAN;assert(sc_camera_side_geometry(p,f,c)==0);
 puts("PASS: camera tap/hold threshold, cancellation, suspend, gaps, repeated absolute requests, switch-axis geometry and invalid views");
 return 0;
}
