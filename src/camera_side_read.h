/* Read-only camera side probe for the supported 20260929 build.
 * Camera view ownership/layout comes from the verified reSkate view reader.
 * Unknown, ambiguous, free-camera or near-centred views do not toggle. */
#ifndef SC_CAMERA_SIDE_READ_H
#define SC_CAMERA_SIDE_READ_H
#include "native_pose_restore.h"
static int sc_camera_side_geometry(const float* position,const float* forward,const float* camera) {
    for(int i=0;i<3;i++)if(!__builtin_isfinite(position[i]) || !__builtin_isfinite(forward[i]) || !__builtin_isfinite(camera[i]) ||
        position[i]<-1000000 || position[i]>1000000 || camera[i]<-1000000 || camera[i]>1000000)return 0;
    float fx=forward[0],fz=forward[2],len=fx*fx+fz*fz;
    float dx=camera[0]-position[0],dz=camera[2]-position[2],distance=dx*dx+dz*dz;
    if(len<.5f || len>1.5f || distance<.25f || distance>400)return 0;
    /* Resolve the board's bidirectional longitudinal axis away from the camera.
     * Switch stance may reverse that axis without changing the viewed side. */
    float rear=dx*fx+dz*fz;
    if(rear*rear<.25f*distance*len)return 0;
    if(rear>0){fx=-fx;fz=-fz;}
    float side=dx*fz-dz*fx;
    if(side*side<.01f*len)return 0;
    return side>0?1:-1;
}
static int sc_camera_side_read(np_address base,np_address selector,np_read_fn read) {
    np_address manager,vt,registered,begin,end,view=0,camera,root,holder,core,context,link;
    if(!base || !np_pointer(read,base+0x75cbc48,&manager) || !np_pointer(read,base+0x75dd288,&registered) || manager!=registered ||
       !read(manager,&vt,8) || vt!=base+0x63ece68 || !np_pointer(read,manager+8,&begin) || !np_pointer(read,manager+16,&end) ||
       end<begin || (end-begin)%8 || (end-begin)/8>32)return 0;
    for(np_address at=begin;at<end;at+=8) {
        np_address item;unsigned id[2];
        if(!np_pointer(read,at,&item) || !read(item,&vt,8) || vt!=base+0x63ed0a0 || !read(item+0x148,id,8))return 0;
        if(!id[0] && !id[1]){if(view)return 0;view=item;}
    }
    unsigned char enabled;
    if(!view || !read(view+0x45d,&enabled,1) || enabled>1 || !np_pointer(read,view+0x2b0,&camera) ||
       !read(camera,&vt,8) || vt!=base+0x6255870 ||
       !np_pointer(read,selector,&root) || !np_pointer(read,selector+8,&context) || !np_pointer(read,selector+16,&holder) ||
       !np_pointer(read,holder+16,&core) || !read(core,&vt,8) || vt!=base+0x6176d30 ||
       !read(core+0x440,&link,8) || link!=selector || !read(core+0x3b8,&link,8) || link!=root ||
       !read(core+0x3c0,&link,8) || link!=context)return 0;
    float position[3],forward[3],camera_position[3];
    if(!read(core+0x100,position,12) || !read(core+0x150,forward,12) || !read(camera+0x80,camera_position,12))return 0;
    np_address again;
    if(!read(view+0x2b0,&again,8) || again!=camera || !read(manager+8,&again,8) || again!=begin || !read(manager+16,&again,8) || again!=end)return 0;
    return sc_camera_side_geometry(position,forward,camera_position);
}
#endif
