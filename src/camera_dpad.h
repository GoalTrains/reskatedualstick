/* Horizontal D-pad tap/hold arbitration. Times are monotonic milliseconds.
 * Camera requests are absolute: -1 left, +1 right, 0 no request. */
#ifndef SC_CAMERA_DPAD_H
#define SC_CAMERA_DPAD_H
#define SC_CAMERA_DPAD_HOLD_MS 500.0
typedef struct {
    unsigned short previous,blocked;
    double started[2];
    unsigned active,held,cancelled;
    int synced,selected,pending;
    double selected_at,pulse_until,last_time,next_toggle;
    unsigned long long owner;
} sc_camera_dpad;
static void sc_camera_dpad_suspend(sc_camera_dpad* s,unsigned short buttons) {
    sc_camera_dpad zero={0};*s=zero;s->previous=buttons;s->blocked=buttons&12;s->synced=1;
}
static unsigned short sc_camera_dpad_step(sc_camera_dpad* s,unsigned short buttons,double now,int enabled,int* request) {
    *request=0;
    if(!enabled || !s->synced || now<s->last_time || (s->last_time>0 && now-s->last_time>500)) {
        sc_camera_dpad_suspend(s,buttons);s->last_time=now;return buttons;
    }
    s->last_time=now;s->blocked&=buttons;
    unsigned short out=buttons&~12;
    for(int i=0;i<2;i++) {
        unsigned bit=4u<<i,mask=1u<<i;
        if(buttons&bit) {
            if(s->blocked&bit){out|=bit;continue;}
            if(!(s->previous&bit)){s->started[i]=now;s->active|=mask;}
            if(buttons&~bit)s->cancelled|=mask;
            if((s->active&mask) && now-s->started[i]>=SC_CAMERA_DPAD_HOLD_MS)s->held|=mask;
            if(s->held&mask)out|=bit;
        } else if(s->previous&bit) {
            if((s->active&mask) && !(s->held&mask) && !(s->cancelled&mask) && !(buttons&~bit) && now-s->started[i]<SC_CAMERA_DPAD_HOLD_MS)
                *request=i?1:-1;
            s->active&=~mask;s->held&=~mask;s->cancelled&=~mask;
        }
    }
    s->previous=buttons;return out;
}
/* Call after mod gestures have consumed the physical clicks. Only a D-pad
 * camera pulse may expose L3 to the native camera while remapped. */
static unsigned short sc_camera_dpad_output_buttons(unsigned short translated,int remapped,int camera_pulse) {
    if(remapped)translated&=(unsigned short)~0x0040;
    if(camera_pulse)translated|=0x0040;
    return translated;
}
/* Use the observed side on the first request, then retain the requested side
 * through the native camera blend to prevent repeated taps toggling it back. */
static int sc_camera_dpad_toggle(sc_camera_dpad* s,int request,int observed,double now) {
    if(!request)return 0;
    int current=(s->selected && now-s->selected_at<1000)?s->selected:observed;
    if(!current)return 0;
    s->selected=request;s->selected_at=now;
    return current!=request;
}
#endif
