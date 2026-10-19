#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/manual_catch.h"
static sc_config config(int g,int layout){
 sc_config c={0};c.enabled=c.pure=c.manual_catch=c.air_controls=c.body_flips=1;
 c.goofy=g;c.front_back=layout==0;c.xl_layout=layout==2;c.switch_flip=c.front_back?1:3;
 c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;return c;
}
static sc_pad gesture(const sc_config*c,int sw){
 sc_pad p={0};int left=c->front_back?1:(!c->goofy)!=sw;
 float y=c->xl_layout && sw?-.8f:.8f;
 if(left){p.lx=sc_short(.6f);p.ly=sc_short(y);}else{p.rx=sc_short(.6f);p.ry=sc_short(y);}return p;
}
static void warm(sc_config*c,sc_state*s,sc_manual_catch*m,int sw,int caught){
 *s=(sc_state){0};*m=(sc_manual_catch){0};s->air=1;s->takeoff=2;s->game_crouch=-1;s->switched=sw;
 sc_manual_catch_translate_observed(c,s,m,(sc_pad){0},1000,200,2,1);
 sc_manual_catch_translate_observed(c,s,m,(sc_pad){0},1020,200,2,1);
 if(caught){m->phase=3;m->requested=3;m->recentred=1;m->release_until=1050;}
}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int caught=0;caught<2;caught++)for(int same=0;same<2;same++){
  sc_config c=config(g,layout);sc_state s;sc_manual_catch m;warm(&c,&s,&m,sw,caught);
  if(same)sc_manual_catch_translate_observed(&c,&s,&m,(sc_pad){0},1090,200,2,0);
  sc_pad out=sc_manual_catch_translate_observed(&c,&s,&m,gesture(&c,sw),1100,200,2,same?0:1);
  assert(m.phase==(caught?3:0));assert(m.emitted_seen && (out.rx||out.ry));
  short x=out.rx,y=out.ry;
  /* Native recognition arrives after physical release. Retain its actual vector. */
  out=sc_manual_catch_translate_observed(&c,&s,&m,(sc_pad){0},1120,200,same?2:37,1);
  assert(m.phase==2 && m.candidate_kind==3 && !m.requested);
  assert(sc_abs((float)out.rx-x)<=3 && sc_abs((float)out.ry-y)<=3);
  out=sc_manual_catch_translate_observed(&c,&s,&m,(sc_pad){0},1300,200,same?2:37,1);
  assert(m.phase==2 && (out.rx||out.ry));
  sc_pad click={0};click.buttons=SC_LTHUMB|SC_RTHUMB;
  out=sc_manual_catch_translate_observed(&c,&s,&m,click,1320,200,same?2:37,1);
  assert(m.phase==3 && m.requested==3 && !out.rx && !out.ry && !s.bf_side);
  count++;
 }
 /* Strong board adjustments alone, stale vectors, first/unknown native samples,
  * a zero/none enum and unchanged active tricks never arm a new hold. */
 for(int bad=0;bad<5;bad++){
  sc_config c=config(0,0);sc_state s;sc_manual_catch m;warm(&c,&s,&m,0,1);
  sc_manual_catch_translate_observed(&c,&s,&m,gesture(&c,0),1100,200,2,1);
  if(bad==1)sc_manual_catch_translate_observed(&c,&s,&m,(sc_pad){0},1110,200,-1,-1);
  sc_manual_catch_translate_observed(&c,&s,&m,(sc_pad){0},bad==0?1300:1120,200,
   bad==2?0:bad==3?-1:bad==4?2:37,bad==3?-1:1);
  assert(m.phase==3);
 }
 /* A click on the recognition poll catches immediately and cannot body flip. */
 {sc_config c=config(0,0);sc_state s;sc_manual_catch m;warm(&c,&s,&m,0,1);
 sc_manual_catch_translate_observed(&c,&s,&m,gesture(&c,0),1100,200,2,1);
 sc_pad p={0};p.buttons=SC_RTHUMB;
 sc_pad out=sc_manual_catch_translate_observed(&c,&s,&m,p,1120,200,37,1);
 assert(m.phase==3 && m.requested && !out.rx && !out.ry && !s.bf_side && !s.bf_arm);
 assert(!(out.buttons&(SC_LTHUMB|SC_RTHUMB)));}
 printf("PASS: %d airborne/repeated-trick re-arms require fresh native recognition plus emitted input; delayed recognition retains endpoint, catch clicks win, unknown/stale/board adjustments cannot arm\n",count);
}
