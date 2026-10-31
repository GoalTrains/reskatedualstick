#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad pair(int inward,float degrees){
 sc_pad p={0};float a=(inward?90:270)+degrees,b=(inward?270:90)-degrees;
 p.lx=sc_short(.95f*sc_sin_deg(a));p.ly=sc_short(-.95f*sc_sin_deg(90-a));
 p.rx=sc_short(.95f*sc_sin_deg(b));p.ry=sc_short(-.95f*sc_sin_deg(90-b));return p;
}
int main(void){int cases=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int inward=0;inward<2;inward++)for(int dir=-1;dir<=1;dir+=2){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.powerslide=1;c.goofy=g;
 c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=layout?3:1;
 c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.revert_input=3;c.revert_ms=300;
 c.revert_shape=5;c.revert_sweep_ms=300;sc_state s={0};s.switched=sw;s.game_crouch=-1;
 sc_pad p=pair(inward,0);sc_translate(&c,&s,p,1000);assert(s.slide_revert.armed);
 for(int i=1;i<=4;i++){p=pair(inward,dir*i*12);sc_translate(&c,&s,p,1000+i*20);assert(!s.rv_start && s.ps_side);}
 p=pair(inward,dir*60);sc_translate(&c,&s,p,1100);assert(s.rv_start==1100 && s.rv_side==-dir);cases++;
 }
 printf("PASS: %d mirrored slide rotations stay slides beyond 45 degrees, then request a revert; shallow angle adjustment does not fire\n",cases);
}
