#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/ground_pose_guard.h"
static nr_frame frame={.selector=0x10000,.core=0x20000,.context=0x30000,.source=0x40000,.forward={0,0,-1}};
static double now=1000;
static int step(sc_ground_pose_guard* guard,int state,int idle,int goofy) {
 now+=40;frame.position[2]+=.2f;return sc_ground_pose_step(guard,&frame,state,idle,goofy,now);
}
static void broken_sequence(sc_ground_pose_guard* guard) {
 *guard=(sc_ground_pose_guard){0};frame.basis=0;frame.native_frame=1;frame.forward[2]=-1;
 for(int i=0;i<10;i++)assert(!step(guard,100,1,0));
 frame.native_frame=0;for(int i=0;i<3;i++)assert(!step(guard,100,1,0));
 frame.basis=1;assert(!step(guard,100,1,0));assert(guard->suspect);
 /* The ordinary air state must retain evidence, never trigger recovery. */
 for(int i=0;i<10;i++)assert(!step(guard,201,1,0));
 frame.forward[2]=1;frame.native_frame=1;
}
int main(int argc,char** argv) {
 if(argc==2) {
  FILE* file=fopen(argv[1],"r");assert(file);sc_ground_pose_guard guard={0};double t;int state,basis,native,idle,generation;float x,y,z,fx,fy,fz;int fired=0;char line[512];
  while(fgets(line,sizeof line,file)) {
   int mirrored=0,fields=sscanf(line,"%lf,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%d",&t,&state,&basis,&native,&idle,&generation,&x,&y,&z,&fx,&fy,&fz,&mirrored);
   if(fields<12)continue;
   frame.core=0x20000+generation*4096;frame.selector=0x10000+generation*4096;
   frame.basis=basis;frame.native_frame=native;frame.mirrored=mirrored;frame.position[0]=x;frame.position[1]=y;frame.position[2]=z;frame.forward[0]=fx;frame.forward[1]=fy;frame.forward[2]=fz;
   int reason=sc_ground_pose_step(&guard,state<0?0:&frame,state,idle,0,t);
   if(reason){printf("Recorded recovery candidate at %.3f seconds (reason %d)\n",t/1000,reason);fired++;}
  }
  fclose(file);assert(fired);return 0;
 }
 sc_ground_pose_guard guard={0};broken_sequence(&guard);
 for(int i=0;i<30;i++)assert(!step(&guard,100,0,0)); /* Held inputs prevent a reset. */
 int fired=0;for(int i=0;i<40;i++)fired+=step(&guard,100,1,0);assert(fired==1);
 /* Same native flags without the specific grounded break are insufficient. */
 guard=(sc_ground_pose_guard){0};for(int i=0;i<50;i++)assert(!step(&guard,100,1,0));
 broken_sequence(&guard);frame.forward[2]=-1;
 for(int i=0;i<50;i++)assert(!step(&guard,100,1,0)); /* Genuine switch travel. */
 broken_sequence(&guard);frame.core++;assert(!step(&guard,100,1,0) && !guard.suspect);frame.core--;
 broken_sequence(&guard);assert(!sc_ground_pose_step(&guard,0,-1,1,0,now+40) && !guard.suspect); /* UI/ownership loss. */
 broken_sequence(&guard);for(int i=0;i<50;i++)assert(!step(&guard,100,1,1)); /* Profile change clears the old regular reference. */
 broken_sequence(&guard);sc_ground_pose_missing(&guard,now+40);assert(guard.suspect && !guard.have_previous);
 now+=40;fired=0;for(int i=0;i<40;i++)fired+=step(&guard,100,1,0);assert(fired==1);
 broken_sequence(&guard);now+=16000;assert(!step(&guard,100,1,0) && !guard.suspect);
 broken_sequence(&guard);frame.position[2]+=1000;assert(!step(&guard,100,1,0) && !guard.neutral); /* Teleport is not movement. */
 /* Learn a settled normal pose, then distinguish ordinary airs from a ground
  * mirror break that survives into the next reversed-basis landing. */
 guard=(sc_ground_pose_guard){0};frame.forward[2]=1;frame.basis=frame.native_frame=0;frame.mirrored=1;
 for(int i=0;i<20;i++)assert(!step(&guard,100,1,0));assert(guard.baseline_valid);
 frame.basis=frame.native_frame=1;
 for(int i=0;i<20;i++)assert(!step(&guard,201,1,0));
 for(int i=0;i<20;i++)assert(!step(&guard,100,1,0));assert(!guard.mirror_break);
 frame.native_frame=0;assert(!step(&guard,100,0,0));
 frame.basis=0;frame.mirrored=0;assert(!step(&guard,100,0,0));assert(guard.mirror_break);
 for(int i=0;i<10;i++)assert(!step(&guard,605,0,0));
 frame.basis=frame.native_frame=1;
 for(int i=0;i<15;i++)assert(!step(&guard,201,1,0)); /* Never reset during a trick. */
 for(int i=0;i<20;i++)assert(!step(&guard,100,0,0)); /* Held controls defer. */
 fired=0;for(int i=0;i<40;i++)fired+=step(&guard,100,1,0)!=0;assert(fired==1);
 /* A mirror change only in an air cannot establish a grounded break. */
 guard.mirror_break=0;frame.basis=frame.native_frame=1;frame.mirrored=1;
 for(int i=0;i<5;i++)assert(!step(&guard,100,1,0));
 frame.mirrored=0;frame.basis=frame.native_frame=0;
 for(int i=0;i<5;i++)assert(!step(&guard,201,1,0));
 frame.basis=frame.native_frame=1;
 for(int i=0;i<30;i++)assert(!step(&guard,100,1,0));assert(!guard.mirror_break);
 /* Restoring the learned mirror cancels the suspect state. */
 guard.mirror_break=1;guard.mirror_break_time=now;frame.mirrored=1;
 assert(!step(&guard,100,1,0) && !guard.mirror_break);
 guard.mirror_break=1;guard.mirror_break_time=now;frame.mirrored=0;frame.forward[2]=-1;
 for(int i=0;i<30;i++)assert(!step(&guard,100,1,0)); /* Backwards travel is insufficient. */
 frame.forward[2]=1;now+=16000;assert(!step(&guard,100,1,0) && !guard.mirror_break);
 /* Without a grounded reference, the same final flags are insufficient. */
 guard=(sc_ground_pose_guard){0};
 for(int i=0;i<30;i++)assert(!step(&guard,100,1,0));assert(!guard.baseline_valid);
 /* A distant reference cannot arm from an unrelated later mirror change. */
 guard=(sc_ground_pose_guard){0};frame.basis=frame.native_frame=0;frame.mirrored=1;
 for(int i=0;i<20;i++)assert(!step(&guard,100,1,0));
 frame.basis=frame.native_frame=1;assert(!step(&guard,100,1,0));
 now+=700;frame.basis=frame.native_frame=0;frame.mirrored=0;
 assert(!step(&guard,100,1,0) && !guard.mirror_break);
 guard.mirror_break=1;guard.mirror_break_time=now;
 assert(!sc_ground_pose_step(&guard,0,-1,1,0,now+40) && !guard.mirror_break && !guard.baseline_valid);
 guard.mirror_break=1;guard.mirror_break_time=now;
 assert(!step(&guard,100,1,1) && !guard.mirror_break);
 for(int goofy=0;goofy<2;goofy++)for(int mirror=0;mirror<2;mirror++) {
  guard=(sc_ground_pose_guard){0};frame.forward[2]=1;frame.basis=frame.native_frame=0;frame.mirrored=mirror;
  for(int i=0;i<20;i++)assert(!step(&guard,100,1,goofy));
  assert(guard.baseline_valid && guard.baseline_mirror==mirror && guard.profile==goofy);
  frame.basis=frame.native_frame=1;
  for(int i=0;i<10;i++)assert(!step(&guard,201,1,goofy));
  for(int i=0;i<20;i++)assert(!step(&guard,100,1,goofy)); /* Ordinary switch air is insufficient. */
  frame.native_frame=0;assert(!step(&guard,100,0,goofy));
  frame.basis=0;frame.mirrored=!mirror;assert(!step(&guard,100,0,goofy) && guard.mirror_break);
  frame.basis=frame.native_frame=1;
  for(int i=0;i<10;i++)assert(!step(&guard,201,1,goofy));
  for(int i=0;i<20;i++)assert(!step(&guard,100,0,goofy));
  frame.forward[2]=-1;for(int i=0;i<20;i++)assert(!step(&guard,100,1,goofy));
  frame.forward[2]=1;
  fired=0;for(int i=0;i<30;i++){int reason=step(&guard,100,1,goofy);if(reason){assert(reason==2);fired++;}}
  assert(fired==1);
  guard.mirror_break=1;guard.mirror_break_time=now;
  assert(!step(&guard,100,1,!goofy) && !guard.mirror_break && !guard.baseline_valid);
 }
 puts("PASS: per-rider regular/goofy reference with either mirror value, ordinary 180s excluded, grounded mirror break plus landing recovers once; air/grind and held/backwards input, missing baseline, stale reference, restored mirror, UI/ownership/profile changes cancel");
 puts("PASS: specific grounded fakie-frame break plus confirmed regular travel and 500-ms neutral gate, one-shot recovery; held controls, genuine switch, unqualified flags, UI/actor change, goofy, expiry and teleport excluded; short export gaps preserve evidence");
}
