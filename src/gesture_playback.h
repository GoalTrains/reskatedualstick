#ifndef SC_GESTURE_PLAYBACK_H
#define SC_GESTURE_PLAYBACK_H
/* Verified FloatGameStateAsset value interface. Scope is one named property
 * and the local rider's property bank; native originals are always retained. */
typedef struct {np_address asset,vtable,functions[7];} sc_gp_binding;
static int sc_gp_bind(np_address asset,np_address base,np_read_fn read,sc_gp_binding*out){
 static const char name[]="Animation/Dingo/Float.Anim.TrickFlipSpeed";
 static const unsigned methods[6]={0x11dafd4,0x5f1350,0x11dcc4c,0x11dd500,0x11dd7d0,0xe443f0};
 np_address vt,type,n;char text[sizeof name];sc_gp_binding b={0};
 asset&=~4ULL;
 if(!np_pointer(read,asset,&vt) || vt!=base+0x61ab4a8 ||
    !np_pointer(read,asset+8,&type) || type!=base+0x72afbb0 ||
    !np_pointer(read,asset+0x18,&n) || !read(n,text,sizeof text) ||
    !np_pointer(read,asset+0x60,&b.vtable) || b.vtable!=base+0x61ab5a0 ||
    !read(b.vtable-8,b.functions,sizeof b.functions))return 0;
 for(unsigned i=0;i<sizeof name;i++)if(text[i]!=name[i])return 0;
 for(unsigned i=0;i<6;i++)if(b.functions[i+1]!=base+methods[i])return 0;
 b.asset=asset;*out=b;return 1;
}
static float sc_gp_value(float original,float factor,int enabled,np_address reference,
                         np_address asset,np_address bank,np_address local_bank){
 if(!enabled || !asset || reference!=asset+0x60 || !local_bank || bank!=local_bank ||
    !__builtin_isfinite(original) || original<=0 || original>16 ||
    !__builtin_isfinite(factor) || factor<.25f || factor>1.75f)return original;
 return original*factor;
}
#endif
