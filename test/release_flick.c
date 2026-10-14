#include <stdio.h>
#include <assert.h>
#include "../src/translate.h"
static void foot(sc_pad *p,sc_config *c,int sw,int front,int y){
 int left=c->front_back?1:!c->goofy;
 if(!c->front_back && c->switch_flip==3 && sw)left=!left;
 if(front==left)p->ly=y;else p->ry=y;
}
int main(void){
for(int goofy=0;goofy<2;goofy++)for(int sw=0;sw<2;sw++)for(int layout=0;layout<2;layout++)for(int first=0;first<2;first++)for(int handoff=0;handoff<3;handoff++){
 sc_config c={0};c.enabled=c.pure=c.air_controls=c.pop_on_release=1;c.goofy=goofy;c.front_back=layout;c.switch_flip=3;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;
 sc_state s={0};s.switched=sw;s.game_crouch=-1;sc_pad p={0},o;
 sc_translate(&c,&s,p,1000);
 foot(&p,&c,sw,first,first?32767:-32768);sc_translate(&c,&s,p,1100);sc_translate(&c,&s,p,1500);
 if(handoff==1)p=(sc_pad){0};
 if(handoff==2){p=(sc_pad){0};sc_translate(&c,&s,p,1510);}
 foot(&p,&c,sw,!first,first?-32768:32767);o=sc_translate(&c,&s,p,1530);
 assert(s.rp_two_foot);unsigned fired=s.fired;
 /* Release setup while keeping the flick, then release the flick. */
 p=(sc_pad){0};foot(&p,&c,sw,!first,first?-32768:32767);sc_translate(&c,&s,p,1630);
 p=(sc_pad){0};o=sc_translate(&c,&s,p,1650);
 assert(s.fired==fired && s.rp_t==0 && o.ry==0 && !s.rp_two_foot);
 /* A fresh one-foot setup/release is allowed after both sticks centre. */
 foot(&p,&c,sw,first,first?32767:-32768);sc_translate(&c,&s,p,1800);sc_translate(&c,&s,p,2000);
 p=(sc_pad){0};o=sc_translate(&c,&s,p,2020);assert(s.fired==fired+1 && s.rp_t>0 && o.ry!=0);
}
puts("PASS: no extra release pop after overlapping/same-poll/short-gap two-foot flicks; fresh one-foot ollie/nollie works, regular/goofy, switch, both layouts");
}
