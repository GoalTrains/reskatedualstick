/* Supported-build, read-only rail-query evidence. No target or pose writes. */
#ifndef SC_QUERY_OBSERVATION_H
#define SC_QUERY_OBSERVATION_H
typedef struct {
 np_address air,query,source;
 unsigned valid,init_enabled,armed,active,contact,profile_type;
 unsigned request_type,flags_c8,flags_d0,flags_dc,selected_active,selected_type,alignment_valid;
 float alignment,request_gate;
 unsigned row_valid[7];float row[7][8];
} qa_snapshot;
static int qa_read(np_address air,np_address context,np_read_fn read,qa_snapshot*out){
 qa_snapshot s={0};np_address link,again,query,source;
 if(!air || !context || !np_pointer(read,air+8,&link) || link!=context ||
    !np_pointer(read,air+0x2c0,&query) || !np_pointer(read,air+0x30,&source) ||
    !np_pointer(read,query+0x98,&link) || link!=context){*out=s;return 0;}
 s.air=air;s.query=query;s.source=source;s.valid=1;
 unsigned char flags[3],b;
 if(read(query+0x180,flags,3)){s.armed=flags[0];s.active=flags[1];s.contact=flags[2];s.valid|=2;}
 if(read(source+0xbb84,&b,1)){s.init_enabled=b;s.valid|=4;}
 if(read(query+0x170,&s.profile_type,4))s.valid|=8;
 if(read(context+0x17d0,&s.request_type,4) && read(context+0x13c8,&s.flags_c8,4) &&
    read(context+0x13d0,&s.flags_d0,4) && read(context+0x13dc,&s.flags_dc,4))s.valid|=16;
 if(read(query+0xad0,&b,1) && read(query+0xad8,&s.selected_type,4)){s.selected_active=b;s.valid|=32;}
 if(read(query+0xb10,&s.alignment,4) && read(query+0xb14,&b,1) && __builtin_isfinite(s.alignment)){s.alignment_valid=b;s.valid|=64;}
 if(read(query+0xaec,&s.request_gate,4) && __builtin_isfinite(s.request_gate))s.valid|=128;
 for(int k=0;k<7;k++){
  np_address row=query+k*20;int index=-1;float* v=s.row[k];
  if(!read(row+0x10,&index,4) || !read(row,v+3,8) || !read(row+8,v+5,4))continue;
  v[0]=(float)index;v[1]=s.request_gate;s.row_valid[k]=1;
  if(index>=0 && index<5 && read(query+0x9a0+index*4,v+2,4) && read(query+0x9b4+index*4,v+6,4)){
   s.row_valid[k]|=2;float distance=v[2];
   if(__builtin_isfinite(distance) && distance>=-.5f && distance<65.5f &&
      read(query+0x9c8+(unsigned)(distance+.5f)*4,v+7,4))s.row_valid[k]|=4;
  }
 }
 if(!np_pointer(read,air+8,&link) || link!=context || !np_pointer(read,air+0x2c0,&again) || again!=query ||
    !np_pointer(read,air+0x30,&again) || again!=source || !np_pointer(read,query+0x98,&link) || link!=context){qa_snapshot zero={0};*out=zero;return 0;}
 *out=s;return 1;
}
#endif
