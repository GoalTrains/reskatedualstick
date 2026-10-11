/* Shared HID framing for direct and native PlayStation paths.
 * Never select the short Bluetooth layout merely because a USB read is long.
 * A read may contain consecutive USB reports or zero padding. Validate the
 * whole framing before any report is decoded or modified. */
typedef struct { unsigned size, count, offset; int dualsense, crc; } sc_ps_frames;
static sc_ps_frames sc_ps_frame(const unsigned char* b, unsigned n, int kind) {
    sc_ps_frames f={0};
    if(!b || n<10 || (kind!=1 && kind!=2))return f;
    if(b[0]==1 && n==10) {f.size=10;f.count=1;f.offset=1;return f;}
    if(b[0]==1 && n>=64 && n<=4096 && n%64==0) {
        int batch=1,padding=1;
        for(unsigned i=64;i<n;i++) {
            if(i%64==0 && b[i]!=1)batch=0;
            if(b[i])padding=0;
        }
        if(!batch && !padding)return f;
        f.size=64;f.count=padding?1:n/64;f.offset=1;f.dualsense=kind==1;return f;
    }
    if(n==78 && ((kind==1 && b[0]==0x31) || (kind==2 && b[0]==0x11))) {
        f.size=78;f.count=1;f.offset=kind==1?2:3;f.dualsense=kind==1;f.crc=1;
    }
    return f;
}
static short sc_ps_axis(unsigned char v) {return (short)(((int)v-128)*256);}
static short sc_ps_axis_inv(unsigned char v) {return (short)((127-(int)v)*256);}
static sc_pad sc_ps_decode(const unsigned char* r,int ds) {
    unsigned char l2=ds?r[4]:r[7],r2=ds?r[5]:r[8];
    unsigned char b0=ds?r[7]:r[4],b1=ds?r[8]:r[5];
    static const unsigned short hat[8]={1,9,8,10,2,6,4,5};
    unsigned short bt=(b0&15)<8?hat[b0&15]:0;
    if(b0&0x10)bt|=0x4000;if(b0&0x20)bt|=0x1000;
    if(b0&0x40)bt|=0x2000;if(b0&0x80)bt|=0x8000;
    if(b1&1)bt|=0x100;if(b1&2)bt|=0x200;
    if(b1&0x10)bt|=0x20;if(b1&0x20)bt|=0x10;
    if(b1&0x40)bt|=0x40;if(b1&0x80)bt|=0x80;
    sc_pad p={0};p.buttons=bt;p.lt=l2;p.rt=r2;
    p.lx=sc_ps_axis(r[0]);p.ly=sc_ps_axis_inv(r[1]);
    p.rx=sc_ps_axis(r[2]);p.ry=sc_ps_axis_inv(r[3]);return p;
}
