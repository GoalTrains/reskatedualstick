#include "catch_hold_assets.h"
static sc_catch_hold_assets g_catch_hold_assets;
static UINT64 cha_next,cha_apply_next,cha_restore_next;
static int cha_checked,cha_supported,cha_last_count,cha_last_failures,cha_last_conflicts;
static volatile int cha_busy;
static UINT64 cha_owners[0xbbf],cha_owner_hash,cha_rescan;
static unsigned cha_domain,cha_name_index;
static int cha_scanning;
static int cha_write(np_address address,const void* data,np_address size){
 SC_MBI m;UINT64 done=0;
 if(size!=4 || (address&3) || address<0x10000 || address>=0x7fffffffffffULL-4 ||
    !VirtualQuery((const void*)address,&m,sizeof m) || m.state!=0x1000 ||
    (m.prot&0xff)!=PAGE_READWRITE || (m.prot&0x100) || address+size>(UINT64)m.base+m.size)return 0;
 return WriteProcessMemory(GetCurrentProcess(),(void*)address,data,size,&done) && done==size;
}
static int cha_contract(UINT64 base){
 static const BYTE expected[32]={0x48,0x83,0xec,0x38,0x0f,0xb7,0xc1,0x48,0x8d,0x0d,0x02,0x15,0x57,0x03,0x48,0x8b,0x0c,0xc1,0x48,0x85,0xc9,0x74,0x28,0x48,0x8b,0x49,0x48,0x48,0x85,0xc9,0x74,0x1f};
 BYTE code[32];if(!base || !rd_copy(base+0x41b5800,code,32))return 0;
 for(unsigned i=0;i<32;i++)if(code[i]!=expected[i])return 0;return 1;
}
static unsigned cha_set(UINT64 set,UINT64 base){
 UINT64 vt,entries,a;DWORD count;unsigned added=0;
 set&=~4ULL;
 if(set<0x10000 || (set&7) || !rd_copy(set,&vt,8) || vt!=base+0x6123038 ||
    !np_pointer(rd_copy,set+0x20,&entries) || !rd_copy(entries-4,&count,4) ||
    !(count&0x7fffffff) || (count&0x7fffffff)>128)return 0;
 for(unsigned i=0;i<(count&0x7fffffff);i++)if(rd_copy(entries+i*8ULL,&a,8)){
  a&=~4ULL;unsigned before=g_catch_hold_assets.count;
  if(sc_cha_add(&g_catch_hold_assets,a,base,rd_copy) && g_catch_hold_assets.count>before)added++;
 }
 return added;
}
typedef UINT64 (*cha_find_fn)(WORD,const char*);
static unsigned cha_discover(UINT64 base,UINT64 now,cha_find_fn find){
 static const char* names[]={
  "gameplay/input/gestures/gestureset_fliptricks",
  "gameplay/input/gestures/gestureset_fliptricks_fromgrind",
  "gameplay/input/gestures/gestureset_underflips",
  "gameplay/input/gestures/gestureset_simple_fliptricks",
  "gameplay/input/gestures/gestureset_hybrid_simple_fliptricks",
  "gameplay/input/gestures/gestureset_hybrid_simple_fliptricks_fromgrind"};
 unsigned added=0;
 if(now>=cha_next){
  static UINT64 owners[0xbbf];
  cha_next=now+2000;
  if(!rd_copy(base+0x7726d10,owners,sizeof owners))return 0;
  UINT64 hash=1469598103934665603ULL;
  for(unsigned i=0;i<0xbbf;i++){hash^=owners[i];hash*=1099511628211ULL;}
  if(hash!=cha_owner_hash || (!cha_scanning && now>=cha_rescan)){
   cha_owner_hash=hash;memcpy(cha_owners,owners,sizeof owners);
   cha_domain=cha_name_index=0;cha_scanning=1;cha_rescan=now+30000;
   sc_cha_prune(&g_catch_hold_assets,base,rd_copy);
  }
 }
 /* The engine's domain-owner table bounds named lookup. Walk only occupied
  * domains, never scan arbitrary memory or infer an asset from float values. */
 /* Never run a whole native lookup pass in one controller poll. Native
  * lookup can be expensive across loaded domains: at most six calls and
  * one millisecond of work per poll, with infrequent background rescans. */
 UINT64 started=GetTickCount64();
 for(unsigned work=0;cha_scanning && work<6;work++){
  while(cha_domain<0xbbf && !cha_owners[cha_domain])cha_domain++;
  if(cha_domain==0xbbf){cha_scanning=0;break;}
  UINT64 asset=find((WORD)cha_domain,names[cha_name_index]);
  if(asset)added+=cha_set(asset,base);
  if(++cha_name_index==sizeof names/sizeof names[0]){cha_name_index=0;cha_domain++;}
  if(GetTickCount64()-started>=1)break;
 }
 return added;
}
static void cha_sync(UINT64 now,int option){
 if(__sync_lock_test_and_set(&cha_busy,1))return;
 UINT64 base=np_base();
 if(!cha_checked && base){cha_checked=1;cha_supported=cha_contract(base);}
 if(!cha_supported)goto done;
 if(!option && (g_catch_hold_assets.active || now>=cha_restore_next)){
  /* Retry any failed rollback on subsequent polls. Only values still owned
   * by this module and the same live asset identity can be restored. */
  sc_cha_restore(&g_catch_hold_assets,base,rd_copy,cha_write);
  cha_restore_next=now+2000;
 }
 unsigned added=0;
 if(option)added=cha_discover(base,now,(cha_find_fn)(base+0x41b5800));
 else if(cha_scanning){cha_scanning=0;cha_rescan=0;}
 /* Prepare ranges before gesture recognition can cache them. Neutral stick
  * output still releases a trick on catch; disabling the mode restores the
  * original ranges. These assets are shared by the native input system. */
 if(option && (added || (!g_catch_hold_assets.active && now>=cha_apply_next))){
  cha_apply_next=now+2000;
  sc_cha_apply(&g_catch_hold_assets,base,rd_copy,cha_write);
 }
 if((int)g_catch_hold_assets.count!=cha_last_count){
  cha_last_count=(int)g_catch_hold_assets.count;
  log_num("manual catch: typed native board-gesture hold ranges discovered ",g_catch_hold_assets.count);
 }
 if(g_catch_hold_assets.failures!=cha_last_failures){cha_last_failures=g_catch_hold_assets.failures;log_num("manual catch: native hold range write/rollback failures ",cha_last_failures);}
 if(g_catch_hold_assets.conflicts!=cha_last_conflicts){cha_last_conflicts=g_catch_hold_assets.conflicts;log_num("manual catch: changed/unloaded native gesture identities skipped ",cha_last_conflicts);}
 done:__sync_lock_release(&cha_busy);
}
