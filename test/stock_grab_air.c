#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/translate.h"
static sc_config config(void){sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.active=.3f;c.deadzone=.15f;c.steer_scale=c.steer_curve=1;c.powerslide=1;c.body_flips=1;c.bump_ms=250;c.prewind=1;return c;}
int main(void){
 for(int g=0;g<2;g++)for(int l=0;l<2;l++)for(int sw=0;sw<2;sw++)for(int mode=0;mode<4;mode++) {
  sc_config c=config();c.goofy=g;c.front_back=l;c.switch_flip=mode;
  sc_state a={0},b={0};a.switched=b.switched=sw;sc_pad p={0};p.lt=150;p.lx=12000;p.ry=-28000;
  sc_pad x=sc_translate(&c,&a,p,1000);c.stock_grab_air=1;sc_pad y=sc_translate(&c,&b,p,1000);assert(!memcmp(&x,&y,sizeof x)); /* ground unchanged */
  b=(sc_state){0};b.switched=sw;b.stock_ramp_eligible=1;b.takeoff=b.air=1;p=(sc_pad){0};p.rt=87;
  y=sc_translate(&c,&b,p,1100);assert(b.stock_air_active && y.rt==87 && !y.lt);
  p.lx=-26000;p.ly=14000;p.rx=11000;p.ry=-25000;p.lt=193;p.rt=72;
  y=sc_translate(&c,&b,p,1120);assert(!memcmp(&p,&y,sizeof p)); /* physical stocks, no goofy mirror */
  p.lt=p.rt=0;y=sc_translate(&c,&b,p,1140);assert(b.stock_air_active && !memcmp(&p,&y,sizeof p)); /* stable until landing */
  b.takeoff=b.air=0;y=sc_translate(&c,&b,p,1160);assert(!b.stock_air_active && y.rx==0 && y.ry==0); /* held air sticks cannot pop */
  p=(sc_pad){0};sc_translate(&c,&b,p,1180);p.ry=-32000;y=sc_translate(&c,&b,p,1200);assert(!b.stock_air_active);
  b=(sc_state){0};b.stock_ramp_eligible=1;b.takeoff=1;p=(sc_pad){0};p.ly=32000;p.ry=-32000;p.rt=100;
  y=sc_translate(&c,&b,p,2000);assert(b.stock_air_block_l && b.stock_air_block_r && !y.ly && !y.ry);
  p.lx=32000;p.ly=0;y=sc_translate(&c,&b,p,2020);assert(!b.stock_air_block_l && y.lx==32000); /* deliberate direction change */
  p.rx=p.ry=0;sc_translate(&c,&b,p,2040);p.ry=26000;y=sc_translate(&c,&b,p,2060);assert(!b.stock_air_block_r && y.ry==26000);
  /* Pre-held sideways steering must reach the game immediately and stay live,
   * even while vertical takeoff carry remains guarded. */
  for(int side=-1;side<=1;side+=2) {
   b=(sc_state){0};b.stock_ramp_eligible=1;b.takeoff=b.air=1;p=(sc_pad){0};p.lx=(short)(side*32000);p.ly=7000;p.rt=180;
   y=sc_translate(&c,&b,p,2200);assert(b.stock_air_block_l && y.lx==p.lx && y.ly==0 && y.rt==180);
   p.lt=p.rt=0;y=sc_translate(&c,&b,p,2250);assert(y.lx==p.lx && y.ly==0);
   p.lx=(short)(side*12000);y=sc_translate(&c,&b,p,2270);assert(y.lx==p.lx);
   b.takeoff=b.air=0;y=sc_translate(&c,&b,p,2300);assert(!b.stock_air_active && !y.rx && !y.ry);
  }
  b=(sc_state){0};b.grind=1;p.rt=255;y=sc_translate(&c,&b,p,3000);assert(!b.stock_air_active); /* stalls don't enter air mode */
 }
 sc_config c=config();c.stock_grab_air=1;sc_state s={0};s.takeoff=1;sc_pad p={0};sc_translate(&c,&s,p,1000);assert(!s.stock_air_active);
 p.buttons=1;p.rt=255;sc_pad o=sc_translate(&c,&s,p,1100);assert(!s.stock_air_active && !memcmp(&p,&o,sizeof p)); /* radial bypass */
 for(int sign=-1;sign<=1;sign+=2) {
  s=(sc_state){0};s.takeoff=0;s.stock_ramp_valid=1;s.stock_ramp_vertical=sign*.9f;p=(sc_pad){0};
  sc_translate(&c,&s,p,4000);sc_translate(&c,&s,p,4080);assert(s.stock_ramp_eligible);
  s.stock_ramp_departing=1;s.stock_ramp_valid=0;sc_translate(&c,&s,p,4090);assert(s.stock_ramp_eligible); /* ramp pop transition */
  s.takeoff=s.air=1;p.rt=100;p.lx=32000;o=sc_translate(&c,&s,p,4100);assert(s.stock_air_active && o.lx==32000);
  s.takeoff=s.air=0;s.stock_ramp_vertical=0;sc_translate(&c,&s,p,4200);assert(!s.stock_air_active && !s.stock_ramp_eligible);
 }
 s=(sc_state){0};s.takeoff=0;s.stock_ramp_valid=1;s.stock_ramp_vertical=0;p=(sc_pad){0};sc_translate(&c,&s,p,5000);
 s.takeoff=s.air=1;s.stock_ramp_vertical=1;p.rt=255;sc_translate(&c,&s,p,5100);assert(!s.stock_air_active); /* flat ollie pitching in air cannot arm */
 s=(sc_state){0};s.takeoff=0;s.stock_ramp_valid=1;s.stock_ramp_vertical=.9f;sc_translate(&c,&s,p,6000);
 s.takeoff=s.air=1;sc_translate(&c,&s,p,6040);assert(!s.stock_air_active); /* brief tilt can't qualify */
 s=(sc_state){0};s.takeoff=0;s.stock_ramp_vertical=1;sc_translate(&c,&s,p,7000);sc_translate(&c,&s,p,7200);
 s.takeoff=s.air=1;sc_translate(&c,&s,p,7300);assert(!s.stock_air_active); /* missing pointer data fails closed */
 s=(sc_state){0};s.takeoff=0;s.stock_ramp_valid=1;s.stock_ramp_vertical=.9f;sc_translate(&c,&s,p,8000);sc_translate(&c,&s,p,8080);
 s.stock_ramp_valid=0;s.stock_ramp_departing=1;sc_translate(&c,&s,p,8300);assert(!s.stock_ramp_eligible); /* expired ramp evidence */
 s=(sc_state){0};s.takeoff=0;s.stock_ramp_departing=1;sc_translate(&c,&s,p,9000);sc_translate(&c,&s,p,9100);
 s.takeoff=s.air=1;sc_translate(&c,&s,p,9200);assert(!s.stock_air_active); /* flat pop transition cannot arm */
 for(int sign=-1;sign<=1;sign+=2) {
  s=(sc_state){0};s.takeoff=0;s.stock_ramp_valid=1;p=(sc_pad){0};
  for(int k=0;k<=5;k++){s.stock_ramp_vertical=sign*(.05f+.042f*k);sc_translate(&c,&s,p,10000+30*k);}
  assert(s.stock_ramp_eligible && s.stock_ramp_curve); /* gentle rapid transition */
  s.stock_ramp_vertical=0;sc_translate(&c,&s,p,10200);assert(s.stock_ramp_eligible); /* flattening crest retains evidence */
  s.stock_ramp_valid=0;s.stock_ramp_departing=1;sc_translate(&c,&s,p,10220);assert(s.stock_ramp_eligible);
  s.takeoff=s.air=1;p.lt=255;p.lx=-28000;o=sc_translate(&c,&s,p,10240);assert(s.stock_air_active && o.lt==255 && o.lx==-28000);
  s.takeoff=s.air=0;s.stock_ramp_departing=0;s.stock_ramp_valid=1;sc_translate(&c,&s,p,11000);assert(!s.stock_ramp_eligible && !s.stock_ramp_history_n);
 }
 s=(sc_state){0};s.takeoff=0;s.stock_ramp_valid=1;p=(sc_pad){0};
 for(int k=0;k<=40;k++){s.stock_ramp_vertical=.02f+.006f*k;sc_translate(&c,&s,p,12000+30*k);assert(!s.stock_ramp_eligible);} /* steady gentle hill */
 s=(sc_state){0};s.takeoff=0;s.stock_ramp_valid=1;
 for(int k=0;k<=12;k++){s.stock_ramp_vertical=k%2?.15f:0;sc_translate(&c,&s,p,14000+30*k);assert(!s.stock_ramp_eligible);} /* low-angle movement */
 s=(sc_state){0};s.takeoff=0;s.stock_ramp_valid=1;
 for(int k=0;k<=5;k++){s.stock_ramp_vertical=.05f+.042f*k;sc_translate(&c,&s,p,15000+30*k);}
 assert(s.stock_ramp_eligible);s.stock_ramp_vertical=0;sc_translate(&c,&s,p,15800);assert(!s.stock_ramp_eligible); /* crest evidence expires */
 puts("PASS: grounded curve gate, both curve directions, crest retention, steady gentle hills and low-angle movement rejected, evidence expiry, landing reset; steep gate/pop transition, flat ollies, grabs and immediate sideways steering");
}
