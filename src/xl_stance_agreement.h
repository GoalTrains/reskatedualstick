#ifndef SC_XL_STANCE_AGREEMENT_H
#define SC_XL_STANCE_AGREEMENT_H
#include "native_menu_respawn.h"
typedef struct {
    nr_frame previous;
    double time,baseline_since,candidate_since;
    int sampled,profile_known,profile,baseline_pending,baseline_valid,mirror,candidate;
} sc_xl_agreement;
static void sc_xl_agreement_missing(sc_xl_agreement* g){g->sampled=g->baseline_pending=0;g->candidate=-1;}
static int sc_xl_agreement_step(sc_xl_agreement* g,const nr_frame* f,int state,int idle,int profile,double now){
    if(!f || profile<0 || profile>1 || !g->profile_known || g->profile!=profile ||
       (g->profile_known && !nr_same(f,&g->previous))){
        sc_xl_agreement empty={0};*g=empty;g->candidate=-1;
        if(!f || profile<0 || profile>1)return -1;
        g->profile_known=1;g->profile=profile;
    }
    double dt=now-g->time;float dot=0;int moving=0;
    if(g->sampled && dt>=20 && dt<=200){
        float x=f->position[0]-g->previous.position[0],z=f->position[2]-g->previous.position[2];
        float d=x*x+z*z,v=f->forward[0]*f->forward[0]+f->forward[2]*f->forward[2];
        float speed=d*1000000/(float)(dt*dt);
        if(__builtin_isfinite(speed) && speed>=1 && speed<=225 && v>=.25f && v<=1.5f){
            dot=(x*f->forward[0]+z*f->forward[2])/__builtin_sqrtf(d*v);moving=1;
        }
    }
    g->previous=*f;g->time=now;g->sampled=1;
    /* Gather evidence during skating; only application requires neutral feet. */
    (void)idle;
    int stable=state==100 && moving && f->basis==0;
    if(!g->baseline_valid){
        if(stable && dot>.92f && f->native_frame==0){
            if(!g->baseline_pending || g->mirror!=f->mirrored){g->baseline_pending=1;g->mirror=f->mirrored;g->baseline_since=now;}
            else if(now-g->baseline_since>=500)g->baseline_valid=1;
        }else g->baseline_pending=0;
    }
    int desired=dot>.92f?0:dot<-.92f?1:-1;
    if(!stable || !g->baseline_valid || f->mirrored!=g->mirror || desired<0 || f->native_frame!=desired){g->candidate=-1;return -1;}
    if(g->candidate!=desired){g->candidate=desired;g->candidate_since=now;return -1;}
    return now-g->candidate_since>=300?desired:-1;
}
static int sc_xl_agreement_can_apply(int hint,int retained,int state,int feet_neutral){return hint>=0 && hint<=1 && hint!=retained && state==100 && feet_neutral;}
static int sc_xl_agreement_cached(unsigned long long now,unsigned long long seen,unsigned long long owner,unsigned long long current,int profile,int current_profile,int hint,int native){
    return owner && owner==current && now>=seen && now-seen<=200 && profile==current_profile && hint==native && hint>=0 && hint<=1?hint:-1;
}
#endif
