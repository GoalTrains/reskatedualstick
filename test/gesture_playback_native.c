#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../src/native_pose_restore.h"
typedef uint64_t UINT64;typedef uint32_t DWORD;typedef uint16_t WORD;
typedef struct {void*base;UINT64 size;DWORD state,prot;} SC_MBI;
#define PAGE_READWRITE 4
static unsigned char arena[4096];static DWORD error_state;static UINT64 now=1000;
static int g_user_enabled=1,g_gesture_speed_option=1,g_native_menu,g_menu,g_ui_open,g_paused,g_walking,g_local;
static struct {UINT64 sel;DWORD state;} g_slots[16];
static int calls;static UINT64 called_ref,called_bank;
static DWORD GetLastError(void){return error_state;}
static void SetLastError(DWORD e){error_state=e;}
static UINT64 GetTickCount64(void){error_state=999;return now;}
static UINT64 np_base(void){return 0x140000000ULL;}
static int rd_copy(UINT64 a,void*d,UINT64 n){
 if(a<(UINT64)arena || a+n>(UINT64)arena+sizeof arena)return 0;
 memcpy(d,(void*)a,(size_t)n);return 1;
}
static UINT64 VirtualQuery(const void*a,SC_MBI*m,UINT64 n){
 (void)a;(void)n;m->base=arena;m->size=sizeof arena;m->state=0x1000;m->prot=4;return sizeof *m;
}
static void log_text(const char*s){(void)s;}
static int cha_contract(UINT64 b){(void)b;return 0;}
typedef UINT64(*cha_find_fn)(WORD,const char*);
static float original(UINT64 ref,UINT64 bank){calls++;called_ref=ref;called_bank=bank;error_state=77;return .5f;}
#include "../src/gesture_playback_native.h"
static void q(unsigned offset,UINT64 value){memcpy(arena+offset,&value,8);}
int main(void){
 UINT64 asset=(UINT64)(arena+128),bank=(UINT64)(arena+2048);gsp_binding.asset=asset;
 gsp_binding.functions[4]=(UINT64)original;gsp_bank=bank;gsp_factor=.25f;gsp_enabled=1;gsp_installed=1;gsp_publish_at=now;
 assert(gsp_get(asset+0x60,bank)==.125f);assert(calls==1 && called_ref==asset+0x60 && called_bank==bank && error_state==77);
 assert(gsp_local_reads==1 && gsp_last_native==.5f && gsp_last_result==.125f);
 assert(gsp_get(asset+0x60,bank+8)==.5f && calls==2); /* remote/parent bank untouched */
 assert(gsp_get(asset+0x68,bank)==.5f && calls==3); /* another interface untouched */
 now+=101;assert(gsp_get(asset+0x60,bank)==.5f && calls==4);
 gsp_publish_at=now;gsp_factor=1.75f;assert(gsp_get(asset+0x60,bank)==.875f && calls==5);
 int* gates[]={&g_user_enabled,&g_gesture_speed_option,&gsp_enabled};
 for(unsigned i=0;i<3;i++){*gates[i]=0;assert(gsp_get(asset+0x60,bank)==.5f);*gates[i]=1;}
 int* blocks[]={&g_native_menu,&g_menu,&g_ui_open,&g_paused,&g_walking};
 for(unsigned i=0;i<5;i++){*blocks[i]=1;assert(gsp_get(asset+0x60,bank)==.5f);*blocks[i]=0;}
 /* Only the owned pointer can be exchanged, and the original interface returns exactly. */
 q(224,0x12345678);assert(!gsp_swap(asset+0x60,0x9999,(UINT64)(gsp_table+1)));
 assert(gsp_swap(asset+0x60,0x12345678,(UINT64)(gsp_table+1)));
 q(128,np_base()+0x61ab4a8);q(136,np_base()+0x72afbb0);q(152,(UINT64)(arena+256));
 strcpy((char*)arena+256,"Animation/Dingo/Float.Anim.TrickFlipSpeed");gsp_binding.vtable=0x12345678;
 gsp_sync(now,0);UINT64 restored=0;rd_copy(asset+0x60,&restored,8);assert(restored==0x12345678 && !gsp_installed && !gsp_enabled);
 puts("native playback: original once, relative return, error preserved, local interface/bank only, stale/disabled/menu/walking pass-through, foreign pointer rejection and exact restoration passed");return 0;
}
