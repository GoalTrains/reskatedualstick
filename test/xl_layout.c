#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/menu_scheme.h"
static void foot(sc_pad* p,int left,short x,short y){if(left){p->lx=x;p->ly=y;}else{p->rx=x;p->ry=y;}}
static sc_config config(int goofy){sc_config c={0};c.enabled=c.pure=c.xl_layout=1;c.switch_flip=3;c.goofy=goofy;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.windup_deg=75;c.one_foot_flips=1;return c;}
int main(void){
 /* Literal user-confirmed regular-skater cases, independent of the layout
  * helper: R DOWN ollie->nollie, L UP nollie->switch ollie. */
 const int cases[4][4]={{0,0,-32768,-1},{1,0,-32768,-1},{0,1,32767,1},{1,1,32767,1}};
 for(int i=0;i<4;i++){
  sc_config c=config(0);c.pop_on_release=1;sc_state s={0};s.switched=cases[i][0];s.game_crouch=-1;sc_pad p={0};
  sc_translate(&c,&s,p,100);foot(&p,cases[i][1],0,(short)cases[i][2]);
  sc_pad out=sc_translate(&c,&s,p,200);assert(out.ry*cases[i][3]>30000);
  sc_translate(&c,&s,p,400);p=(sc_pad){0};out=sc_translate(&c,&s,p,500);
  assert(s.fired==1 && out.ry*cases[i][3]<-30000);
 }
 const int goofy_cases[4][4]={{0,1,-32768,-1},{1,1,-32768,-1},{0,0,32767,1},{1,0,32767,1}};
 for(int i=0;i<4;i++){
  sc_config c=config(1);c.pop_on_release=1;sc_state s={0};s.switched=goofy_cases[i][0];s.game_crouch=-1;sc_pad p={0};
  sc_translate(&c,&s,p,100);foot(&p,goofy_cases[i][1],0,(short)goofy_cases[i][2]);
  sc_pad out=sc_translate(&c,&s,p,200);assert(out.ry*goofy_cases[i][3]>30000);
  sc_translate(&c,&s,p,400);p=(sc_pad){0};out=sc_translate(&c,&s,p,500);
  assert(s.fired==1 && out.ry*goofy_cases[i][3]<-30000);
 }
 for(int goofy=0;goofy<2;goofy++)for(int sw=0;sw<2;sw++)for(int front=0;front<2;front++){
  sc_config c=config(goofy);c.pop_on_release=1;sc_state s={0};s.switched=sw;s.game_crouch=-1;
  int left_front=sc_menu_left_front(0,goofy,3,sw,0),setup_left=front?left_front:!left_front;
  short setup_y=(setup_left!=goofy)?32767:-32768;short flick_y=setup_y>0?-32768:32767;
  int outgoing_sign=sw?-1:1;
  sc_pad p={0};sc_translate(&c,&s,p,1000);foot(&p,setup_left,0,setup_y);
  sc_pad out=sc_translate(&c,&s,p,1100);assert(s.feet_map_left==left_front);
  assert((front?1:-1)*out.ry*outgoing_sign>30000);
  sc_translate(&c,&s,p,1300);p=(sc_pad){0};out=sc_translate(&c,&s,p,1400);
  assert(s.fired==1);assert((front?1:-1)*out.ry*outgoing_sign<-30000);
  for(int side=-1;side<=1;side++){
   c.pop_on_release=0;s=(sc_state){0};s.switched=sw;s.game_crouch=-1;p=(sc_pad){0};
   sc_translate(&c,&s,p,2000);foot(&p,setup_left,0,setup_y);sc_translate(&c,&s,p,2100);sc_translate(&c,&s,p,2300);
   foot(&p,!setup_left,side*23166,side?(flick_y>0?23166:-23166):flick_y);out=sc_translate(&c,&s,p,2400);
   assert((front?1:-1)*out.ry*outgoing_sign<-20000);
   if(side)assert(out.rx*side>20000);
  }
  c.air_controls=1;s=(sc_state){0};s.switched=sw;s.game_crouch=-1;s.grind=1;p=(sc_pad){0};
  sc_translate(&c,&s,p,2500);foot(&p,setup_left,0,setup_y);sc_translate(&c,&s,p,2600);sc_translate(&c,&s,p,2750);
  foot(&p,!setup_left,0,flick_y);out=sc_translate(&c,&s,p,2800);assert(s.grind_pop);
  assert((front?1:-1)*out.ry*outgoing_sign<-20000);
  /* Old physical layout still uses front UP and back DOWN. */
  c.xl_layout=0;c.pop_on_release=1;s=(sc_state){0};s.switched=sw;s.game_crouch=-1;p=(sc_pad){0};
  sc_translate(&c,&s,p,3000);foot(&p,setup_left,0,front?32767:-32768);out=sc_translate(&c,&s,p,3100);
  assert(front?out.ry>30000:out.ry<-30000);
 }
 for(int kind=0;kind<3;kind++){
  int fb,mode,xl;sc_menu_choose_input_layout(kind,&fb,&mode,&xl);assert(sc_menu_input_layout_kind(fb,mode,xl)==kind);
  assert(sc_menu_next_input_layout(kind,1)==(kind+1)%3);assert(sc_menu_next_input_layout(kind,-1)==(kind+2)%3);
 }
 assert(sc_menu_input_layout_kind(0,1,0)==3);
 assert(!sc_menu_xl_card_y(1,0));assert(sc_menu_xl_card_y(1,1));assert(!sc_menu_xl_card_y(0,1));
 /* Remount can retain switch riding while the live native input frame clears.
  * Use that live frame for output without losing physical foot ownership. */
 for(int goofy=0;goofy<2;goofy++)for(int native=0;native<2;native++) {
  sc_config c=config(goofy);sc_state s={0};s.switched=1;s.game_switch_known=1;s.game_switch=native;s.game_crouch=-1;
  sc_pad p={0};sc_translate(&c,&s,p,5000);if(goofy)p.ly=-32768;else p.ry=-32768;
  sc_pad out=sc_translate(&c,&s,p,5100);assert(s.feet_map_left==goofy);
  assert(native?out.ry<-30000:out.ry>30000);
 }
 /* Physical steering, pumping and powerslides do not use foot-axis remapping. */
 sc_config c=config(0);c.stick_steer=2;c.steer_scale=c.steer_curve=1;c.powerslide=1;
 sc_state s={0};s.game_crouch=-1;sc_pad p={0};p.lx=30000;
 sc_pad out=sc_translate(&c,&s,p,10000);assert(s.st_latch && out.lx>20000);
 s=(sc_state){0};s.game_crouch=-1;p=(sc_pad){0};p.lx=30000;p.rx=-30000;
 out=sc_translate(&c,&s,p,11000);assert(s.ps_side && out.ly<0);
 s=(sc_state){0};s.game_crouch=-1;p=(sc_pad){0};p.buttons=SC_RTHUMB;
 sc_translate(&c,&s,p,12000);out=sc_translate(&c,&s,p,12100);assert(out.rt==255 && out.rx==0 && out.ry==0);
 puts("PASS: XL fixed physical setup directions, swapping release pops and two-foot diagonal flicks, regular/goofy normal/switch both ends; old physical setup unchanged, three menu layouts and matching setup/flick diagram directions");
}
