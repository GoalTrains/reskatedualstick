#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/translate.h"
#include "../src/ps_reports.h"
static void usb(unsigned char* b) {
    memset(b,0,64);b[0]=1;b[1]=b[3]=128;b[2]=b[4]=127;b[8]=8;
}
int main(void) {
    unsigned char b[384];sc_ps_frames f;sc_pad p;
    for(int i=0;i<6;i++)usb(b+i*64);
    /* Reproduce the exact long-read size from the failing user's log. */
    f=sc_ps_frame(b,sizeof b,1);assert(f.count==6 && f.dualsense && f.size==64);
    for(unsigned i=0;i<f.count;i++) {
        p=sc_ps_decode(b+i*f.size+f.offset,f.dualsense);
        assert(!p.buttons && !p.lt && !p.rt && !p.lx && !p.ly && !p.rx && !p.ry);
    }
    /* The old layout invented the log's L2=8 and D-pad Up. */
    p=sc_ps_decode(b+1,0);assert(p.lt==8 && (p.buttons&1));
    b[64+6]=128;
    sc_config c={0};sc_state s={0};c.enabled=c.pure=1;
    c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.steer_curve=c.steer_scale=1;
    s.game_crouch=-1;
    p=sc_ps_decode(b+65,1);assert(p.rt==128 && !p.buttons);
    sc_pad o=sc_translate(&c,&s,p,1000);assert(o.lx>16000 && o.lx<17000 && !s.dp_t);
    /* One USB report with zero padding is also not Bluetooth simple. */
    memset(b+64,0,320);f=sc_ps_frame(b,384,1);assert(f.count==1 && f.dualsense);
    b[65]=42;assert(!sc_ps_frame(b,384,1).count);
    assert(!sc_ps_frame(b,63,1).count && !sc_ps_frame(b,65,1).count);
    assert(!sc_ps_frame(b,0,1).count && !sc_ps_frame(0,64,1).count);
    assert(!sc_ps_frame(b,64,0).count);
    /* Genuine short Bluetooth and full Bluetooth retain their layouts. */
    memset(b,0,sizeof b);b[0]=1;b[1]=b[3]=128;b[2]=b[4]=127;b[5]=8;
    f=sc_ps_frame(b,10,1);assert(f.count==1 && !f.dualsense && f.offset==1);
    p=sc_ps_decode(b+f.offset,f.dualsense);assert(!p.buttons && !p.lt && !p.rt);
    b[0]=0x31;f=sc_ps_frame(b,78,1);assert(f.count==1 && f.dualsense && f.crc && f.offset==2);
    assert(!sc_ps_frame(b,77,1).count && !sc_ps_frame(b,79,1).count);
    b[0]=0x11;f=sc_ps_frame(b,78,2);assert(f.count==1 && !f.dualsense && f.crc && f.offset==3);
    usb(b);f=sc_ps_frame(b,64,2);assert(f.count==1 && !f.dualsense);
    puts("PlayStation HID framing: long USB, padding, Bluetooth and translator checks passed");
    return 0;
}
