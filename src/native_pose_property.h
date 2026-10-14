/* Restore an existing per-rider animation property, never its exported flags.
 * Native bool getter 11dd4c0 and setter 11dd7a0 use the same occupied byte.
 * No allocation, parent/default mutation, virtual calls or repeated forcing. */
#ifndef SC_NATIVE_POSE_PROPERTY_H
#define SC_NATIVE_POSE_PROPERTY_H
#include "native_menu_frame.h"
typedef struct {
    np_frame frame;
    np_address provider,graph,model,node,page,value;
    unsigned key;
    unsigned char mirrored,backwards,goofy;
} npp_pose;
static int npp_value(np_address graph,unsigned offset,np_address model,np_address base,
                     np_read_fn read,np_address* node_out,np_address* page_out,
                     np_address* value_out,unsigned* key_out,unsigned char* value) {
    np_address node,vt,page;unsigned key,bits;
    if(!read(graph+offset,&node,8))return 0;
    node&=~4ULL;
    if(node<0x10000 || (node&7) || !read(node+0x20,&vt,8) || vt!=base+0x61ab030 ||
       !read(node+0x60,&vt,8) || vt!=base+0x61ab080 ||
       !read(node+0x28,&key,4) || !(key>>24) ||
       !np_pointer(read,model+0x48+0x18+(np_address)(key&0xfff)*8,&page))return 0;
    unsigned index=(key>>12)&0x7ff;
    if(!read(page+0x71c+(index>>5)*4,&bits,4) || !(bits&(1u<<(index&31))) ||
       !read(page+index,value,1) || *value>1)return 0;
    *node_out=node;*page_out=page;*value_out=page+index;*key_out=key;return 1;
}
static int npp_read(np_address selector,np_address base,np_read_fn read,npp_pose* out) {
    np_address vt,node,page,value;unsigned key;unsigned char backwards,goofy;
    if(!np_frame_read(selector,base,read,&out->frame) ||
       !np_pointer(read,out->frame.pose.core+0x630,&out->provider) ||
       !read(out->provider+0x28,&out->graph,8))return 0;
    out->graph&=~4ULL;
    if(out->graph<0x10000 || (out->graph&7) ||
       !np_pointer(read,out->frame.pose.core+0x610,&out->model) ||
       !npp_value(out->graph,0x80,out->model,base,read,&out->node,&out->page,&out->value,&out->key,&out->mirrored) ||
       !npp_value(out->graph,0x60,out->model,base,read,&node,&page,&value,&key,&backwards) ||
       !npp_value(out->graph,0x68,out->model,base,read,&node,&page,&value,&key,&goofy))return 0;
    /* Enforce the descriptor identity observed in the supported game build. */
    char name[40];
    static const char expected[]="Animation/Dingo/Bool.Anim.Mirrored";
    if(!read(out->node+0x88,name,sizeof expected))return 0;
    for(unsigned i=0;i<sizeof expected;i++)if(name[i]!=expected[i])return 0;
    if(out->mirrored!=out->frame.pose.pair[0] ||
       (backwards!=out->mirrored)!=out->frame.basis)return 0;
    out->backwards=backwards;out->goofy=goofy;(void)vt;return 1;
}
static int npp_same(const npp_pose* a,const npp_pose* b) {
    return a->frame.pose.selector==b->frame.pose.selector && a->frame.pose.core==b->frame.pose.core &&
        a->frame.pose.source==b->frame.pose.source && a->frame.pose.context==b->frame.pose.context &&
        a->provider==b->provider && a->graph==b->graph && a->model==b->model &&
        a->node==b->node && a->page==b->page && a->value==b->value && a->key==b->key &&
        a->backwards==b->backwards && a->goofy==b->goofy;
}
static int npp_restore(const npp_pose* saved,np_address selector,np_address base,np_read_fn read,np_write_fn write) {
    npp_pose current;
    if(saved->mirrored!=1 || saved->backwards!=1 || saved->goofy!=0 ||
       saved->frame.basis!=0 || saved->frame.native_frame!=1)return -2;
    if(!npp_read(selector,base,read,&current))return -1;
    if(!npp_same(saved,&current))return -2;
    if(current.mirrored==saved->mirrored)return 0;
    if(current.mirrored!=0 || current.frame.basis!=1 || current.frame.native_frame!=0)return -2;
    return write(current.value,&saved->mirrored,1)?1:-2;
}
#endif
