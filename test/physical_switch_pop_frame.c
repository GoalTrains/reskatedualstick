#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static void foot(sc_pad*p,int left,short y){if(left)p->ly=y;else p->ry=y;}
int main(void){
 int count=0;
 for(int goofy=0;goofy<2;goofy++)for(int sw=0;sw<2;sw++)
 for(int frame=-1;frame<2;frame++)for(int release=0;release<2;release++)
 for(int nollie=0;nollie<2;nollie++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.goofy=goofy;
  c.switch_flip=3;c.pop_on_release=release;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
  sc_state s={0};s.switched=sw;s.game_crouch=-1;
  s.game_switch_known=frame>=0;s.game_switch=frame>=0?frame:0;
  sc_pad p={0};sc_translate(&c,&s,p,1000);
  int front_left=(!goofy)!=sw,physical_sign=nollie?1:-1;
  int native_sign=(frame<0?sw:frame)?-1:1;
  foot(&p,nollie?front_left:!front_left,physical_sign*30000);
  sc_pad setup=sc_translate(&c,&s,p,1100);
  assert(s.feet_map_left==front_left);
  assert(setup.ry*physical_sign*native_sign>28000);
  sc_translate(&c,&s,p,1300);
  if(release)p=(sc_pad){0};
  else foot(&p,nollie?!front_left:front_left,-physical_sign*30000);
  sc_pad pop=sc_translate(&c,&s,p,1400);
  assert(release?s.fired==1:s.d_log==2);
  assert(pop.ry*physical_sign*native_sign<-28000);
  count++;
 }
 printf("PASS: %d physical-foot ollie/nollie setups and Flick/Release pops: regular/goofy, normal/switch, native frame overrides and fallback\n",count);
 return 0;
}
