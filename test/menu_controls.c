#include <assert.h>
#include <stdio.h>
#include "../src/menu_controls.h"
int main(void) {
    assert(sc_controls_bumper_direction(0x100)==-1);
    assert(sc_controls_bumper_direction(0x200)==1);
    assert(sc_controls_bumper_direction(0x300)==0);
    assert(sc_controls_bumper_direction(0x1000)==0);
    for(int page=0;page<4;page++) {
        assert(sc_controls_next_page(page,1,19,6)==(page+1)%4);
        assert(sc_controls_next_page(page,-1,19,6)==(page+3)%4);
        assert(sc_controls_next_page(page,0,19,6)==page);
    }
    for(int width=1280;width<=3840;width+=640)for(int height=800;height<=2160;height+=400) {
        int k=sc_menu_fit_scale(250,width,height,1408,816);
        assert(1408*k/100<=width-80 && 816*k/100<=height-80);
    }
    assert(sc_menu_fit_scale(75,1920,1080,1408,816)==75);
    puts("PASS: controls bumper paging, simultaneous press, wraparound, unchanged page without input; two-pane menu fits desktop and handheld sizes");
}
