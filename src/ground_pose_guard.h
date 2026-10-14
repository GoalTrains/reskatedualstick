#ifndef SC_GROUND_POSE_GUARD_H
#define SC_GROUND_POSE_GUARD_H
#include "native_menu_respawn.h"
typedef struct {
    nr_frame previous,switch_reference,pose_reference;
    double previous_time,switch_time,suspect_time,neutral_since;
    int have_previous,have_switch,suspect,neutral;
    double baseline_since,pose_time,mirror_break_time;
    int baseline_pending,baseline_valid,baseline_mirror,have_pose,mirror_break;
    int profile_known,profile;
} sc_ground_pose_guard;
static void sc_ground_pose_missing(sc_ground_pose_guard* guard,double now) {
    guard->neutral=guard->have_previous=0;
    guard->baseline_pending=0;
    if(now-guard->suspect_time>15000)guard->suspect=0;
    if(now-guard->switch_time>600)guard->have_switch=0;
    if(now-guard->pose_time>600)guard->have_pose=0;
    if(now-guard->mirror_break_time>15000)guard->mirror_break=0;
}
/* Experimental recovery: learn each rider's mirror reference, rather than
 * assuming regular and goofy share a mirror value. The older geometry-only
 * signature remains regular-only; the learned mirror path supports both. */
static int sc_ground_pose_step(sc_ground_pose_guard* guard,const nr_frame* frame,int state,int idle,int goofy,double now) {
    if(goofy<0 || goofy>1 || !frame || !guard->profile_known || guard->profile!=goofy ||
       (guard->profile_known && !nr_same(frame,&guard->previous))) {
        sc_ground_pose_guard empty={0};*guard=empty;
        if(goofy<0 || goofy>1 || !frame)return 0;
        guard->profile_known=1;guard->profile=goofy;
    }
    double dt=now-guard->previous_time;float direction=0;int moving=0;
    if(guard->have_previous && dt>=20 && dt<=200) {
        float x=frame->position[0]-guard->previous.position[0],z=frame->position[2]-guard->previous.position[2];
        float distance=x*x+z*z,facing=frame->forward[0]*frame->forward[0]+frame->forward[2]*frame->forward[2];
        float speed2=distance*(1000000/(float)(dt*dt));
        if(__builtin_isfinite(speed2) && speed2>=.25f && speed2<=225 && facing>.25f) {
            direction=(x*frame->forward[0]+z*frame->forward[2])/__builtin_sqrtf(distance*facing);moving=1;
        }
    }
    guard->previous=*frame;guard->previous_time=now;guard->have_previous=1;
    /* Learn once per owned rider from settled normal riding, not from an air,
     * grind or fakie timer. Keep this reference frozen until ownership/UI loss. */
    if(!guard->baseline_valid) {
        if(state==100 && moving && direction>.92f && frame->basis==0 && frame->native_frame==0) {
            if(!guard->baseline_pending || guard->baseline_mirror!=frame->mirrored) {
                guard->baseline_pending=1;guard->baseline_since=now;guard->baseline_mirror=frame->mirrored;
            } else if(now-guard->baseline_since>=500)guard->baseline_valid=1;
        } else guard->baseline_pending=0;
    }
    /* Latest recording: the mirror changes on the ground as the fakie timer
     * clears, before the subsequent air. Ordinary 180s retain the learned
     * mirror and cannot arm this path merely by reversing the board basis. */
    if(guard->baseline_valid && state==100 && frame->basis==1 && frame->native_frame==1 &&
       frame->mirrored==guard->baseline_mirror) {
        guard->pose_reference=*frame;guard->pose_time=now;guard->have_pose=1;
    }
    if(guard->have_pose && now-guard->pose_time<=600 && state==100 &&
       frame->basis==0 && frame->native_frame==0 && frame->mirrored!=guard->baseline_mirror &&
       nr_same(frame,&guard->pose_reference)) {
        guard->mirror_break=1;guard->mirror_break_time=now;guard->have_pose=0;
    }
    if(guard->mirror_break && (now-guard->mirror_break_time>15000 || frame->mirrored==guard->baseline_mirror))
        guard->mirror_break=0;
    if(!goofy && state==100 && moving && direction<-.92f && frame->native_frame==1 && frame->basis==0) {
        guard->switch_reference=*frame;guard->switch_time=now;guard->have_switch=1;
    }
    /* The source timer clears before the basis flips; preserve a bounded reference. */
    if(!goofy && state==100 && moving && direction<-.92f && guard->have_switch &&
       now-guard->switch_time<=600 && frame->native_frame==0 && frame->basis==1 &&
       nr_same(frame,&guard->switch_reference)) {
        guard->suspect=1;guard->suspect_time=now;guard->have_switch=0;
    }
    if(guard->suspect && (now-guard->suspect_time>15000 || frame->basis==0))guard->suspect=0;
    int reason=guard->suspect?1:guard->mirror_break?2:0;
    int candidate=reason && state==100 && idle && moving && direction>.92f &&
        frame->basis==1 && frame->native_frame==1;
    if(!candidate)guard->neutral=0;
    else if(!guard->neutral){guard->neutral=1;guard->neutral_since=now;}
    else if(now-guard->neutral_since>=500){guard->suspect=guard->mirror_break=guard->neutral=0;return reason;}
    return 0;
}
#endif
