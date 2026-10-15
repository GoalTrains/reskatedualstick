#include <stdio.h>
#include <assert.h>
#include "../src/xl_stance_agreement.h"
static nr_frame f={.selector=0x10000,.core=0x20000,.context=0x30000,.source=0x40000,.forward={0,0,1}};
static double now;
static int step(sc_xl_agreement*g,int profile){now+=40;f.position[2]+=.2f;return sc_xl_agreement_step(g,&f,100,1,profile,now);}
int main(int argc,char**argv){
 if(argc==2){FILE* file=fopen(argv[1],"r");assert(file);char line[512];sc_xl_agreement g={0};double t;int state,basis,native,idle,gen,mirror;float x,y,z,fx,fy,fz;unsigned reversed_regular=0,reversed_switch=0,mirror_change=0,eligible=0;
  while(fgets(line,sizeof line,file)){
   if(sscanf(line,"%lf,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%d",&t,&state,&basis,&native,&idle,&gen,&x,&y,&z,&fx,&fy,&fz,&mirror)!=13)continue;
   f.selector=0x10000+gen*4096;f.core=0x20000+gen*4096;f.basis=basis;f.native_frame=native;f.mirrored=mirror;
   f.position[0]=x;f.position[1]=y;f.position[2]=z;f.forward[0]=fx;f.forward[1]=fy;f.forward[2]=fz;
   int hint=sc_xl_agreement_step(&g,state<0?0:&f,state,idle,0,t);
   if(hint>=0 && idle){eligible++;assert(hint==(native^mirror^g.mirror));reversed_regular+=basis==1 && native==1 && hint==0;reversed_switch+=basis==1 && native==0 && hint==1;mirror_change+=mirror!=g.mirror;}
  }fclose(file);printf("Recorded run: basis 1/1 regular=%u, basis 1/0 switch=%u, changed mirror=%u eligible samples\n",reversed_regular,reversed_switch,mirror_change);printf("Eligible physical-foot samples=%u\n",eligible);fflush(stdout);assert(eligible>0);return 0;
 }
 for(int profile=0;profile<2;profile++)for(int mirror=0;mirror<2;mirror++){
  sc_xl_agreement g={0};f.basis=f.native_frame=0;f.mirrored=mirror;f.forward[2]=1;
  for(int i=0;i<30;i++)step(&g,profile);assert(g.baseline_valid);
  /* The recorded reversed-board regular case: raw frame is 1, physical jobs are normal. */
  f.basis=f.native_frame=1;f.mirrored=!mirror;
  for(int i=0;i<20;i++)step(&g,profile);assert(step(&g,profile)==0);
  assert(sc_xl_agreement_cached(1100,1000,7,7,profile,profile,0,1,1)==0);
  assert(sc_xl_agreement_cached(1100,1000,7,7,profile,profile,0,1,0)==-1);
  nr_frame actual=f;assert(sc_xl_agreement_frame_matches(0,mirror,&f,&actual));actual.basis=0;assert(!sc_xl_agreement_frame_matches(0,mirror,&f,&actual));actual=f;actual.source++;assert(!sc_xl_agreement_frame_matches(0,mirror,&f,&actual));
  f.native_frame=0;f.forward[2]=-1;
  for(int i=0;i<20;i++)step(&g,profile);assert(step(&g,profile)==1);
  assert(!sc_xl_agreement_can_apply(1,0,100,0));assert(sc_xl_agreement_can_apply(1,0,100,1));
  /* Turning the board without changing the physical animation reference
   * must NOT exchange anatomical foot jobs, even though motion vs board does. */
  f.mirrored=mirror;f.native_frame=0;f.forward[2]=-1;
  for(int i=0;i<20;i++)step(&g,profile);assert(step(&g,profile)==0);
  f.native_frame=1;f.forward[2]=1;
  for(int i=0;i<20;i++)step(&g,profile);assert(step(&g,profile)==1);
  f.forward[2]=-1;
  /* Frame that disagrees with aligned travel cannot qualify. */
  f.native_frame=1;for(int i=0;i<20;i++)assert(step(&g,profile)==-1);
  f.basis=0;f.native_frame=0;f.mirrored=mirror;f.forward[2]=1;for(int i=0;i<20;i++)step(&g,profile);assert(step(&g,profile)==0);
 }
 puts("PASS: XL physical-foot agreement separates board and animation orientation, changed animation mirror, regular/goofy and both directions; live frame/owner cache guards and held-foot protection");
}