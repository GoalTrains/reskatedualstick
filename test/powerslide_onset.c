#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void){for(int g=0;g<2;g++)for(int dir=-1;dir<=1;dir+=2){sc_config c={0};c.enabled=c.pure=c.powerslide=1;c.goofy=g;c.front_back=1;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.steer_curve=c.steer_scale=1;sc_state s={0};sc_pad p={0};sc_translate(&c,&s,p,1000);
 p.lx=sc_short(dir*.32f);p.rx=sc_short(-dir*.38f);sc_pad o=sc_translate(&c,&s,p,1020);assert(o.rx==0 && o.ry==0);
 p.lx=sc_short(dir*.48f);p.rx=sc_short(-dir*.58f);p.ry=sc_short(-.05f);o=sc_translate(&c,&s,p,1024);assert(s.ps_side && o.ly< -20000 && o.rx==0 && o.ry==0);
 }puts("PASS: opposing slide onset reserves soft intermediate polls; recorded 48/58-percent gesture activates without a preceding synthetic flick, both stances/directions");}
