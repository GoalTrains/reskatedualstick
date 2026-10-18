/* Optional native entry experiment. No Windows/game memory access here. */
#ifndef SC_GRIND_ENTRY_PLAN_H
#define SC_GRIND_ENTRY_PLAN_H
typedef struct {int active,type;float yaw,pitch;} sc_entry_plan;
static sc_entry_plan sc_back_entry_plan(const sc_config*c,const sc_state*s){
 sc_entry_plan p={0};
 if(!c->enabled || !c->pure || !c->split_grinds || c->one_foot_flips ||
    !c->air_controls || !s->air || s->grind || !s->entry_back_active ||
    s->radial_passthrough)return p;
 float gain=c->grind_sensitivity>0?c->grind_sensitivity:1;
 float x=-s->entry_back_x*gain;
 if(x>1)x=1;if(x<-1)x=-1;
 /* Native GrindTargetType: 1=Tipslide_Nose, 2=Tipslide_Tail. Keep the
  * existing native input frame separate from physical stick/foot selection. */
 p.type=s->sw_lat?1:2;
 if(s->sw_lat && c->switch_flip==2)x=-x;
 p.yaw=x*1.570796327f;p.pitch=s->entry_back_y*.35f;
 if(s->sw_lat)p.pitch=-p.pitch;
 p.active=1;return p;
}
static int sc_entry_fresh(unsigned long long now,unsigned long long seen,int state){
 return seen && now>=seen && now-seen<=60 && state==201;
}
#endif
