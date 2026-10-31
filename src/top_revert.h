#ifndef SC_TOP_REVERT_H
#define SC_TOP_REVERT_H
typedef struct {int armed,blocked,fired,start_side,passed_top,owner_valid,owner_left,owner_y;float last,travel;double time,moving;} sc_top_revert;
static float sc_top_abs(float v){return v<0?-v:v;}
static float sc_top_wrap(float v){while(v>180)v-=360;while(v<-180)v+=360;return v;}
/* The caller selects the unused foot/hemisphere and rotates it to a top arc:
 * sc_angle 90 right, 180 top, 270 left. Own-end scoops and second-foot input
 * cannot qualify. Physical circular direction is preserved by the rotation. */
static int sc_top_step(sc_top_revert*s,int enabled,int eligible,float left_mag,float right_mag,float angle,double now){
 if(!enabled || !eligible){*s=(sc_top_revert){0};return 0;}
 if(right_mag<.18f){*s=(sc_top_revert){0};return 0;}
 if(s->blocked)return 0;
 if(left_mag>=.2f || angle<80 || angle>280){s->armed=0;s->blocked=1;return 0;}
 if(!s->armed){
  if(right_mag<.3f)return 0;
  if(!((angle>=80&&angle<=140)||(angle>=220&&angle<=280))){s->blocked=1;return 0;}
  s->armed=1;s->start_side=angle<180?1:-1;s->last=angle;s->time=now;return 0;
 }
 if(right_mag<.2f){s->armed=0;s->blocked=1;return 0;}
 if(now<s->time || now-s->time>100){s->armed=0;s->blocked=1;return 0;}
 float delta=sc_top_wrap(angle-s->last);s->last=angle;s->time=now;
 if(sc_top_abs(delta)>60){s->armed=0;s->blocked=1;return 0;}
 s->travel+=delta*s->start_side;
 if(s->travel < -15){s->armed=0;s->blocked=1;return 0;}
 if(!s->moving && s->travel>8)s->moving=now;
 if(s->moving && now-s->moving>450){s->armed=0;s->blocked=1;return 0;}
 if(angle>=155&&angle<=205)s->passed_top=1;
 int crossed=s->start_side>0?angle>=220:angle<=140;
 if(s->passed_top && crossed && s->travel>=90 && right_mag>=.65f){
  s->armed=0;s->blocked=s->fired=1;return -s->start_side;
 }
 return 0;
}
#endif
