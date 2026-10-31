#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad point(int left,int y,float angle){sc_pad p={0};short x=sc_short(.95f*sc_sin_deg(angle)*y),v=sc_short(-.95f*sc_sin_deg(90-angle)*y);if(left){p.lx=x;p.ly=v;}else{p.rx=x;p.ry=v;}return p;}
int main(void){int count=0;
 for(int g=0;g<2;g++)for(int layout=0;layout<3;layout++)for(int sw=0;sw<2;sw++)
 for(int swapped=0;swapped<2;swapped++)for(int invert=0;invert<2;invert++)
 for(int mode=0;mode<4;mode++)for(int one=0;one<2;one++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.goofy=g;c.front_back=layout==0;c.xl_layout=layout==2;
  c.switch_flip=mode;c.feet_swapped=swapped;c.switch_invert_y=invert;c.one_foot_flips=one;
  c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.revert_input=4;c.revert_ms=300;c.revert_shape=5;c.revert_sweep_ms=300;
  sc_state s={0};s.switched=sw;s.game_crouch=0;
  int front=layout==0?1:!g;front^=swapped;if(layout!=0&&mode==3)front^=sw;
  int left=layout!=0&&mode==3?!front:sw?front:!front;
  int y=left==front?-1:1;if(sw&&((layout==2&&mode==3)!=invert))y=-y;
  sc_translate(&c,&s,(sc_pad){0},1000);
  for(int i=0;i<6;i++){sc_pad p=point(left,y,100+i*25);sc_translate(&c,&s,p,1020+i*20);}
  assert(s.rv_start>0 && s.rv_side==-1);count++;
 }
 /* A published physical switch frame may outlast the direct switch flag. */
 {sc_config c={0};c.enabled=c.pure=c.air_controls=c.front_back=1;c.switch_flip=1;c.revert_input=4;c.active=.3f;c.deadzone=.15f;c.manual_max=.85f;
  sc_state s={0};s.game_crouch=0;s.game_mirror_known=1;s.xl_absolute_raw=1;
  for(int i=0;i<6;i++)sc_translate(&c,&s,point(1,-1,100+i*25),1000+i*20);
  assert(s.rv_start>0);}
 /* A stance publication cannot join half of one stick's arc to the other. */
 {sc_config c={0};c.enabled=c.pure=c.front_back=1;c.switch_flip=1;c.revert_input=4;c.active=.3f;c.deadzone=.15f;
  sc_state s={0};sc_translate(&c,&s,point(0,1,100),1000);assert(s.top_revert.armed);
  s.switched=1;sc_translate(&c,&s,point(0,1,150),1020);assert(!s.rv_start && !s.top_revert.armed);
  sc_translate(&c,&s,(sc_pad){0},1040);
  for(int i=0;i<6;i++)sc_translate(&c,&s,point(1,-1,100+i*25),1060+i*20);
  assert(s.rv_start>0);}
 printf("PASS: %d unused-foot arc contracts: all layouts, regular/goofy, switch, foot swaps, vertical inversion, legacy switch modes and one-foot options; physical-frame fallback and stance-change cancellation\n",count);
}
