#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/translate.h"
#include "../src/manual_catch.h"
static sc_config config(int goofy,int layout,int release){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.manual_catch=c.body_flips=1;
 c.front_back=layout==0;c.xl_layout=layout==2;c.goofy=goofy;c.switch_flip=c.front_back?1:3;
 c.pop_on_release=release;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
 return c;
}
static sc_pad feet(const sc_config*c,int sw,float fx,float fy,float bx,float by){
 if(c->xl_layout && sw){fy=-fy;by=-by;}
 int left=c->front_back?1:(!c->goofy)!=sw;
 sc_pad p={0};p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);
 p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
static void launch(sc_config*c,sc_state*s,sc_manual_catch*m,int sw,int side,int nollie,unsigned short buttons){
 *s=(sc_state){0};*m=(sc_manual_catch){0};s->switched=sw;s->game_crouch=-1;
 sc_pad p={0};p.buttons=buttons;sc_manual_catch_translate(c,s,m,p,1000,100);
 p=feet(c,sw,0,nollie?1:0,0,nollie?0:-1);p.buttons=buttons;
 sc_manual_catch_translate(c,s,m,p,1100,100);
 sc_manual_catch_translate(c,s,m,p,1250,100);
 p=feet(c,sw,nollie?0:side*.6f,nollie?1:.8f,nollie?side*.6f:0,nollie?-.8f:-1);p.buttons=buttons;
 sc_manual_catch_translate(c,s,m,p,1300,100);assert(m->candidate);
 sc_manual_catch_translate(c,s,m,p,1310,103);assert(m->phase==2);
 s->air=1;s->takeoff=2;
 sc_manual_catch_translate(c,s,m,p,1320,200);assert(m->phase==2);
}
int main(void){int cases=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int release=0;release<2;release++)for(int side=-1;side<=1;side+=2)for(int n=0;n<2;n++){
  sc_config c=config(g,layout,release);sc_state s;sc_manual_catch m;
  launch(&c,&s,&m,sw,side,n,0);
  sc_pad o=sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1500,200);
  assert(m.phase==2 && (o.rx || o.ry)); /* physical release must keep held input */
  int front_left=m.front_left;
  sc_pad p={0};p.buttons=SC_LTHUMB;
  o=sc_manual_catch_translate(&c,&s,&m,p,1510,200);
  assert(m.phase==3 && m.requested==(front_left?1:2));
  assert(!o.rx && !o.ry && !s.bf_side && !s.bf_arm);
  assert(!(o.buttons&(SC_LTHUMB|SC_RTHUMB)));
  p.buttons=SC_LTHUMB|SC_RTHUMB;
  o=sc_manual_catch_translate(&c,&s,&m,p,1520,200);assert(m.requested==3 && !o.rx && !o.ry);
  sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1600,200);assert(m.phase==3);
  s.air=0;s.takeoff=0;sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1700,100);assert(m.phase==0 && !m.candidate);
  cases++;
 }
 /* Right-only and simultaneous catches have distinct requested masks. */
 for(int which=0;which<2;which++){
  sc_config c=config(1,2,1);sc_state s;sc_manual_catch m;
  launch(&c,&s,&m,1,-1,0,0);
  sc_pad p=feet(&c,1,-.6f,.8f,0,-1);
  p.buttons=which?(SC_LTHUMB|SC_RTHUMB):SC_RTHUMB;
  sc_pad o=sc_manual_catch_translate(&c,&s,&m,p,1400,200);
  assert(m.phase==3 && m.requested==(which?3:(m.front_left?2:1)));
  assert(!o.rx && !o.ry && !s.bf_side);
  o=sc_manual_catch_translate(&c,&s,&m,p,1600,200);
  assert(!o.rx && !o.ry && m.phase==3); /* held raw flick cannot re-hold */
  sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1620,200);
  assert(m.recentred);
 }
 /* A click held before launch is not a catch. Release/repress is required. */
 {
  sc_config c=config(0,0,0);c.body_flips=0;sc_state s;sc_manual_catch m;
  launch(&c,&s,&m,0,1,0,SC_LTHUMB);
  sc_pad p={0};p.buttons=SC_LTHUMB;
  sc_manual_catch_translate(&c,&s,&m,p,1400,200);assert(m.phase==2);
  sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1420,200);
  sc_manual_catch_translate(&c,&s,&m,p,1440,200);assert(m.phase==3);
 }
 /* Reframe the held endpoint once when the game's switch/mirror frame changes. */
 {
  sc_config c=config(0,2,1);sc_state s;sc_manual_catch m;launch(&c,&s,&m,0,1,0,0);
  sc_pad a=sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1400,200);
  s.game_mirror_known=s.game_mirror=1;s.switched=1;
  sc_pad b=sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1420,200);
  assert(sc_abs((float)a.rx+b.rx)<=2 && sc_abs((float)a.ry+b.ry)<=2);
 }
 /* Disabled catch is byte-for-byte normal translation, including ground R3
  * pump and rail bump. A fresh enable does not latch an old click. */
 for(int state=0;state<3;state++){
  sc_config c=config(0,0,0);c.manual_catch=0;c.bump_ms=80;
  sc_state a={0},b={0};a.game_crouch=b.game_crouch=-1;
  a.air=b.air=state==1;a.grind=b.grind=state==2;
  sc_manual_catch m={0};
  for(int i=0;i<50;i++){
   sc_pad p={0};p.lx=(short)(i*317);p.ly=(short)(i*-151);
   p.rx=(short)(i*-521);p.ry=(short)(i*-613);p.buttons=i<25?SC_RTHUMB:0;
   sc_pad x=sc_translate(&c,&a,p,2000+i*10);
   sc_pad y=sc_manual_catch_translate(&c,&b,&m,p,2000+i*10,state==1?200:100);
   assert(!memcmp(&x,&y,sizeof x) && !m.phase);
  }
 }
 /* Ground pumping / rail movement are also unchanged with catch enabled. */
 for(int rail=0;rail<2;rail++){
  sc_config c=config(0,0,0);c.bump_ms=80;sc_state a={0},b={0};a.grind=b.grind=rail;
  sc_manual_catch m={0};
  for(int i=0;i<8;i++){
   sc_pad p={0};p.buttons=i<4?SC_RTHUMB:0;
   sc_pad x=sc_translate(&c,&a,p,1000+i*20),y=sc_manual_catch_translate(&c,&b,&m,p,1000+i*20,rail?300:100);
   assert(!memcmp(&x,&y,sizeof x) && !m.phase);
  }
 }
 /* Ground diagonal setup alone is not a flip. Cancelled pop, rail contact,
  * invalid state, disabled controls and menu-style explicit cancellation clear. */
 {
  sc_config c=config(0,0,0);sc_state s={0};sc_manual_catch m={0};
  sc_pad p=feet(&c,0,0,0,.6f,-.8f);
  sc_manual_catch_translate(&c,&s,&m,p,1000,100);assert(!m.phase && !m.candidate);
  launch(&c,&s,&m,0,1,0,0);s.air=s.takeoff=0;m.was_air=0;
  sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},2000,100);assert(!m.phase);
  launch(&c,&s,&m,0,1,0,0);s.grind=1;
  sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1400,300);assert(!m.phase);
  launch(&c,&s,&m,0,1,0,0);
  sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1400,-1);assert(!m.phase);
  launch(&c,&s,&m,0,1,0,0);c.manual_catch=0;
  sc_manual_catch_translate(&c,&s,&m,(sc_pad){0},1400,200);assert(!m.phase);
  c.manual_catch=1;launch(&c,&s,&m,0,1,0,0);sc_mc_cancel(&m,SC_LTHUMB);assert(!m.phase && !m.candidate);
 }
 printf("PASS: %d held-flip/catch combinations\n",cases);
 return 0;
}


