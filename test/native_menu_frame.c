#define main pose_fixture_main
#include "native_pose_restore.c"
#undef main
#include "../src/native_menu_frame.h"
static void basis(int b){source[0x10]=b;unsigned bits=b?0x2000:0;memcpy(context+0x13c4,&bits,4);}
static int write_frame(np_address a,const void* data,np_address n){assert(a==(np_address)source+0x10 && n==4);memcpy((void*)a,data,n);writes++;return 1;}
static int reject_write(np_address a,const void* data,np_address n){(void)a;(void)data;(void)n;return 0;}
int main(void){
 pose_fixture_main();np_address sel=(np_address)selector,base=0x140000000ULL;np_frame saved;
 flags(1,1);basis(0);source[0x11]=0x7f;assert(np_frame_read(sel,base,read_mem,&saved));
 assert(np_frame_recover(&saved,sel,base,read_mem,write_frame)==0);
 flags(0,0);basis(1);unsigned char before[256];memcpy(before,source,256);
 assert(np_frame_recover(&saved,sel,base,read_mem,reject_write)==-2 && !memcmp(before,source,256));
 unsigned old=writes;assert(np_frame_recover(&saved,sel,base,read_mem,write_frame)==1 && writes==old+1);
 before[0x10]=0;before[0x12]=1;before[0x13]=0;
 assert(!memcmp(before,source,256)); /* +11, switch +14 and every other byte unchanged. */
 /* Recorded menu signatures: context/export mismatch defers; unexpected
  * frame transitions or active nose setup do not cause writes. */
 flags(0,1);basis(1);assert(np_frame_recover(&saved,sel,base,read_mem,write_frame)==-2);
 flags(1,0);basis(1);assert(np_frame_recover(&saved,sel,base,read_mem,write_frame)==-2);
 flags(0,0);basis(1);source[0x10]=0;assert(np_frame_recover(&saved,sel,base,read_mem,write_frame)==-1);
 basis(1);saved.native_frame=0;assert(np_frame_recover(&saved,sel,base,read_mem,write_frame)==-2);
 saved.native_frame=1;saved.basis=1;assert(np_frame_recover(&saved,sel,base,read_mem,write_frame)==-2);
 assert(writes==old+1);
 puts("PASS: exact recorded reversed-frame signature; original frame restored, unrelated byte and native switch preserved; failed write, invalid exports, legitimate frame/foot-position changes rejected");
 return 0;
}
