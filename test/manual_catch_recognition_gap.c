#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/manual_catch.h"
static sc_pad foot(int back,float angle){
 sc_pad p={0};float x,y;sc_vec_at(back,angle,1,&x,&y);
 if(back){p.rx=sc_short(x);p.ry=sc_short(y);}else{p.lx=sc_short(x);p.ly=sc_short(y);}return p;
}
static void prepare(sc_config*c,sc_state*s,sc_manual_catch*m,int sw,int back,int side,int full){
 *s=(sc_state){0};*m=(sc_manual_catch){0};s->game_crouch=-1;s->switched=sw;
 sc_manual_catch_translate_observed(c,s,m,(sc_pad){0},1000,100,0,0);
 for(int a=full?-90:0;a<=90;a+=15)
  sc_manual_catch_translate_observed(c,s,m,foot(back,a*side),1100+(a+90)*2,100,0,0);
 assert(m->candidate && m->phase==0);
}
int main(void){int cases=0;int ids[]={2,4,7,10,13};
 for(int g=0;g<2;g++)for(int sw=0;sw<2;sw++)for(int back=0;back<2;back++)
 for(int side=-1;side<=1;side+=2)for(int full=0;full<2;full++)for(unsigned id=0;id<5;id++)for(int delay=0;delay<3;delay++)for(int unknown=0;unknown<2;unknown++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.manual_catch=c.front_back=1;
  c.goofy=g;c.switch_flip=1;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.windup_deg=60;
  sc_state s;sc_manual_catch m;prepare(&c,&s,&m,sw,back,side,full);
  int state=sw?101:100;
  if(unknown)sc_manual_catch_translate_observed(&c,&s,&m,foot(back,90.f*side),1462,state,0,-1);
  sc_pad held=sc_manual_catch_translate_observed(&c,&s,&m,foot(back,90.f*side),1464,state,ids[id],delay==0?1:delay==1?0:-1);
  assert(m.phase==2 && (held.rx || held.ry));
  /* Recorded failures centred 4-48 ms before state103, after recognition.
   * Keep native held intent continuous throughout that riding-state gap. */
  for(int t=1468;t<=1512;t+=4){
   sc_pad out=sc_manual_catch_translate_observed(&c,&s,&m,(sc_pad){0},t,state,ids[id],delay && t<1496?(delay==1?0:-1):1);
   assert(m.phase==2 && out.rx==held.rx && out.ry==held.ry);
  }
  sc_pad out=sc_manual_catch_translate_observed(&c,&s,&m,(sc_pad){0},1516,103,ids[id],1);
  assert(out.rx==held.rx && out.ry==held.ry);
  s.air=1;s.takeoff=2;out=sc_manual_catch_translate_observed(&c,&s,&m,(sc_pad){0},1616,200,ids[id],1);
  assert(m.phase==2 && out.rx==held.rx && out.ry==held.ry);
  sc_pad click={0};click.buttons=SC_RTHUMB;
  out=sc_manual_catch_translate_observed(&c,&s,&m,click,1620,200,ids[id],1);
  assert(m.phase==3 && !out.rx && !out.ry);
  cases++;
 }
 /* A cached input alone, unknown/unchanged native samples, or expired input
  * must not freeze an unaccepted setup while still riding. */
 for(int invalid=0;invalid<7;invalid++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.manual_catch=c.front_back=1;
  c.switch_flip=1;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.windup_deg=60;
  sc_state s;sc_manual_catch m;prepare(&c,&s,&m,0,1,1,0);
  int id=4,doing=1;double now=1464;
  if(invalid==0){id=-1;doing=-1;}
  if(invalid==1){id=0;doing=0;}
  if(invalid==2){m.observed_id=m.id_previous=4;m.observed_doing=1;}
  if(invalid==3)m.id_seen=m.observed_seen=0;
  if(invalid==4)now=1700;
  if(invalid==5){m.observed_id=m.id_previous=4;m.observed_doing=0;doing=0;}
  if(invalid==6){m.id_previous=4;doing=-1;}
  sc_manual_catch_translate_observed(&c,&s,&m,(sc_pad){0},now,100,id,doing);assert(!m.phase);
 }
 puts("PASS: native recognition cannot arm a stationary crouch or positioning input without a captured trick gesture");
 {sc_config c={0};c.enabled=c.pure=c.manual_catch=c.front_back=1;c.active=.3f;c.deadzone=.15f;
 sc_state s={0};s.game_crouch=-1;sc_manual_catch m={0};
 sc_manual_catch_translate_observed(&c,&s,&m,(sc_pad){0},1000,100,0,0);
 sc_manual_catch_translate_observed(&c,&s,&m,(sc_pad){0},1010,100,4,1);assert(!m.phase);}
 printf("PASS: %d recognized gestures retain the endpoint through pre-pop release, delayed/unavailable DoingTrick, 103 and takeoff; catch clicks stop hold; unknown/stale/unchanged IDs cannot arm\n",cases);
}
