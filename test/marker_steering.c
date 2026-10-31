#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/translate.h"
static sc_config config(void){
 sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.deadzone=.15f;c.active=.3f;
 c.manual_max=.85f;c.steer_curve=c.steer_scale=1;c.revert_ms=300;c.revert_input=2;
 c.grab_mode=1;return c;
}
int main(void){int cases=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int side=0;side<2;side++)for(int mode=0;mode<4;mode++){
  sc_config c=config();c.goofy=g;c.front_back=layout==0;c.xl_layout=layout==2;
  c.switch_flip=layout?3:1;c.revert_input=mode;
  sc_state s={0};s.switched=sw;sc_pad p={0};p.buttons=SC_DPAD_UP|0x1000;
  p.ly=12000;p.rx=-9000;p.ry=7000;if(side)p.rt=255;else p.lt=255;
  sc_pad o=sc_translate(&c,&s,p,1000);
  assert(side?o.lx>32000:o.lx<-32000);
  assert(!o.lt&&!o.rt && o.ly==p.ly && o.rx==p.rx && o.ry==p.ry && o.buttons==p.buttons);
  assert(s.radial_passthrough && !s.rv_start);
  p.buttons=0;o=sc_translate(&c,&s,p,1100);assert(side?o.lx>32000:o.lx<-32000);
  p.lx=p.ly=p.rx=p.ry=0;
  o=sc_translate(&c,&s,p,1300);assert(!s.radial_passthrough && !s.rv_start);
  assert(side?o.lx>32000:o.lx<-32000);cases++;
 }
 /* Analog steering curve/strength and physical marker-position X combine. */
 {sc_config c=config();c.steer_scale=.5f;sc_state s={0};sc_pad p={0};
  p.buttons=SC_DPAD_UP;p.rt=128;p.lx=8192;sc_pad o=sc_translate(&c,&s,p,1000);
  assert(o.lx>16200&&o.lx<16600);
  p.rt=p.lt=200;o=sc_translate(&c,&s,p,1020);assert(sc_abs(o.lx-p.lx)<=1 && !s.rv_start);
  p.lx=30000;p.lt=0;p.rt=255;o=sc_translate(&c,&s,p,1040);assert(o.lx==32767);}
 /* Holding both triggers through marker release must not trigger a revert. */
 {sc_config c=config();sc_state s={0};sc_pad p={0};p.buttons=SC_DPAD_UP;p.lt=p.rt=255;
  sc_translate(&c,&s,p,1000);p.buttons=0;sc_translate(&c,&s,p,1100);
  sc_translate(&c,&s,p,1300);assert(!s.rv_start);
  p.lt=p.rt=0;sc_translate(&c,&s,p,1350);p.lt=p.rt=255;
  sc_translate(&c,&s,p,1400);assert(s.rv_start);}
 /* Other radials/chords, stock controls, air and rails remain byte-for-byte. */
 for(int dpad=1;dpad<=15;dpad++)for(int context=0;context<4;context++){
  if(dpad==SC_DPAD_UP && context==0)continue;
  sc_config c=config();sc_state s={0};sc_pad p={0};
  p.buttons=dpad;p.lx=15000;p.ly=-17000;p.rx=5000;p.ry=9000;p.lt=55;p.rt=230;
  if(context==1)c.enabled=0;if(context==2)s.air=s.takeoff=1;if(context==3)s.grind=1;
  sc_pad o=sc_translate(&c,&s,p,1000);assert(!memcmp(&o,&p,sizeof p));
  if(c.enabled){p.buttons=0;o=sc_translate(&c,&s,p,1100);assert(!memcmp(&o,&p,sizeof p));}
 }
 puts("PASS: marker analog/combined steering, no grab leakage, no post-marker revert, stock/radial/air/rail preservation");
 printf("PASS: %d marker trigger cases across stances/layouts/switch/trigger sides/revert modes, release-tail continuity\n",cases);
}
