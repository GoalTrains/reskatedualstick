#define PROPERTY_FIXTURE_MAIN property_fixture_main
#include "native_property_poll.c"
static int g_native_respawn=1;
static int g_native_ground_respawn=1,g_game_goofy=0;
static double g_now_ms;
static unsigned char client[256],native_context[512],connection[128],manager[64];
static unsigned char player_manager[2048],local_player[256],entity[2048],collection[128],component[256],handle[16],players[16];
static unsigned board_writes;
static unsigned char replacement_core[2048],replacement_selector[64],replacement_context[256];
static UINT64 native_clock=1000;
static UINT64 mock_tick_time(void){return native_clock;}
static unsigned long last_error;
static unsigned calls,ctor_calls,destroy_calls,original_calls;
static unsigned phase;
static float sent[16];
static int hook_test,hook_race;
static _Alignas(16) BYTE hook_code[0x2e77200],official_module[4096],hook_stub[32];
static UINT64 test_base(void){return hook_test?(UINT64)hook_code:np_base();}
static void* test_module(void* name){return hook_test && name?official_module:GetModuleHandleW(name);}
static void* native_context_call(void* out){assert(original_calls);memcpy(out,&(UINT64){(UINT64)native_context},8);return out;}
static void* ctor_call(void* p){assert(!((UINT64)p&15));assert(phase==0);phase=1;ctor_calls++;return p;}
static void destroy_call(void* p){assert(!((UINT64)p&15));assert(phase==2);phase=0;destroy_calls++;}
static DWORD teleport_call(UINT64 m,const float* matrix,unsigned mode,const void* transition){
 assert(original_calls && m==(UINT64)manager && mode==1 && phase==1);
 assert(component[0xc0]==1);
 assert(!((UINT64)transition&15) && !((UINT64)matrix&15));
 memcpy(sent,matrix,sizeof sent);calls++;phase=2;return 42;
}
static void* native_function(unsigned rva){switch(rva){case 0x18b6ae0:return native_context_call;case 0x5956f80:return ctor_call;case 0x3dc5e90:return destroy_call;case 0x565010:return teleport_call;default:assert(0);return 0;}}
static int respawn_read(UINT64 a,void* out,UINT64 n){
 const UINT64 base=test_base();
 const unsigned rva[]={0x565010,0x5956f80,0x3dc5e90,0x18b6ae0};
 const BYTE code[]={0x48,0x89,0x5c,0x24,0x18,0x56,0x57,0x41,0x54,0x41,0x56,0x41,0x57,0x48,0x81,0xec};
 const BYTE ctor[]={0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8b,0xd9,0x33,0xc9,0xe8,0xc0,0x9e,0xbb,0xfa};
 const BYTE destroy[]={0x48,0x8b,0x01,0x48,0x85,0xc0,0x74,0x1c,0x48,0x8b,0xc8,0x48,0x83,0xe1,0xfb,0x74};
 const BYTE ctx[]={0x48,0x89,0x5c,0x24,0x08,0x57,0x48,0x83,0xec,0x20,0x65,0x48,0x8b,0x04,0x25,0x58};
 const BYTE* contracts[]={code,ctor,destroy,ctx};
 for(int i=0;i<4;i++)if(a==base+rva[i] && n==16){memcpy(out,contracts[i],16);return 1;}
 if(a==base+0x566898 && n==8){const BYTE consumer[]={0x44,0x0f,0xb6,0x88,0xc0,0,0,0};memcpy(out,consumer,8);return 1;}
 if(a==base+0x7161720 && n==4){unsigned offset=24;memcpy(out,&offset,4);return 1;}
 if(hook_test){
  BYTE* blocks[]={hook_code,official_module,hook_stub};unsigned lengths[]={sizeof hook_code,sizeof official_module,sizeof hook_stub};
  for(int i=0;i<3;i++)if(a>=(UINT64)blocks[i] && a+n<=(UINT64)blocks[i]+lengths[i]){memcpy(out,(void*)a,n);return 1;}
  return 0;
 }
 if((a==base+0x7215038 || a==base+0x6fc25f0 || a==base+0x715d5e8) && n==4){DWORD offset=a==base+0x7215038?0:a==base+0x6fc25f0?8:16;memcpy(out,&offset,4);return 1;}
 if(a==base+0x71ee790 && n==8){UINT64 p=(UINT64)manager;memcpy(out,&p,8);return 1;}
 unsigned char* b[]={client,native_context,connection,manager,player_manager,local_player,entity,collection,component,handle,players,replacement_core,replacement_selector,replacement_context};
 unsigned sizes[]={256,512,128,64,2048,256,2048,128,256,16,16,2048,64,256};
 for(int i=0;i<14;i++)if(a>=(UINT64)b[i] && a+n<=(UINT64)b[i]+sizes[i]){memcpy(out,(void*)a,n);return 1;}
 return rd_copy(a,out,n);
}
static BYTE* alloc_near(UINT64 a){(void)a;return hook_stub;}
static int VirtualProtect(void* p,UINT64 n,DWORD prot,DWORD* old){(void)n;*old=4;if(hook_race && prot==0x40){((BYTE*)p)[0]=0xcc;hook_race=0;}return 1;}
static int FlushInstructionCache(void* h,void* p,UINT64 n){(void)h;(void)p;(void)n;return 1;}
static void log_num(const char* s,UINT64 n){(void)s;(void)n;}
static UINT64 board_query(const void* address,SC_MBI* m,UINT64 n){(void)n;assert(address==component+0xc0);m->base=component;m->size=sizeof component;m->state=0x1000;m->prot=4;return sizeof *m;}
static BOOL board_write(HANDLE h,void* address,const void* data,UINT64 n,UINT64* done){(void)h;assert(address==component+0xc0 && n==1 && *(BYTE*)data==1);memcpy(address,data,n);*done=n;board_writes++;return 1;}
typedef long long INT64;
#define rd_copy respawn_read
#define np_base test_base
#define GetModuleHandleW test_module
#define GetTickCount64 mock_tick_time
#define VirtualQuery board_query
#define WriteProcessMemory board_write
#define GetLastError nr_mock_GetLastError
#define SetLastError nr_mock_SetLastError
#define NR_FUNCTION(base,rva,type) ((type)native_function(rva))
#include "../src/native_respawn_poll.h"
#undef rd_copy
#undef np_base
#undef GetModuleHandleW
#undef GetTickCount64
#undef VirtualQuery
#undef WriteProcessMemory
DWORD GetLastError(void){return last_error;}
void SetLastError(DWORD error){last_error=error;}
static void original_tick(UINT64 c,UINT64 u){assert(c==(UINT64)client && u==7);original_calls++;last_error=99;}
static void begin_respawn(void){
 g_native_respawn=1;g_user_enabled=1;g_native_menu=g_menu=g_ui_open=g_paused=0;g_cfg.enabled=1;g_cfg.goofy=0;
 g_slots[0].state=100;g_local=0;flags(1,1);basis(0);
 component[0xc0]=0;
 float p[]={42,10,-9},f[]={0,0,1};memcpy(core+0x100,p,12);memcpy(core+0x150,f,12);
 g_now_ms+=3000;nr_capture_live(g_now_ms);assert(g_nr_valid);
 sc_pad start={0x10};g_cfg.enabled=0;g_menu=1;
 nr_note_menu(g_now_ms+10,1,&start,&start);assert(g_nr_armed);
 nr_capture_live(g_now_ms+100);assert(g_nr_valid);
 nr_begin_menu(g_now_ms+254,1);assert(g_nr_valid && !g_nr_armed);
 g_native_menu=1;flags(0,0);basis(1);
 g_now_ms+=2000;nr_end_menu(g_now_ms);assert(g_nr_pending);
 g_native_menu=g_menu=0;g_cfg.enabled=1;sc_pad neutral={0};nr_note_menu(g_now_ms,1,&neutral,&neutral);
}
static void tick(void){nr_client_tick((UINT64)client,7);assert(last_error==99);}
static void prepare_hook(int indirect){
 hook_test=1;g_native_respawn=1;g_nr_hooked=0;g_nr_next_hook=0;memset(hook_code,0,sizeof hook_code);memset(official_module,0,sizeof official_module);
 official_module[0]='M';official_module[1]='Z';unsigned pe=0x80,size=4096,signature=0x4550;
 memcpy(official_module+0x3c,&pe,4);memcpy(official_module+pe,&signature,4);memcpy(official_module+pe+0x50,&size,4);
 BYTE* entry=hook_code+0x2e77190;UINT64 target=(UINT64)official_module+0x200;
 if(indirect){entry[0]=0xff;entry[1]=0x25;int rel=0x100-(0x2e77190+6);memcpy(entry+2,&rel,4);memcpy(hook_code+0x100,&target,8);}
 else {entry[0]=0xe9;int rel=0x100-(0x2e77190+5);memcpy(entry+1,&rel,4);entry[5]=0x90;entry[6]=0x90;entry[7]=0x90;
  hook_code[0x100]=0xff;hook_code[0x101]=0x25;memcpy(hook_code+0x106,&target,8);}
}
int main(void){
 property_fixture_main();g_nr_tick_prev=original_tick;
 const UINT64 base=0x140000000ULL;
 unsigned s=13;memcpy(client+0xc4,&s,4);native_context[0x100]=1;
 ptr(client,0,base+0x60842c0);ptr(client,8,(UINT64)native_context);ptr(native_context,24,(UINT64)player_manager);
 ptr(player_manager,0,base+0x63e30a0);ptr(player_manager,0x4c8,(UINT64)players);ptr(player_manager,0x4d0,(UINT64)players+8);ptr(players,0,(UINT64)local_player);
 ptr(local_player,0,base+0x63e2d30);ptr(local_player,0x78,(UINT64)native_context);local_player[0x45]=1;
 ptr(local_player,0xb8,(UINT64)entity);ptr(local_player,0xb0,(UINT64)handle);ptr(handle,0,(UINT64)entity+8);
 ptr(entity,0,base+0x6087908);ptr(entity,0xf8,(UINT64)local_player);ptr(entity,0x20,(UINT64)native_context);
 ptr(entity,0x70,(UINT64)collection);ptr(collection,0,(UINT64)entity);ptr(entity,0x628,(UINT64)component);
 ptr(component,0,base+0x6170410);ptr(component,0x18,(UINT64)collection);ptr(component,0x70,(UINT64)core);
 ptr(native_context,8,(UINT64)client);ptr(native_context,16,(UINT64)connection);ptr(connection,0x50,(UINT64)client);
 begin_respawn();tick();assert(calls==1 && ctor_calls==1 && destroy_calls==1 && !g_nr_pending && original_calls==1 && sent[12]==42 && sent[10]==-1);
 tick();assert(calls==1 && original_calls==2);
 begin_respawn();sc_pad raw={0},out={0};out.rx=20000;nr_note_menu(g_now_ms,1,&raw,&out);tick();assert(calls==1 && g_nr_pending);
 out.rx=0;raw.rt=80;nr_note_menu(g_now_ms,1,&raw,&out);tick();assert(calls==1 && g_nr_pending);
 raw.rt=0;nr_note_menu(g_now_ms,0,&raw,&out);tick();assert(calls==1 && g_nr_pending);
 nr_note_menu(g_now_ms,1,&raw,&out);manager[0]=1;tick();assert(calls==1 && g_nr_pending);
 manager[0]=0;native_context[0x100]=0;tick();assert(calls==1 && g_nr_pending);
 native_context[0x100]=1;g_cfg.enabled=0;tick();assert(calls==1 && g_nr_pending);g_cfg.enabled=1;
 tick();assert(calls==2 && !g_nr_pending && ctor_calls==2 && destroy_calls==2);
 begin_respawn();g_slots[0].state=200;tick();assert(calls==2 && !g_nr_pending);
 begin_respawn();g_now_ms+=1501;tick();assert(calls==2 && !g_nr_pending);
 begin_respawn();g_cfg.goofy=1;tick();assert(calls==2 && !g_nr_pending);
 begin_respawn();g_native_respawn=0;tick();assert(calls==2 && !g_nr_pending);
 begin_respawn();basis(0);tick();assert(calls==2 && g_nr_pending);
 g_now_ms+=120;basis(1);tick();assert(calls==3 && !g_nr_pending);
 begin_respawn();basis(0);tick();g_now_ms+=1501;tick();assert(calls==3 && !g_nr_pending);
 begin_respawn();local_player[0x44]=1;unsigned old=board_writes;tick();assert(calls==3 && board_writes==old && g_nr_pending);local_player[0x44]=0;
 ptr(component,0x70,(UINT64)source);tick();assert(calls==3 && board_writes==old);ptr(component,0x70,(UINT64)core);
 entity[0x7e0]=1;tick();assert(calls==3 && board_writes==old);entity[0x7e0]=0;
 component[0xc0]=2;tick();assert(calls==3 && board_writes==old);component[0xc0]=0;
 tick();assert(calls==4 && board_writes==old+1 && component[0xc0]==1 && !g_nr_pending);
 old=board_writes;nr_frame local;assert(nr_read((UINT64)selector,base,respawn_read,&local));
 assert(nr_keep_board(base,(UINT64)client,&local) && board_writes==old);
 assert(native_local_slot()==0);
 ptr(replacement_core,0,base+0x6176d30);ptr(replacement_core,0x440,(UINT64)replacement_selector);
 ptr(replacement_core,0x3c0,(UINT64)replacement_context);ptr(replacement_selector,8,(UINT64)replacement_context);
 ptr(component,0x70,(UINT64)replacement_core);g_slots[1].sel=(UINT64)replacement_selector;g_slots[1].state=100;
 /* Recorded failure: old slot stays in 100 while another actor rides. */
 g_slots[0].state=100;g_native_respawn=0;native_clock+=40;tick();assert(native_local_slot()==1 && calls==4 && board_writes==old);
 local_player[0x44]=1;native_clock+=40;tick();assert(native_local_slot()==-1);local_player[0x44]=0;
 native_clock+=40;tick();assert(native_local_slot()==1);native_clock+=251;assert(native_local_slot()==-1);
 ptr(component,0x70,(UINT64)core);native_clock+=40;tick();assert(native_local_slot()==0);
 /* Respawn/loading screen without a Start/Back request must not queue a reset. */
 begin_respawn();g_nr_pending=0;g_nr_valid=1;g_nr_armed=0;g_nr_last=g_now_ms;
 nr_begin_menu(g_now_ms+10,1);nr_end_menu(g_now_ms+100);assert(!g_nr_valid && !g_nr_pending);
 begin_respawn();flags(1,1);basis(1);assert(nr_read((UINT64)selector,base,respawn_read,&g_nr_before));
 g_nr_last_motion=g_nr_before;g_nr_last=g_now_ms;g_nr_ground=1;
 unsigned ground_calls=calls;tick();assert(calls==ground_calls+1 && !g_nr_pending && sent[10]==1);
 tick();assert(calls==ground_calls+1); /* No repeated reset; regular heading is not reversed. */
 begin_respawn();g_cfg.goofy=g_nr_goofy=g_game_goofy=1;flags(0,1);source[0x13]=1;basis(1);
 assert(nr_read((UINT64)selector,base,respawn_read,&g_nr_before));
 g_nr_last_motion=g_nr_before;g_nr_last=g_now_ms;g_nr_ground=1;ground_calls=calls;
 tick();assert(calls==ground_calls+1 && !g_nr_pending && sent[10]==1);
 tick();assert(calls==ground_calls+1); /* Goofy reset also keeps the forward travel heading. */
 g_game_goofy=0;
 /* Native tick publishes the XL agreement independently of pose-reset flags. */
 begin_respawn();g_nr_pending=0;g_native_respawn=0;g_cfg.xl_layout=1;g_cfg.goofy=0;
 g_xl_agreement=(sc_xl_agreement){0};g_xl_idle=1;flags(1,0);basis(0);
 float xl_position[3]={0,0,0},xl_forward[3]={0,0,1};memcpy(core+0x150,xl_forward,12);
 unsigned agreement_calls=calls;
 for(int i=0;i<30;i++){xl_position[2]+=.2f;memcpy(core+0x100,xl_position,12);native_clock+=40;tick();}
 assert(g_xl_agreement_owner==(UINT64)selector && g_xl_agreement_hint==0 && calls==agreement_calls);
 flags(1,1);xl_forward[2]=-1;memcpy(core+0x150,xl_forward,12);
 for(int i=0;i<20;i++){xl_position[2]+=.2f;memcpy(core+0x100,xl_position,12);native_clock+=40;tick();}
 assert(g_xl_agreement_owner==(UINT64)selector && g_xl_agreement_hint==1 && calls==agreement_calls);
 g_cfg.xl_layout=0;native_clock+=40;tick();assert(!g_xl_agreement_owner);
 sc_pad steering={0},steered={0};steering.lt=255;steered.lx=-32768;
 nr_note_menu(g_now_ms,0,&steering,&steered);assert(g_xl_idle); /* Trigger steering is not held foot input. */
 steering.ry=-20000;nr_note_menu(g_now_ms,0,&steering,&steered);assert(!g_xl_idle);
 steering.ry=0;steered.ry=20000;nr_note_menu(g_now_ms,0,&steering,&steered);assert(!g_xl_idle); /* Synthetic board gesture still blocks. */
 prepare_hook(0);nr_try_hook(1000);assert(g_nr_hooked==1 && (UINT64)g_nr_tick_prev==(UINT64)hook_code+0x100);
 assert(hook_code[0x2e77195]==0x90 && hook_code[0x2e77196]==0x90 && hook_code[0x2e77197]==0x90);
 UINT64 destination;memcpy(&destination,hook_stub+6,8);assert(destination==(UINT64)nr_client_tick);
 prepare_hook(1);nr_try_hook(1000);assert(g_nr_hooked==1 && (UINT64)g_nr_tick_prev==(UINT64)official_module+0x200);
 memcpy(&destination,hook_code+0x100,8);assert(destination==(UINT64)nr_client_tick);
 prepare_hook(1);g_native_respawn=0;nr_try_hook(1000);assert(g_nr_hooked==1);
 prepare_hook(0);official_module[0x80]=0;nr_try_hook(1000);assert(!g_nr_hooked);
 prepare_hook(0);hook_race=1;nr_try_hook(1000);assert(g_nr_hooked==-1 && hook_code[0x2e77190]==0xcc);
 puts("PASS: chained native tick preserved exactly once, transition/API lifetime and alignment, delayed Start capture, neutral raw/output same-user guard, busy/unready deferral, single native reset, profile/air/expiry/disabled cancellation");
 puts("PASS: E9 relay and FF25 hook chaining retains official target, validates module owner, preserves adjacent instructions, and rejects concurrent hook mutation");
 puts("PASS: native keep-board choice armed before reset, local player/entity/component/core ownership checked, remote/queued-teleport/stale/nonboolean rejected, exactly one byte written and already-onboard untouched");
 puts("PASS: authoritative local owner follows a respawn while old slot stays riding, remote owner and stale cache rejected, regular owner restored, tracking stays active with experimental recovery disabled and preserves controller/native state");
}
