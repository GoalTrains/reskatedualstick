#ifndef SC_NATIVE_RESPAWN_BOARD_H
#define SC_NATIVE_RESPAWN_BOARD_H
#include "native_menu_respawn.h"
/* ReSkate's set_teleport_on_board uses the component's native choice at +c0.
 * Resolve its local owner afresh; never retain this address across respawns. */
typedef struct {np_address component,core,selector,physics_context;} nr_local_owner;
static int nr_local_owner_read(np_address base,np_address client,np_read_fn read,nr_local_owner* out) {
    np_address context,manager,begin,end,player,entity,collection,component,handle,link;
    unsigned offset;unsigned char local,remote,teleport;
    if(!base || !np_pointer(read,client,&link) || link!=base+0x60842c0 ||
       !np_pointer(read,client+8,&context) || !read(base+0x7161720,&offset,4) || offset>0x1000000 ||
       !np_pointer(read,context+offset,&manager) || !np_pointer(read,manager,&link) || link!=base+0x63e30a0 ||
       !np_pointer(read,manager+0x4c8,&begin) || !np_pointer(read,manager+0x4d0,&end) || end!=begin+8 ||
       !np_pointer(read,begin,&player) || !np_pointer(read,player,&link) || link!=base+0x63e2d30 ||
       !np_pointer(read,player+0x78,&link) || link!=context ||
       !read(player+0x45,&local,1) || local!=1 || !read(player+0x44,&remote,1) || remote ||
       !np_pointer(read,player+0xb8,&entity) || !np_pointer(read,entity,&link) || link!=base+0x6087908 ||
       !np_pointer(read,entity+0xf8,&link) || link!=player || !np_pointer(read,entity+0x20,&link) || link!=context ||
       !read(entity+0x7e0,&teleport,1) || teleport ||
       !np_pointer(read,player+0xb0,&handle) || !np_pointer(read,handle,&link) || link!=entity+8 ||
       !np_pointer(read,entity+0x70,&collection) || !np_pointer(read,collection,&link) || link!=entity ||
       !np_pointer(read,entity+0x628,&component) || !np_pointer(read,component,&link) || link!=base+0x6170410 ||
       !np_pointer(read,component+0x18,&link) || link!=collection ||
       !np_pointer(read,component+0x70,&out->core) || !np_pointer(read,out->core,&link) || link!=base+0x6176d30 ||
       !np_pointer(read,out->core+0x440,&out->selector) || !np_pointer(read,out->core+0x3c0,&out->physics_context) ||
       !np_pointer(read,out->selector+8,&link) || link!=out->physics_context)return 0;
    out->component=component;return 1;
}
static np_address nr_board_choice(np_address base,np_address client,const nr_frame* frame,np_read_fn read) {
    nr_local_owner owner;unsigned char code[8],value;
    const unsigned char consumer[8]={0x44,0x0f,0xb6,0x88,0xc0,0,0,0};
    if(!base || !read(base+0x566898,code,8))return 0;
    for(int i=0;i<8;i++)if(code[i]!=consumer[i])return 0;
    if(!nr_local_owner_read(base,client,read,&owner) || owner.core!=frame->core ||
       owner.selector!=frame->selector || owner.physics_context!=frame->context ||
       !read(owner.component+0xc0,&value,1) || value>1)return 0;
    return owner.component+0xc0;
}
#endif
