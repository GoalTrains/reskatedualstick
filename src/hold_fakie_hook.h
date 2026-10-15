/* Chain the official settings hook, just as the client-tick hook is chained.
 * Override two return values only. No profile/database writes or native setters. */
#include "hold_fakie_keys.h"
typedef BYTE (*hf_bool_fn)(UINT64,const void*);
static hf_bool_fn hf_previous;
static int hf_hooked;static UINT64 hf_next;static volatile int hf_noted[2];
static BYTE hf_get_bool(UINT64 cache,const void* reference){
 BYTE original=hf_previous(cache,reference);DWORD error=GetLastError();BYTE result=original;
 if(g_user_enabled && g_cfg.disable_hold_fakie){
  int key=hf_reference((UINT64)reference,rd_copy);
  result=hf_bool_value(key,1,1,original);
  if(key && __sync_bool_compare_and_swap(&hf_noted[key-1],0,1))
   log_text(key==1?"hold-fakie compatibility: Hold Fakie Stance overridden OFF while Dual Stick is enabled":"hold-fakie compatibility: Hold Fakie Push overridden OFF while Dual Stick is enabled");
 }
 SetLastError(error);return result;
}
static void hf_try_hook(UINT64 now){
 if(hf_hooked || now<hf_next)return;hf_next=now+500;
 UINT64 base=np_base(),module=(UINT64)GetModuleHandleW(L"ReSkate.dll");if(!base || !module)return;
 BYTE* entry=(BYTE*)(base+0x804f60);BYTE bytes[16];UINT64 previous=0,slot=0,original;
 if(!rd_copy((UINT64)entry,bytes,16))return;memcpy(&original,bytes,8);
 if(bytes[0]==0xe9){int rel;memcpy(&rel,bytes+1,4);previous=(UINT64)entry+5+rel;}
 else if(bytes[0]==0xff && bytes[1]==0x25){int rel;memcpy(&rel,bytes+2,4);slot=(UINT64)entry+6+rel;if(!rd_copy(slot,&previous,8))return;}
 else return;
 UINT64 target=previous;
 for(int depth=0;depth<4 && !nr_in_module(module,target);depth++){
  if(!rd_copy(target,bytes,16))return;
  if(bytes[0]==0xe9){int rel;memcpy(&rel,bytes+1,4);target+=5+rel;}
  else if(bytes[0]==0xff && bytes[1]==0x25){int rel;memcpy(&rel,bytes+2,4);if(!rd_copy(target+6+rel,&target,8))return;}
  else return;
 }
 if(!nr_in_module(module,target))return;
 DWORD old;
 if(slot){
  if((slot&7) || !VirtualProtect((void*)slot,8,0x40,&old))return;
  hf_previous=(hf_bool_fn)previous;UINT64 changed=__sync_val_compare_and_swap((volatile UINT64*)slot,previous,(UINT64)hf_get_bool);
  DWORD unused;VirtualProtect((void*)slot,8,old,&unused);if(changed!=previous){hf_hooked=-1;log_text("hold-fakie compatibility: settings hook changed; override skipped");return;}
 }else{
  BYTE* stub=alloc_near((UINT64)entry);if(!stub)return;
  stub[0]=0xff;stub[1]=0x25;memset(stub+2,0,4);UINT64 hook=(UINT64)hf_get_bool;memcpy(stub+6,&hook,8);FlushInstructionCache(GetCurrentProcess(),stub,14);
  INT64 rel=(INT64)stub-(INT64)(entry+5);if(rel>0x7fffffffLL || rel<-0x7fffffffLL)return;
  UINT64 replacement=original;((BYTE*)&replacement)[0]=0xe9;int rel32=(int)rel;memcpy((BYTE*)&replacement+1,&rel32,4);
  if(!VirtualProtect(entry,8,0x40,&old))return;hf_previous=(hf_bool_fn)previous;
  UINT64 changed=__sync_val_compare_and_swap((volatile UINT64*)entry,original,replacement);
  DWORD unused;VirtualProtect(entry,8,old,&unused);FlushInstructionCache(GetCurrentProcess(),entry,8);
  if(changed!=original){hf_hooked=-1;log_text("hold-fakie compatibility: settings hook changed; override skipped");return;}
 }
 hf_hooked=1;log_text("hold-fakie compatibility: chained official gameplay bool getter; saved settings retained; defaults OFF only while Dual Stick is enabled");
}