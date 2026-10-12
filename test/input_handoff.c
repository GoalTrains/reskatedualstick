#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
#include "../src/stance_track.h"
int main(void) {
    sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.stock_grab_air=1;
    c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.grind_sensitivity=1.2f;
    c.steer_curve=c.steer_scale=1;c.revert_input=2;c.revert_ms=300;
    for(int side=0;side<2;side++) {
        sc_state s={0};sc_pad p={0};s.stock_ramp_eligible=1;s.air=s.takeoff=1;
        if(side)p.rt=255;else p.lt=255;
        sc_translate(&c,&s,p,1000);assert(s.stock_air_active);
        s.air=s.takeoff=0;
        sc_pad o=sc_translate(&c,&s,p,1100);
        assert(!s.stock_air_active && !s.rv_hold && !s.rv_start);
        assert(side?o.lx>30000:o.lx<-30000);
        o=sc_translate(&c,&s,p,1120);assert(side?o.lx>30000:o.lx<-30000);
    }
    c.stock_grab_air=0;
    for(int foot=0;foot<2;foot++) {
        sc_state s={0};sc_pad p={0};s.takeoff=1;s.air=0;
        if(foot)p.rx=18000;else p.lx=18000;
        sc_pad o=sc_translate(&c,&s,p,2000);
        assert(foot?o.rx<-16000:o.rx>16000);
        /* A foot held from pop may pivot before the delayed air flag. */
        s=(sc_state){0};s.takeoff=1;s.d_took_off=1;
        if(foot){s.d_back_on=s.d_back_spent=1;s.tk_ba=90;}
        else {s.d_front_on=s.d_front_spent=1;s.tk_fa=90;}
        o=sc_translate(&c,&s,p,2020);assert(foot?o.rx<-16000:o.rx>16000);
    }
    sc_stance_track t={0};
    assert(sc_resolve_stance(&t,4,0,1,1,100,0,0,0,0)==0);
    assert(sc_resolve_stance(&t,4,0,1,1,100,1,0,0,34)==0);
    assert(sc_resolve_stance(&t,4,0,1,1,100,0,0,0,69)==0);
    assert(sc_resolve_stance(&t,4,0,1,1,201,1,0,0,100)==1);
    assert(sc_resolve_stance(&t,4,0,1,1,100,0,0,1,130)==1);
    assert(sc_resolve_stance(&t,4,0,1,1,100,0,0,1,210)==0);
    puts("PASS: held grab becomes immediate ground steering without revert; front/back pivots before delayed air flag; transient ground stance rejected, air turns immediate");
}
