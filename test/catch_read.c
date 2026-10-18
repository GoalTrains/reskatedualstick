#define main pose_fixture_main
#include "native_pose_restore.c"
#undef main
#include "../src/catch_read.h"
static unsigned char cp_provider[256],cp_graph[1280],cp_output[1280],cp_model[2048],cp_nodes[10][256],cp_page[2048];
static int cp_read(np_address a,void*out,np_address n){
 if(read_mem(a,out,n))return 1;
 unsigned char*b[]={cp_provider,cp_graph,cp_output,cp_model,(unsigned char*)cp_nodes,cp_page};
 unsigned sizes[]={sizeof cp_provider,sizeof cp_graph,sizeof cp_output,sizeof cp_model,sizeof cp_nodes,sizeof cp_page};
 for(int i=0;i<6;i++)if(a>=(np_address)b[i] && a+n<=(np_address)b[i]+sizes[i]){memcpy(out,(void*)a,n);return 1;}return 0;
}
static int catch_fixture_run(void){
 np_address base=0x140000000ULL;
 ptr(selector,0,(np_address)root);ptr(selector,8,(np_address)context);ptr(selector,16,(np_address)holder);ptr(holder,16,(np_address)core);
 ptr(core,0,base+0x6176d30);ptr(core,0x440,(np_address)selector);ptr(core,0x3b8,(np_address)root);ptr(core,0x3c0,(np_address)context);ptr(core,0x3c8,(np_address)source);ptr(root,0x60,(np_address)state);flags(1,0);
 ptr(core,0x630,(np_address)cp_provider);ptr(core,0x610,(np_address)cp_model);ptr(cp_provider,0x28,(np_address)cp_graph|4);ptr(cp_provider,0x38,(np_address)cp_output|4);
 ptr(cp_model,0x48+0x18+8,(np_address)cp_page);
 const char*names[]={"Animation/Dingo/Bool.Anim.CanLandTrick","Animation/Dingo/Bool.Anim.DoingTrick",
 "Animation/Dingo/Bool.GP.IsDark","Animation/Dingo/Bool.Physics.IsBoardUpright","Animation/Dingo/Bool.Physics.Landing",
 "Animation/Dingo/Bool.Physics.LandingOnSkateboard","Animation/Dingo/Bool.Physics.LandingOnEdge",
 "Animation/Dingo/Bool.Physics.PrelandAngleAllowed","Animation/Dingo/Bool.Physics.OffBoard.IsWipingOut"};
 unsigned offsets[]={0x1f0,0x278,0x408,0x190,0x318,0x320,0x328,0x350,0x3d8};
 for(int i=0;i<9;i++){
  ptr(i<3?cp_graph:cp_output,offsets[i],(np_address)cp_nodes[i]|4);ptr(cp_nodes[i],0x20,base+0x61ab030);ptr(cp_nodes[i],0x60,base+0x61ab080);
  unsigned key=0x01000001|((602+i)<<12);memcpy(cp_nodes[i]+0x28,&key,4);memcpy(cp_nodes[i]+0x88,names[i],strlen(names[i])+1);
 }
 unsigned bits=0xfc000000;memcpy(cp_page+0x71c+18*4,&bits,4);bits=7;memcpy(cp_page+0x71c+19*4,&bits,4);
 for(int land=0;land<2;land++)for(int trick=0;trick<2;trick++){
  cp_page[602]=land;cp_page[603]=trick;sc_catch_sample v=sc_catch_read((np_address)selector,base,cp_read);assert(v.valid==511 && v.can_land==land && v.doing_trick==trick);
 }
 /* Independently validate every dark/upright/contact/bail flag and unknown. */
 for(int i=2;i<9;i++)for(int value=0;value<2;value++){
  cp_page[602+i]=value;sc_catch_sample v=sc_catch_read((np_address)selector,base,cp_read);
  int values[]={v.can_land,v.doing_trick,v.dark,v.upright,v.landing,v.on_board,v.on_edge,v.preland_allowed,v.wiping_out};
  assert(v.valid==511 && values[i]==value);
  cp_nodes[i][0x88]='X';v=sc_catch_read((np_address)selector,base,cp_read);
  assert(v.valid==(511u & ~(1u<<i)));cp_nodes[i][0x88]='A';
 }
 /* Typed integer enum: no calls, no default fallback, and no bool coercion. */
 ptr(cp_graph,0x1e8,(np_address)cp_nodes[9]|4);ptr(cp_nodes[9],0x20,base+0x61926d8);ptr(cp_nodes[9],0x60,base+0x6192728);
 unsigned enum_key=0x01000001|(640<<12),enum_bits=1;memcpy(cp_nodes[9]+0x28,&enum_key,4);memcpy(cp_page+0x71c+20*4,&enum_bits,4);
 strcpy((char*)cp_nodes[9]+0x88,"Animation/Dingo/EnumGS.Anim.CurrentFlipTrick");
 for(int trick=0;trick<=40;trick++){
  memcpy(cp_page+640,&trick,4);sc_catch_sample v=sc_catch_read((np_address)selector,base,cp_read);assert(v.valid==1023 && v.trick_id==trick);
 }
 cp_nodes[9][0x88]='X';assert(sc_catch_read((np_address)selector,base,cp_read).trick_id==-1);cp_nodes[9][0x88]='A';
 enum_bits=0;memcpy(cp_page+0x71c+20*4,&enum_bits,4);assert(sc_catch_read((np_address)selector,base,cp_read).trick_id==-1);
 ptr(cp_graph,0x1e8,0);
 cp_nodes[0][0x88]='X';sc_catch_sample v=sc_catch_read((np_address)selector,base,cp_read);assert(v.valid==510&&v.can_land==-1);cp_nodes[0][0x88]='A';
 cp_page[602]=2;v=sc_catch_read((np_address)selector,base,cp_read);assert(v.valid==510&&v.can_land==-1);cp_page[602]=0;
 bits=0;memcpy(cp_page+0x71c+18*4,&bits,4);memcpy(cp_page+0x71c+19*4,&bits,4);v=sc_catch_read((np_address)selector,base,cp_read);assert(!v.valid&&v.can_land==-1&&v.doing_trick==-1);
 ptr(core,0x440,(np_address)root);assert(!sc_catch_read((np_address)selector,base,cp_read).valid);
 assert(!sc_catch_read(0,base,cp_read).valid);assert(!sc_catch_read((np_address)selector,0,cp_read).valid);assert(!writes);
 puts("PASS: read-only catch research flags, nine named flags including dark/upright/edge/landing/wipeout, both boolean values, local owner/type/name/occupied-page validation, unknown distinct from false, invalid/missing state rejected; zero writes");return 0;
}
#ifndef SC_CATCH_FIXTURE_ONLY
int main(void){return catch_fixture_run();}
#endif
