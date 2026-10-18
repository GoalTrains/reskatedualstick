#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad feet(sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout && sw){fy=-fy;by=-by;}int left=c->front_back?1:(!c->goofy)!=sw;
 sc_pad p={0};p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int l=0;l<3;l++)for(int sw=0;sw<2;sw++)for(int sign=-1;sign<=1;sign+=2){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=l==0;c.xl_layout=l==2;c.switch_flip=c.front_back?1:3;
 c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1;
 for(int back=0;back<2;back++){
  sc_state a={0};a.air=1;a.takeoff=2;a.switched=sw;a.game_crouch=-1;
  sc_pad p=feet(&c,sw,back?0:sign*.8f,0,back?sign*.8f:0,0);sc_pad air=sc_translate(&c,&a,p,1000);
  a.air=0;a.takeoff=0;a.grind=1;sc_pad rail=sc_translate(&c,&a,p,1100);
  assert(!a.grind_pop && rail.lx==0 && rail.ry==0);
  assert(rail.rx==air.rx);
  /* Vertical-only control and neutral are unchanged by the X correction. */
  a=(sc_state){0};a.grind=1;a.switched=sw;a.game_crouch=-1;
  p=feet(&c,sw,0,back?0:.6f,0,back?-.6f:0);rail=sc_translate(&c,&a,p,1200);assert(rail.rx==0 && sc_abs(rail.ry)>12000);
 }
 count++;
 }
 puts("PASS: 24 both-foot rail reference cases: same mapping before/after contact, pre-reversal directions restored, all layouts/stances/directions, vertical and neutral unchanged");
}
