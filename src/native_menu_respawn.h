#ifndef SC_NATIVE_MENU_RESPAWN_H
#define SC_NATIVE_MENU_RESPAWN_H
#include "native_pose_restore.h"
typedef struct {
    np_address selector,core,context,source;
    unsigned char basis,native_frame,mirrored;
    float position[3],forward[3];
} nr_frame;
static int nr_read(np_address selector,np_address base,np_read_fn read,nr_frame* out) {
    np_address root,holder,vt,link,state;unsigned bits;unsigned char exported;
    if(!base || !np_pointer(read,selector,&root) || !np_pointer(read,selector+8,&out->context) ||
       !np_pointer(read,selector+16,&holder) || !np_pointer(read,holder+16,&out->core) ||
       !np_pointer(read,out->core,&vt) || vt!=base+0x6176d30 ||
       !np_pointer(read,out->core+0x440,&link) || link!=selector ||
       !np_pointer(read,out->core+0x3b8,&link) || link!=root ||
       !np_pointer(read,out->core+0x3c0,&link) || link!=out->context ||
       !np_pointer(read,out->core+0x3c8,&out->source) || !np_pointer(read,root+0x60,&state) ||
       !read(out->source+0x10,&out->basis,1) || out->basis>1 ||
       !read(out->source+0x14,&out->native_frame,1) || out->native_frame>1 ||
       !read(out->source+0x12,&out->mirrored,1) || out->mirrored>1 ||
       !read(state+0xa4,&exported,1) || exported!=out->mirrored ||
       !read(out->context+0x13c4,&bits,4) || out->basis!=((bits>>13)&1) ||
       !read(state+0xa3,&exported,1) || exported!=out->native_frame ||
       !read(out->core+0x100,out->position,12) || !read(out->core+0x150,out->forward,12))return 0;
    for(int i=0;i<3;i++)if(!__builtin_isfinite(out->position[i]) || out->position[i]<-1000000 || out->position[i]>1000000 ||
       !__builtin_isfinite(out->forward[i]))return 0;
    float length=out->forward[0]*out->forward[0]+out->forward[2]*out->forward[2];
    if(length<.25f || length>1.5f)return 0;
    out->selector=selector;return 1;
}
static int nr_same(const nr_frame* a,const nr_frame* b) {
    return a->selector==b->selector && a->core==b->core && a->context==b->context && a->source==b->source;
}
static int nr_mismatch(const nr_frame* before,const nr_frame* after) {
    return nr_same(before,after) && before->native_frame==1 && after->native_frame==0 && before->basis!=after->basis;
}
static int nr_transform(const nr_frame* current,const float* last_position,double elapsed,int entry_frame,float* matrix) {
    float x=current->forward[0],z=current->forward[2];
    if(entry_frame){x=-x;z=-z;}
    if(last_position && elapsed>=40 && elapsed<=250) {
        float dx=current->position[0]-last_position[0],dz=current->position[2]-last_position[2];
        float length=dx*dx+dz*dz;
        if(__builtin_isfinite(length) && length>.0025f && length<64){x=dx;z=dz;}
    }
    float length=x*x+z*z;
    if(!__builtin_isfinite(length) || length<.01f)return 0;
    float scale=1/__builtin_sqrtf(length);x*=scale;z*=scale;
    for(int i=0;i<16;i++)matrix[i]=0;
    matrix[0]=z;matrix[2]=-x;matrix[5]=1;matrix[8]=x;matrix[10]=z;
    matrix[12]=current->position[0];matrix[13]=current->position[1]+.05f;matrix[14]=current->position[2];matrix[15]=1;
    return 1;
}
#endif
