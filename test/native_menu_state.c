#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/native_menu_state.h"
static BYTE bytes[16384];static UINT64 storage=0x12200;
static int read_mem(UINT64 at,void* out,UINT64 size){if(at<0x10000 || at+size>0x14000)return 0;memcpy(out,bytes+at-0x10000,size);return 1;}
static void put(UINT64 at,UINT64 value,int n){memcpy(bytes+at-0x10000,&value,n);}
static UINT64 value_at(UINT64 manager,UINT64 root,UINT64 zero,UINT64 typed){assert(manager==0x10100 && root==123 && zero==0 && typed==1);return storage;}
int main(void){
 put(0x10198,0x11000,8);put(0x101a0,0x11028,8);
 put(0x10700,0x11800,8);put(0x10708,0x11848,8);
 put(0x11008,0x12200,8);put(0x11010,123,8);put(0x1101c,0xffffffff,4);
 put(0x11808,0x12100,8);put(0x12100,0x12000,8);put(0x12000,0x12080,8);
 put(0x12080,0x2c5645a1,4);put(0x12084,2<<5,2);put(0x12086,96,2);
 sc_ui_cache c={0};assert(sc_ui_menu_read(0x10100,read_mem,value_at,&c)==0 && c.record==0x11000);
 put(0x12214,1,1);assert(sc_ui_menu_read(0x10100,read_mem,value_at,&c)==1);
 put(0x12214,0,1);assert(sc_ui_menu_read(0x10100,read_mem,value_at,&c)==0);
 put(0x12214,2,1);assert(sc_ui_menu_read(0x10100,read_mem,value_at,&c)==-1 && !c.record);
 put(0x12214,1,1);assert(sc_ui_menu_read(0x10100,read_mem,value_at,&c)==1);
 put(0x11018,2,2);assert(sc_ui_menu_read(0x10100,read_mem,value_at,&c)==-1);
 put(0x11018,0,2);put(0x12080,0x1234,4);assert(sc_ui_menu_read(0x10100,read_mem,value_at,&c)==-1);
 put(0x12080,0x2c5645a1,4);storage=0;assert(sc_ui_menu_read(0x10100,read_mem,value_at,&c)==-1);
 assert(sc_ui_menu_read(0,read_mem,value_at,&c)==-1);
 puts("PASS: native screen model active/closed flags, cached roots, expired/out-of-range types, invalid booleans, unknown schema and missing storage fail safely");
}
