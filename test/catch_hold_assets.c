#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/catch_hold_assets.h"
#define START 0x100000ULL
static unsigned char memory[0x8000];static int writes,fail_max,fail_after_write,read_failure,fail_restore;
static np_address asset=START+0x100;
static int read_mem(np_address a,void*p,np_address n){
 if(read_failure){read_failure=0;return 0;}
 if(a<START || a+n>START+sizeof memory)return 0;memcpy(p,memory+a-START,(size_t)n);return 1;
}
static int write_mem(np_address a,const void*p,np_address n){
 assert(n==4 && (a==asset+0x28 || a==asset+0x38));
 float value;memcpy(&value,p,4);if(fail_restore && a==asset+0x38 && value==90)return 0;
 if(fail_max && a==asset+0x28){fail_max=0;return 0;}
 memcpy(memory+a-START,p,4);writes++;
 if(fail_after_write){fail_after_write=0;read_failure=1;}return 1;
}
static void put(np_address a,const void*p,unsigned n){memcpy(memory+a-START,p,n);}
static void ptr(np_address a,np_address v){put(a,&v,8);}
static void real(np_address a,float v){put(a,&v,4);}
static float get(np_address a){float x;memcpy(&x,memory+a-START,4);return x;}
static void fixture(float lo,float hi){
 memset(memory,0,sizeof memory);writes=fail_max=fail_after_write=read_failure=fail_restore=0;
 ptr(asset,0x140000000ULL+0x6123018);ptr(asset+0x18,START+0x1001);ptr(asset+0x20,START+0x2000);
 const char name[]="Gameplay/Input/Gestures/FlipTricks/Gesture_FlipTrick_Flowerflip_Underflip";
 put(START+0x1001,name,sizeof name);unsigned n=2,h=123;put(START+0x1ffc,&n,4);put(asset+0x68,&h,4);
 real(asset+0x38,lo);real(asset+0x28,hi);real(asset+0x44,.85f);
}
int main(void){
 static const float ranges[][2]={{90,180},{30,120},{240,330},{300,180},{0,0},{0,360}};
 for(unsigned i=0;i<sizeof ranges/sizeof ranges[0];i++){
  fixture(ranges[i][0],ranges[i][1]);unsigned char original[sizeof memory];memcpy(original,memory,sizeof memory);
  sc_catch_hold_assets m={0};assert(sc_cha_add(&m,asset,0x140000000ULL,read_mem));
  assert(sc_cha_add(&m,asset,0x140000000ULL,read_mem) && m.count==1);
  assert(sc_cha_apply(&m,0x140000000ULL,read_mem,write_mem));
  assert(get(asset+0x38)==0 && get(asset+0x28)==360 && get(asset+0x44)==.85f);
  int before=writes;assert(sc_cha_apply(&m,0x140000000ULL,read_mem,write_mem));assert(writes==before);
  sc_cha_restore(&m,0x140000000ULL,read_mem,write_mem);assert(!memcmp(original,memory,sizeof memory));
  before=writes;sc_cha_restore(&m,0x140000000ULL,read_mem,write_mem);assert(writes==before);
 }
 /* Invalid type/name, bogus array, nonfinite/out-of-range scalars never write. */
 for(int bad=0;bad<5;bad++){
  fixture(90,180);
  if(bad==0)ptr(asset,0x140000000ULL+0x6123038);
  if(bad==1)memory[0x1001]='X';
  if(bad==2){unsigned n=999;put(START+0x1ffc,&n,4);}
  if(bad==3)real(asset+0x44,0);
  if(bad==4)real(asset+0x38,361);
  sc_catch_hold_assets m={0};assert(!sc_cha_add(&m,asset,0x140000000ULL,read_mem));assert(!writes);
 }
 /* A partial write and a failed post-write verification both roll back. */
 for(int which=0;which<2;which++){
  fixture(90,180);sc_catch_hold_assets m={0};assert(sc_cha_add(&m,asset,0x140000000ULL,read_mem));
  fail_max=which==0;fail_after_write=which==1;
  assert(!sc_cha_apply(&m,0x140000000ULL,read_mem,write_mem));
  assert(get(asset+0x38)==90 && get(asset+0x28)==180 && !m.active && m.failures);
 }
 /* Another writer owns its changed value; restore only our surviving field. */
 {fixture(90,180);sc_catch_hold_assets m={0};sc_cha_add(&m,asset,0x140000000ULL,read_mem);
 sc_cha_apply(&m,0x140000000ULL,read_mem,write_mem);real(asset+0x38,12.5f);
 sc_cha_restore(&m,0x140000000ULL,read_mem,write_mem);
 assert(get(asset+0x38)==12.5f && get(asset+0x28)==180 && m.conflicts==1);}
 /* Recycled/unloaded identities cannot receive a saved original. */
 {fixture(90,180);sc_catch_hold_assets m={0};sc_cha_add(&m,asset,0x140000000ULL,read_mem);
 sc_cha_apply(&m,0x140000000ULL,read_mem,write_mem);unsigned hash=456;put(asset+0x68,&hash,4);
 int before=writes;sc_cha_restore(&m,0x140000000ULL,read_mem,write_mem);assert(writes==before && m.conflicts);}
 /* Retired loading/map copies cannot fill the bounded cache indefinitely. */
 {fixture(90,180);sc_catch_hold_assets m={0};sc_cha_add(&m,asset,0x140000000ULL,read_mem);
 sc_cha_apply(&m,0x140000000ULL,read_mem,write_mem);
 sc_cha_prune(&m,0x140000000ULL,read_mem);assert(m.count==1 && m.slots[0].min==90 && m.slots[0].max==180);
 unsigned hash=456;put(asset+0x68,&hash,4);int before=writes;
 sc_cha_prune(&m,0x140000000ULL,read_mem);assert(!m.count && writes==before);
 assert(sc_cha_add(&m,asset,0x140000000ULL,read_mem) && m.count==1);}
 /* A failed restore remains owned and cannot be mistaken for an active mode. */
 {fixture(90,180);sc_catch_hold_assets m={0};sc_cha_add(&m,asset,0x140000000ULL,read_mem);
 assert(sc_cha_apply(&m,0x140000000ULL,read_mem,write_mem));fail_restore=1;
 sc_cha_restore(&m,0x140000000ULL,read_mem,write_mem);
 assert(!m.active && m.slots[0].own_min && !m.slots[0].own_max);
 assert(!sc_cha_apply(&m,0x140000000ULL,read_mem,write_mem));assert(!m.active && get(asset+0x28)==180);
 fail_restore=0;assert(sc_cha_apply(&m,0x140000000ULL,read_mem,write_mem));
 sc_cha_restore(&m,0x140000000ULL,read_mem,write_mem);assert(get(asset+0x38)==90 && get(asset+0x28)==180);}
 puts("PASS: typed native hold ranges cover straight/wrapped/zero-width/full-circle rules; exact restoration, duplicate suppression, field-only writes, foreign/conflicting identities, failed writes and failed verification rollback");
}
