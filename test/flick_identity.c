#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static void foot(sc_pad *p,int leftfront,int front,int x,int y){if(front==leftfront){p->lx=x;p->ly=y;}else{p->rx=x;p->ry=y;}}
int main(void){for(int g=0;g<2;g++)for(int sw=0;sw<2;sw++)for(int fb=0;fb<2;fb++)for(int first=0;first<2;first++)for(int same=0;same<2;same++){
 sc_config c={0};c.enabled=c.pure=1;c.goofy=g;c.front_back=fb;c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
 sc_state s={0};s.switched=sw;s.game_crouch=-1;sc_pad p={0},a,b;int lf=fb?1:((!g)!=sw);
 sc_translate(&c,&s,p,1000);foot(&p,lf,first,0,first?32767:-32768);sc_translate(&c,&s,p,1100);sc_translate(&c,&s,p,1400);
 if(same)p=(sc_pad){0};foot(&p,lf,!first,-32768,first?-4096:4096);a=sc_translate(&c,&s,p,1420);
 p=(sc_pad){0};foot(&p,lf,!first,-32768,first?-4096:4096);b=sc_translate(&c,&s,p,1460);
 assert(a.rx==b.rx && a.ry==b.ry);assert(sc_abs(b.ry)>20000);
 p=(sc_pad){0};sc_translate(&c,&s,p,1500);assert(!s.d_front_flick && !s.d_back_flick);
 /* The same physical foot used alone is still constrained. */
 foot(&p,lf,!first,0,first?-32768:32767);sc_translate(&c,&s,p,1600);
 foot(&p,lf,!first,-32768,first?-4096:4096);b=sc_translate(&c,&s,p,1650);assert(sc_abs(b.ry)<15000);
}puts("PASS: second-foot identity survives setup release/same-poll handoff, both pop ends, stances/layouts; neutral rearms one-stick blocker");}
