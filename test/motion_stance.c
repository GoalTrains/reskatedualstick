#include <assert.h>
#include <stdio.h>
#include "../src/motion_stance.h"
int main(void) {
    sc_motion_stance s={0};
    for(int i=0;i<5;i++) sc_motion_stance_step(&s,100,1,i*.4,0,1,0,0,i*100);
    assert(s.bias_valid && !s.bias);
    /* Regression: normal riding used to repeatedly rewrite this to backwards.
     * A sustained opposite facing/velocity cue without a remount is not a new
     * input-frame reference. Raw turn flags must continue to determine feet. */
    for(int i=0;i<8;i++) {
        int out=sc_motion_stance_step(&s,100,1,2+i*.4,0,-1,0,0,500+i*100);
        assert(out==0 && s.bias_valid && !s.bias);
    }
    assert(sc_motion_stance_step(&s,201,1,6,0,-1,0,1,1350)==1);
    assert(sc_motion_stance_step(&s,100,1,6.4,0,-1,0,0,1450)==0);
    sc_motion_stance_step(&s,504,1,0,0,1,0,0,1600);
    assert(!s.bias_valid);
    /* Remount backwards while the game's flag has reset to zero. */
    for(int i=0;i<5;i++) sc_motion_stance_step(&s,100,1,i*.4,0,-1,0,0,700+i*100);
    assert(s.bias_valid && s.bias);
    assert(sc_motion_stance_step(&s,201,1,2,0,1,0,1,1200)==0);
    /* A new regular remount must not retain the switch reference. */
    sc_motion_stance_step(&s,504,1,0,0,1,0,0,1300);
    for(int i=0;i<5;i++) sc_motion_stance_step(&s,100,1,i*.4,0,1,0,0,1400+i*100);
    assert(s.bias_valid && !s.bias);
    /* Sideways slide, stationary, teleport, invalid and air establish nothing. */
    for(int kind=0;kind<5;kind++) {
        s=(sc_motion_stance){0};
        sc_motion_stance_step(&s,100,1,0,0,1,0,0,1700);
        sc_motion_stance_step(&s,504,1,0,0,1,0,0,1800);
        for(int i=0;i<8;i++) sc_motion_stance_step(&s,kind==4?103:100,kind!=3,
            kind==1?0:kind==2?i*100:i*.4,0,kind==0?0:1,kind==0?1:0,0,2000+i*100);
        assert(!s.bias_valid);
    }
    /* Recorded failure: new skater slot, spawn on foot, mount then tricks;
     * a later backwards motion cue incorrectly inverted regular True Foot. */
    s=(sc_motion_stance){0};
    sc_motion_stance_step(&s,700,1,0,0,-1,0,0,4000);
    sc_motion_stance_step(&s,504,1,0,0,-1,0,0,4100);
    sc_motion_stance_step(&s,103,1,0,0,-1,0,0,4200);
    sc_motion_stance_step(&s,201,1,0,0,-1,0,0,4300);
    for(int i=0;i<8;i++)assert(sc_motion_stance_step(&s,100,1,i*.4,0,-1,0,0,4400+i*100)==0);
    assert(s.bias_valid && !s.bias && !s.remount_pending);
    /* A genuine remount interrupted by a trick cannot calibrate afterwards. */
    sc_motion_stance_step(&s,504,1,0,0,-1,0,0,5300);
    assert(s.remount_pending && !s.bias_valid);
    sc_motion_stance_step(&s,103,1,0,0,-1,0,0,5400);
    assert(sc_motion_stance_step(&s,201,1,0,0,-1,0,0,5500)==0);
    for(int i=0;i<8;i++)assert(sc_motion_stance_step(&s,100,1,i*.4,0,-1,0,0,5600+i*100)==0);
    assert(!s.bias && !s.remount_pending);
    puts("PASS: motion reference corrects backwards remount, follows air turns, resets on regular remount; sideways/stationary/teleport/invalid/non-ground rejected; spawn-on-foot and interrupted-remount cannot invert stance");
}
