/* Experimental recovery of the recorded menu-only frame mismatch. Source
 * +10 exports context+13c4 bit13 and reverses the orientation basis. Never
 * force it during normal riding or write the separate switch byte +14. */
#ifndef SC_NATIVE_MENU_FRAME_H
#define SC_NATIVE_MENU_FRAME_H
#include "native_pose_restore.h"
typedef struct {np_pose pose;unsigned char basis,native_frame;} np_frame;
static int np_frame_read(np_address selector,np_address base,np_read_fn read,np_frame* out) {
    unsigned bits;unsigned char basis;
    if(!np_pose_read(selector,base,read,&out->pose) ||
       !read(out->pose.source+0x10,&basis,1) || basis>1 ||
       !read(out->pose.context+0x13c4,&bits,4) || basis!=((bits>>13)&1) ||
       !read(out->pose.source+0x14,&out->native_frame,1))return 0;
    out->basis=basis;return 1;
}
static int np_frame_recover(const np_frame* saved,np_address selector,np_address base,
                            np_read_fn read,np_write_fn write) {
    np_frame current;unsigned char bytes[4];
    if(saved->basis!=0 || saved->native_frame!=1 || saved->pose.pair[0]!=1 || saved->pose.pair[1]!=0)return -2;
    if(!np_frame_read(selector,base,read,&current))return -1;
    if(saved->pose.selector!=selector || saved->pose.core!=current.pose.core ||
       saved->pose.source!=current.pose.source || saved->pose.context!=current.pose.context)return -2;
    if(current.basis==0)return 0;
    if(current.native_frame!=0 || current.pose.pair[0]!=0 || current.pose.pair[1]!=1)return -2;
    if(!read(current.pose.source+0x10,bytes,4))return -1;
    /* Preserve the unrelated +11 byte and every byte from +14 onward. */
    bytes[0]=saved->basis;bytes[2]=saved->pose.pair[0];bytes[3]=saved->pose.pair[1];
    return write(current.pose.source+0x10,bytes,4)?1:-2;
}
#endif
