#include <stdio.h>
#include <assert.h>
#include "../src/xl_stance_agreement.h"
static nr_frame f={.selector=0x10000,.core=0x20000,.context=0x30000,.source=0x40000,.forward={0,0,1}};
static double now;
static int step(sc_xl_agreement* g,int state,int idle,int profile){now+=40;f.position[2]+=.2f;return sc_xl_agreement_step(g,&f,state,idle,profile,now);}
int main(int argc,char** argv){
 if(argc==2){
  FILE* file=fopen(argv[1],"r");assert(file);sc_xl_agreement g={0};char line[512];double t;int state,basis,native,idle,gen,mirror;float x,y,z,fx,fy,fz;int last=-1,count=0;
  while(fgets(line,sizeof line,file)){
   if(sscanf(line,"%lf,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%d",&t,&state,&basis,&native,&idle,&gen,&x,&y,&z,&fx,&fy,&fz,&mirror)!=13)continue;
   f.selector=0x10000+gen*4096;f.core=0x20000+gen*4096;f.basis=basis;f.native_frame=native;f.mirrored=mirror;
   f.position[0]=x;f.position[1]=y;f.position[2]=z;f.forward[0]=fx;f.forward[1]=fy;f.forward[2]=fz;
   int hint=sc_xl_agreement_step(&g,state<0?0:&f,state,idle,0,t);
   if(hint>=0 && idle && hint!=last){printf("Agreement %.3f: %s\n",t/1000,hint?"switch":"regular");count++;}
   if(hint>=0 && idle)last=hint;
  }fclose(file);assert(count);return 0;
 }
 for(int profile=0;profile<2;profile++)for(int mirror=0;mirror<2;mirror++){
  sc_xl_agreement g={0};f.basis=f.native_frame=0;f.mirrored=mirror;f.forward[2]=1;
  for(int i=0;i<30;i++)step(&g,100,0,profile);assert(g.baseline_valid && step(&g,100,1,profile)==0);
  assert(step(&g,100,0,profile)==0);assert(!sc_xl_agreement_can_apply(0,1,100,0));
  assert(sc_xl_agreement_can_apply(0,1,100,1));assert(!sc_xl_agreement_can_apply(0,1,201,1));
  assert(step(&g,201,1,profile)==-1);assert(step(&g,402,1,profile)==-1);
  f.native_frame=1;for(int i=0;i<20;i++)assert(step(&g,100,1,profile)==-1); /* Flag alone disagrees with facing. */
  f.forward[2]=-1;for(int i=0;i<20;i++)step(&g,100,1,profile);assert(step(&g,100,1,profile)==1);
  f.native_frame=0;for(int i=0;i<20;i++)assert(step(&g,100,1,profile)==-1); /* Ground flag reset while switch must not rewrite jobs. */
  f.forward[2]=1;for(int i=0;i<20;i++)step(&g,100,1,profile);assert(step(&g,100,1,profile)==0);
  f.basis=1;assert(step(&g,100,1,profile)==-1);f.basis=0;f.mirrored=!mirror;assert(step(&g,100,1,profile)==-1);f.mirrored=mirror;
  f.position[2]+=1000;assert(step(&g,100,1,profile)==-1);
  sc_xl_agreement_missing(&g);assert(g.baseline_valid && !g.sampled);
  f.core++;assert(step(&g,100,1,profile)==-1 && !g.baseline_valid);f.core--;
  assert(sc_xl_agreement_step(&g,0,-1,1,profile,now)==-1 && !g.baseline_valid);
 }
 assert(sc_xl_agreement_cached(1100,1000,7,7,0,0,0,0,0)==0);
 assert(sc_xl_agreement_cached(1300,1000,7,7,0,0,0,0,0)==-1);
 assert(sc_xl_agreement_cached(1100,1000,7,8,0,0,0,0,0)==-1);
 assert(sc_xl_agreement_cached(1100,1000,7,7,1,0,0,0,0)==-1);
 assert(sc_xl_agreement_cached(1100,1000,7,7,0,0,0,0,1)==-1);
 puts("PASS: settled native/facing agreement restores XL normal/switch reference for regular/goofy without pose writes; native reset while actual switch, contradictory normalized frame, active controls, air/grind/UI, teleport, stale/foreign cache excluded");
}
