/* Supported 20260929 board-body transform and native grind inputs, read only.
 * Caller validates nr_frame ownership before and after this bounded capture.
 * Inputs are exporter values; numeric contact types are not display names. */
#ifndef SC_GRIND_STEP_READ_H
#define SC_GRIND_STEP_READ_H
typedef struct {
 UINT64 time_us,poll_us,core,context;
 DWORD before,selected,valid,sequence;
 sc_pad raw,out;
 DWORD input_meta[6]; /* enabled, goofy, switched, FrontBackFoot, split, OneFootFlips */
 float board[16]; /* body +20: three axis rows, then world position */
 float inputs[13]; /* context +18e0..1910; see local exporter evidence */
 DWORD flags[12],contact[2]; /* context +13c4..13f3 and +db8/+dbc */
} gs_sample;
static unsigned gs_read(const nr_frame*f,UINT64 base,np_read_fn read,gs_sample*s){
 UINT64 holder,physics,vt,bodies,owner,again;DWORD count;
 if(np_pointer(read,f->core+0x430,&holder)&&np_pointer(read,holder+0x18,&physics)&&
    read(physics,&vt,8)&&vt==base+0x65e67a0&&np_pointer(read,physics+0x20,&bodies)&&
    read(bodies,&count,4)&&count==9&&read(bodies+0x10,&owner,8)&&owner==physics&&
    read(bodies+0x20,s->board,sizeof s->board)&&np_pointer(read,physics+0x20,&again)&&again==bodies&&
    np_pointer(read,holder+0x18,&again)&&again==physics&&np_pointer(read,f->core+0x430,&again)&&again==holder){
  int good=1;for(int i=0;i<16;i++)if(!__builtin_isfinite(s->board[i])||sc_abs(s->board[i])>1000000)good=0;
  for(int row=0;row<3;row++){float n=0;for(int i=0;i<3;i++)n+=s->board[row*4+i]*s->board[row*4+i];if(n<.5f||n>1.5f)good=0;}
  if(good)s->valid|=1;
 }
 if(!(s->valid&1))memset(s->board,0,sizeof s->board);
 if(read(f->context+0x18e0,s->inputs,sizeof s->inputs)){
  int good=1;for(int i=0;i<13;i++)if(!__builtin_isfinite(s->inputs[i])||sc_abs(s->inputs[i])>1000000)good=0;
  if(good)s->valid|=2;
 }
 if(!(s->valid&2))memset(s->inputs,0,sizeof s->inputs);
 if(read(f->context+0x13c4,s->flags,sizeof s->flags))s->valid|=4;else memset(s->flags,0,sizeof s->flags);
 if(read(f->context+0xdb8,s->contact,sizeof s->contact))s->valid|=8;else memset(s->contact,0,sizeof s->contact);
 return s->valid;
}
#endif
