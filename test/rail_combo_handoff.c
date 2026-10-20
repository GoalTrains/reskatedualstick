#include <assert.h>
#include <stdio.h>
#include "../src/translate.h"
typedef struct { int ms,lx,ly,rx,ry; } sample;
static const sample report0[]={
 {0,263,-600,5272,-14139},
 {15,23,-440,8985,-4281},
 {27,167,-520,9273,-2425},
 {40,407,-728,8889,-2425},
 {56,231,-552,11226,-4121},
 {71,55,-440,16651,-7514},
 {83,407,-728,20492,-10714},
 {99,407,-728,24781,-15388},
 {115,55,-376,26797,-20733},
 {132,231,-488,23772,-23981},
 {147,311,-880,18843,-27678},
 {160,167,-696,14650,-30415},
 {175,119,-264,8441,-32768},
 {192,303,-664,823,-32768},
 {204,439,-872,-5162,-32768},
 {219,87,-440,-10523,-31775},
 {231,-1657,600,-13116,-30543},
 {244,-2538,1432,-14524,-29775},
 {256,-8075,4777,-15325,-29367},
 {271,-23774,15099,-16333,-28318},
 {283,-28143,18940,-17053,-26462},
 {296,-27967,18692,-17533,-24350},
 {312,-27791,18492,-18093,-18748},
 {324,-27999,18620,-5978,-4249},
 {340,-27855,18844,-1641,-2553},
};
static const sample report1[]={
 {0,951,-296,711,-920},
 {12,951,-264,743,-856},
 {28,951,-232,781,-1064},
 {44,1271,-440,647,-1064},
 {56,1303,-456,-1257,-1944},
 {68,903,-264,-2298,-2313},
 {84,903,1,-5562,-5257},
 {100,1303,-520,-11052,-11771},
 {112,1255,-590,-18237,-22813},
 {128,807,1,-20190,-26174},
 {140,807,-24,-19149,-27134},
 {156,1399,-664,-15020,-29823},
 {172,1015,-296,-9099,-32559},
 {184,766,168,-4474,-32768},
 {200,759,-120,2872,-32768},
 {212,-985,1,7273,-32768},
 {224,-1945,536,10777,-32495},
 {240,-7243,3129,15226,-30463},
 {252,-18861,7594,19676,-27518},
 {268,-31168,12635,26765,-20477},
 {284,-31680,12907,32270,-11595},
 {300,-31680,12987,32767,-6906},
 {312,-30784,12667,32767,-5433},
 {324,-30704,12379,28510,-3513},
 {340,-30224,12747,-553,-808},
};
int main(void){
 int checked=0;
 /* The supplied regular, fixed-foot v6.3 recording: fast rear-foot
  * wind-ups followed by an opposite-foot flip, with single-stick flips off. */
 for(int run=0;run<2;run++){
  sc_config c={0};c.enabled=c.pure=c.air_controls=1;c.deadzone=.15f;c.active=.3f;c.manual_max=.85f;c.windup_deg=60;c.grind_sensitivity=1.2f;
  sc_state s={0};s.grind=1;s.game_crouch=-1;sc_pad neutral={0};sc_translate(&c,&s,neutral,1000);
  const sample* a=run?report1:report0;int count=run?sizeof(report1)/sizeof(*a):sizeof(report0)/sizeof(*a);
  int candidate=0,exit=0;
  for(int i=0;i<count;i++){
   sc_pad p={0};p.lx=a[i].lx;p.ly=a[i].ly;p.rx=a[i].rx;p.ry=a[i].ry;
   sc_pad o=sc_translate(&c,&s,p,1100+a[i].ms);
   if(s.rail_scoop_candidate==2&&!s.grind_pop&&sc_abs((float)p.rx)>3000){
    assert((long)o.rx*p.rx>0);candidate++;checked++;
   }
   if(s.rail_scoop_exit==2)exit=1;
  }
  assert(candidate&&exit);
 }
 printf("PASS: %d recorded rear-foot wind-up samples retain trick direction before grind-exit confirmation\n",checked);return 0;
}
