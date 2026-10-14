/* Experimental native state-pair restoration; no inferred stance direction.
 * Verified exporter: core+3c8 bytes 12/13 -> state+60 bytes A4/A5, byte14
 * -> A3 and context+13dc bit11. Never write exported copies or switch byte14. */
#ifndef SC_NATIVE_POSE_RESTORE_H
#define SC_NATIVE_POSE_RESTORE_H
typedef unsigned long long np_address;
typedef int (*np_read_fn)(np_address,void*,np_address);
typedef int (*np_write_fn)(np_address,const void*,np_address);
typedef struct {np_address selector,core,source,context;unsigned char pair[2];} np_pose;
static int np_pointer(np_read_fn read,np_address address,np_address* out) {
    return address>=0x10000 && address<0x7fffffffffffULL-8 && read(address,out,8) &&
           *out>=0x10000 && *out<0x7fffffffffffULL && !(*out&7);
}
static int np_pose_read(np_address selector,np_address base,np_read_fn read,np_pose* out) {
    np_address root,holder,core,context,source,vt,link,state;
    unsigned char flags[3],exported[3];unsigned context_bits;
    if(!base || !np_pointer(read,selector,&root) || !np_pointer(read,selector+8,&context) ||
       !np_pointer(read,selector+16,&holder) || !np_pointer(read,holder+16,&core) ||
       !np_pointer(read,core,&vt) || vt!=base+0x6176d30 ||
       !np_pointer(read,core+0x440,&link) || link!=selector ||
       !np_pointer(read,core+0x3b8,&link) || link!=root ||
       !np_pointer(read,core+0x3c0,&link) || link!=context ||
       !np_pointer(read,core+0x3c8,&source) || !np_pointer(read,root+0x60,&state) ||
       !read(source+0x12,flags,3) || !read(state+0xA3,exported,3) ||
       !read(context+0x13dc,&context_bits,4))return 0;
    if(flags[0]>1 || flags[1]>1 || flags[2]>1 || flags[0]+flags[1]!=1 ||
       flags[0]!=exported[1] || flags[1]!=exported[2] || flags[2]!=exported[0] ||
       flags[2]!=((context_bits>>11)&1))return 0;
    out->selector=selector;out->core=core;out->source=source;out->context=context;
    out->pair[0]=flags[0];out->pair[1]=flags[1];return 1;
}
static int np_pose_restore(const np_pose* saved,np_address selector,np_address base,
                           np_read_fn read,np_write_fn write) {
    np_pose current;
    if(saved->selector!=selector || !np_pose_read(selector,base,read,&current) ||
       saved->core!=current.core || saved->source!=current.source ||
       saved->context!=current.context || saved->pair[0]>1 || saved->pair[1]>1 ||
       saved->pair[0]+saved->pair[1]!=1)return -1;
    if(current.pair[0]==saved->pair[0] && current.pair[1]==saved->pair[1])return 0;
    return write(current.source+0x12,saved->pair,2)?1:-1;
}
#endif
