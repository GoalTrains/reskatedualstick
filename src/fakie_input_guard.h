#ifndef SC_FAKIE_INPUT_GUARD_H
#define SC_FAKIE_INPUT_GUARD_H
#include "translate.h"
static int sc_fakie_vertical_hold(const sc_config* c,short x,short y){
 float fx=sc_axis(x),fy=sc_axis(y);
 float low=c->deadzone>.1f?c->deadzone:.15f;
 /* Both partial manual leans and full fakie/nollie setups express held
  * intent. In After landing mode neither should be normalized away. */
 return sc_abs(fy)>=low&&sc_abs(fx)<=sc_abs(fy)*.6f;
}
static int sc_fakie_protect_input(const sc_config* c,const sc_state* s,const sc_pad* raw,const sc_pad* out){
 if(raw->buttons&(SC_LSHOULDER|SC_RSHOULDER))return 1;
 if(s->d_land_hold)return 1;
 return sc_fakie_vertical_hold(c,raw->lx,raw->ly)||sc_fakie_vertical_hold(c,raw->rx,raw->ry)||sc_fakie_vertical_hold(c,out->rx,out->ry);
}
#endif
