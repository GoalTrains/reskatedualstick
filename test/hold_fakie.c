#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/hold_fakie_keys.h"
static unsigned char mem[1024];
static int read_key(np_address a,void*out,np_address n){if(a<(np_address)mem || a+n>(np_address)mem+sizeof mem)return 0;memcpy(out,(void*)a,n);return 1;}
int main(void){
 assert(hf_key_id("egf_holdfakiestance")==1 && hf_key_id("egf_holdfakiepush")==2);
 const char* other[]={"DefaultStanceIsGoofy","egf_holdfakiestance_extra","egf_holdfakiepus","EGF_holdfakiepush",""};
 for(unsigned i=0;i<sizeof other/sizeof *other;i++)assert(!hf_key_id(other[i]));
 np_address asset=(np_address)mem+64,name=(np_address)mem+512,tagged=asset|4;memcpy(mem,&tagged,8);memcpy(mem+64+0x60,&name,8);strcpy((char*)mem+512,"egf_holdfakiepush");
 assert(hf_reference((np_address)mem,read_key)==2 && !hf_reference(0,read_key));
 memset(mem+512,'x',32);assert(!hf_reference((np_address)mem,read_key));
 for(int enabled=0;enabled<2;enabled++)for(int option=0;option<2;option++)for(int original=0;original<2;original++)for(int key=0;key<4;key++){
  int expected=enabled && option && (key==1||key==2)?0:original;
  assert(hf_bool_value(key,enabled,option,original)==expected);
 }
 puts("PASS: exact hold-fakie setting names, bounded tagged-asset lookup, unrelated/invalid names untouched, reversible option/mod-off pass-through");
}