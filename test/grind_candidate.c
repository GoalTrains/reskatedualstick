#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/translate.h"
typedef unsigned long long np_address;
typedef int (*np_read_fn)(np_address,void*,np_address);
#include "../src/grind_candidate.h"
static unsigned char object[0xb20];
static int read_fixture(np_address a,void*out,np_address n){
 np_address b=(np_address)object;
 if(a<b || a+n<a || a+n>b+sizeof object)return 0;
 memcpy(out,(void*)a,n);return 1;
}
static void f(unsigned off,float v){memcpy(object+off,&v,4);}
static void i(unsigned off,int v){memcpy(object+off,&v,4);}
int main(void){
 np_address b=(np_address)object;
 /* Dev8 at 52.632 s: valid tail candidate at an 85.699-degree angle. */
 i(2*20+16,4);f(2*20,90);f(2*20+4,40);f(2*20+8,.239544f);
 f(0xaec,1.42f);f(0x9b0,2.553f);f(0x9c4,.225f);f(0x9d4,85.699f);
 assert(sc_candidate_accepts(b,2,read_fixture));
 f(0x9c4,.239544f);assert(sc_candidate_accepts(b,2,read_fixture));
 f(0x9c4,.24f);assert(!sc_candidate_accepts(b,2,read_fixture));f(0x9c4,.225f);
 f(0x9d4,50);assert(sc_candidate_accepts(b,2,read_fixture));
 f(0x9d4,-50);assert(sc_candidate_accepts(b,2,read_fixture));
 f(0x9d4,49.9f);assert(!sc_candidate_accepts(b,2,read_fixture));f(0x9d4,85.699f);
 f(0xaec,5);assert(!sc_candidate_accepts(b,2,read_fixture));f(0xaec,1.42f);
 i(2*20+16,-1);assert(!sc_candidate_accepts(b,2,read_fixture));
 i(2*20+16,5);assert(!sc_candidate_accepts(b,2,read_fixture));i(2*20+16,4);
 f(0x9b0,0);assert(!sc_candidate_accepts(b,2,read_fixture));
 f(0x9b0,65.5f);assert(!sc_candidate_accepts(b,2,read_fixture));f(0x9b0,2.553f);
 i(0x9d4,0x7fc00000);assert(!sc_candidate_accepts(b,2,read_fixture));
 assert(!sc_candidate_accepts(b,7,read_fixture));assert(!sc_candidate_accepts(0,2,read_fixture));
 puts("PASS: captured tail-candidate operands, exact native score/angle boundaries, invalid types/indices/distances, nonfinite and failed reads");
}
