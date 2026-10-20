#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_config config(int goofy,int split){
 sc_config c={0};c.enabled=c.pure=c.xl_layout=c.air_controls=1;
 c.goofy=goofy;c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
 c.grind_sensitivity=1.2f;c.split_grinds=split;return c;
}
int main(void){
 int cases=0;
 for(int g=0;g<2;g++)for(int stance=0;stance<2;stance++)for(int split=0;split<2;split++)
 for(int foot=0;foot<2;foot++)for(int side=-1;side<=1;side+=2){
  sc_config c=config(g,split);sc_state s[2]={{0}};sc_pad out[2];
  for(int mirror=0;mirror<2;mirror++){
   s[mirror].switched=stance;s[mirror].game_switch_known=1;s[mirror].game_switch=stance^mirror;
   s[mirror].game_mirror_known=1;s[mirror].game_mirror=mirror;s[mirror].game_crouch=-1;
   sc_pad p={0};sc_translate(&c,&s[mirror],p,100);
   s[mirror].takeoff=1;sc_translate(&c,&s[mirror],p,110);
   s[mirror].takeoff=2;s[mirror].air=1;sc_translate(&c,&s[mirror],p,250);
   if(foot)p.rx=(short)(side*28000);else p.lx=(short)(side*28000);
   out[mirror]=sc_translate(&c,&s[mirror],p,300);
   assert(sc_abs((float)out[mirror].rx)>10000);
   // A body turn or missing mirror read must not reverse an ongoing adjustment.
   s[mirror].game_mirror=!mirror;s[mirror].game_switch^=1;s[mirror].switched^=1;
   sc_pad held=sc_translate(&c,&s[mirror],p,304);
   assert(held.rx==out[mirror].rx);
   s[mirror].game_mirror_known=0;held=sc_translate(&c,&s[mirror],p,308);
   assert(held.rx==out[mirror].rx);
  }
  // The same anatomical input needs opposite native X in the mirrored fakie frame.
  assert(out[0].rx*(long)out[1].rx<0);
  assert(sc_abs((float)out[0].rx+out[1].rx)<=1);
  cases++;
 }
 printf("PASS: %d XL normal/switch versus mirrored fakie air-adjustment pairs; both stances, feet, directions and grind modes; held turn/missing-read continuity\n",cases);
 return 0;
}
