#ifndef SC_CONTROLLER_SHORTCUTS_H
#define SC_CONTROLLER_SHORTCUTS_H
#include "overlay_handoff.h"
typedef struct {double pair_since,master_since,last;int pair_held,master_held,done;} sc_controller_hold;
/* 1 clears stale menu ownership, 2 swaps feet, 3 toggles Dual Stick. */
static int sc_controller_shortcut(sc_controller_hold* hold,unsigned short buttons,double now,double duration,int menu,unsigned short* consume) {
    const unsigned short pair=SC_LTHUMB|SC_RTHUMB,master=pair|SC_LSHOULDER|SC_RSHOULDER;
    *consume=0;
    if((buttons&pair)!=pair || (hold->pair_held && (now<hold->last || now-hold->last>200))) {
        sc_controller_hold empty={0};*hold=empty;
    }
    hold->last=now;
    if((buttons&pair)!=pair)return 0;
    if(!hold->pair_held){hold->pair_held=1;hold->pair_since=now;}
    if((buttons&master)==master) {
        *consume=master;
        if(!hold->master_held){hold->master_held=1;hold->master_since=now;}
        if(!hold->done && now-hold->master_since>=duration){hold->done=1;return 3;}
    } else {
        hold->master_held=0;
        if(menu || (buttons&SC_LSHOULDER)) {
            *consume=pair|(buttons&SC_LSHOULDER);
            if(!hold->done && now-hold->pair_since>=400){hold->done=1;return menu?1:2;}
        }
    }
    return 0;
}
static void sc_respawn_input_reset(sc_state* states,sc_controller_hold* shortcuts,unsigned count) {
    for(unsigned i=0;i<count;i++) {
        sc_controller_hold empty={0};shortcuts[i]=empty;
        sc_overlay_suspend(&states[i]);
    }
}
#endif
