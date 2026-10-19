#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/camera_side_read.h"
static unsigned char manager[32],items[16],view[0x480],camera[0x190],selector[32],root[16],context[16],holder[32],core[0x450];
static unsigned char* blocks[]={manager,items,view,camera,selector,root,context,holder,core};
static unsigned sizes[]={sizeof manager,sizeof items,sizeof view,sizeof camera,sizeof selector,sizeof root,sizeof context,sizeof holder,sizeof core};
static np_address base=0x140000000ULL,registered;
static int reads;
static int read_mem(np_address a,void* out,np_address n){
 reads++;
 if((a==base+0x75cbc48 || a==base+0x75dd288) && n==8){np_address p=a==base+0x75dd288?registered:(np_address)manager;memcpy(out,&p,8);return 1;}
 for(unsigned i=0;i<sizeof sizes/sizeof sizes[0];i++)if(a>=(np_address)blocks[i] && n<=sizes[i] && a-(np_address)blocks[i]<=sizes[i]-n){memcpy(out,(void*)a,n);return 1;}
 return 0;
}
static void ptr(unsigned char* b,unsigned at,np_address p){memcpy(b+at,&p,8);}
int main(void){
 registered=(np_address)manager;ptr(manager,0,base+0x63ece68);ptr(manager,8,(np_address)items);ptr(manager,16,(np_address)items+8);
 ptr(items,0,(np_address)view);ptr(view,0,base+0x63ed0a0);ptr(view,0x2b0,(np_address)camera);view[0x45d]=1;
 ptr(camera,0,base+0x6255870);ptr(selector,0,(np_address)root);ptr(selector,8,(np_address)context);ptr(selector,16,(np_address)holder);
 ptr(holder,16,(np_address)core);ptr(core,0,base+0x6176d30);ptr(core,0x440,(np_address)selector);ptr(core,0x3b8,(np_address)root);ptr(core,0x3c0,(np_address)context);
 float f[3]={0,0,1},c[3]={1,1,-3};memcpy(core+0x150,f,12);memcpy(camera+0x80,c,12);
 assert(sc_camera_side_read(base,(np_address)selector,read_mem)==1);
 c[0]=-1;memcpy(camera+0x80,c,12);assert(sc_camera_side_read(base,(np_address)selector,read_mem)==-1);
 ptr(camera,0,base+0x63fc220);assert(!sc_camera_side_read(base,(np_address)selector,read_mem));ptr(camera,0,base+0x6255870);
 ptr(core,0x440,0);assert(!sc_camera_side_read(base,(np_address)selector,read_mem));ptr(core,0x440,(np_address)selector);
 registered=0;assert(!sc_camera_side_read(base,(np_address)selector,read_mem));registered=(np_address)manager;
 ptr(manager,16,(np_address)items+9);assert(!sc_camera_side_read(base,(np_address)selector,read_mem));ptr(manager,16,(np_address)items+16);ptr(items,8,(np_address)view);assert(!sc_camera_side_read(base,(np_address)selector,read_mem));
 assert(!sc_camera_side_read(0,(np_address)selector,read_mem));
 puts("PASS: checked camera/rider ownership, left/right observed positions, foreign camera, stale backlinks, registration changes, malformed and ambiguous local views");
}
