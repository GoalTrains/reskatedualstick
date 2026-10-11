#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
int main(void) {
    for(int g=0;g<2;g++)for(int sw=0;sw<2;sw++)for(int mode=0;mode<4;mode++)for(int pop=0;pop<2;pop++) {
        sc_config c={0};c.enabled=1;c.pure=1;c.air_controls=1;c.body_flips=1;
        c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.steer_scale=1;
        c.steer_curve=1;c.bump_ms=250;c.goofy=g;c.switch_flip=mode;c.pop_on_release=pop;
        sc_state s={0};s.switched=sw;sc_pad p={0};
        sc_translate(&c,&s,p,1000);p.buttons=SC_RTHUMB;
        sc_pad o=sc_translate(&c,&s,p,1010);
        assert(s.pump_r3 && !s.bf_arm && !s.bf_side);
        assert(o.rx==0 && o.ry==0 && o.ly==0 && o.rt>0 && o.rt<25 && !(o.buttons&SC_RTHUMB));
        s.game_crouch=1;o=sc_translate(&c,&s,p,1100);
        assert(s.pump_r3 && !s.bf_arm && o.ly==0 && o.rt==255);
        p.lx=6500;o=sc_translate(&c,&s,p,1110);
        assert(s.pump_r3 && o.ly==0 && o.rt==255); /* drift above deadzone doesn't cancel */
        p.lx=0;
        p.buttons=0;o=sc_translate(&c,&s,p,1200);
        assert(!s.pump_r3 && o.rx==0 && o.ry==0 && o.ly==0 && o.rt==255 && !s.fired && !s.bf_arm);
        o=sc_translate(&c,&s,p,1225);assert(o.rt>110 && o.rt<140);
        o=sc_translate(&c,&s,p,1251);assert(o.rt==0 && !s.pump_release_t);
        s=(sc_state){0};s.grind=1;p.buttons=SC_RTHUMB;
        o=sc_translate(&c,&s,p,2000);assert(!s.pump_r3 && o.lx>32000 && o.ly==0);
        s=(sc_state){0};s.air=s.takeoff=1;
        o=sc_translate(&c,&s,p,2000);assert(!s.pump_r3 && s.bf_side);
        s=(sc_state){0};p.buttons=SC_LTHUMB|SC_RTHUMB;
        o=sc_translate(&c,&s,p,2000);assert(!s.pump_r3);
        s=(sc_state){0};p.buttons=SC_RTHUMB|SC_DPAD_UP;
        o=sc_translate(&c,&s,p,2000);assert(!s.pump_r3 && o.buttons==p.buttons);
        s=(sc_state){0};s.switched=sw;p=(sc_pad){0};p.buttons=SC_RTHUMB;
        int lf=!g;if(mode==3 && sw)lf=!lf;
        if(lf)p.ry=-32768;else p.ly=-32768;
        o=sc_translate(&c,&s,p,2000);assert(!s.pump_r3 && s.bf_arm);
    }
    puts("PASS: hold/release pumping crouch, stance modes, no synthetic release pop, preserved grind exits, airborne and foot-crouched flips, combined clicks and radial menus");
}
