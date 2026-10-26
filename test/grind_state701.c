#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/stance_track.h"
static int classify(int state){
#ifdef SC_TEST_LEGACY_GRIND_STATES
 return state>=400 && state<500;
#else
 return sc_is_grind_state(state);
#endif
}
static sc_pad feet(const sc_config*c,int sw,int foot,int x,int y){
 float fx=foot==1?sc_axis((short)x):0,fy=foot==1?sc_axis((short)y):0;
 float bx=foot==2?sc_axis((short)x):0,by=foot==2?sc_axis((short)y):0;
 if(c->xl_layout&&sw){fy=-fy;by=-by;}
 int left=c->front_back?1:(!c->goofy)!=sw;sc_pad p={0};
 p.lx=sc_short(left?fx:bx);p.ly=sc_short(left?fy:by);p.rx=sc_short(left?bx:fx);p.ry=sc_short(left?by:fy);return p;
}
static void same(sc_pad a,sc_pad b){
 assert(a.lx==b.lx&&a.ly==b.ly&&a.rx==b.rx&&a.ry==b.ry);
 assert(a.buttons==b.buttons&&a.lt==b.lt&&a.rt==b.rt);
}
int main(void){unsigned frames=0;
 const int poses[][2]={{32767,-769},{32767,-1025},{30464,-15105},{-32768,-9217}};
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)for(int foot=1;foot<=2;foot++)for(int release=0;release<2;release++)for(int pose=0;pose<4;pose++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=c.split_grinds=1;c.front_back=layout==0;c.xl_layout=layout==2;c.goofy=g;
  c.switch_flip=c.front_back?1:3;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;c.grind_sensitivity=1.2f;c.pop_on_release=release;
  sc_state actual={0},reference={0};actual.switched=reference.switched=sw;actual.game_crouch=reference.game_crouch=-1;
  int x=poses[pose][0],y=foot==2?poses[pose][1]:-poses[pose][1];sc_pad p=feet(&c,sw,foot,x,y);
  /* A recorded rail/701/rail handoff, plus a sustained one-second 701 hold.
   * Compare all output channels against the same uninterrupted rail gesture. */
  for(int poll=0;poll<260;poll++){
   int state=poll==0||poll==259?403:701;
   actual.grind=classify(state);reference.grind=1;
   sc_pad a=sc_translate(&c,&actual,p,1000+poll*4),b=sc_translate(&c,&reference,p,1000+poll*4);
   same(a,b);assert(actual.split_grind_active && !actual.grind_pop);frames++;
  }
  p=(sc_pad){0};actual.grind=classify(701);reference.grind=1;
  same(sc_translate(&c,&actual,p,2200),sc_translate(&c,&reference,p,2200));
  assert(!actual.split_rail_front_load&&!actual.split_rail_back_load);
  /* Native air, ground, bail and real respawn still leave the rail immediately. */
  int exits[]={100,103,200,201,300,504,700,702};
  for(unsigned i=0;i<sizeof exits/sizeof exits[0];i++)assert(!classify(exits[i]));
 }
#ifndef SC_TEST_LEGACY_GRIND_STATES
 assert(sc_is_grind_state(701));for(int i=400;i<500;i++)assert(sc_is_grind_state(i));
 assert(!sc_is_grind_state(399)&&!sc_is_grind_state(500)&&!sc_is_grind_state(-1));
#endif
 /* Entering/leaving 701 preserves the same takeoff/landing stance history
  * as an ordinary grind, including a directly entered 701. */
 for(int direct=0;direct<2;direct++)for(int sw=0;sw<2;sw++){
  sc_stance_track a={0},b={0};int states[]={100,201,701,100};
  if(direct)states[1]=701;
  for(int step=0;step<4;step++){
   int r=step?1-sw:sw;
   int av=sc_track_stance(&a,states[step],r,0,sw,1000+step*100);
   int bv=sc_track_stance(&b,states[step]==701?403:states[step],r,0,sw,1000+step*100);
   assert(av==bv && a.reference==b.reference && a.takeoff_stance==b.takeoff_stance);
  }
 }
 printf("PASS: %u recorded-pose grind frames preserve all controls through short and sustained state 701, with release and genuine state exits\n",frames);
}
