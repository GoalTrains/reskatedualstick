#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/overlay_handoff.h"
int main(void) {
    sc_config c={0}; c.enabled=c.pure=c.front_back=1; c.active=.3f;c.deadzone=.15f;
    c.manual_max=.85f;c.steer_scale=c.steer_curve=1;
    for(int goofy=0;goofy<2;goofy++)for(int pop=0;pop<2;pop++) {
        c.goofy=goofy;c.pop_on_release=pop;
        sc_state s={0};s.ps_side=1;s.stock_air_active=1;s.d_back_on=1;s.pw_side=1;
        sc_overlay_suspend(&s);
        assert(!s.ps_side && !s.stock_air_active && !s.d_back_on && !s.pw_side);
        sc_pad p={0};p.ly=30000;p.ry=-30000;
        sc_pad o=sc_translate(&c,&s,p,1000);
        assert(!s.d_front_on && !s.d_back_on && !o.ry);
        p=(sc_pad){0};sc_translate(&c,&s,p,1020);
        assert(!s.d_front_ignored && !s.d_back_ignored);
        p.ry=-30000;sc_translate(&c,&s,p,1040);assert(s.d_back_on);
    }
    puts("PASS: overlay suspension clears gestures; held feet are guarded until release; fresh controls resume");
}
