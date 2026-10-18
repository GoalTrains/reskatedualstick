#define SC_CATCH_FIXTURE_ONLY
#include "catch_read.c"
typedef unsigned long long UINT64;
static int g_catch_diag,g_local,g_native_menu,g_menu,g_paused,g_walking,g_ui_open;
static struct {UINT64 sel;int state;}g_slots[16];
static unsigned reads;
static UINT64 np_base(void){return 0x140000000ULL;}
static int rd_copy(np_address a,void*p,np_address n){reads++;return cp_read(a,p,n);}
#include "../src/catch_diag.h"
int main(void){
 catch_fixture_run();ptr(core,0x440,(np_address)selector);unsigned bits=(1u<<26)|(1u<<27);memcpy(cp_page+0x71c+18*4,&bits,4);g_slots[0].sel=(UINT64)selector;
 sc_catch_poll(1000);assert(!reads && !g_catch_sample.valid);
 g_catch_diag=1;sc_catch_poll(1000);assert(g_catch_sample.valid==3);unsigned before=reads;
 sc_catch_poll(1001);sc_catch_poll(1019);assert(reads==before);
 sc_catch_poll(1020);assert(reads>before);
 before=reads;sc_catch_poll_force(1021,1);assert(reads>before && g_catch_sample_time==1021);
 before=reads;g_slots[0].state=200;sc_catch_poll(1022);assert(reads>before && g_catch_sample_state==200);
 before=reads;g_ui_open=1;sc_catch_poll_force(1023,1);assert(reads==before && !g_catch_sample.valid);g_ui_open=0;
 g_walking=1;g_slots[0].state=504;sc_catch_poll(1024);assert(g_catch_sample.valid==3 && g_catch_sample_state==504);g_walking=0;
 g_native_menu=1;sc_catch_poll(1021);assert(!g_catch_sample.valid && g_catch_sample.can_land==-1);before=reads;sc_catch_poll(1100);assert(reads==before);
 g_native_menu=0;g_local=16;sc_catch_poll(1101);assert(reads==before);
 g_local=0;g_catch_diag=0;sc_catch_poll(1200);assert(reads==before);assert(!writes);
 puts("PASS: catch recorder disabled/UI/invalid-slot paths perform no native reads, 20ms sampling limit with fresh button/state-edge samples, bail-state sampling, cached data invalidated, no writes");return 0;
}
