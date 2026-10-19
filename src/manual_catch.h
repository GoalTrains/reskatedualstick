/* Experimental catch assist: preserve an actual emitted flip endpoint and
 * release it on a fresh stick click. Foot masks describe the requested foot;
 * the backend is Skate's shared held-trick release, not independent foot IK. */
#ifndef SC_MANUAL_CATCH_H
#define SC_MANUAL_CATCH_H
typedef struct {
 int phase,seen,front_left,requested,was_air,candidate,recentred,candidate_kind,scoop_track;
 int hold_window_id;
 int observed_seen,observed_id,observed_doing,emitted_seen;
 int id_seen,id_previous;
 int held_recentred;
 unsigned short previous;
 float x,y,scoop_last,scoop_sweep;
 float emitted_x,emitted_y;
 double candidate_at,release_until,started,scoop_time;
 double emitted_at;
 short replay_x[32],replay_y[32];
 unsigned replay_head,replay_count;
 int replay_running;
 double replay_sample_at,replay_started;
} sc_manual_catch;
static void sc_mc_replay_record(sc_manual_catch*m,float x,float y,double now){
 if(m->replay_count && now-m->replay_sample_at<8)return;
 if(now-m->replay_sample_at>80)m->replay_head=m->replay_count=0;
 m->replay_x[m->replay_head]=sc_short(x);m->replay_y[m->replay_head]=sc_short(y);
 m->replay_head=(m->replay_head+1)%32;if(m->replay_count<32)m->replay_count++;
 m->replay_sample_at=now;
}
static void sc_mc_cancel(sc_manual_catch*m,unsigned short buttons){
 *m=(sc_manual_catch){0};m->seen=1;m->previous=buttons&(SC_LTHUMB|SC_RTHUMB);
}
static void sc_mc_frame(const sc_config*c,const sc_state*s,float*x,float*y){
 if(c->xl_layout && s->game_mirror_known && s->game_mirror)*x=-*x;
 if(s->sw_lat){*y=-*y;if(c->switch_flip==2)*x=-*x;}
}
static sc_pad sc_manual_catch_translate_observed(const sc_config*c,sc_state*s,sc_manual_catch*m,
                                      sc_pad raw,double now,int state,int trick_id,int doing_trick){
 int air=(s->air || s->takeoff>0) && !s->grind;
 int valid=c->manual_catch && c->enabled && c->pure && state>=100 && state<400;
 unsigned short clicks=raw.buttons&(SC_LTHUMB|SC_RTHUMB);
 unsigned short edge=m->seen?(clicks&~m->previous):0;
 m->seen=1;m->previous=clicks;
 if(!valid || s->stock_air_active || s->stock_early_active || s->radial_passthrough){
  sc_mc_cancel(m,raw.buttons);return sc_translate(c,s,raw,now);
 }
 if(m->was_air && !air){sc_mc_cancel(m,raw.buttons);}
 /* Rail contact ends an airborne hold, but rail polls must still reach the
  * translator below: it recognizes an actual trick-out during state 300. */
 if(s->grind && m->phase)sc_mc_cancel(m,raw.buttons);
 /* Treat the native enum as an event identity, never as an input direction.
  * A repeated ID can also begin another trick after DoingTrick falls/rises.
  * Unknown samples break continuity, so an old value cannot invent an event. */
 int native_event=0,native_id_event=0;
 /* CurrentFlipTrick is independently validated. Its continuity must not
  * disappear when the animation boolean is not initialized yet. */
 if(trick_id>=0){
  native_id_event=m->id_seen && trick_id>0 && trick_id!=m->id_previous;
  m->id_seen=1;m->id_previous=trick_id;
 }else m->id_seen=0;
 if(trick_id>=0 && (doing_trick==0 || doing_trick==1)){
  native_event=m->observed_seen && doing_trick && trick_id>0 &&
               (trick_id!=m->observed_id || !m->observed_doing);
  m->observed_seen=1;m->observed_id=trick_id;m->observed_doing=doing_trick;
 }else m->observed_seen=0;
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
   m->phase=3;m->release_until=now+60;m->recentred=0;m->emitted_seen=0;
   s->hold_foot=0;s->bf_arm=0;
  }
 }else if(air && m->phase==3){
  input.buttons&=(unsigned short)~(SC_LTHUMB|SC_RTHUMB);
  if(edge&SC_LTHUMB)m->requested|=m->front_left?1:2;
  if(edge&SC_RTHUMB)m->requested|=m->front_left?2:1;
 }
 sc_pad out=sc_translate(c,s,input,now);
 if(s->stock_air_active || s->stock_early_active || s->radial_passthrough){sc_mc_cancel(m,raw.buttons);return out;}
 /* Capture the actual game-stick trajectory, never a guessed trick/enum.
  * Freeze it once airborne; replay changes only the game right stick. */
 if(c->manual_catch_replay && ((!air && m->phase!=3) ||
    (air && (m->phase==0 || (m->phase==3 && m->recentred && now>=m->release_until))))){
  float x=sc_axis(out.rx),y=sc_axis(out.ry);sc_mc_frame(c,s,&x,&y);
  sc_mc_replay_record(m,x,y,now);
 }
 /* Once the preceding catch has centred, let a fresh emitted gesture reach
  * Skate. Cache its endpoint briefly because recognition follows input by a
  * poll or two. A native trick event is required: a rail/manual adjustment
  * on its own must never re-arm manual catch. */
 if(air && (m->phase==0 || (m->phase==3 && m->recentred && now>=m->release_until))){
  float x=sc_axis(out.rx),y=sc_axis(out.ry),mag=x*x+y*y;
  if(mag>=.75f*.75f){
   sc_mc_frame(c,s,&x,&y);float scale=sc_sqrt(mag);
   m->emitted_x=x/scale;m->emitted_y=y/scale;m->emitted_at=now;m->emitted_seen=1;
  }
  if(native_event && m->emitted_seen && now-m->emitted_at<=160){
   m->x=m->emitted_x;m->y=m->emitted_y;m->phase=2;m->requested=0;
   m->candidate=1;m->candidate_kind=3;m->front_left=s->feet_map_left;
   m->candidate_at=m->started=now;m->emitted_seen=0;m->recentred=0;m->held_recentred=0;
   if(edge){
    if(edge&SC_LTHUMB)m->requested|=m->front_left?1:2;
    if(edge&SC_RTHUMB)m->requested|=m->front_left?2:1;
    m->phase=3;m->release_until=now+60;
    s->hold_foot=s->bf_arm=s->bf_side=0;
    out.buttons&=(unsigned short)~(SC_LTHUMB|SC_RTHUMB);
   }
  }
 }
 if(!air && m->phase==0){
  float x=sc_axis(out.rx),y=sc_axis(out.ry),mag=x*x+y*y;
  sc_mc_frame(c,s,&x,&y);
  /* A one-foot shuv/impossible has no opposite-foot flick. Follow the
   * emitted trick path, including when single-stick flips are BLOCKED.
   * Ground steering, slides and rail positioning cannot qualify a catch. */
  int foot=s->d_front_on && !s->d_front_spent && !s->d_back_on?1:
           s->d_back_on && !s->d_back_spent && !s->d_front_on?2:0;
  int scoop=s->rail_scoop_exit!=0;
  if(foot && mag>=.5f*.5f && !s->ps_side && !s->st_latch && !s->pump_r3 &&
     (!s->grind || s->rail_scoop_exit)){
   float a=sc_angle(x,y);double dt=now-m->scoop_time;
   if(m->scoop_track==foot && dt>0 && dt<=100){
    float delta=sc_wrap(a-m->scoop_last);
    if(sc_abs(delta)<=110)m->scoop_sweep+=delta;
    else m->scoop_sweep=0;
   }else m->scoop_sweep=0;
   m->scoop_track=foot;m->scoop_last=a;m->scoop_time=now;
   scoop|=sc_abs(m->scoop_sweep)>=35;
  }else {m->scoop_track=0;m->scoop_sweep=0;}
  int flick=s->d_front_flick || s->d_back_flick || s->d_log==2 ||
            (c->one_foot_flips && s->d_windup) || s->rail_scoop_exit;
  if((scoop || (flick && sc_abs(x)>=.2f)) && mag>=.75f*.75f && !s->ps_side && !s->st_latch && !s->pump_r3){
   float scale=sc_sqrt(mag);m->x=x/scale;m->y=y/scale;
   m->candidate=1;m->candidate_kind=scoop?2:1;m->candidate_at=now;m->front_left=s->feet_map_left;
  }
  if(m->candidate && now-m->candidate_at>160)m->candidate=0;
 }
 /* Recognition can precede 103 by tens of milliseconds. The recorded bad
  * shuvs released to neutral in that gap, then re-held too late at 103.
  * CurrentFlipTrick can also change before DoingTrick rises. A fresh ID
  * plus our captured gesture bridges that extra animation-start delay. */
 if(!s->grind && m->phase!=3 && m->phase!=2 && m->candidate &&
    (state==103 || air || ((native_id_event || native_event) && (state==100 || state==101))) &&
    now-m->candidate_at<=160){m->phase=2;m->requested=0;m->started=now;m->held_recentred=0;}
 if(m->phase==2){
  /* A deliberate continuation after the physical sticks have centred must
   * still reach the native recognizer (late/underflip arcs, for example).
   * Neutral itself retains the endpoint; a new strong translated vector
   * moves it without dropping the outstanding catch requirement. */
  if(air && !c->manual_catch_replay){
   float lx=sc_axis(raw.lx),ly=sc_axis(raw.ly),rx=sc_axis(raw.rx),ry=sc_axis(raw.ry);
   if(lx*lx+ly*ly<.18f*.18f && rx*rx+ry*ry<.18f*.18f)m->held_recentred=1;
   float x=sc_axis(out.rx),y=sc_axis(out.ry),mag=x*x+y*y;
   if(m->held_recentred && mag>=.85f*.85f){
    sc_mc_frame(c,s,&x,&y);float scale=sc_sqrt(mag);m->x=x/scale;m->y=y/scale;
   }
  }
  /* Finish the real gesture through 103 before freezing its endpoint. Both
   * a scoop and a two-foot flick can still be moving after pop acceptance.
   * Once input eases off, keep the last full vector without a neutral gap. */
  if(!air){
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
  if(c->manual_catch_replay && air){
   if(!m->replay_running){
    /* A late event without a recent captured path falls back to a flick
     * of its actual endpoint, rather than replaying the previous trick. */
    if(!m->replay_count || now-m->replay_sample_at>160){
     m->replay_head=m->replay_count=0;
     sc_mc_replay_record(m,m->x,m->y,now);
    }
    m->replay_running=1;m->replay_started=now;
   }
   unsigned cycle=16+m->replay_count*8;
   unsigned step=(unsigned)(now-m->replay_started)%cycle;
   if(step<16)out.rx=out.ry=0;
   else{
    unsigned i=(m->replay_head+32-m->replay_count+(step-16)/8)%32;
    x=sc_axis(m->replay_x[i]);y=sc_axis(m->replay_y[i]);sc_mc_frame(c,s,&x,&y);
    out.rx=sc_short(x);out.ry=sc_short(y);
   }
  }else m->replay_running=0;
 }else if(m->phase==3){
  m->replay_running=0;
  /* Holding the physical flick through a catch must not immediately hold the
   * trick again. After centring, fresh board adjustments are allowed. */
  float lx=sc_axis(raw.lx),ly=sc_axis(raw.ly),rx=sc_axis(raw.rx),ry=sc_axis(raw.ry);
  if(lx*lx+ly*ly<.18f*.18f && rx*rx+ry*ry<.18f*.18f)m->recentred=1;
  if(now<m->release_until || !m->recentred)out.rx=out.ry=0;
 }
 m->was_air=air;
 return out;
}
static sc_pad sc_manual_catch_translate_ex(const sc_config*c,sc_state*s,sc_manual_catch*m,
                                      sc_pad raw,double now,int state,int trick_id){
 return sc_manual_catch_translate_observed(c,s,m,raw,now,state,trick_id,-1);
}
static sc_pad sc_manual_catch_translate(const sc_config*c,sc_state*s,sc_manual_catch*m,
                                      sc_pad raw,double now,int state){
 return sc_manual_catch_translate_ex(c,s,m,raw,now,state,-1);
}
#endif
