#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
/* Reversing a physical gesture and enabling the matching switch inversion
 * must reproduce the original foot output, in both layouts and stances. */
static sc_pad reverse(sc_pad p,int x,int y){
 if(x){p.lx=-p.lx;p.rx=-p.rx;}if(y){p.ly=-p.ly;p.ry=-p.ry;}return p;
}
int main(void){
 for(int g=0;g<2;g++)for(int fb=0;fb<2;fb++)for(int sw=0;sw<2;sw++)
 for(int x=0;x<2;x++)for(int y=0;y<2;y++)for(int release=0;release<2;release++){
  sc_config a={0};a.enabled=a.pure=a.air_controls=1;a.goofy=g;a.front_back=fb;
  a.switch_flip=fb?1:3;a.deadzone=.15f;a.active=.3f;a.manual_max=.85f;a.pop_on_release=release;
  sc_config b=a;b.switch_invert_x=x;b.switch_invert_y=y;
  sc_state sa={0},sb={0};sa.switched=sb.switched=sw;sa.game_crouch=sb.game_crouch=-1;
  int lf=fb?1:((!g)!=sw);
  sc_pad p[5]={{0},{0},{0},{0},{0}};
  if(lf)p[1].ry=-30000;else p[1].ly=-30000;p[2]=p[1];p[3]=p[2];
  if(lf){p[3].lx=-22000;p[3].ly=22000;}else{p[3].rx=-22000;p[3].ry=22000;}
  for(int i=0;i<5;i++){
   sc_pad oa=sc_translate(&a,&sa,p[i],1000+i*150);
   sc_pad ob=sc_translate(&b,&sb,reverse(p[i],sw&&x,sw&&y),1000+i*150);
   /* Signed axes use 32768 negative / 32767 positive; allow two units of
    * quantisation while requiring identical gesture/button behaviour. */
   if(!(sc_abs(oa.rx-ob.rx)<=2 && sc_abs(oa.ry-ob.ry)<=2 && sc_abs(oa.lx-ob.lx)<=2 && sc_abs(oa.ly-ob.ly)<=2 && oa.buttons==ob.buttons)){
    printf("Mismatch g=%d fb=%d sw=%d x=%d y=%d release=%d i=%d out=%d,%d,%d,%d vs %d,%d,%d,%d\n",g,fb,sw,x,y,release,i,oa.lx,oa.ly,oa.rx,oa.ry,ob.lx,ob.ly,ob.rx,ob.ry);fflush(stdout);return 1;
   }
  }
 }
 for(int x=0;x<2;x++)for(int y=0;y<2;y++){
  sc_config c={0};c.enabled=c.pure=1;c.switch_flip=3;c.switch_invert_x=x;c.switch_invert_y=y;
  c.stick_steer=2;c.steer_scale=c.steer_curve=1;c.deadzone=.15f;c.active=.3f;
  sc_state s={0};s.switched=1;sc_pad p={0};p.lx=30000;
  sc_pad o=sc_translate(&c,&s,p,1000);assert(s.st_latch && o.lx>25000);
  c.stick_steer=0;c.powerslide=1;c.manual_max=.85f;
  sc_state slide={0};slide.switched=1;p=(sc_pad){0};p.lx=30000;p.rx=-30000;
  o=sc_translate(&c,&slide,p,2000);assert(slide.ps_side && o.ly<0);
  sc_config baseline=c;baseline.switch_invert_x=baseline.switch_invert_y=0;
  sc_state original={0};original.switched=1;
  sc_pad expected=sc_translate(&baseline,&original,p,2000);
  assert(o.lx==expected.lx && o.ly==expected.ly && o.rx==expected.rx && o.ry==expected.ry);
 }
 puts("PASS: switch axis choices preserve equivalent ollie/flip/release gestures across regular/goofy, both layouts; normal inputs, physical steering and powerslides unchanged");
}
