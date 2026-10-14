#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
static sc_pad attempt(int native_known,int native_switch) {
    sc_config c={0};c.enabled=1;c.pure=1;c.switch_flip=1;
    c.deadzone=.15f;c.active=.30f;c.manual_max=.85f;
    sc_state s={0};s.switched=1;s.game_switch_known=native_known;
    s.game_switch=native_switch;s.takeoff=0;s.game_crouch=0;
    sc_pad in={0};in.ly=32767;
    sc_translate(&c,&s,in,1000);
    sc_translate(&c,&s,in,1500);
    in.ry=-32768;
    return sc_translate(&c,&s,in,1510);
}
int main(void) {
    sc_pad old=attempt(0,0),normal_switch=attempt(1,1),remount_switch=attempt(1,0);
    assert(old.ry && normal_switch.ry==old.ry);
    printf("native frame output: switch=%d remount=%d\n",normal_switch.ry,remount_switch.ry);
    assert((long)remount_switch.ry*normal_switch.ry < 0);
    assert(remount_switch.ry + normal_switch.ry >= -1 && remount_switch.ry + normal_switch.ry <= 1);
    sc_config c={0};sc_state s={0};c.switch_flip=1;s.switched=1;
    assert(SC_SWFLIP(&c,&s));s.game_switch_known=1;s.game_switch=0;
    assert(!SC_SWFLIP(&c,&s));s.game_switch=1;assert(SC_SWFLIP(&c,&s));
    c.switch_flip=0;assert(!SC_SWFLIP(&c,&s));
    for(int sw=0;sw<2;sw++) {
        sc_config feet={0};sc_state state={0};sc_pad pad={0};
        feet.enabled=feet.pure=1;feet.switch_flip=3;feet.deadzone=.15f;
        feet.active=.3f;feet.manual_max=.85f;state.switched=sw;state.game_crouch=-1;
        sc_translate(&feet,&state,pad,1000);
        if(sw)pad.ly=-32767;else pad.ry=-32767;
        sc_pad held=sc_translate(&feet,&state,pad,1100);
        assert(state.feet_map_left==!sw && held.ry<-30000);
        if(sw)pad.ry=32767;else pad.ly=32767;
        sc_pad flick=sc_translate(&feet,&state,pad,1200);
        assert(flick.ry>30000);
    }
    puts("PASS: left-up setup/right-down flick preserves existing switch output; remount uses native input frame separately from corrected riding direction; disabled/unknown routes unchanged");
}
