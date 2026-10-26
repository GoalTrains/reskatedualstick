#include <stdio.h>
#include <assert.h>
#include "../src/xl_stance_agreement.h"
int main(void){
 assert(!sc_physical_foot_layout(1,1));
 assert(!sc_physical_foot_layout(1,3));
 assert(!sc_physical_foot_layout(0,1));
 assert(sc_physical_foot_layout(0,3));
 FILE* file=fopen("test/data/dev4_physical_feet.csv","r");assert(file);
 char line[512];sc_xl_agreement g={0};int n=0,validated=0,disagree=0,neutral=0,regular=0,switched=0;
 double t;unsigned long long owner;int state,profile,fb,mode,xl,retained,valid,idle,basis,native,mirror;float x,y,z,fx,fy,fz;
 while(fgets(line,sizeof line,file)){
  if(sscanf(line,"%lf,%llu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f",&t,&owner,&state,&profile,&fb,&mode,&xl,&retained,&valid,&idle,&basis,&native,&mirror,&x,&y,&z,&fx,&fy,&fz)!=19)continue;
  n++;
  if(!valid || !sc_physical_foot_layout(fb,mode)){sc_xl_agreement_missing(&g);continue;}
  nr_frame f={.selector=owner,.core=owner+16,.context=owner+32,.source=owner+48,.basis=basis,.native_frame=native,.mirrored=mirror,.position={x,y,z},.forward={fx,fy,fz}};
  int hint=sc_xl_agreement_step(&g,&f,state,idle,profile,t);
  if(hint<0)continue;
  validated++;
  assert(hint==sc_xl_physical_raw(native,mirror,g.mirror));
  if(!xl && retained!=hint){
   disagree++;
   if(sc_xl_agreement_can_apply(hint,retained,state,idle)){
    neutral++;if(hint)switched++;else regular++;
    if(neutral==1)printf("First neutral mismatch: %.3fs retained=%d physical=%d\n",t/1000,retained,hint);
   }
  }
 }
 fclose(file);
 printf("Frames=%d validated=%d Left/Right disagreements=%d neutral=%d normal=%d switch=%d\n",n,validated,disagree,neutral,regular,switched);
 fflush(stdout);assert(n==6694 && validated>500 && disagree>100 && neutral>20 && regular>0);
 /* Native input coordinates remain layout-specific even when foot jobs agree. */
 assert(sc_xl_output_frame(0,1,0,0)==1);
 assert(sc_xl_output_frame(1,1,0,0)==0);
 puts("PASS: recorded Left/Right foot drift passes existing native/motion/neutral guards; XL coordinate convention stays separate");
 return 0;
}
