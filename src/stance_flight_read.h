/* Capture disagreement as data: strict nr_read can reject inconsistent exports,
 * precisely the evidence the recorder must retain. Validate ownership first,
 * then report each field independently with a validity mask. Never write it. */
static unsigned sf_native_read(UINT64 base,UINT64 selector,int* v) {
 UINT64 root,holder,core,context,source,vt,link,state;
 if(!base || !np_pointer(rd_copy,selector,&root) || !np_pointer(rd_copy,selector+8,&context) ||
  !np_pointer(rd_copy,selector+16,&holder) || !np_pointer(rd_copy,holder+16,&core) ||
  !np_pointer(rd_copy,core,&vt) || vt!=base+0x6176d30 ||
  !np_pointer(rd_copy,core+0x440,&link) || link!=selector ||
  !np_pointer(rd_copy,core+0x3b8,&link) || link!=root ||
  !np_pointer(rd_copy,core+0x3c0,&link) || link!=context ||
  !np_pointer(rd_copy,core+0x3c8,&source) || !np_pointer(rd_copy,root+0x60,&state))return 0;
 unsigned mask=0;BYTE b;DWORD bits;const UINT64 addresses[5]={source+0x10,source+0x14,source+0x12,state+0xa3,state+0xa4};
 for(int i=0;i<5;i++)if(rd_copy(addresses[i],&b,1)){v[10+i]=b;mask|=1u<<i;}
 if(rd_copy(context+0x13c4,&bits,4)){v[15]=(bits>>13)&1;mask|=32;}
 float pos[3],forward[3];
 if(rd_copy(core+0x100,pos,12))for(int i=0;i<3;i++)if(__builtin_isfinite(pos[i]) && pos[i]>=-1000000 && pos[i]<=1000000){v[17+i]=(int)(pos[i]*1000);mask|=1u<<(6+i);}
 if(rd_copy(core+0x150,forward,12))for(int i=0;i<3;i++)if(__builtin_isfinite(forward[i]) && forward[i]>=-2 && forward[i]<=2){v[20+i]=(int)(forward[i]*1000);mask|=1u<<(9+i);}
 return mask;
}
