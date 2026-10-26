#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad feet(sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout&&sw){fy=-fy;by=-by;}
 int left=c->front_back?1:(!c->goofy)!=sw;sc_pad p={0};
 p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
int main(void){int cases=0;
 for(int g=0;g<2;g++)for(int l=0;l<3;l++)for(int sw=0;sw<2;sw++)for(int foot=0;foot<2;foot++)for(int side=-1;side<=1;side+=2){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=l==0;c.xl_layout=l==2;c.switch_flip=c.front_back?1:3;
  c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;
  sc_state s={0};s.grind=1;s.switched=sw;s.game_crouch=-1;
  sc_pad p=feet(&c,sw,foot?0:side*.95f,foot?0:.55f,foot?side*.95f:0,foot?-.55f:0);
  sc_pad loaded=sc_translate(&c,&s,p,1000);assert(loaded.ry&&!s.grind_pop);
  /* Moving the same foot sideways retains support. Rear angular control
   * may increase tail load as it rotates; front support stays byte-stable. */
  p=feet(&c,sw,foot?0:side*.95f,0,foot?side*.95f:0,0);
  sc_pad flat=sc_translate(&c,&s,p,1020);
  assert(loaded.ry*(double)flat.ry>0 && (foot?sc_abs(flat.ry)>=sc_abs(loaded.ry):sc_abs((float)flat.ry-loaded.ry)<4) && !s.grind_pop);
  p=feet(&c,sw,foot?0:side*.95f,foot?0:-.55f,foot?side*.95f:0,foot?.55f:0);
  sc_pad opposite=sc_translate(&c,&s,p,1040);
  assert(loaded.ry*(double)opposite.ry>0 && (foot?sc_abs(opposite.ry)>=sc_abs(loaded.ry):sc_abs((float)opposite.ry-loaded.ry)<4) && !s.grind_pop && !s.rail_scoop_exit);
  /* Small deliberate opposite-foot pressure can progressively flatten it. */
  p=feet(&c,sw,foot?0:side*.95f,foot?.30f:0,foot?side*.95f:0,foot?0:-.30f);
  sc_pad other=sc_translate(&c,&s,p,1100);
  assert(sc_abs(other.ry)<sc_abs(opposite.ry) && !s.grind_pop);
  /* Reported front-down/back-up movement must also load its own end,
   * rather than being discarded by the single-foot flip limiter. */
  p=feet(&c,sw,foot?0:side*.95f,foot?-.30f:0,foot?side*.95f:0,foot?0:.30f);
  sc_pad down=sc_translate(&c,&s,p,1120);
  assert(sc_abs((float)down.rx-other.rx)<=4 && sc_abs((float)down.ry-other.ry)<=4 && !s.grind_pop);
  p=feet(&c,sw,foot?0:side*.95f,foot?-.50f:0,foot?side*.95f:0,foot?0:.50f);
  sc_pad lower=sc_translate(&c,&s,p,1140);
  assert(lower.rx!=down.rx || lower.ry!=down.ry);
  /* Neutral and leaving rail cannot carry a sticky synthetic command. */
  p=(sc_pad){0};sc_translate(&c,&s,p,1200);assert(!s.split_rail_front_load&&!s.split_rail_back_load);
  p=feet(&c,sw,foot?0:side*.95f,0,foot?side*.95f:0,0);
  flat=sc_translate(&c,&s,p,1220);assert(!flat.ry);
  s.split_rail_back_load=.7f;s.grind=0;s.air=1;s.takeoff=2;sc_translate(&c,&s,p,1300);
  assert(!s.split_rail_front_load&&!s.split_rail_back_load);
  cases++;
 }
 printf("PASS: %d rail support profiles: retained own-end hold; opposite foot progressively flattens in either vertical direction without ignored down/up pressure or solo pop/scoop; neutral/air clear\n",cases);
}
