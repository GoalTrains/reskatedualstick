#define main pose_fixture_main
#include "native_pose_restore.c"
#undef main
#include "../src/fakie_exit.h"
static unsigned char provider[256],graph[512],model[256],bool_node[256],timer_node[256],page[2048];
static unsigned timer_writes;static int deny_write;
static int read_all(np_address a,void* out,np_address n){
 if(read_mem(a,out,n))return 1;
 unsigned char* blocks[]={provider,graph,model,bool_node,timer_node,page};unsigned sizes[]={256,512,256,256,256,2048};
 for(int i=0;i<6;i++)if(a>=(np_address)blocks[i]&&a+n<=(np_address)blocks[i]+sizes[i]){memcpy(out,(void*)a,n);return 1;}return 0;
}
static int timer_write(np_address a,const void* data,np_address n){assert(a==(np_address)page+16&&n==4);if(deny_write)return 0;memcpy((void*)a,data,n);timer_writes++;return 1;}
static void timer(float value){memcpy(page+16,&value,4);}
static int step(fke_state* s,int mode,int phase,int idle,double now){return fke_step(s,1,mode,phase,idle,0,now,(np_address)selector,0x140000000ULL,read_all,timer_write);}
int main(void){
 pose_fixture_main();np_address base=0x140000000ULL;
 flags(1,1);source[0x10]=0;unsigned bits=0;memcpy(context+0x13c4,&bits,4);
 float forward[3]={0,0,1};memcpy(core+0x150,forward,12);
 ptr(core,0x630,(np_address)provider);ptr(core,0x610,(np_address)model);ptr(provider,0x38,(np_address)graph|4);
 ptr(graph,0x170,(np_address)bool_node|4);ptr(graph,0x188,(np_address)timer_node|4);
 ptr(bool_node,0x20,base+0x61ab030);ptr(bool_node,0x60,base+0x61ab080);strcpy((char*)bool_node+0x88,"Animation/Dingo/Bool.Physics.RidingFakie");
 ptr(timer_node,0x20,base+0x61ab550);strcpy((char*)timer_node+0x78,"Animation/Dingo/Float.Physics.TimeRidingFakie");
 unsigned key=0x01008001;memcpy(bool_node+0x28,&key,4);key=0x04010001;memcpy(timer_node+0x28,&key,4);
 ptr(model,0x68,(np_address)page);bits=(1u<<8)|(1u<<16);memcpy(page+0x71c,&bits,4);page[8]=1;timer(.1f);
 fke_sample sample;assert(fke_read((np_address)selector,base,read_all,&sample));
 unsigned char original_source[256];memcpy(original_source,source,256);
 fke_state s={0};assert(step(&s,1,100,1,0)==0);assert(step(&s,1,100,1,19)==0);assert(step(&s,1,100,1,20)==1);assert(sample.value==(np_address)page+16);
 assert(step(&s,1,100,1,40)==0); /* Never reduce an already advanced value. */
 timer(.2f);assert(step(&s,1,100,1,60)==2);timer(.25f);assert(step(&s,1,100,1,100)==2);
 timer(.3f);assert(step(&s,1,100,1,120)==0);assert(step(&s,1,100,1,2000)==0);assert(timer_writes==3);
 assert(!memcmp(original_source,source,256));
 /* Default preserves held input, even after landing. Mode 2 only bypasses
  * neutral following an actual air/rail-to-ground edge, never in air/slide. */
 fke_state held={0};assert(step(&held,1,201,0,2100)==0);assert(step(&held,1,101,0,2200)==0);assert(step(&held,1,100,0,2300)==0);
 assert(step(&held,2,103,0,2340)==1);assert(timer_writes==4);
 fke_state protected={0};timer(.2f);
 assert(!fke_step(&protected,1,2,201,0,1,2350,(np_address)selector,base,read_all,timer_write));
 assert(!fke_step(&protected,1,2,100,0,1,2390,(np_address)selector,base,read_all,timer_write));
 assert(!fke_step(&protected,1,2,100,0,1,2430,(np_address)selector,base,read_all,timer_write));
 assert(timer_writes==4);
 /* Releasing protected setup lets the pending landing request proceed. */
 fke_state release_setup=protected;
 assert(fke_step(&release_setup,1,2,100,1,0,2470,(np_address)selector,base,read_all,timer_write)==1);
 assert(timer_writes==5);timer_writes=4;timer(.2f);
 fke_state ordinary={0};timer(.2f);assert(step(&ordinary,2,100,0,2400)==0);assert(step(&ordinary,2,100,0,2500)==0);
 fke_state expired={0};assert(step(&expired,2,201,0,2600)==0);assert(step(&expired,2,101,0,2700)==0);assert(step(&expired,2,100,0,4000)==0);
 /* Disabled, wrong owner, conflicting exports, unbound fields, wrong types,
  * renamed descriptors and invalid floats cannot write gameplay data. */
 assert(!fke_step(&held,0,2,100,1,0,4100,(np_address)selector,base,read_all,timer_write));
 ptr(core,0x440,(np_address)root);assert(!fke_read((np_address)selector,base,read_all,&sample));ptr(core,0x440,(np_address)selector);
 state[0xa3]=0;assert(!fke_read((np_address)selector,base,read_all,&sample));state[0xa3]=1;
 timer_node[0x78]='X';assert(!fke_read((np_address)selector,base,read_all,&sample));timer_node[0x78]='A';
 ptr(timer_node,0x20,base+0x61ab030);assert(!fke_read((np_address)selector,base,read_all,&sample));ptr(timer_node,0x20,base+0x61ab550);
 bits=1u<<8;memcpy(page+0x71c,&bits,4);assert(!fke_read((np_address)selector,base,read_all,&sample));bits|=1u<<16;memcpy(page+0x71c,&bits,4);
 timer(__builtin_nanf(""));assert(!fke_read((np_address)selector,base,read_all,&sample));timer(-1);assert(!fke_read((np_address)selector,base,read_all,&sample));timer(.1f);
 deny_write=1;fke_state failure={0};assert(step(&failure,1,100,1,4200)==0);assert(step(&failure,1,100,1,4220)==-1);assert(failure.writes==0&&timer_writes==4);
 puts("PASS: named owned fakie timer only, 20 ms neutral confirmation, active/air/slide protection, optional landing window, bounded retries, unchanged source flags, stale/type/name/occupancy/float failures rejected");return 0;
}
