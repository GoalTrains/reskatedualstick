/* Experimental catch assist: preserve an actual emitted flip endpoint and
 * release it on a fresh stick click. Foot masks describe the requested foot;
 * the backend is Skate's shared held-trick release, not independent foot IK. */
#ifndef SC_MANUAL_CATCH_H
#define SC_MANUAL_CATCH_H
typedef struct {
 int phase,seen,front_left,requested,was_air,candidate,recentred,candidate_kind,scoop_track;
 int hold_window_id;
 unsigned short previous;
 float x,y,scoop_last,scoop_sweep;
 double candidate_at,release_until,started,scoop_time;
} sc_manual_catch;
static void sc_mc_cancel(sc_manual_catch*m,unsigned short buttons){
 *m=(sc_manual_catch){0};m->seen=1;m->previous=buttons&(SC_LTHUMB|SC_RTHUMB);
}
static void sc_mc_frame(const sc_config*c,const sc_state*s,float*x,float*y){
 if(c->xl_layout && s->game_mirror_known && s->game_mirror)*x=-*x;
 if(s->sw_lat){*y=-*y;if(c->switch_flip==2)*x=-*x;}
}
static sc_pad sc_manual_catch_translate_ex(const sc_config*c,sc_state*s,sc_manual_catch*m,
                                      sc_pad raw,double now,int state,int trick_id){
 int air=(s->air || s->takeoff>0) && !s->grind;
 int valid=c->manual_catch && c->enabled && c->pure && state>=100 && state<300;
 unsigned short clicks=raw.buttons&(SC_LTHUMB|SC_RTHUMB);
 unsigned short edge=m->seen?(clicks&~m->previous):0;
 m->seen=1;m->previous=clicks;
 if(!valid || s->stock_air_active || s->stock_early_active || s->radial_passthrough){
  sc_mc_cancel(m,raw.buttons);return sc_translate(c,s,raw,now);
 }
 if(m->was_air && !air){sc_mc_cancel(m,raw.buttons);}
 /* A cancelled pop must not leave a cached flip latched on the ground. */
 if(m->phase==2 && !air && now-m->started>250)sc_mc_cancel(m,raw.buttons);
 sc_pad input=raw;
 int ready=m->phase==2 || (m->phase==0 && m->candidate && now-m->candidate_at<=160 && air);
 if(air && ready){
  input.buttons&=(unsigned short)~(SC_LTHUMB|SC_RTHUMB);
  if(edge){
   if(m->phase!=3)m->requested=0;
   if(edge&SC_LTHUMB)m->requested|=m->front_left?1:2;
   if(edge&SC_RTHUMB)m->requested|=m->front_left?2:1;
   m->phase=3;m->release_until=now+60;m->recentred=0;
   s->hold_foot=0;s->bf_arm=0;
  }
 }else if(air && m->phase==3){
  input.buttons&=(unsigned short)~(SC_LTHUMB|SC_RTHUMB);
  if(edge&SC_LTHUMB)m->requested|=m->front_left?1:2;
  if(edge&SC_RTHUMB)m->requested|=m->front_left?2:1;
 }
 sc_pad out=sc_translate(c,s,input,now);
 if(s->stock_air_active || s->stock_early_active || s->radial_passthrough){sc_mc_cancel(m,raw.buttons);return out;}
 if(!air && m->phase==0){
  float x=sc_axis(out.rx),y=sc_axis(out.ry),mag=x*x+y*y;
  sc_mc_frame(c,s,&x,&y);
  /* A one-foot shuv/impossible has no opposite-foot flick. Follow the
   * emitted trick path, including when single-stick flips are BLOCKED.
   * Ground steering, slides and rail positioning cannot qualify a catch. */
  int foot=s->d_front_on && !s->d_front_spent && !s->d_back_on?1:
           s->d_back_on && !s->d_back_spent && !s->d_front_on?2:0;
  int scoop=0;
  if(foot && mag>=.5f*.5f && !s->ps_side && !s->st_latch && !s->pump_r3 &&
     (!s->grind || s->rail_scoop_exit)){
   float a=sc_angle(x,y);double dt=now-m->scoop_time;
   if(m->scoop_track==foot && dt>0 && dt<=100){
    float delta=sc_wrap(a-m->scoop_last);
    if(sc_abs(delta)<=110)m->scoop_sweep+=delta;
    else m->scoop_sweep=0;
   }else m->scoop_sweep=0;
   m->scoop_track=foot;m->scoop_last=a;m->scoop_time=now;
   scoop=sc_abs(m->scoop_sweep)>=35;
  }else {m->scoop_track=0;m->scoop_sweep=0;}
  int flick=s->d_front_flick || s->d_back_flick || s->d_log==2 ||
            (c->one_foot_flips && s->d_windup) || s->rail_scoop_exit;
  if((scoop || (flick && sc_abs(x)>=.2f)) && mag>=.75f*.75f && !s->ps_side && !s->st_latch && !s->pump_r3){
   float scale=sc_sqrt(mag);m->x=x/scale;m->y=y/scale;
   m->candidate=1;m->candidate_kind=scoop?2:1;m->candidate_at=now;m->front_left=s->feet_map_left;
  }
  if(m->candidate && now-m->candidate_at>160)m->candidate=0;
 }
 if(m->phase!=3 && m->phase!=2 && m->candidate &&
    (state==103 || air) && now-m->candidate_at<=160){m->phase=2;m->requested=0;m->started=now;}
 if(m->phase==2){
  /* Skate has accepted the pop in 103, but still samples held-trick intent.
   * Waiting for 200 left a 40-70 ms neutral gap in the recorded shuvs.
   * Keep an active scoop moving through 103; once it eases off, preserve the
   * last full endpoint rather than releasing and re-holding after takeoff. */
  if(m->candidate_kind==2 && !air){
   float x=sc_axis(out.rx),y=sc_axis(out.ry),mag=x*x+y*y;
   if(mag>=.85f*.85f){
    sc_mc_frame(c,s,&x,&y);float scale=sc_sqrt(mag);m->x=x/scale;m->y=y/scale;
   }
  }
  float x=m->x,y=m->y;sc_mc_frame(c,s,&x,&y);
  /* Preserve the actual translated trick vector. Native trick IDs do not
   * identify a safe replacement direction: targeting an angle can reverse
   * flicks (hospital flips) or redirect scoop/shuv tricks. */
  m->hold_window_id=0;
  out.rx=sc_short(x);out.ry=sc_short(y);
 }else if(m->phase==3){
  /* Holding the physical flick through a catch must not immediately hold the
   * trick again. After centring, fresh board adjustments are allowed. */
  float lx=sc_axis(raw.lx),ly=sc_axis(raw.ly),rx=sc_axis(raw.rx),ry=sc_axis(raw.ry);
  if(lx*lx+ly*ly<.18f*.18f && rx*rx+ry*ry<.18f*.18f)m->recentred=1;
  if(now<m->release_until || !m->recentred)out.rx=out.ry=0;
 }
 m->was_air=air;
 return out;
}
static sc_pad sc_manual_catch_translate(const sc_config*c,sc_state*s,sc_manual_catch*m,
                                      sc_pad raw,double now,int state){
 return sc_manual_catch_translate_ex(c,s,m,raw,now,state,-1);
}
#endif
