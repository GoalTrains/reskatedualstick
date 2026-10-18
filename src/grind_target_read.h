/* Read-only fields used by the supported game's grind-output exporter.
 * core vtable +b0 (RVA feee80) returns core+3b8's root.
 * ff2cb5/ff2d07 export root->20 +b4/+c0 as GrindingType/SkateGrindingType.
 * ff554a/ff55b3 export root->10 +1fb/+c0 as TargetingGrind/TargetGrindDirection.
 * These are native numeric classifications, NOT validated displayed trick names.
 * Caller validates the local core with nr_read; never calls or writes the game.
 */
#ifndef SC_GRIND_TARGET_READ_H
#define SC_GRIND_TARGET_READ_H
typedef struct {
    int grinding_type,skate_grinding_type;
    unsigned targeting_grind,valid;
    float target_direction[3];
} gt_sample;
static unsigned gt_read(np_address core,np_read_fn read,gt_sample*out) {
    np_address root,types,target,again,link;gt_sample s={0};
    if(!np_pointer(read,core+0x3b8,&root)) {*out=s;return 0;}
    if(np_pointer(read,root+0x20,&types) &&
       read(types+0xb4,&s.grinding_type,4) && read(types+0xc0,&s.skate_grinding_type,4) &&
       np_pointer(read,root+0x20,&again) && again==types)s.valid|=1;
    else s.grinding_type=s.skate_grinding_type=0;
    unsigned char active;
    if(np_pointer(read,root+0x10,&target) && read(target+0x1fb,&active,1) && active<=1 &&
       read(target+0xc0,s.target_direction,12) &&
       np_pointer(read,root+0x10,&again) && again==target) {
        int finite=1;
        for(int i=0;i<3;i++)if(!__builtin_isfinite(s.target_direction[i]) ||
            s.target_direction[i]<-100 || s.target_direction[i]>100)finite=0;
        if(finite){s.targeting_grind=active;s.valid|=2;}
    }
    if(!(s.valid&2)){s.targeting_grind=0;for(int i=0;i<3;i++)s.target_direction[i]=0;}
    if(!np_pointer(read,core+0x3b8,&link) || link!=root)s=(gt_sample){0};
    *out=s;return s.valid;
}
#endif
