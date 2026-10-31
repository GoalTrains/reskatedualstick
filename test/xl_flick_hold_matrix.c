#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static void put(sc_pad*p,int left,short x,short y){if(left){p->lx=x;p->ly=y;}else{p->rx=x;p->ry=y;}}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int sw=0;sw<2;sw++)for(int mirror=0;mirror<2;mirror++)
 for(int native=0;native<2;native++)for(int nollie=0;nollie<2;nollie++)for(int side=-1;side<=1;side+=2)for(int shape=0;shape<3;shape++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.xl_layout=c.split_grinds=c.one_foot_flips=c.pop_on_release=1;c.goofy=g;c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.grind_sensitivity=1.2f;
  sc_state s={0};s.switched=sw;s.game_crouch=0;s.game_known=1;s.game_mirror_known=1;s.game_mirror=mirror;s.game_switch_known=1;s.game_switch=native;
  sc_pad p={0};sc_translate(&c,&s,p,1000);int front=(!g)!=sw,physical_y=sw?-1:1;
  put(&p,nollie?front:!front,0,(nollie?30000:-30000)*physical_y);sc_translate(&c,&s,p,1100);sc_translate(&c,&s,p,1200);
  float own=shape==0?-.3f:shape==1?0:.45f;
  put(&p,nollie?!front:front,side*30000,sc_short((nollie?-own:own)*physical_y));
  sc_pad before=sc_translate(&c,&s,p,1300);s.takeoff=s.air=1;
  sc_pad after=sc_translate(&c,&s,p,1320);
  float a=sc_sqrt((float)before.rx*before.rx+(float)before.ry*before.ry),b=sc_sqrt((float)after.rx*after.rx+(float)after.ry*after.ry);
  assert(s.hold_foot && a>20000 && b>20000);
  assert(((float)before.rx*after.rx+(float)before.ry*after.ry)/(a*b)>.99f);count++;
 }
 printf("PASS: %d XL clamped flick holds retain the native endpoint across takeoff: both stances, riding/input/mirror frames, trick ends, directions and flick shapes\n",count);
}
