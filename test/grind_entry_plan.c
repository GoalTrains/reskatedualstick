#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/grind_entry_plan.h"
static sc_pad feet(sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout&&sw){fy=-fy;by=-by;}int left=c->front_back?1:(!c->goofy)!=sw;
 sc_pad p={0};p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
int main(void){int cases=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=c.native_grind_entry=1;c.goofy=g;c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=c.front_back?1:3;
  c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;
  sc_state s={0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;
  sc_pad p=feet(&c,sw,0,0,side,-.3f);sc_pad output=sc_translate(&c,&s,p,1000);
  sc_entry_plan entry=sc_back_entry_plan(&c,&s);
  assert(entry.active && entry.type==(s.sw_lat?1:2));assert(sc_abs(entry.yaw+side*1.570796327f)<.001f);
  assert(entry.pitch!=0 && output.rx!=0 && output.ry!=0); /* No flattened pressure or suppressed fallback. */
  /* Two feet rotating together toward a loaded tail must reach the same
   * request path as a sole back foot. Test both native frames and stances. */
  p=feet(&c,sw,-side,0,side,-.3f);sc_translate(&c,&s,p,1001);
  entry=sc_back_entry_plan(&c,&s);assert(entry.active && entry.type==(s.sw_lat?1:2));
  assert(sc_abs(entry.yaw+side*1.570796327f)<.001f);
  p=feet(&c,sw,-side,.2f,side,-.5f);sc_translate(&c,&s,p,1002);assert(sc_back_entry_plan(&c,&s).active);
  /* Same lateral direction cancels yaw; front-loaded/vertical setups are
   * not reclassified as tail slides, even if both feet are active. */
  p=feet(&c,sw,side,0,side,-.3f);sc_translate(&c,&s,p,1003);assert(!sc_back_entry_plan(&c,&s).active);
  p=feet(&c,sw,-side,.7f,side,-.3f);sc_translate(&c,&s,p,1004);assert(!sc_back_entry_plan(&c,&s).active);
  p=feet(&c,sw,-side*.3f,.5f,side,-.7f);sc_translate(&c,&s,p,1005);assert(!sc_back_entry_plan(&c,&s).active);
  p=feet(&c,sw,0,1,side,-.3f);sc_translate(&c,&s,p,1010);assert(!sc_back_entry_plan(&c,&s).active);
  p=feet(&c,sw,0,0,0,-1);sc_translate(&c,&s,p,1020);assert(!sc_back_entry_plan(&c,&s).active);
  s.air=s.takeoff=0;s.grind=1;p=feet(&c,sw,0,0,side,-.3f);sc_translate(&c,&s,p,1030);assert(!sc_back_entry_plan(&c,&s).active);
  s.grind=0;s.air=1;s.takeoff=2;c.one_foot_flips=1;sc_translate(&c,&s,p,1040);assert(!sc_back_entry_plan(&c,&s).active);
  c.one_foot_flips=0;p.buttons=SC_DPAD_UP;sc_translate(&c,&s,p,1050);assert(!sc_back_entry_plan(&c,&s).active);
  cases++;
 }
 assert(sc_entry_fresh(1000,940,201));assert(!sc_entry_fresh(1000,939,201));
 assert(!sc_entry_fresh(1000,1001,201));assert(!sc_entry_fresh(1000,999,400));assert(!sc_entry_fresh(1000,999,100));
 printf("PASS: %d native entry profiles: sole rear foot and coordinated two-foot tail requests; native-frame end/yaw/pressure preserved; cancelling, front-loaded, vertical, rail, allowed-flip and radial cases excluded; freshness\n",cases);
}
