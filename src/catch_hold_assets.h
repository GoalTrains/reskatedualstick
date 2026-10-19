/* Reversible, typed native gesture hold ranges. Recognition patterns, enums,
 * timing, magnitude and emitted stick direction are never replaced. */
#ifndef SC_CATCH_HOLD_ASSETS_H
#define SC_CATCH_HOLD_ASSETS_H
#include "native_pose_restore.h"
#define SC_CATCH_ASSET_CAP 256
typedef struct {np_address asset,name,patterns;unsigned hash;float min,max;int own_min,own_max;} sc_catch_hold_asset;
typedef struct {sc_catch_hold_asset slots[SC_CATCH_ASSET_CAP];unsigned count;int active,failures,conflicts;} sc_catch_hold_assets;
static int sc_cha_name(np_read_fn read,np_address name){
 static const char prefix[]="gameplay/input/gestures/fliptricks/gesture_fliptrick_";
 char b[sizeof prefix-1];if(!read(name,b,sizeof b))return 0;
 for(unsigned i=0;i<sizeof b;i++){char c=b[i];if(c>='A'&&c<='Z')c+=32;if(c!=prefix[i])return 0;}return 1;
}
static int sc_cha_read(np_address a,np_address base,np_read_fn read,sc_catch_hold_asset*out){
 np_address vt,name,patterns;unsigned count,hash;float lo,hi,mag;
 if(a<0x10000 || (a&7) || !read(a,&vt,8) || vt!=base+0x6123018 ||
    !read(a+0x18,&name,8) || name<0x10000 || name>=0x7fffffffffffULL || !sc_cha_name(read,name) ||
    !np_pointer(read,a+0x20,&patterns) || !read(patterns-4,&count,4) ||
    !(count&0x7fffffff) || (count&0x7fffffff)>128 || !read(a+0x68,&hash,4) ||
    !read(a+0x38,&lo,4) || !read(a+0x28,&hi,4) || !read(a+0x44,&mag,4) ||
    !(lo>=0 && lo<=360 && hi>=0 && hi<=360 && mag>0 && mag<=1))return 0;
 *out=(sc_catch_hold_asset){0};out->asset=a;out->name=name;out->patterns=patterns;out->hash=hash;out->min=lo;out->max=hi;return 1;
}
static int sc_cha_same(const sc_catch_hold_asset*a,const sc_catch_hold_asset*b){
 return a->asset==b->asset && a->name==b->name && a->patterns==b->patterns && a->hash==b->hash;
}
static int sc_cha_store(np_address address,float value,np_read_fn read,np_write_fn write){
 float check;return write(address,&value,4) && read(address,&check,4) && check==value;
}
static void sc_cha_restore(sc_catch_hold_assets*m,np_address base,np_read_fn read,np_write_fn write){
 for(unsigned i=0;i<m->count;i++){
  sc_catch_hold_asset*a=&m->slots[i],v;
  if(!a->own_min && !a->own_max)continue;
  if(!sc_cha_read(a->asset,base,read,&v) || !sc_cha_same(a,&v)){a->own_min=a->own_max=0;m->conflicts++;continue;}
  if(a->own_min){
   if(v.min!=0){a->own_min=0;m->conflicts++;}
   else if(sc_cha_store(a->asset+0x38,a->min,read,write))a->own_min=0;
   else m->failures++;
  }
  if(a->own_max){
   if(v.max!=360){a->own_max=0;m->conflicts++;}
   else if(sc_cha_store(a->asset+0x28,a->max,read,write))a->own_max=0;
   else m->failures++;
  }
 }
 m->active=0;
}
static int sc_cha_add(sc_catch_hold_assets*m,np_address a,np_address base,np_read_fn read){
 sc_catch_hold_asset v;if(!sc_cha_read(a,base,read,&v))return 0;
 for(unsigned i=0;i<m->count;i++)if(sc_cha_same(&m->slots[i],&v))return 1;
 if(m->count==SC_CATCH_ASSET_CAP){m->failures++;return 0;}
 m->slots[m->count++]=v;return 1;
}
static void sc_cha_prune(sc_catch_hold_assets*m,np_address base,np_read_fn read){
 unsigned out=0;
 for(unsigned i=0;i<m->count;i++){
  sc_catch_hold_asset v;
  if(sc_cha_read(m->slots[i].asset,base,read,&v) && sc_cha_same(&m->slots[i],&v)){
   if(out!=i)m->slots[out]=m->slots[i];out++;
  }else m->conflicts++;
 }
 m->count=out;
}
static int sc_cha_apply(sc_catch_hold_assets*m,np_address base,np_read_fn read,np_write_fn write){
 /* An earlier failed rollback must finish before this becomes active again. */
 if(!m->active){
  int pending=0;for(unsigned i=0;i<m->count;i++)pending|=m->slots[i].own_min|m->slots[i].own_max;
  if(pending){
   sc_cha_restore(m,base,read,write);
   for(unsigned i=0;i<m->count;i++)if(m->slots[i].own_min || m->slots[i].own_max)return 0;
  }
 }
 int failures=m->failures;
 for(unsigned i=0;i<m->count;i++){
  sc_catch_hold_asset*a=&m->slots[i],v;
  if(a->own_min || a->own_max)continue;
  if(!sc_cha_read(a->asset,base,read,&v) || !sc_cha_same(a,&v)){m->conflicts++;continue;}
  a->min=v.min;a->max=v.max;
  if(v.min!=0){
   /* Claim ownership before verification: a successful write followed by a
    * failed read must still be considered for rollback. */
   a->own_min=1;
   if(!sc_cha_store(a->asset+0x38,0,read,write)){m->failures++;continue;}
  }
  if(v.max!=360){a->own_max=1;if(!sc_cha_store(a->asset+0x28,360,read,write))m->failures++;}
 }
 if(m->failures!=failures){sc_cha_restore(m,base,read,write);return 0;}
 m->active=1;return 1;
}
#endif
