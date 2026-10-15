#define main pose_fixture_main
#include "native_pose_restore.c"
#undef main
typedef unsigned long long UINT64;typedef unsigned char BYTE;typedef unsigned long DWORD;
#define rd_copy read_mem
#include "../src/stance_flight_read.h"
#undef rd_copy
int main(void){
 pose_fixture_main();const UINT64 base=0x140000000ULL;int v[49];for(int i=0;i<49;i++)v[i]=-1;
 float position[3]={42,10,-9},forward[3]={0,0,1};memcpy(core+0x100,position,12);memcpy(core+0x150,forward,12);
 flags(1,0);source[0x10]=1;unsigned bits=0;memcpy(context+0x13c4,&bits,4);state[0xa3]=1;state[0xa4]=0;
 unsigned before=writes;BYTE snapshot[256];memcpy(snapshot,source,256);
 assert(sf_native_read(base,(UINT64)selector,v)==4095);
 assert(v[10]==1 && v[11]==0 && v[12]==1 && v[13]==1 && v[14]==0 && v[15]==0);
 assert(v[17]==42000 && v[19]==-9000 && v[22]==1000);
 assert(writes==before && !memcmp(snapshot,source,256));
 assert(sf_native_read(base,0,v)==0 && sf_native_read(0,(UINT64)selector,v)==0);
 ptr(core,0x440,(UINT64)root);assert(sf_native_read(base,(UINT64)selector,v)==0);ptr(core,0x440,(UINT64)selector);
 position[0]=__builtin_nanf("");memcpy(core+0x100,position,12);v[17]=-1;
 assert(sf_native_read(base,(UINT64)selector,v)==(4095&~64) && v[17]==-1);
 puts("PASS: stance diagnostic preserves disagreeing native/export/basis values, rejects wrong ownership and invalid coordinates, never writes game state");
}