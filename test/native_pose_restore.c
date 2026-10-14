#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/native_pose_restore.h"
static unsigned char selector[64],root[256],holder[64],core[2048],context[6144],source[256],state[256];
static unsigned char* blocks[]={selector,root,holder,core,context,source,state};
static unsigned sizes[]={sizeof selector,sizeof root,sizeof holder,sizeof core,sizeof context,sizeof source,sizeof state};
static unsigned writes;
static void ptr(unsigned char* p,unsigned at,np_address address){memcpy(p+at,&address,8);}
static int read_mem(np_address a,void* out,np_address n){for(int i=0;i<7;i++)if(a>=(np_address)blocks[i] && a+n<=(np_address)blocks[i]+sizes[i]){memcpy(out,(void*)a,n);return 1;}return 0;}
static int write_mem(np_address a,const void* data,np_address n){assert(a==(np_address)source+0x12 && n==2);memcpy((void*)a,data,n);writes++;return 1;}
static void flags(int pair,int sw){source[0x12]=pair;source[0x13]=!pair;source[0x14]=sw;state[0xA3]=sw;state[0xA4]=pair;state[0xA5]=!pair;unsigned bits=0x12340000|(sw?2048:0);memcpy(context+0x13dc,&bits,4);}
int main(void){
 np_address base=0x140000000ULL;
 ptr(selector,0,(np_address)root);ptr(selector,8,(np_address)context);ptr(selector,16,(np_address)holder);
 ptr(holder,16,(np_address)core);ptr(core,0,base+0x6176d30);ptr(core,0x440,(np_address)selector);
 ptr(core,0x3b8,(np_address)root);ptr(core,0x3c0,(np_address)context);ptr(core,0x3c8,(np_address)source);ptr(root,0x60,(np_address)state);
 for(int pair=0;pair<2;pair++)for(int sw=0;sw<2;sw++){
  np_pose saved;flags(pair,sw);assert(np_pose_read((np_address)selector,base,read_mem,&saved));
  assert(np_pose_restore(&saved,(np_address)selector,base,read_mem,write_mem)==0);
  flags(!pair,sw);unsigned char original[256];memcpy(original,source,256);
  unsigned old=writes;assert(np_pose_restore(&saved,(np_address)selector,base,read_mem,write_mem)==1 && writes==old+1);
  assert(source[0x12]==pair && source[0x13]==!pair && source[0x14]==sw);
  original[0x12]=pair;original[0x13]=!pair;assert(!memcmp(original,source,256));
  /* Transient source/export mismatch must defer further writes. */
  assert(np_pose_restore(&saved,(np_address)selector,base,read_mem,write_mem)==-1);
 }
 flags(1,0);np_pose saved;assert(np_pose_read((np_address)selector,base,read_mem,&saved));
 state[0xA3]=1;assert(!np_pose_read((np_address)selector,base,read_mem,&saved));state[0xA3]=0;
 source[0x13]=1;assert(!np_pose_read((np_address)selector,base,read_mem,&saved));flags(1,0);
 ptr(core,0x440,(np_address)root);assert(!np_pose_read((np_address)selector,base,read_mem,&saved));ptr(core,0x440,(np_address)selector);
 ptr(core,0,base+0x1000);assert(!np_pose_read((np_address)selector,base,read_mem,&saved));ptr(core,0,base+0x6176d30);
 assert(!np_pose_read(0,base,read_mem,&saved));assert(!np_pose_read((np_address)selector,0,read_mem,&saved));
 flags(1,0);assert(np_pose_read((np_address)selector,base,read_mem,&saved));flags(0,0);
 saved.source=(np_address)state;assert(np_pose_restore(&saved,(np_address)selector,base,read_mem,write_mem)==-1);
 puts("PASS: native owner/type/context checks, complementary pose pair, source/export agreement, both pair directions and switch frames, exactly two source bytes restored, switch and surrounding state untouched, invalid/stale/mismatched state rejected");
 return 0;
}
