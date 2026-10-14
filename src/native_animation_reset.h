/* Request a native animation rebuild via its named output property.
 * Do not restore board/mirror results or alter transforms/controller input. */
#ifndef SC_NATIVE_ANIMATION_RESET_H
#define SC_NATIVE_ANIMATION_RESET_H
#include "native_pose_property.h"
typedef struct {
    np_frame frame;np_address provider,graph,model,node,page,value;unsigned key;
    unsigned char reset;
} nar_pose;
static int nar_read(np_address selector,np_address base,np_read_fn read,nar_pose* out) {
    if(!np_frame_read(selector,base,read,&out->frame) ||
       !np_pointer(read,out->frame.pose.core+0x630,&out->provider) ||
       !read(out->provider+0x38,&out->graph,8))return 0;
    out->graph&=~4ULL;
    if(out->graph<0x10000 || (out->graph&7) ||
       !np_pointer(read,out->frame.pose.core+0x610,&out->model) ||
       !npp_value(out->graph,0x60,out->model,base,read,&out->node,&out->page,&out->value,&out->key,&out->reset))return 0;
    const char expected[]="Animation/Dingo/Bool.Skater.ResetAnimation";char name[sizeof expected];
    if(!read(out->node+0x88,name,sizeof name))return 0;
    for(unsigned i=0;i<sizeof name;i++)if(name[i]!=expected[i])return 0;
    return 1;
}
static int nar_same(const nar_pose* a,const nar_pose* b) {
    return a->frame.pose.selector==b->frame.pose.selector && a->frame.pose.core==b->frame.pose.core &&
        a->frame.pose.source==b->frame.pose.source && a->frame.pose.context==b->frame.pose.context &&
        a->provider==b->provider && a->graph==b->graph && a->model==b->model &&
        a->node==b->node && a->page==b->page && a->value==b->value && a->key==b->key;
}
static int nar_request(const nar_pose* saved,np_address selector,np_address base,np_read_fn read,np_write_fn write) {
    nar_pose current;unsigned char one=1;
    if(saved->reset!=0 || saved->frame.basis!=0 || saved->frame.native_frame!=1 ||
       saved->frame.pose.pair[0]!=1 || saved->frame.pose.pair[1]!=0)return -2;
    if(!nar_read(selector,base,read,&current))return -1;
    if(!nar_same(saved,&current) || current.reset!=0)return -2;
    if(current.frame.basis==0)return 0;
    if(current.frame.basis!=1 || current.frame.native_frame!=0 || current.frame.pose.pair[0]!=0)return -2;
    return write(current.value,&one,1)?1:-2;
}
static int nar_release(const nar_pose* saved,np_address base,np_read_fn read,np_write_fn write) {
    np_address root,holder,core,vt,link,provider,graph,model,node,page,value;unsigned key;unsigned char reset;
    /* Clear even during transient source/export disagreement, but require the
     * same live actor, provider, descriptor, occupied page and key. */
    if(!np_pointer(read,saved->frame.pose.selector,&root) ||
       !np_pointer(read,saved->frame.pose.selector+16,&holder) || !np_pointer(read,holder+16,&core) ||
       core!=saved->frame.pose.core || !np_pointer(read,core,&vt) || vt!=base+0x6176d30 ||
       !np_pointer(read,core+0x440,&link) || link!=saved->frame.pose.selector ||
       !np_pointer(read,core+0x3b8,&link) || link!=root ||
       !np_pointer(read,core+0x630,&provider) || provider!=saved->provider ||
       !read(provider+0x38,&graph,8) || (graph&~4ULL)!=saved->graph ||
       !np_pointer(read,core+0x610,&model) || model!=saved->model ||
       !npp_value(saved->graph,0x60,model,base,read,&node,&page,&value,&key,&reset) ||
       node!=saved->node || page!=saved->page || value!=saved->value || key!=saved->key)return -1;
    if(!reset)return 0;
    unsigned char zero=0;return write(value,&zero,1)?1:-1;
}
#endif
