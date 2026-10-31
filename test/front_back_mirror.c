#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void){int count=0;
 for(int goofy=0;goofy<2;goofy++)for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2)for(int nollie=0;nollie<2;nollie++){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.front_back=1;c.switch_flip=1;c.goofy=goofy;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
 sc_state a={0},b={0};a.switched=b.switched=sw;a.game_crouch=b.game_crouch=0;a.game_mirror_known=b.game_mirror_known=1;b.game_mirror=1;
 sc_pad p={0};sc_translate(&c,&a,p,1000);sc_translate(&c,&b,p,1000);
 if(nollie)p.ly=32767;else p.ry=-32767;
 sc_translate(&c,&a,p,1100);sc_translate(&c,&b,p,1100);
 if(nollie){p.rx=side*23166;p.ry=-23166;}else{p.lx=side*23166;p.ly=23166;}
 sc_pad x=sc_translate(&c,&a,p,1250),y=sc_translate(&c,&b,p,1250);
 assert((long)x.rx*y.rx<0 && sc_abs((float)x.rx+y.rx)<=2 && x.ry==y.ry);
 a.air=b.air=a.takeoff=b.takeoff=1;
 x=sc_translate(&c,&a,p,1300);y=sc_translate(&c,&b,p,1300);
 assert(sc_abs((float)x.rx+y.rx)<=2 && x.ry==y.ry);count++;
 }
 printf("PASS: %d Front/Back flip gestures compensate the native mirror in both stances/switch/trick ends and retain the emitted held endpoint at takeoff\n",count);
}
