#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_config config(int goofy,int layout,int release,int mirror) {
 sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.goofy=goofy;c.front_back=layout==0;
 c.xl_layout=layout==2;c.switch_flip=c.front_back?1:3;c.pop_on_release=release;
 c.flip_x=mirror;c.deadzone=.15f;c.active=.3f;c.windup_deg=60;c.manual_max=.85f;c.grind_sensitivity=1.2f;return c;
}
static sc_pad foot(sc_config*c,int sw,int back,float angle,float mag) {
 sc_pad p={0};float x,y;sc_vec_at(back,angle,mag,&x,&y);
 if(c->xl_layout && sw)y=-y;
 int left=c->front_back?1:((!c->goofy)!=sw);
 if(back!=left){p.lx=sc_short(x);p.ly=sc_short(y);}else{p.rx=sc_short(x);p.ry=sc_short(y);}return p;
}
int main(void) {
 int cases=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int release=0;release<2;release++)for(int back=0;back<2;back++)for(int side=-1;side<=1;side+=2)for(int mirror=0;mirror<2;mirror++) {
  sc_config c=config(g,layout,release,mirror);sc_state rail={0},ground={0};
  rail.grind=1;rail.switched=ground.switched=sw;rail.game_crouch=ground.game_crouch=-1;
  sc_pad zero={0};sc_translate(&c,&rail,zero,1000);sc_translate(&c,&ground,zero,1000);
  for(int i=0;i<6;i++) {
   sc_pad in=foot(&c,sw,back,(float)(i*15*side),1),r=sc_translate(&c,&rail,in,1100+i*20),f=sc_translate(&c,&ground,in,1100+i*20);
   if(i>=3){assert(rail.grind_pop);assert(rail.rail_scoop_exit==(back?2:1));assert(r.rx==f.rx && r.ry==f.ry);}
  }
  assert(!rail.rp_t);sc_translate(&c,&rail,zero,1300);assert(!rail.grind_pop && !rail.rail_scoop_exit);cases++;
 }
 /* A long held position with small jitter is still a pivot, and does not
  * consume the quick scoop that follows. */
 for(int back=0;back<2;back++) {
  sc_config c=config(0,0,0,0);sc_state s={0};s.grind=1;s.game_crouch=-1;sc_pad z={0};sc_translate(&c,&s,z,1000);
  for(int i=0;i<60;i++){sc_pad p=foot(&c,0,back,(float)((i%3)-1)*3,1);sc_translate(&c,&s,p,1100+i*20);assert(!s.grind_pop);}
  for(int i=0;i<6;i++){sc_pad p=foot(&c,0,back,(float)(i*15),1);sc_translate(&c,&s,p,2400+i*20);}
  assert(s.grind_pop && s.rail_scoop_exit==(back?2:1));
  c.air_controls=0;sc_translate(&c,&s,z,2600);assert(!s.rail_scoop_exit && !s.rail_scoop_track);
 }
 /* Sideways holds / two-foot adjustments cannot become a solo scoop. */
 {sc_config c=config(0,0,0,0);sc_state s={0};s.grind=1;s.game_crouch=-1;sc_pad p={0};sc_translate(&c,&s,p,1000);
  p.rx=32767;for(int i=0;i<30;i++){sc_pad o=sc_translate(&c,&s,p,1100+i*20);assert(!s.grind_pop && o.rx<0);}
  p.lx=-32768;p.ry=-32768;for(int i=0;i<10;i++){sc_translate(&c,&s,p,1800+i*20);assert(!s.rail_scoop_exit);}}
 /* Coarse 30-Hz polling still recognizes a scoop; stalls in polling,
  * non-grind samples and a change of the working foot must not borrow history. */
 {sc_config c=config(0,0,0,0);sc_state s={0};s.grind=1;s.game_crouch=-1;sc_pad z={0};sc_translate(&c,&s,z,1000);
  sc_translate(&c,&s,foot(&c,0,1,0,1),1100);sc_translate(&c,&s,foot(&c,0,1,45,1),1133);assert(s.rail_scoop_exit==2);
  sc_translate(&c,&s,z,1166);sc_translate(&c,&s,foot(&c,0,1,0,1),1200);sc_translate(&c,&s,foot(&c,0,1,60,1),1401);assert(!s.grind_pop);
  s.grind=0;sc_translate(&c,&s,foot(&c,0,1,0,1),1500);s.grind=1;sc_translate(&c,&s,foot(&c,0,1,60,1),1520);assert(!s.rail_scoop_exit);
  sc_translate(&c,&s,z,1600);sc_translate(&c,&s,foot(&c,0,1,0,1),1700);sc_translate(&c,&s,foot(&c,0,0,60,1),1720);assert(!s.rail_scoop_exit);
 }
 printf("PASS: %d quick rail scoops match ground shove directions for both ends, regular/goofy, three layouts, switch, pop styles and mirror; neutral clears, held/noisy/two-foot pivots preserved\n",cases);
}
