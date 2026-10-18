#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include "../src/native_menu_respawn.h"
#include "../src/grind_target_read.h"
static unsigned char core[0x450],root[0x70],types[0xd0],target[0x200];
static int deny_types,deny_target,race;static unsigned root_reads;
static void ptr(void*b,unsigned off,np_address v){memcpy((char*)b+off,&v,8);}
static int read_fixture(np_address a,void*out,np_address size){
 if(a==(np_address)core+0x3b8 && ++root_reads==2 && race)return 0;
 unsigned char*blocks[]={core,root,types,target};unsigned sizes[]={sizeof core,sizeof root,sizeof types,sizeof target};
 for(int i=0;i<4;i++)if(a>=(np_address)blocks[i] && a+size<=(np_address)blocks[i]+sizes[i]){
  if((i==2 && deny_types)||(i==3 && deny_target))return 0;
  memcpy(out,(void*)a,size);return 1;
 }return 0;
}
static unsigned capture(gt_sample*s){root_reads=0;return gt_read((np_address)core,read_fixture,s);}
int main(void){
 ptr(core,0x3b8,(np_address)root);ptr(root,0x20,(np_address)types);ptr(root,0x10,(np_address)target);
 *(int*)(types+0xb4)=3;*(int*)(types+0xc0)=7;target[0x1fb]=1;
 *(float*)(target+0xc0)=.6f;*(float*)(target+0xc8)=-.8f;
 gt_sample s;assert(capture(&s)==3 && s.grinding_type==3 && s.skate_grinding_type==7 && s.targeting_grind==1 && s.target_direction[2]==-.8f);
 deny_types=1;assert(capture(&s)==2 && !s.grinding_type && !s.skate_grinding_type);deny_types=0;
 deny_target=1;assert(capture(&s)==1 && !s.targeting_grind && s.target_direction[2]==0);deny_target=0;
 target[0x1fb]=2;assert(capture(&s)==1);target[0x1fb]=1;
 *(float*)(target+0xc0)=0.0f/0.0f;assert(capture(&s)==1 && s.target_direction[0]==0);
 *(float*)(target+0xc0)=101;assert(capture(&s)==1);*(float*)(target+0xc0)=0;
 race=1;assert(!capture(&s) && !s.valid && !s.targeting_grind);race=0;
 ptr(core,0x3b8,0);assert(!capture(&s));
 puts("PASS: native grind exporter field reads, independent validity, invalid/missing target, finite vectors and changing root rejection");
}
