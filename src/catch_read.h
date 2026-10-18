/* Read-only evidence for a future manual catch. CanLandTrick is permission,
 * not proof that the board was caught. Never write animation state here. */
#ifndef SC_CATCH_READ_H
#define SC_CATCH_READ_H
#include "native_pose_property.h"
/* Named flags are observations, not a measured board angle or proof of a
 * successful catch. Bits validate each field independently; -1 is unknown. */
typedef struct {unsigned valid;int can_land,doing_trick,dark,upright,landing,
    on_board,on_edge,preland_allowed,wiping_out,trick_id;} sc_catch_sample;
#define SC_CATCH_UNKNOWN {0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
/* Enum getter 1100420 -> 11351f0 -> 11dcd40 reads an occupied per-rider
 * four-byte value at the encoded BYTE offset. Read only that local value;
 * do not invoke native functions or treat a descriptor default as live. */
static int sc_catch_trick(np_address graph,np_address model,np_address base,np_read_fn read,int* out){
 np_address node,vt,page;unsigned key,bits;int value;
 if(!read(graph+0x1e8,&node,8))return 0;node&=~4ULL;
 if(node<0x10000 || (node&7) || !read(node+0x20,&vt,8) || vt!=base+0x61926d8 ||
    !read(node+0x60,&vt,8) || vt!=base+0x6192728 ||
    !read(node+0x28,&key,4) || !(key>>24) ||
    !np_pointer(read,model+0x48+0x18+(np_address)(key&0xfff)*8,&page))return 0;
 unsigned index=(key>>12)&0x7ff;
 if((index&3) || index>0x718 || !read(page+0x71c+(index>>5)*4,&bits,4) || !(bits&(1u<<(index&31))) ||
    !read(page+index,&value,4) || value<0 || value>4095)return 0;
 static const char expected[]="Animation/Dingo/EnumGS.Anim.CurrentFlipTrick";
 char name[sizeof expected];if(!read(node+0x88,name,sizeof name))return 0;
 for(unsigned i=0;i<sizeof name;i++)if(name[i]!=expected[i])return 0;
 *out=value;return 1;
}
static int sc_catch_bool(np_address graph,unsigned offset,np_address model,np_address base,
                        np_read_fn read,const char* expected,unsigned char* out){
 np_address node,page,value;unsigned key;
 if(!npp_value(graph,offset,model,base,read,&node,&page,&value,&key,out))return 0;
 char name[96];unsigned n=0;while(n<sizeof name && expected[n])n++;
 if(n==sizeof name || !read(node+0x88,name,n+1))return 0;
 for(unsigned i=0;i<=n;i++)if(name[i]!=expected[i])return 0;return 1;
}
static sc_catch_sample sc_catch_read(np_address selector,np_address base,np_read_fn read){
 sc_catch_sample out=SC_CATCH_UNKNOWN;np_pose rider;np_address provider,graph,model,output=0;unsigned char v;
 if(!np_pose_read(selector,base,read,&rider)||!np_pointer(read,rider.core+0x630,&provider)||
    !read(provider+0x28,&graph,8)||!np_pointer(read,rider.core+0x610,&model))return out;
 graph&=~4ULL;if(graph<0x10000 || (graph&7))return out;
 if(sc_catch_bool(graph,0x1f0,model,base,read,"Animation/Dingo/Bool.Anim.CanLandTrick",&v)){out.can_land=v;out.valid|=1;}
 if(sc_catch_bool(graph,0x278,model,base,read,"Animation/Dingo/Bool.Anim.DoingTrick",&v)){out.doing_trick=v;out.valid|=2;}
 if(sc_catch_bool(graph,0x408,model,base,read,"Animation/Dingo/Bool.GP.IsDark",&v)){out.dark=v;out.valid|=4;}
 if(sc_catch_trick(graph,model,base,read,&out.trick_id))out.valid|=512;
 /* Provider +38 is its output graph. Use the same typed per-rider property
  * reader and descriptor-name checks as the input graph; no native calls. */
 if(!read(provider+0x38,&output,8))return out;
 output&=~4ULL;if(output<0x10000 || (output&7))return out;
 if(sc_catch_bool(output,0x190,model,base,read,"Animation/Dingo/Bool.Physics.IsBoardUpright",&v)){out.upright=v;out.valid|=8;}
 if(sc_catch_bool(output,0x318,model,base,read,"Animation/Dingo/Bool.Physics.Landing",&v)){out.landing=v;out.valid|=16;}
 if(sc_catch_bool(output,0x320,model,base,read,"Animation/Dingo/Bool.Physics.LandingOnSkateboard",&v)){out.on_board=v;out.valid|=32;}
 if(sc_catch_bool(output,0x328,model,base,read,"Animation/Dingo/Bool.Physics.LandingOnEdge",&v)){out.on_edge=v;out.valid|=64;}
 if(sc_catch_bool(output,0x350,model,base,read,"Animation/Dingo/Bool.Physics.PrelandAngleAllowed",&v)){out.preland_allowed=v;out.valid|=128;}
 if(sc_catch_bool(output,0x3d8,model,base,read,"Animation/Dingo/Bool.Physics.OffBoard.IsWipingOut",&v)){out.wiping_out=v;out.valid|=256;}
 return out;
}
#endif
