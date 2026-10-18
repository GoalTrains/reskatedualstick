#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/grind_entry_plan.h"
static sc_pad feet(sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout&&sw){fy=-fy;by=-by;}
 int left=c->front_back?1:(!c->goofy)!=sw;sc_pad p={0};
 p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
int main(void){int cases=0;
 for(int g=0;g<2;g++)for(int l=0;l<3;l++)for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=l==0;c.xl_layout=l==2;c.switch_flip=c.front_back?1:3;
  c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;
  sc_state s={0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;
  sc_pad p=feet(&c,sw,0,0,side,-.5f);sc_pad old=sc_translate(&c,&s,p,1000);assert(old.ry && !s.entry_rotation_active);
  s.entry_rail_target=1;sc_pad assisted=sc_translate(&c,&s,p,1010);
  assert(s.entry_rotation_active && sc_back_entry_plan(&c,&s).active);
  assert(old.rx*(double)assisted.rx>0 && sc_abs((float)assisted.ry)<4);
  assert(!s.grind_pop && assisted.lx==old.lx && assisted.ly==old.ly && assisted.buttons==old.buttons);
  /* A partial adjustment keeps pitch; compensation has no snap at onset. */
  p=feet(&c,sw,0,0,side*.7f,-.4f);assisted=sc_translate(&c,&s,p,1020);assert(s.entry_rotation_active&&assisted.ry);
  /* Cooperative front foot still reaches the same requested tail. */
  p=feet(&c,sw,-side,.1f,side,-.5f);assisted=sc_translate(&c,&s,p,1030);assert(s.entry_rotation_active&&sc_abs((float)assisted.ry)<4);
  /* Nose-loaded or vertical intent cannot be commandeered. */
  p=feet(&c,sw,-side,1,side,-.3f);sc_translate(&c,&s,p,1040);assert(!s.entry_rotation_active);
  p=feet(&c,sw,0,0,0,-1);sc_translate(&c,&s,p,1050);assert(!s.entry_rotation_active);
  p=feet(&c,sw,0,0,side,-.5f);s.entry_rail_target=0;sc_pad restored=sc_translate(&c,&s,p,1060);
  assert(!s.entry_rotation_active && sc_abs((float)restored.rx-old.rx)<4 && sc_abs((float)restored.ry-old.ry)<4);
  s.entry_rail_target=1;s.air=s.takeoff=0;s.grind=1;sc_translate(&c,&s,p,1070);assert(!s.entry_rotation_active);
  s.grind=0;s.air=1;s.takeoff=2;c.one_foot_flips=1;sc_translate(&c,&s,p,1080);assert(!s.entry_rotation_active);
  cases++;
 }
 printf("PASS: %d regular/goofy/layout/switch/side rail approaches: full sideways tail request reaches flat rotation without X reversal; partial pressure proportional; non-target/nose/vertical/rail/allowed cases excluded\n",cases);
}
