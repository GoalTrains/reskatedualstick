#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/stance_track.h"
#include "../src/translate.h"

static sc_config config(int goofy, int front_back) {
    sc_config c = {0};
    c.enabled = 1; c.pure = 1; c.air_controls = 1;
    c.deadzone = 0.15f; c.active = 0.30f; c.manual_max = 0.85f;
    c.switch_flip = 3; c.goofy = goofy; c.front_back = front_back;
    c.windup_deg = 60; c.steer_scale = 1; c.steer_curve = 1.5f;
    return c;
}

static void test_turn_tracking(void) {
    sc_stance_track t = {0};
    assert(sc_track_stance(&t,100,0,1,0,1000)==0);
    assert(sc_track_stance(&t,200,0,0,0,2000)==0);
    assert(sc_track_stance(&t,201,1,0,0,2100)==1);
    assert(sc_track_stance(&t,101,1,0,1,2200)==1);
    assert(t.landing_turn_ms==100);
    /* From the log: b resets during the landing slide. Stance must not follow
     * that reset or be committed for a second time when 101 ends. */
    assert(sc_track_stance(&t,100,0,0,1,2250)==1);
    assert(sc_track_stance(&t,103,0,0,1,2300)==1);
    assert(sc_track_stance(&t,200,0,0,1,2400)==1);
    assert(sc_track_stance(&t,201,1,0,1,2500)==0);
    assert(sc_track_stance(&t,100,1,0,0,2600)==0);
    assert(sc_track_stance(&t,100,0,0,0,3100)==0);
    /* A full 360 returns to the takeoff stance. */
    assert(sc_track_stance(&t,200,0,0,0,4000)==0);
    assert(sc_track_stance(&t,201,1,0,0,4100)==1);
    assert(sc_track_stance(&t,201,0,0,1,4200)==0);
    assert(sc_track_stance(&t,100,0,0,0,4300)==0);
    /* A revert gets one decision; subsequent rolling flag noise is ignored. */
    assert(sc_track_stance(&t,101,0,0,0,5000)==0);
    assert(sc_track_stance(&t,101,1,0,0,5100)==0);
    assert(sc_track_stance(&t,100,1,0,0,5200)==1);
    assert(sc_track_stance(&t,100,0,0,1,5250)==1);
    assert(sc_track_stance(&t,100,-1,0,1,5300)==1);
    /* Respawn resets stale references before the next turn. */
    assert(sc_track_stance(&t,504,0,1,1,6000)==0);
    assert(sc_track_stance(&t,100,0,0,0,6100)==0);
    assert(sc_track_stance(&t,200,0,0,0,6200)==0);
    assert(sc_track_stance(&t,201,1,0,0,6300)==1);
    assert(sc_track_stance(&t,100,1,0,1,6400)==1);
    sc_stance_track unknown = {0};
    assert(sc_track_stance(&unknown,100,-1,0,1,1000)==1);
    assert(unknown.prev_raw==-1);
    puts("PASS: turns, landing slides, 360s, reverts, invalid flags, respawn");
}

static void test_held_manual(int goofy, int front_back, int back_foot) {
    sc_config c = config(goofy,front_back);
    sc_state s = {0}; s.game_crouch = -1;
    sc_pad p = {0};
    sc_translate(&c,&s,p,1000);
    s.takeoff = s.air = 1;
    sc_translate(&c,&s,p,1016);
    int left = front_back ? 1 : !goofy;
    if (back_foot) left = !left;
    short lean = sc_short(back_foot ? -0.65f : 0.65f);
    if (left) p.ly = lean; else p.ry = lean;
    sc_pad a = sc_translate(&c,&s,p,1032);
    assert(sc_abs(sc_axis(a.ry)) > 0.5f);
    unsigned seq = s.d_seq;
    s.switched = 1;
    sc_pad b = sc_translate(&c,&s,p,1048);
    assert(s.d_seq == seq); /* no invented press when only the stance changed */
    assert(sc_abs(sc_axis(b.ry)) > 0.5f); /* no release gap or easing */
    s.takeoff = s.air = 0;
    sc_pad landed = sc_translate(&c,&s,p,1064);
    assert(sc_abs(sc_axis(landed.rx)-sc_axis(b.rx)) < 0.02f);
    assert(sc_abs(sc_axis(landed.ry)-sc_axis(b.ry)) < 0.02f);
    for (double now=1080; now<1600; now+=16) {
        sc_pad o = sc_translate(&c,&s,p,now);
        assert(sc_abs(sc_axis(o.ry)-sc_axis(b.ry)) < 0.02f);
        assert(s.d_seq==seq);
    }
    p.ly = p.ry = 0;
    sc_pad released = sc_translate(&c,&s,p,1700);
    assert(released.rx==0 && released.ry==0);
    assert(!s.d_land_hold);
    /* Start an ordinary two-foot switch ollie after the manual is released. */
    int left_front = front_back ? 1 : !goofy;
    if (!front_back) left_front = !left_front;
    if (left_front) p.ry=-32767; else p.ly=-32767;
    sc_pad crouch=sc_translate(&c,&s,p,1800);
    assert(sc_abs(sc_axis(crouch.ry))>0.8f);
    if (left_front) p.ly=32767; else p.ry=32767;
    sc_pad pop=sc_translate(&c,&s,p,1920);
    assert(sc_axis(crouch.ry)*sc_axis(pop.ry)<-0.7f);
    printf("PASS: %s %s %s held through 180 and manual, then switch pop\n",
           goofy?"goofy":"regular",front_back?"front/back":"left/right",back_foot?"back-foot":"front-foot");
}

static void test_physical_foot_history(void) {
    sc_config c=config(0,0);
    sc_state s={0}; s.game_crouch=-1;
    sc_pad p={0}; sc_translate(&c,&s,p,1000);
    s.takeoff=s.air=1;
    p.ly=32767; sc_translate(&c,&s,p,1016);
    assert(s.d_front_spent && !s.d_back_on);
    unsigned seq=s.d_seq;
    s.switched=1; sc_translate(&c,&s,p,1032);
    assert(s.d_back_spent && s.d_back_on && !s.d_front_on);
    assert(s.d_seq==seq);
    puts("PASS: carried trick stays attached to its physical foot");
}

static void test_delayed_turns(void) {
    sc_stance_track t={0};
    assert(sc_track_stance(&t,100,0,1,0,1000)==0);
    assert(sc_track_stance(&t,200,0,0,0,2000)==0);
    assert(sc_track_stance(&t,201,1,0,0,2100)==1);
    assert(sc_track_stance(&t,101,1,0,1,2400)==1);
    assert(sc_track_stance(&t,100,0,0,1,2450)==1);
    /* Next turn flag changes only AFTER landing, then resets. */
    assert(sc_track_stance(&t,200,0,0,1,3000)==1);
    assert(sc_track_stance(&t,201,0,0,1,3100)==1);
    assert(sc_track_stance(&t,100,0,0,1,3400)==1);
    assert(sc_track_stance(&t,100,1,0,1,3430)==0);
    assert(t.late_turn_ms==30);
    assert(sc_track_stance(&t,100,0,0,0,3500)==0);
    /* Same-sample landing edge counts once too. */
    assert(sc_track_stance(&t,200,0,0,0,4000)==0);
    assert(sc_track_stance(&t,201,0,0,0,4100)==0);
    assert(sc_track_stance(&t,100,1,0,0,4400)==1);
    assert(sc_track_stance(&t,100,0,0,1,4500)==1);
    /* Ground noise after the bounded landing window is ignored. */
    assert(sc_track_stance(&t,200,0,0,1,5000)==1);
    assert(sc_track_stance(&t,100,0,0,1,5300)==1);
    assert(sc_track_stance(&t,100,1,0,1,5900)==1);
    puts("PASS: delayed landing turn, same-frame edge, reset, expired window");
}

static void test_touchdown_final_edge(void) {
    sc_stance_track t={0};
    assert(sc_track_stance(&t,100,0,1,0,1000)==0);
    assert(sc_track_stance(&t,200,0,0,0,2000)==0);
    assert(sc_track_stance(&t,201,1,0,0,2200)==1);
    assert(sc_track_stance(&t,201,0,0,1,2381)==0);
    /* The final 0->1 edge arrives on the landing into state 101. */
    assert(sc_track_stance(&t,101,1,0,0,2400)==1);
    assert(sc_track_stance(&t,100,1,0,1,2416)==1);
    assert(sc_track_stance(&t,100,0,0,1,2500)==1);
    /* A subsequent 180 must return to normal, rather than invert all turns. */
    assert(sc_track_stance(&t,200,0,0,1,3000)==1);
    assert(sc_track_stance(&t,201,1,0,1,3200)==0);
    assert(sc_track_stance(&t,100,1,0,0,3400)==0);
    /* An invalid touchdown flag preserves the last valid air sample. */
    assert(sc_track_stance(&t,200,1,0,0,4000)==0);
    assert(sc_track_stance(&t,201,0,0,0,4200)==1);
    assert(sc_track_stance(&t,100,-1,0,1,4400)==1);
    puts("PASS: 1->0->1 touchdown flag, ground reset, next turn, invalid sample");
}

static void test_late_manual(void) {
    sc_config c=config(0,0); sc_state s={0}; s.game_crouch=-1;
    sc_pad p={0}; sc_translate(&c,&s,p,1000);
    s.takeoff=s.air=1; sc_translate(&c,&s,p,1016);
    p.ly=sc_short(0.65f); sc_translate(&c,&s,p,1032);
    s.takeoff=s.air=0;
    sc_pad before=sc_translate(&c,&s,p,1048);
    s.switched=1;
    sc_pad after=sc_translate(&c,&s,p,1064);
    assert(before.ry==after.ry && before.rx==after.rx);
    p.ly=0; sc_translate(&c,&s,p,1080); sc_translate(&c,&s,p,1096);
    /* In switch left is now the back foot; release unlocked its new job. */
    p.ly=-32767; sc_pad crouch=sc_translate(&c,&s,p,1200);
    assert(sc_axis(crouch.ry)<-0.8f);
    p.ry=32767; sc_pad pop=sc_translate(&c,&s,p,1320);
    assert(sc_axis(pop.ry)>0.8f);
    puts("PASS: late stance correction preserves manual, then swaps foot jobs");
}

int main(void) {
    test_touchdown_final_edge();
    test_delayed_turns();
    test_late_manual();
    test_turn_tracking();
    for (int goofy=0;goofy<2;goofy++)
        for (int fb=0;fb<2;fb++)
            for (int back=0;back<2;back++) test_held_manual(goofy,fb,back);
    test_physical_foot_history();
    return 0;
}
