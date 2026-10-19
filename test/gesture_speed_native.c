#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../src/translate.h"
#include "../src/gesture_speed.h"
typedef uint64_t UINT64;typedef uint32_t DWORD;typedef uint16_t WORD;
typedef UINT64(*cha_find_fn)(WORD,const char*);
static unsigned char arena[4096];static int writes,fail_write;
static UINT64 np_base(void){return 0x140000000ULL;}
static int rd_copy(UINT64 a,void*d,UINT64 n){
 if(a<(UINT64)arena || a+n>(UINT64)arena+sizeof arena)return 0;
 memcpy(d,(void*)a,(size_t)n);return 1;
}
static int np_pointer(int(*read)(UINT64,void*,UINT64),UINT64 a,UINT64*out){return a>=0x10000 && a<0x7fffffffffffULL-8 && read(a,out,8) && *out>=0x10000 && *out<0x7fffffffffffULL && !(*out&7);}
static int cha_write(UINT64 a,const void*d,UINT64 n){
 writes++;if(writes==fail_write)return 0;
 if(a<(UINT64)arena || a+n>(UINT64)arena+sizeof arena)return 0;
 memcpy((void*)a,d,(size_t)n);return 1;
}
static int cha_contract(UINT64 b){(void)b;return 0;}
static void log_text(const char*s){(void)s;}
#include "../src/gesture_speed_native.h"
static void q(unsigned at,UINT64 value){memcpy(arena+at,&value,8);}
static void d(unsigned at,DWORD value){memcpy(arena+at,&value,4);}
static void f(unsigned at,float value){memcpy(arena+at,&value,4);}
static void setup(void){
 memset(arena,0,sizeof arena);memset(gs_curves,0,sizeof gs_curves);memset(gs_points,0,sizeof gs_points);
 writes=fail_write=gs_dirty=0;gs_asset=(UINT64)(arena+0x100);UINT64 base=np_base();
 q(0x100,base+0x65cf308);q(0x108,base+0x777e8b0);q(0x140,(UINT64)(arena+0x200));
 q(0x228,(UINT64)(arena+0x300));d(0x2fc,8);
 for(int c=0;c<2;c++){
  q(0x300+c*24,base+0x76de278);q(0x308+c*24,(UINT64)(arena+0x500+c*8));
  d(0x310+c*24,c?4106527452U:2799703042U);q(0x500+c*8,(UINT64)(arena+0x600+c*0x40));
  q(0x600+c*0x40,base+0x6281558);q(0x608+c*0x40,base+0x748d020);
  q(0x618+c*0x40,(UINT64)(arena+0x704+c*0x80));d(0x700+c*0x80,3);
  for(int i=0;i<3;i++){
   const float x[2][3]={{0,.3f,.9f},{.35f,.82f,.98f}},y[2][3]={{.4f,.4f,1.4f},{.65f,.7f,1}};
   f(0x704+c*0x80+i*28+12,x[c][i]);f(0x704+c*0x80+i*28+20,y[c][i]);
  }
 }
}
int main(void){
 setup();assert(gs_capture(np_base()));unsigned char original[4096];memcpy(original,arena,sizeof arena);
 assert(gs_apply(np_base(),.5f));assert(gs_apply(np_base(),1.75f));assert(gs_apply(np_base(),1));
 assert(!memcmp(original,arena,sizeof arena)); /* all tangents, X and MinX/MaxX preserved */
 assert(gs_apply(np_base(),1.75f));gs_sync(1000,1.75f,0);
 assert(!memcmp(original,arena,sizeof arena)); /* independent option/master disable restores */
 fail_write=writes+5;assert(!gs_apply(np_base(),1.5f));assert(gs_dirty);
 assert(gs_apply(np_base(),1.f));assert(!memcmp(original,arena,sizeof arena));
 f(0x704+20,9.f);int before=writes;assert(!gs_apply(np_base(),.5f));assert(writes==before);
 setup();q(0x600,np_base()+0x6281550);assert(!gs_capture(np_base()));
 setup();d(0x700,4);assert(!gs_capture(np_base()));
 setup();f(0x704+12,.2f);assert(!gs_capture(np_base()));
 puts("gesture speed native: typed asset and curves, bounded scale, exact restoration, partial-write recovery and foreign edit/type/shape rejection passed");return 0;
}
