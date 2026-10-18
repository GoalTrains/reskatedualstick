#include <assert.h>
#include <stdio.h>
#include "../src/xl_stance_agreement.h"
#include "../src/translate.h"
int main(void){
 for(int profile=0;profile<2;profile++)for(int baseline=0;baseline<2;baseline++){
  sc_xl_absolute a={0};int stance=sc_xl_absolute_step(&a,0,1,0);
  /* A no-comply/revert can publish the same native flag before and after
   * its mirrored animation frame changes. Resolve absolute feet each time. */
  const int samples[][3]={{1,0,1},{0,1,1},{1,1,0},{0,0,0}};
  double now=100;
  for(unsigned i=0;i<4;i++){
   int native=samples[i][0],mirror=samples[i][1]^baseline,expected=samples[i][2];
   int raw=sc_xl_physical_raw(native,mirror,baseline);
   sc_xl_absolute_step(&a,raw,stance,now);stance=sc_xl_absolute_step(&a,raw,stance,now+12);
   assert(stance==expected);
   for(int bias=0;bias<2;bias++)assert(sc_xl_output_frame(1,stance,bias,native)==native);
   sc_config c={0};c.enabled=c.pure=c.xl_layout=c.one_foot_flips=1;c.switch_flip=3;c.goofy=profile;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
   sc_state s={0};s.game_crouch=-1;s.switched=stance;s.game_switch_known=1;
   s.game_switch=sc_xl_output_frame(1,stance,1,native);s.game_mirror_known=1;s.game_mirror=samples[i][1];
   sc_pad p={0};sc_translate(&c,&s,p,now+20);
   if(profile)p.ly=-32768;else p.ry=-32768;
   sc_pad out=sc_translate(&c,&s,p,now+40);
   int sign=(stance^native)?1:-1;assert(out.ry*sign>30000);
   assert(s.feet_map_left==(!profile^stance));now+=200;
  }
 }
 for(int stance=0;stance<2;stance++)for(int bias=0;bias<2;bias++)assert(sc_xl_output_frame(0,stance,bias,1)==(stance^bias));
 puts("PASS: XL no-comply/revert paired-frame sequences retain absolute jobs and independent native output; regular/goofy, both mirror baselines; non-XL motion frame unchanged");
 return 0;
}
