#ifndef SC_SLIDE_REVERT_H
#define SC_SLIDE_REVERT_H
/* Physical stick angles: 0 down, 90 right. Matching circles use a 30-degree
 * threshold; mirrored thumb circles use 55 to preserve shallow slide adjustment. */
typedef struct {
 int armed,blocked;
 float left,right,travel_left,travel_right;
 double last,moving;
} sc_slide_revert;
static float sc_sr_abs(float v){return v<0?-v:v;}
static float sc_sr_wrap(float v){while(v>180)v-=360;while(v<-180)v+=360;return v;}
static void sc_sr_cancel(sc_slide_revert*s){*s=(sc_slide_revert){0};}
static void sc_sr_begin(sc_slide_revert*s,float left,float right,double now){
 if(s->blocked || s->armed)return;
 s->armed=1;s->left=left;s->right=right;s->last=now;
 s->travel_left=s->travel_right=0;s->moving=0;
}
static int sc_sr_step(sc_slide_revert*s,int enabled,int eligible,int sliding,
                     float lm,float rm,float left,float right,double now){
 if(!enabled || !eligible){sc_sr_cancel(s);return 0;}
 if(lm<.18f && rm<.18f){sc_sr_cancel(s);return 0;}
 if(!s->armed)return 0;
 if(!sliding || lm<.4f || rm<.4f || now<s->last || now-s->last>100){
  s->armed=0;s->blocked=1;return 0;
 }
 float dl=sc_sr_wrap(left-s->left),dr=sc_sr_wrap(right-s->right);
 s->left=left;s->right=right;s->last=now;
 if(sc_sr_abs(dl)>60 || sc_sr_abs(dr)>60){s->armed=0;s->blocked=1;return 0;}
 s->travel_left+=dl;s->travel_right+=dr;
 if(!s->moving && (sc_sr_abs(s->travel_left)>8 || sc_sr_abs(s->travel_right)>8))s->moving=now;
 if(s->moving && now-s->moving>350){s->armed=0;s->blocked=1;return 0;}
 int parallel=sc_sr_abs(sc_sr_abs(sc_sr_wrap(left-right))-180)<=30 &&
   ((s->travel_left>=30 && s->travel_right>=30) ||
    (s->travel_left<=-30 && s->travel_right<=-30));
 /* Mirrored thumb circles are the natural continuation of opposing slide
  * sticks. Require a larger arc so ordinary fore/aft slide adjustment stays
  * available; the recorded attempts sweep well past this angle. */
 int mirrored=(s->travel_left>=55 && s->travel_right<=-55) ||
              (s->travel_left<=-55 && s->travel_right>=55);
 if(parallel || mirrored){
  int side=s->travel_left<0?1:-1;
  s->armed=0;s->blocked=1;return side;
 }
 return 0;
}
#endif
