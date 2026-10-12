#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_config config(void) {
    sc_config c={0};c.enabled=c.pure=c.air_controls=c.powerslide=c.body_flips=1;
    c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.steer_scale=c.steer_curve=1;
    c.bump_ms=250;return c;
}
int main(void) {
    sc_config c=config();sc_state s={0};sc_pad p={0},o;
    for(int direction=-1;direction<=1;direction+=2) {
        /* Flight trace: both feet on, one spent from the preceding trick. */
        s=(sc_state){0};s.d_front_on=s.d_back_on=1;s.d_back_spent=1;
        s.pf_t[0]=s.pf_t[1]=1000;
        p=(sc_pad){0};p.lx=direction*30000;p.rx=-direction*30000;
        o=sc_translate(&c,&s,p,3000);assert(s.ps_side && o.ly<-20000);
        /* Slow/asynchronous thumbs must not permanently lock out the gesture. */
        s=(sc_state){0};p.rx=0;sc_translate(&c,&s,p,4000);
        p.rx=-direction*30000;o=sc_translate(&c,&s,p,4700);
        assert(s.ps_side && o.ly<-20000);
    }
    /* Deliberate vertical crouch then flick still belongs to the trick. */
    s=(sc_state){0};p=(sc_pad){0};p.ry=-32768;sc_translate(&c,&s,p,5000);
    p.lx=32767;p.rx=-32768;p.ry=0;sc_translate(&c,&s,p,5140);assert(!s.ps_side);
    /* But that guard expires instead of leaving slides broken forever. */
    o=sc_translate(&c,&s,p,5401);assert(s.ps_side && o.ly<-20000);
    /* R3 pressed while off-centre recovers when centred without another click. */
    s=(sc_state){0};p=(sc_pad){0};p.buttons=SC_RTHUMB;p.rx=12000;
    sc_translate(&c,&s,p,6000);assert(!s.pump_r3);
    p.rx=0;o=sc_translate(&c,&s,p,6100);assert(s.pump_r3 && !s.bf_arm && !s.bf_side);
    o=sc_translate(&c,&s,p,6180);assert(o.rt==255);
    /* Same recovery after touchdown, with the click already held. */
    s=(sc_state){0};s.air=s.takeoff=1;
    sc_translate(&c,&s,p,7000);assert(!s.pump_r3);
    s.air=s.takeoff=0;o=sc_translate(&c,&s,p,7100);assert(s.pump_r3 && !s.bf_side);
    o=sc_translate(&c,&s,p,7180);assert(o.rt==255);
    s.grind=1;sc_translate(&c,&s,p,7200);assert(!s.pump_r3);
    puts("PASS: stale feet/asynchronous thumbs recover powerslide; recent pop remains protected; held R3 recovers after centring/touchdown, cancels on rail");
}
