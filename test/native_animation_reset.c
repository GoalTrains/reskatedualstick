#define main pose_fixture_main
#include "native_pose_restore.c"
#undef main
#include "../src/native_animation_reset.h"
static unsigned char provider[256],graph[256],model[2048],nodes[3][256],pages[2][2048];
static int read_all(np_address a,void* out,np_address n){
 if(read_mem(a,out,n))return 1;
 unsigned char* b[]={provider,graph,model,nodes[0],nodes[1],nodes[2],pages[0],pages[1]};
 unsigned size[]={256,256,2048,256,256,256,2048,2048};
 for(int i=0;i<8;i++)if(a>=(np_address)b[i] && a+n<=(np_address)b[i]+size[i]){memcpy(out,(void*)a,n);return 1;}
 return 0;
}
static int write_value(np_address a,const void* b,np_address n){assert(a==(np_address)pages[0]+602 && n==1);memcpy((void*)a,b,n);writes++;return 1;}
static void frame(int mirror,int native){flags(mirror,native);source[0x10]=!mirror;unsigned bits=mirror?0:0x2000;memcpy(context+0x13c4,&bits,4);pages[0][602]=mirror;}
static int write_reset(np_address a,const void* b,np_address n){assert(a==(np_address)pages[0]+533 && n==1);memcpy((void*)a,b,n);writes++;return 1;}
int main(void){
 pose_fixture_main();np_address base=0x140000000ULL;
 ptr(core,0x630,(np_address)provider);ptr(provider,0x28,(np_address)graph|4);ptr(core,0x610,(np_address)model);
 unsigned offsets[]={0x60,0x80,0x68},keys[]={0x01259001,0x0125a001,0x010a3002};
 for(int i=0;i<3;i++){ptr(graph,offsets[i],(np_address)nodes[i]|4);ptr(nodes[i],0x20,base+0x61ab030);ptr(nodes[i],0x60,base+0x61ab080);memcpy(nodes[i]+0x28,&keys[i],4);}
 strcpy((char*)nodes[1]+0x88,"Animation/Dingo/Bool.Anim.Mirrored");
 ptr(model,0x48+0x18+8,(np_address)pages[0]);ptr(model,0x48+0x18+16,(np_address)pages[1]);
 unsigned bits=(1u<<25)|(1u<<26);memcpy(pages[0]+0x71c+18*4,&bits,4);bits=1u<<3;memcpy(pages[1]+0x71c+5*4,&bits,4);
 pages[0][601]=1;pages[1][163]=0;frame(1,1);

 ptr(provider,0x38,(np_address)graph);unsigned reset_key=0x01215001;memcpy(nodes[0]+0x28,&reset_key,4);
 strcpy((char*)nodes[0]+0x88,"Animation/Dingo/Bool.Skater.ResetAnimation");bits=1u<<21;memcpy(pages[0]+0x71c+16*4,&bits,4);pages[0][533]=0;
 nar_pose saved;assert(nar_read((np_address)selector,base,read_all,&saved));
 frame(0,0);unsigned char src_before[256],pg_before[2048];memcpy(src_before,source,256);memcpy(pg_before,pages[0],2048);unsigned before=writes;
 assert(nar_request(&saved,(np_address)selector,base,read_all,write_reset)==1 && writes==before+1 && pages[0][533]==1);
 assert(!memcmp(src_before,source,256));pg_before[533]=1;assert(!memcmp(pg_before,pages[0],2048));
 assert(nar_request(&saved,(np_address)selector,base,read_all,write_reset)==-2 && writes==before+1);
 /* A transient exported flag mismatch must not prevent releasing the request. */
 state[0xa4]=1;assert(nar_release(&saved,base,read_all,write_reset)==1 && pages[0][533]==0);state[0xa4]=0;
 assert(nar_release(&saved,base,read_all,write_reset)==0);
 ptr(provider,0x38,(np_address)model);assert(nar_release(&saved,base,read_all,write_reset)<0);ptr(provider,0x38,(np_address)graph);
 nodes[0][0x88]='X';assert(!nar_read((np_address)selector,base,read_all,&saved));
 assert(writes==before+2);
 puts("PASS: named native reset property only, occupied same-owner page/type, one request and release, no board/mirror/export mutation, active-frame disagreement release, duplicate request and changed owner/name rejected");
}
