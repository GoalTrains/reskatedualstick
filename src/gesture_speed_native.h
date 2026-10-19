/* Named, typed EATFlipTrick tuning only. Never scan the heap on an input poll. */
static sc_gesture_speed gs_input[4];
static UINT64 gs_asset,gs_curves[2],gs_points[2],gs_next,gs_identity_next;
static float gs_original[2][3][3],gs_written=1.f;
static float gs_last[2][3][3];
static unsigned gs_domain;
static int gs_busy,gs_reported,gs_dirty;
static int gs_equal(float a,float b){float d=a-b;return d>-.0002f && d<.0002f;}
static int gs_identity(UINT64 base){
 UINT64 vt,type,patch,fields,box,curve,points;DWORD n,hash;
 if(!gs_asset || !rd_copy(gs_asset,&vt,8) || vt!=base+0x65cf308 ||
    !rd_copy(gs_asset+8,&type,8) || type!=base+0x777e8b0 ||
    !np_pointer(rd_copy,gs_asset+0x40,&patch) ||
    !np_pointer(rd_copy,patch+0x28,&fields) || !rd_copy(fields-4,&n,4) || (n&0x7fffffff)!=8)return 0;
 unsigned found=0;
 for(unsigned i=0;i<8;i++){
  if(!rd_copy(fields+24*i+16,&hash,4))return 0;
  int index=hash==2799703042U?0:hash==4106527452U?1:-1;
  if(index<0)continue;
  if(!rd_copy(fields+24*i,&type,8) || type!=base+0x76de278 ||
     !np_pointer(rd_copy,fields+24*i+8,&box) || !rd_copy(box,&curve,8))return 0;
  curve&=~4ULL;
  if(!rd_copy(curve,&vt,8) || vt!=base+0x6281558 ||
     !rd_copy(curve+8,&type,8) || type!=base+0x748d020 ||
     !rd_copy(curve+0x18,&points,8) || points<0x10004 || points>=0x7fffffffffffULL-84 || (points&3) ||
     !rd_copy(points-4,&n,4) || (n&0x7fffffff)!=3)return 0;
  if(gs_curves[index] && (gs_curves[index]!=curve || gs_points[index]!=points))return 0;
  gs_curves[index]=curve;gs_points[index]=points;found++;
 }
 return found==2;
}
static int gs_capture(UINT64 base){
 if(!gs_identity(base))return 0;
 static const float x[2][3]={{0,.3f,.9f},{.35f,.82f,.98f}};
 static const float y[2][3]={{.4f,.4f,1.4f},{.65f,.7f,1.f}};
 for(unsigned c=0;c<2;c++)for(unsigned i=0;i<3;i++){
  float actual;
  if(!rd_copy(gs_points[c]+28*i+12,&actual,4) || !gs_equal(actual,x[c][i]) ||
     !rd_copy(gs_points[c]+28*i+20,&actual,4) || !gs_equal(actual,y[c][i]))return 0;
  for(unsigned j=0;j<3;j++){
   if(!rd_copy(gs_points[c]+28*i+(j==0?4:j==1?16:20),&gs_original[c][i][j],4))return 0;
   gs_last[c][i][j]=gs_original[c][i][j];
  }
 }
 gs_written=1.f;return 1;
}
static int gs_apply(UINT64 base,float factor){
 if(!gs_identity(base))return 0;
 /* Validate every owned ordinate before any write; external tuning wins. */
 gs_dirty=1;
 for(unsigned c=0;c<2;c++)for(unsigned i=0;i<3;i++)for(unsigned j=0;j<3;j++){
  float have;UINT64 a=gs_points[c]+28*i+(j==0?4:j==1?16:20);
  if(!rd_copy(a,&have,4) || !gs_equal(have,gs_last[c][i][j]))return 0;
 }
 for(unsigned c=0;c<2;c++)for(unsigned i=0;i<3;i++)for(unsigned j=0;j<3;j++){
  float value=gs_original[c][i][j]*factor;
  if(!cha_write(gs_points[c]+28*i+(j==0?4:j==1?16:20),&value,4))return 0;
  gs_last[c][i][j]=value;
 }
 gs_written=factor;gs_dirty=0;return 1;
}
static void gs_sync(UINT64 now,float factor,int enabled){
 if(__sync_lock_test_and_set(&gs_busy,1))return;
 UINT64 base=np_base();
 if(!base)goto done;
 if(gs_asset){
  if(now>=gs_identity_next && !gs_identity(base)){
   gs_asset=0;gs_curves[0]=gs_curves[1]=gs_points[0]=gs_points[1]=0;
   gs_dirty=0;gs_written=1.f;gs_next=now+2000;
   log_text("gesture speed: native tuning unloaded/replaced; waiting for typed rediscovery");
   goto done;
  }
  if(now>=gs_identity_next)gs_identity_next=now+1000;
  if(!enabled)factor=1.f;
  if((gs_dirty || !gs_equal(factor,gs_written)) && !gs_apply(base,factor)){
   /* Refuse stale/replaced assets or another mod's curve edits. */
   if(gs_reported!=2){log_text("gesture speed: tuning identity/ownership changed; speed writes suspended");gs_reported=2;}
  }
  goto done;
 }
 if(!enabled || now<gs_next || !cha_contract(base))goto done;
 /* One occupied-domain lookup per poll; never a whole pass in one poll. */
 for(unsigned skipped=0;skipped<0xbbf;skipped++){
  UINT64 owner=0;unsigned domain=gs_domain++;
  if(gs_domain>=0xbbf){gs_domain=0;gs_next=now+10000;}
  if(rd_copy(base+0x7726d10+8ULL*domain,&owner,8) && owner){
   gs_asset=((cha_find_fn)(base+0x41b5800))((WORD)domain,"gameplay/skater/tuning/eatfliptricktuningcomponent_schematics");
   gs_asset&=~4ULL;
   if(gs_asset){
    if(gs_capture(base)){gs_reported=1;log_text("gesture speed: verified native flip speed curves; physical velocity control ready");}
    else{gs_asset=0;gs_curves[0]=gs_curves[1]=0;gs_points[0]=gs_points[1]=0;}
   }
   break;
  }
  if(gs_domain==0)break;
 }
 done:__sync_lock_release(&gs_busy);
}
