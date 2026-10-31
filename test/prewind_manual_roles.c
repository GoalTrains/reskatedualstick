#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static void put(sc_pad*p,int left,short y){if(left)p->ly=y;else p->ry=y;}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int turn=-1;turn<=1;turn+=2)for(int foot=0;foot<2;foot++)for(int late=0;late<2;late++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.goofy=g;c.front_back=layout==0;
  c.xl_layout=layout==2;c.switch_flip=layout?3:1;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
  c.steer_scale=c.steer_curve=1;c.prewind=1;c.prewind_ms=60;
  sc_config plain=c;plain.prewind=0;
  sc_state a={0},b={0};a.game_crouch=b.game_crouch=-1;a.switched=b.switched=sw;
  a.game_switch_known=b.game_switch_known=1;a.game_switch=b.game_switch=sw;
  sc_pad p={0};sc_translate(&c,&a,p,1000);sc_translate(&plain,&b,p,1000);
  int front=a.feet_map_left;
  put(&p,!front,(layout==2 && sw)?30000:-30000);
  if(turn>0)p.rt=230;else p.lt=230;
  sc_translate(&c,&a,p,1100);sc_translate(&plain,&b,p,1100);assert(a.pw_side==turn);
  sc_translate(&c,&a,p,1200);sc_translate(&plain,&b,p,1200);
  a.air=b.air=a.takeoff=b.takeoff=1;
  p.lx=p.ly=p.rx=p.ry=0;sc_translate(&c,&a,p,1300);sc_translate(&plain,&b,p,1300);
  put(&p,foot?!front:front,foot?-21000:21000);
  sc_pad x=sc_translate(&c,&a,p,1320),y=sc_translate(&plain,&b,p,1320);
  assert(x.rx==y.rx && x.ry==y.ry && a.feet_map_left==b.feet_map_left);
  /* Publish the 180 stance either just before or at touchdown, then allow
   * the native input frame to follow later while the manual stays held. */
  if(!late){a.switched=b.switched=!sw;sc_translate(&c,&a,p,1340);sc_translate(&plain,&b,p,1340);}
  a.air=b.air=a.takeoff=b.takeoff=0;a.switched=b.switched=!sw;
  x=sc_translate(&c,&a,p,1400);y=sc_translate(&plain,&b,p,1400);
  assert(x.rx==y.rx && x.ry==y.ry && a.feet_map_left==b.feet_map_left && a.d_land_hold);
  short held=x.ry;
  a.game_switch=b.game_switch=!sw;
  for(int t=1420;t<1700;t+=20){x=sc_translate(&c,&a,p,t);y=sc_translate(&plain,&b,p,t);
   assert(x.rx==y.rx && x.ry==y.ry && x.ry==held && a.feet_map_left==b.feet_map_left);}
  assert(!a.pw_side);count++;
 }
 printf("PASS: %d prewind/no-prewind 180 manual comparisons preserve the same foot and right-stick output, both stance publication timings, all layouts/stances/trucks/turn directions\n",count);
}
