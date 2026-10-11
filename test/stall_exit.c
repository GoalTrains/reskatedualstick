#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"

static void check(int goofy,int layout,int mode,int switched,int right,int direction) {
    sc_config c={0}; c.enabled=1; c.pure=1; c.air_controls=1;
    c.deadzone=.15f; c.active=.3f; c.manual_max=.85f;
    c.steer_scale=1; c.steer_curve=1; c.bump_ms=250;
    c.goofy=goofy; c.front_back=layout; c.switch_flip=mode;
    sc_state s={0}; s.grind=1; s.switched=switched;
    sc_pad p={0};
    sc_translate(&c,&s,p,1000);
    if(right) p.ry=(short)(direction*25000); else p.ly=(short)(direction*25000);
    sc_pad o=sc_translate(&c,&s,p,1010);
    assert(o.ly==0); /* Bare foot tilt still adjusts the board. */
    p.buttons=right?SC_RTHUMB:SC_LTHUMB;
    o=sc_translate(&c,&s,p,1020);
    assert(o.lx==0 && (direction<0?o.ly< -32000:o.ly>32000));
    assert(!(o.buttons&(SC_LTHUMB|SC_RTHUMB)));
    assert(s.d_log==7 && s.d_log_b==direction);
    p=(sc_pad){0};
    o=sc_translate(&c,&s,p,1200);
    assert(direction<0?o.ly< -32000:o.ly>32000);
    o=sc_translate(&c,&s,p,1271); assert(o.ly==0);
    p.buttons=right?SC_RTHUMB:SC_LTHUMB;
    o=sc_translate(&c,&s,p,1300);
    assert(o.ly==0 && (right?o.lx>32000:o.lx< -32000));
}
int main(void) {
    for(int g=0;g<2;g++) for(int l=0;l<2;l++) for(int m=0;m<4;m++)
    for(int sw=0;sw<2;sw++) for(int r=0;r<2;r++) for(int d=-1;d<=1;d+=2)
        check(g,l,m,sw,r,d);
    sc_config c={0};c.enabled=1;c.pure=1;c.bump_ms=250;c.active=.3f;
    c.deadzone=.15f;c.steer_scale=1;c.steer_curve=1;
    sc_state s={0};s.grind=1;
    /* Actual attempted exit from the captured stall, QPC 117119924626. */
    sc_pad p={0};p.lx=11264;p.ly=-19713;p.buttons=SC_LTHUMB;
    sc_pad o=sc_translate(&c,&s,p,1000);
    assert(o.lx==0 && o.ly< -32000);
    s=(sc_state){0};s.grind=1;p.buttons=SC_LTHUMB|SC_RTHUMB;
    o=sc_translate(&c,&s,p,1000);assert(s.bump_until==0);
    s=(sc_state){0};s.grind=1;c.bump_ms=0;p.buttons=SC_LTHUMB;
    o=sc_translate(&c,&s,p,1000);assert(o.ly==0 && s.bump_until==0);
    s=(sc_state){0};c.bump_ms=250;
    o=sc_translate(&c,&s,p,1000);assert(o.ly==0 && s.bump_until==0);
    puts("PASS: stall input, both sticks, regular/goofy, all stance modes/layouts, hold/expiry, rail bumps, combined clicks, disabled bumps, ground");
}
