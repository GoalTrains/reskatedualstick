#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/grind_entry_plan.h"
int main(void){
 int n=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<2;layout++)for(int sw=0;sw<2;sw++)for(int side=-1;side<=1;side+=2){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.goofy=g;c.front_back=layout;c.switch_flip=layout?1:3;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;
 sc_state s={0};s.air=1;s.takeoff=2;s.switched=sw;s.game_crouch=-1;
 int left=layout?1:(!g)!=sw;sc_pad in={0};if(left){in.rx=sc_short(side);in.ry=sc_short(-.1f);}else{in.lx=sc_short(side);in.ly=sc_short(-.1f);}
 sc_pad out=sc_translate(&c,&s,in,1000);sc_entry_plan p=sc_side_entry_plan(&c,&s);
 assert(p.active && p.accepted_only && p.type==(s.sw_lat?1:2));assert(out.rx!=0 && out.ry!=0);
 sc_config original=c;original.split_grinds=0; /* plan exclusion only; output is tested below against old experiment-off geometry. */
 assert(!sc_side_entry_plan(&original,&s).active);
 s.entry_side_back=0;assert(!sc_side_entry_plan(&c,&s).active);s.entry_side_back=1;
 s.entry_back_y=-.5f;assert(!sc_side_entry_plan(&c,&s).active);s.entry_back_y=-.1f;
 s.grind=1;assert(!sc_side_entry_plan(&c,&s).active);s.grind=0;
 c.one_foot_flips=1;assert(!sc_side_entry_plan(&c,&s).active);c.one_foot_flips=0;
 c.native_grind_entry=1;assert(!sc_side_entry_plan(&c,&s).active);c.native_grind_entry=0;
 s.radial_passthrough=1;assert(!sc_side_entry_plan(&c,&s).active);
 /* Back-up diagonals are clipped by BLOCKED for board output, but must
  * not be mislabeled as horizontal tail-slide intent after clipping. */
 sc_state up={0};up.air=1;up.takeoff=2;up.switched=sw;up.game_crouch=-1;
 if(left)in.ry=sc_short(.6f);else in.ly=sc_short(.6f);
 sc_translate(&c,&up,in,1001);assert(!sc_side_entry_plan(&c,&up).active);n++;
 }
 printf("PASS: %d sideways rear-foot stance/layout/switch profiles; diagonal/two-foot/rail/stock exclusions\n",n);
}
