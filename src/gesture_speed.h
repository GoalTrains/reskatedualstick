/* Physical input only: translated holds must never feed the speed estimate. */
#ifndef SC_GESTURE_SPEED_H
#define SC_GESTURE_SPEED_H
typedef struct {
 float x[2],y[2],distance[2],factor;
 double at,window;
 int seen,air;
} sc_gesture_speed;
static float sc_gs_factor(float speed){
 if(speed<=3.f)return .25f;
 if(speed>=24.f)return 1.75f;
 return .25f+(speed-3.f)*(1.5f/21.f);
}
static float sc_gs_step(sc_gesture_speed*s,sc_pad raw,double now,int state,int enabled){
 float x[2]={sc_axis(raw.lx),sc_axis(raw.rx)},y[2]={sc_axis(raw.ly),sc_axis(raw.ry)};
 int air=state>=200 && state<300;
 if(!enabled || state<100 || state>=400){*s=(sc_gesture_speed){0};return 1.f;}
 if(s->seen && now==s->at)return s->factor>0?s->factor:1.f;
 if(!s->seen || now-s->at>100 || now<s->at || (s->air && !air)){
  *s=(sc_gesture_speed){0};s->seen=1;s->window=now;s->factor=1.f;
 }else if(!s->air && !air){
  double dt=now-s->at;
  if(now-s->window>40){s->distance[0]=s->distance[1]=0;s->window=s->at;}
  int moved=0;
  for(int i=0;i<2;i++){
   float old=s->x[i]*s->x[i]+s->y[i]*s->y[i],mag=x[i]*x[i]+y[i]*y[i];
   float dx=x[i]-s->x[i],dy=y[i]-s->y[i];
   /* Reject release-to-centre and idle sensor noise. Tangential scoops count. */
   if(mag>.09f && mag>=old*.7f && dx*dx+dy*dy>.0001f && dt>=1){
    s->distance[i]+=sc_sqrt(dx*dx+dy*dy);moved=1;
   }
  }
  double elapsed=now-s->window;
  if(moved && elapsed>=12){
   float d=s->distance[0]>s->distance[1]?s->distance[0]:s->distance[1];
   if(d>.08f)s->factor=sc_gs_factor(d*1000.f/(float)elapsed);
  }
 }
 for(int i=0;i<2;i++){s->x[i]=x[i];s->y[i]=y[i];}
 s->at=now;s->air=air;
 return s->factor>0?s->factor:1.f;
}
#endif
