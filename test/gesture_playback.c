#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "../src/native_pose_restore.h"
#include "../src/gesture_playback.h"
static unsigned char arena[1024];
static int readmem(np_address a,void*d,np_address n){
 if(a<(np_address)arena || a+n>(np_address)arena+sizeof arena)return 0;
 memcpy(d,(void*)a,(size_t)n);return 1;
}
static void q(unsigned n,np_address v){memcpy(arena+n,&v,8);}
int main(void){
 const np_address base=0x140000000ULL,asset=(np_address)(arena+128),bank=0x987650;
 const unsigned funcs[6]={0x11dafd4,0x5f1350,0x11dcc4c,0x11dd500,0x11dd7d0,0xe443f0};
 /* Relocate the fake executable base so its expected vtable is in the fixture. */
 const np_address fakebase=(np_address)(arena+512)-0x61ab5a0;
 q(128,fakebase+0x61ab4a8);q(136,fakebase+0x72afbb0);q(152,(np_address)(arena+256));
 strcpy((char*)arena+256,"Animation/Dingo/Float.Anim.TrickFlipSpeed");
 q(224,(np_address)(arena+512));q(504,0x12345678);
 for(int i=0;i<6;i++)q(512+8*i,fakebase+funcs[i]);
 sc_gp_binding b={0};assert(sc_gp_bind(asset,fakebase,readmem,&b));
 assert(b.asset==asset && b.functions[0]==0x12345678 && b.functions[4]==fakebase+0x11dd500);
 for(int i=0;i<6;i++)assert(b.functions[i+1]==fakebase+funcs[i]);
 assert(sc_gp_value(.5f,.25f,1,asset+0x60,asset,bank,bank)==.125f);
 assert(sc_gp_value(.5f,1.75f,1,asset+0x60,asset,bank,bank)==.875f);
 assert(sc_gp_value(.5f,1.f,1,asset+0x60,asset,bank,bank)==.5f);
 assert(sc_gp_value(.5f,.25f,0,asset+0x60,asset,bank,bank)==.5f);
 assert(sc_gp_value(.5f,.25f,1,asset+0x60,asset,bank+8,bank)==.5f);
 assert(sc_gp_value(.5f,.25f,1,asset+0x68,asset,bank,bank)==.5f);
 assert(sc_gp_value(.5f,.25f,1,asset+0x60,asset,bank,0)==.5f);
 assert(sc_gp_value(-1.f,.25f,1,asset+0x60,asset,bank,bank)==-1.f);
 assert(sc_gp_value(.5f,NAN,1,asset+0x60,asset,bank,bank)==.5f);
 assert(sc_gp_value(.5f,.1f,1,asset+0x60,asset,bank,bank)==.5f);
 arena[256]='X';assert(!sc_gp_bind(asset,fakebase,readmem,&b));arena[256]='A';
 q(512+24,base);assert(!sc_gp_bind(asset,fakebase,readmem,&b));
 puts("playback speed: exact property and six native methods, RTTI retained, quarter/fast relative values, local bank and reference scope, disabled/sentinel/invalid input pass-through passed");return 0;
}
