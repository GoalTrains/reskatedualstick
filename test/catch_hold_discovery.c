#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef unsigned long long UINT64;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef struct {void* base;UINT64 size;unsigned state,prot;} SC_MBI;
#define PAGE_READWRITE 4
static UINT64 clock_ms,owners[0xbbf];
static unsigned calls,slow_ms,seen[0xbbf][6],reads;
static int unavailable;
static UINT64 GetTickCount64(void){return clock_ms;}
static int VirtualQuery(const void*a,SC_MBI*b,unsigned n){(void)a;(void)b;(void)n;return 0;}
static void* GetCurrentProcess(void){return 0;}
static int WriteProcessMemory(void*p,void*a,const void*b,UINT64 n,UINT64*d){(void)p;(void)a;(void)b;(void)n;(void)d;assert(0);return 0;}
static UINT64 np_base(void){return 0x140000000ULL;}
static void log_num(const char*s,int n){(void)s;(void)n;}
static int rd_copy(UINT64 a,void*p,UINT64 n){
 if(a==np_base()+0x7726d10 && n==sizeof owners){reads++;if(unavailable)return 0;memcpy(p,owners,sizeof owners);return 1;}
 return 0;
}
#include "../src/catch_hold_native.h"
static UINT64 lookup(WORD domain,const char*name){
 static const char* suffix[]={"fliptricks","fliptricks_fromgrind","underflips","simple_fliptricks","hybrid_simple_fliptricks","hybrid_simple_fliptricks_fromgrind"};
 unsigned i;assert(domain<0xbbf && owners[domain]);
 const char*p=name+strlen("gameplay/input/gestures/gestureset_");
 for(i=0;i<6;i++)if(!strcmp(p,suffix[i]))break;
 assert(i<6);seen[domain][i]++;calls++;clock_ms+=slow_ms;return 0;
}
static void reset(void){
 memset(owners,0,sizeof owners);memset(seen,0,sizeof seen);memset(cha_owners,0,sizeof cha_owners);
 cha_next=cha_owner_hash=cha_rescan=0;cha_domain=cha_name_index=cha_scanning=0;
 calls=slow_ms=reads=unavailable=0;clock_ms=100;
}
int main(void){
 /* A fully occupied table must still yield after six native calls per poll. */
 reset();for(unsigned i=0;i<0xbbf;i++)owners[i]=0x10000+i*8;
 unsigned polls=0;
 do{unsigned before=calls;cha_discover(np_base(),clock_ms,lookup);assert(calls-before<=6);polls++;}while(cha_scanning);
 assert(calls==0xbbf*6 && polls>=0xbbf);
 for(unsigned d=0;d<0xbbf;d++)for(unsigned n=0;n<6;n++)assert(seen[d][n]==1);
 unsigned before=calls;clock_ms=2100;cha_discover(np_base(),clock_ms,lookup);assert(calls==before);
 clock_ms=30100;cha_discover(np_base(),clock_ms,lookup);assert(calls==before+6);
 /* A slow lookup yields after one call, and sparse domains never get queried. */
 reset();owners[0]=0x10000;owners[0xbbe]=0x20000;slow_ms=3;
 do{unsigned previous=calls;cha_discover(np_base(),clock_ms,lookup);assert(calls-previous<=1);}while(cha_scanning);
 assert(calls==12);for(unsigned n=0;n<6;n++)assert(seen[0][n]==1 && seen[0xbbe][n]==1);
 /* Owner changes restart discovery before the periodic 30-second rescan. */
 owners[4]=0x30000;clock_ms=2100;before=calls;cha_discover(np_base(),clock_ms,lookup);assert(calls==before+1 && cha_scanning);
 /* No native lookup is attempted from an unreadable initial owner table. */
 reset();unavailable=1;cha_discover(np_base(),clock_ms,lookup);assert(!calls && !cha_scanning && reads==1);
 reset();cha_discover(np_base(),clock_ms,lookup);assert(!calls && !cha_scanning);
 puts("PASS: native discovery budgets dense/sparse domain tables, yields on slow lookups, visits all six sets, throttles rescans, refreshes changed owners and rejects unreadable tables");
}
