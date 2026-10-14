#define main pose_fixture_main
#include "native_pose_restore.c"
#undef main
#include "../src/native_menu_respawn.h"
static void basis(int value){source[0x10]=value;unsigned bits=value?0x2000:0;memcpy(context+0x13c4,&bits,4);}
int main(void){
 pose_fixture_main();np_address base=0x140000000ULL;
 float position[]={42,10,-9},forward[]={0,0,1};
 memcpy(core+0x100,position,12);memcpy(core+0x150,forward,12);
 nr_frame before,after;float matrix[16];
 for(int goofy=0;goofy<2;goofy++) {
  flags(1,1);source[0x13]=goofy?1:0;basis(0);
  assert(nr_read((np_address)selector,base,read_mem,&before));
  assert(!nr_mismatch(&before,&before));
  flags(0,0);source[0x13]=goofy?0:1;basis(1);
  assert(nr_read((np_address)selector,base,read_mem,&after));
  assert(nr_mismatch(&before,&after));
  nr_frame wrong=after;wrong.core++;assert(!nr_mismatch(&before,&wrong));
  assert(nr_transform(&after,0,1000,1,matrix));
  assert(matrix[12]==42 && matrix[13]>10 && matrix[13]<10.1f && matrix[14]==-9);
  assert(matrix[10]==-1 && matrix[5]==1 && matrix[15]==1);
  assert(matrix[0]*matrix[10]-matrix[2]*matrix[8]>.999f);
  float last[]={41,10,-9};assert(nr_transform(&after,last,100,1,matrix));
  assert(matrix[8]==1 && matrix[10]==0); /* Prefer known movement. */
  last[0]=-1000;assert(nr_transform(&after,last,100,1,matrix));assert(matrix[10]==-1);
  assert(nr_transform(&after,position,100,1,matrix));assert(matrix[10]==-1);
 }
 assert(!nr_read((np_address)selector,0,read_mem,&after));
 state[0xa4]=1;assert(!nr_read((np_address)selector,base,read_mem,&after));state[0xa4]=0;
 state[0xa3]=1;assert(!nr_read((np_address)selector,base,read_mem,&after));state[0xa3]=0;
 float nan=__builtin_nanf("");memcpy(core+0x100,&nan,4);assert(!nr_read((np_address)selector,base,read_mem,&after));
 memcpy(core+0x100,position,12);memset(core+0x150,0,12);assert(!nr_read((np_address)selector,base,read_mem,&after));
 puts("PASS: native respawn owner/export validation for regular and goofy, menu mismatch, upright movement-aligned reset, stationary fallback and invalid geometry rejection");
}
