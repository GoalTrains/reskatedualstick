/* Exact native gameplay setting names, with bounded checked asset/name reads. */
#ifndef SC_HOLD_FAKIE_KEYS_H
#define SC_HOLD_FAKIE_KEYS_H
#include "native_pose_restore.h"
static int hf_key_id(const char* key) {
 static const char stance[]="egf_holdfakiestance",push[]="egf_holdfakiepush",held[]="HeldFlipTricks";
 int a=1,b=1,c=1;for(unsigned i=0;i<32;i++){
  char ch=key[i];if(a && (i>=sizeof stance || ch!=stance[i]))a=0;if(b && (i>=sizeof push || ch!=push[i]))b=0;
  if(c && (i>=sizeof held || ch!=held[i]))c=0;
  if(!ch)return a?1:b?2:c?3:0;if(!a && !b && !c)return 0;
 }return 0;
}
static int hf_reference(np_address reference,np_read_fn read) {
 np_address asset=0,name=0;
 if(reference<0x10000 || !read(reference,&asset,8))return 0;asset&=~4ULL;
 if(asset<0x10000 || asset>0x7fffffffff00ULL || (asset&7) || !read(asset+0x60,&name,8) || name<0x10000 || name>0x7fffffffff00ULL)return 0;
 char key[32];if(!read(name,key,sizeof key))return 0;return hf_key_id(key);
}
static unsigned char hf_bool_value(int setting,int enabled,int compatibility,unsigned char original){
 return enabled && compatibility && (setting==1 || setting==2)?0:original;
}
#endif
