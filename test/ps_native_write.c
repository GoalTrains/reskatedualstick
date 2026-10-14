#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/translate.h"
#include "../src/ps_reports.h"
#include "../src/ps_native_write.h"
int main(void){
 for(int ds=0;ds<2;ds++)for(int axis=0;axis<256;axis++){
  BYTE report[64],saved[64];for(int i=0;i<64;i++)report[i]=(BYTE)(i*17+axis);
  report[0]=axis;report[1]=255-axis;report[2]=axis;report[3]=255-axis;
  report[ds?7:4]=(report[ds?7:4]&0xf0)|8;
  memcpy(saved,report,64);sc_pad raw=sc_ps_decode(report,ds);
  assert(!sc_ps_native_write(report,ds,raw,raw));assert(!memcmp(report,saved,64));
  sc_pad changed=raw;changed.rx=32767;changed.rt=255;changed.buttons|=SC_RSHOULDER;
  int result=sc_ps_native_write(report,ds,raw,changed);
  if(changed.rx!=raw.rx || changed.rt!=raw.rt || changed.buttons!=raw.buttons){
   assert(result==1);sc_pad decoded=sc_ps_decode(report,ds);
   assert(decoded.rx>32000 && decoded.rt==255 && (decoded.buttons&SC_RSHOULDER));
   /* Motion/touchpad/sequence payload outside the rewritten input fields stays intact. */
   for(int i=9;i<64;i++)assert(report[i]==saved[i]);
  }
 }
 puts("PASS: actual native HID writer preserves 512 unchanged DS4/DualSense reports byte-for-byte; changed sticks/triggers/buttons rewrite while other payload remains intact");
}
