/* Native report writer: unchanged translated state must preserve all HID bytes. */
static BYTE psn_axis(short v) { int a = 128 + (v >> 8); return (BYTE)(a < 0 ? 0 : a > 255 ? 255 : a); }
static BYTE psn_axis_inv(short v) { int a = 127 - (v >> 8); return (BYTE)(a < 0 ? 0 : a > 255 ? 255 : a); }

static int sc_ps_native_write(BYTE* r,int ds,sc_pad original,sc_pad o){
    if(o.lx==original.lx && o.ly==original.ly && o.rx==original.rx && o.ry==original.ry &&
       o.lt==original.lt && o.rt==original.rt && o.buttons==original.buttons)return 0;
    BYTE* pl2=ds?&r[4]:&r[7];BYTE* pr2=ds?&r[5]:&r[8];
    BYTE* pb0=ds?&r[7]:&r[4];BYTE* pb1=ds?&r[8]:&r[5];
    BYTE in_b1=*pb1,in_l2=*pl2,in_r2=*pr2;
    r[0] = psn_axis(o.lx); r[1] = psn_axis_inv(o.ly); r[2] = psn_axis(o.rx); r[3] = psn_axis_inv(o.ry);
    *pl2 = o.lt; *pr2 = o.rt;
    WORD b = o.buttons; BYTE hat = 8;
    int up = (b & 1) != 0, dn = (b & 2) != 0, lf = (b & 4) != 0, rt = (b & 8) != 0;
    if (up && rt) hat = 1; else if (dn && rt) hat = 3; else if (dn && lf) hat = 5; else if (up && lf) hat = 7;
    else if (up) hat = 0; else if (rt) hat = 2; else if (dn) hat = 4; else if (lf) hat = 6;
    *pb0 = (BYTE)(hat | ((b & 0x4000) ? 0x10 : 0) | ((b & 0x1000) ? 0x20 : 0) | ((b & 0x2000) ? 0x40 : 0) | ((b & 0x8000) ? 0x80 : 0));
    *pb1 = (BYTE)(((b & 0x0100) ? 0x01 : 0) | ((b & 0x0200) ? 0x02 : 0) | (o.lt == in_l2 ? (in_b1 & 0x04) : o.lt > 8 ? 0x04 : 0) | (o.rt == in_r2 ? (in_b1 & 0x08) : o.rt > 8 ? 0x08 : 0) |
                  ((b & 0x0020) ? 0x10 : 0) | ((b & 0x0010) ? 0x20 : 0) | ((b & 0x0040) ? 0x40 : 0) | ((b & 0x0080) ? 0x80 : 0));
    return 1;
}