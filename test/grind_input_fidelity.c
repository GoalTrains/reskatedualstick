#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/grind_entry_plan.h"
static sc_pad feet(const sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout&&sw){fy=-fy;by=-by;}
 int left=c->front_back?1:(!c->goofy)!=sw;sc_pad p={0};
 p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);
 p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
static sc_state air(int sw){sc_state s={0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;return s;}
static void same(sc_pad a,sc_pad b){assert(a.buttons==b.buttons&&a.lt==b.lt&&a.rt==b.rt);assert(a.lx==b.lx&&a.ly==b.ly&&a.rx==b.rx&&a.ry==b.ry);}
int main(void){unsigned frames=0;
 for(int goofy=0;goofy<2;goofy++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;
  c.goofy=goofy;c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=c.front_back?1:3;
  c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;
  for(int foot=0;foot<2;foot++)for(int lateral=1;lateral<=10;lateral++)for(int pressure=1;pressure<=8;pressure++){
   float x=side*lateral*.1f,y=pressure*.1f;
   sc_pad p=feet(&c,sw,foot?0:x,foot?0:y,foot?x:0,foot?-y:0);
   sc_state ordinary=air(sw),near=ordinary;near.entry_rail_target=1;
   sc_pad a=sc_translate(&c,&ordinary,p,1000),b=sc_translate(&c,&near,p,1000);
   same(a,b);assert(!near.entry_rotation_active);assert(!sc_back_entry_plan(&c,&near).active);
   /* Detection may fluctuate while the physical pose stays held. */
   for(int poll=1;poll<=6;poll++){
    near.entry_rail_target=poll&1;
    a=sc_translate(&c,&ordinary,p,1000+poll*8);b=sc_translate(&c,&near,p,1000+poll*8);
    same(a,b);assert(!near.entry_rotation_active);frames++;
   }
  }
  /* Equivalent end loads reach opposite native board vectors. Exclude
   * clip-boundary cases from the one-raw-unit quantization tolerance. */
  for(int lateral=1;lateral<=8;lateral++)for(int pressure=1;pressure<=8;pressure++){
   float x=side*lateral*.1f,y=pressure*.1f;
   sc_state nose=air(sw),tail=air(sw);nose.entry_rail_target=tail.entry_rail_target=1;
   sc_pad a=sc_translate(&c,&nose,feet(&c,sw,x,y,0,0),2000);
   sc_pad b=sc_translate(&c,&tail,feet(&c,sw,0,0,x,-y),2000);
   assert(sc_abs(sc_axis(a.rx)+sc_axis(b.rx))<.0002f);
   assert(sc_abs(sc_axis(a.ry)+sc_axis(b.ry))<.0002f);frames++;
  }
 }
 printf("PASS: %u frames preserve nose/back-foot inputs across rail-target changes; mirrored end response across regular/goofy, all layouts, switch and lateral signs; no implicit target requests\n",frames);
}
