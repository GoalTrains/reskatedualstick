#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/manual_catch.h"
static sc_pad foot(const sc_config*c,int sw,int back,float a){
 sc_pad p={0};float x,y;sc_vec_at(back,a,1,&x,&y);
 if(c->xl_layout && sw)y=-y;
 int left=c->front_back?1:(!c->goofy)!=sw;
 if(back!=left){p.lx=sc_short(x);p.ly=sc_short(y);}else{p.rx=sc_short(x);p.ry=sc_short(y);}return p;
}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int one=0;one<2;one++)for(int back=0;back<2;back++)for(int side=-1;side<=1;side+=2)
 for(int degrees=90;degrees<=360;degrees*=2)for(int start=-90;start<=0;start+=90){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.manual_catch=c.body_flips=1;
  c.goofy=g;c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=c.front_back?1:3;
  c.windup_deg=60;c.one_foot_flips=one;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;
  sc_state s={0},normal={0};s.switched=normal.switched=sw;s.game_crouch=normal.game_crouch=-1;
  sc_manual_catch m={0};sc_pad z={0},last={0};
  sc_manual_catch_translate(&c,&s,&m,z,1000,100);sc_translate(&c,&normal,z,1000);
  for(int a=0;a<=degrees;a+=15){
   double now=1100+a*2;int state=a>=degrees-30?103:100;
   sc_pad p=foot(&c,sw,back,(float)((a+start)*side));
   sc_pad out=sc_manual_catch_translate(&c,&s,&m,p,now,state);
   last=sc_translate(&c,&normal,p,now);
   /* Never freeze a circle at pop: the complete emitted scoop must survive. */
   assert(sc_abs((float)out.rx-last.rx)<=2 && sc_abs((float)out.ry-last.ry)<=2);
   assert(m.phase==(state==103?2:0));
  }
  assert(m.candidate && m.candidate_kind==2);
  s.air=1;s.takeoff=2;
  sc_pad out=sc_manual_catch_translate(&c,&s,&m,z,1110+degrees*2,200);
  assert(m.phase==2 && (out.rx || out.ry));
  assert(sc_abs((float)out.rx-last.rx)<5 && sc_abs((float)out.ry-last.ry)<5);
  out=sc_manual_catch_translate(&c,&s,&m,z,1210+degrees*2,200);assert(m.phase==2 && (out.rx || out.ry));
  z.buttons=back?SC_RTHUMB:SC_LTHUMB;
  out=sc_manual_catch_translate(&c,&s,&m,z,1230+degrees*2,200);
  assert(m.phase==3 && !out.rx && !out.ry && !s.bf_side && !s.bf_arm);
  count++;
 }
 /* Recorded failure: strong scoop -> pop 103 -> physical stick eases off
  * for 40-70 ms before air. There must be no neutral/re-hold gap. */
 for(int back=0;back<2;back++)for(int side=-1;side<=1;side+=2){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.manual_catch=c.front_back=1;c.switch_flip=1;
  c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.windup_deg=60;
  sc_state s={0};s.game_crouch=-1;sc_manual_catch m={0};sc_pad z={0};
  sc_manual_catch_translate(&c,&s,&m,z,1000,100);
  for(int a=0;a<=90;a+=15)sc_manual_catch_translate(&c,&s,&m,foot(&c,0,back,(float)(a*side)),1100+a,100);
  sc_manual_catch_translate(&c,&s,&m,foot(&c,0,back,90.f*side),1220,103);assert(m.phase==2);
  for(int t=1224;t<1300;t+=4){sc_pad out=sc_manual_catch_translate(&c,&s,&m,z,t,103);assert(m.phase==2 && (out.rx || out.ry));}
  s.air=1;s.takeoff=2;sc_pad out=sc_manual_catch_translate(&c,&s,&m,z,1300,200);assert(m.phase==2 && (out.rx || out.ry));
 }
 printf("PASS: %d single-foot shuv/360/impossible paths; complete preair circle preserved, no neutral gap through 103 release, automatic hold after release, click catches\n",count);
}

