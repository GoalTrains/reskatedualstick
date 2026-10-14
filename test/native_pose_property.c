#define main pose_fixture_main
#include "native_pose_restore.c"
#undef main
#include "../src/native_pose_property.h"
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
int main(void){
 pose_fixture_main();np_address base=0x140000000ULL;
 ptr(core,0x630,(np_address)provider);ptr(provider,0x28,(np_address)graph|4);ptr(core,0x610,(np_address)model);
 unsigned offsets[]={0x60,0x80,0x68},keys[]={0x01259001,0x0125a001,0x010a3002};
 for(int i=0;i<3;i++){ptr(graph,offsets[i],(np_address)nodes[i]|4);ptr(nodes[i],0x20,base+0x61ab030);ptr(nodes[i],0x60,base+0x61ab080);memcpy(nodes[i]+0x28,&keys[i],4);}
 strcpy((char*)nodes[1]+0x88,"Animation/Dingo/Bool.Anim.Mirrored");
 ptr(model,0x48+0x18+8,(np_address)pages[0]);ptr(model,0x48+0x18+16,(np_address)pages[1]);
 unsigned bits=(1u<<25)|(1u<<26);memcpy(pages[0]+0x71c+18*4,&bits,4);bits=1u<<3;memcpy(pages[1]+0x71c+5*4,&bits,4);
 pages[0][601]=1;pages[1][163]=0;frame(1,1);
 npp_pose saved;assert(npp_read((np_address)selector,base,read_all,&saved));
 assert(npp_restore(&saved,(np_address)selector,base,read_all,write_value)==0);
 frame(0,0);unsigned char old_source[256],old_page[2048];memcpy(old_source,source,256);memcpy(old_page,pages[0],2048);
 unsigned before=writes;assert(npp_restore(&saved,(np_address)selector,base,read_all,write_value)==1 && writes==before+1);
 assert(!memcmp(old_source,source,256));old_page[602]=1;assert(!memcmp(old_page,pages[0],2048));
 /* Until the native producer refreshes exports, refuse a second write. */
 assert(npp_restore(&saved,(np_address)selector,base,read_all,write_value)==-1);
 frame(1,0);assert(npp_restore(&saved,(np_address)selector,base,read_all,write_value)==0);
 frame(0,0);pages[0][601]=0;assert(npp_restore(&saved,(np_address)selector,base,read_all,write_value)<0);pages[0][601]=1;
 pages[1][163]=1;assert(npp_restore(&saved,(np_address)selector,base,read_all,write_value)<0);pages[1][163]=0;
 pages[0][602]=2;assert(!npp_read((np_address)selector,base,read_all,&saved));pages[0][602]=0;
 nodes[1][0x88]='X';assert(!npp_read((np_address)selector,base,read_all,&saved));nodes[1][0x88]='A';
 bits=1u<<25;memcpy(pages[0]+0x71c+18*4,&bits,4);assert(!npp_read((np_address)selector,base,read_all,&saved));
 assert(writes==before+1);
 puts("PASS: actual occupied animation property byte, tagged nodes/type/name/owner validation, native exports unchanged, nonboolean/missing occupancy/goofy/baseline changes rejected; no repeated write");
}
