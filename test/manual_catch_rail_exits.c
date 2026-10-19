#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/translate.h"
#include "../src/manual_catch.h"
static sc_pad foot(const sc_config*c,int sw,int back,float angle){
 sc_pad p={0};float x,y;sc_vec_at(back,angle,1,&x,&y);
 if(c->xl_layout && sw)y=-y;
 int left=c->front_back?1:(!c->goofy)!=sw;
 if(back!=left){p.lx=sc_short(x);p.ly=sc_short(y);}else{p.rx=sc_short(x);p.ry=sc_short(y);}return p;
}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int release=0;release<2;release++)for(int back=0;back<2;back++)for(int side=-1;side<=1;side+=2)
 for(int split=0;split<2;split++){
  sc_config c={0};c.enabled=c.pure=c.manual_catch=c.air_controls=1;c.goofy=g;
  c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=c.front_back?1:3;
  c.pop_on_release=release;c.split_grinds=split;c.active=.3f;c.deadzone=.15f;
  c.windup_deg=60;c.manual_max=.85f;c.grind_sensitivity=1.2f;
  sc_state s={0},normal={0};s.grind=normal.grind=1;s.switched=normal.switched=sw;
  s.game_crouch=normal.game_crouch=-1;sc_manual_catch m={0};sc_pad z={0},last={0};
  sc_manual_catch_translate(&c,&s,&m,z,1000,300);sc_translate(&c,&normal,z,1000);
  /* On-rail output must match ordinary translation throughout recognition. */
  for(int i=0;i<=6;i++){
   sc_pad p=foot(&c,sw,back,(float)(i*15*side));
   sc_pad out=sc_manual_catch_translate(&c,&s,&m,p,1100+i*16,300);
   last=sc_translate(&c,&normal,p,1100+i*16);
   assert(!memcmp(&out,&last,sizeof out) && !m.phase);
  }
  assert(s.rail_scoop_exit && m.candidate && m.candidate_kind==2);
  /* Physical release at takeoff must not lose the rail gesture. */
  s.grind=0;s.air=1;s.takeoff=2;
  sc_pad out=sc_manual_catch_translate(&c,&s,&m,z,1210,200);
  assert(m.phase==2);
  assert(sc_abs((float)out.rx-last.rx)<=3 && sc_abs((float)out.ry-last.ry)<=3);
  out=sc_manual_catch_translate(&c,&s,&m,z,1350,200);assert(m.phase==2 && (out.rx||out.ry));
  z.buttons=SC_LTHUMB;out=sc_manual_catch_translate(&c,&s,&m,z,1370,200);
  assert(m.phase==3 && !out.rx && !out.ry);
  s.air=s.takeoff=0;s.grind=1;
  sc_manual_catch_translate(&c,&s,&m,z,1400,300);assert(!m.phase && !m.candidate);
  count++;
 }
 /* Rail positioning alone cannot arm a hold. */
 {sc_config c={0};c.enabled=c.pure=c.manual_catch=c.air_controls=c.split_grinds=1;
 c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;sc_state s={0};s.grind=1;s.game_crouch=-1;
 sc_manual_catch m={0};sc_pad p={0};p.rx=32767;
 for(int i=0;i<20;i++)sc_manual_catch_translate(&c,&s,&m,p,2000+i*20,300);
 assert(!m.phase && !m.candidate);}
 printf("PASS: %d manual-catch rail exits preserve complete on-rail input, hold after release/takeoff, catch and clear on contact; rail positioning never arms\n",count);
}
