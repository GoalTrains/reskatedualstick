/* Read-only preflight for native selector 47f7b90, target types 1/2 only.
 * Checked 20260929 binary; preserve its inclusive boundaries. No candidate,
 * alignment, threshold or physics writes. Invalid/incomplete reads fail closed. */
#ifndef SC_GRIND_CANDIDATE_H
#define SC_GRIND_CANDIDATE_H
static int sc_candidate_accepts(np_address object,int type,np_read_fn read){
 if(type!=1 && type!=2)return 0;
 int index=-1;float gate,distance,score,maximum,angle,wanted,tolerance;
 np_address row=object+(unsigned)type*20;
 if(!read(row+16,&index,4) || index<0 || index>=5 ||
    !read(object+0xaec,&gate,4) || !read(row+8,&maximum,4) ||
    !read(row,&wanted,4) || !read(row+4,&tolerance,4) ||
    !read(object+0x9a0+index*4,&distance,4) || !read(object+0x9b4+index*4,&score,4))return 0;
 if(!__builtin_isfinite(gate) || !__builtin_isfinite(distance) || !__builtin_isfinite(score) ||
    !__builtin_isfinite(maximum) || !__builtin_isfinite(wanted) || !__builtin_isfinite(tolerance) ||
    gate>=5 || distance<=0 || distance>=65.5f || sc_abs(gate-distance)>3 || score>maximum)return 0;
 int angle_index=(int)(distance+.5f);
 if(angle_index<0 || angle_index>=66 || !read(object+0x9c8+angle_index*4,&angle,4) ||
    !__builtin_isfinite(angle))return 0;
 return sc_abs(wanted-sc_abs(angle))<=tolerance;
}
#endif
