#include "native_menu_state.h"
static volatile int g_native_menu=-1;
/* These functions are checked against ReSkate's supported build contracts.
 * They are called only on the menu worker, never inside controller polling. */
static void native_menu_poll(void){
    static UINT64 base;static int checked,supported,previous=-2;static double next;
    static sc_ui_cache cache;
    double now=ov_now();if(now<next)return;next=now+80;
    if(!checked){
        checked=1;base=(UINT64)GetModuleHandleW(0);
        static const BYTE lp[]={0x48,0x89,0x5c,0x24,0x08,0x57,0x48,0x83,0xec,0x20,0x48,0x8b,0xd9,0xe8,0xfe,0xc9,0xc5,0xff,0x48,0x8b,0x53,0x08,0x48,0x8b,0xf8,0x48,0x3b,0xd0,0x75,0x0f,0xf0,0xff};
        static const BYTE up[]={0xb8,0xff,0xff,0xff,0xff,0xf0,0x0f,0xc1,0x41,0x10,0x83,0xe8,0x01,0x7f,0x25,0x48,0xc7,0x41,0x08,0x00,0x00,0x00,0x00,0x48,0xc7,0xc2,0xc1,0xff,0xff,0xff,0xf0,0x48};
        static const BYTE vp[]={0x40,0x53,0x55,0x56,0x57,0x41,0x56,0x48,0x83,0xec,0x20,0x41,0x0f,0xb6,0xe9,0x41,0x0f,0xb6,0xd8,0x45,0x33,0xc9,0x48,0x8b,0xf9,0xe8,0x52,0x3d,0x00,0x00,0x44,0x0f};
        BYTE bytes[32];supported=base!=0;
        const BYTE* prefixes[]={lp,up,vp};UINT64 offsets[]={0x1933460,0x1933d70,0x19190b0};
        for(int j=0;j<3 && supported;j++){
            if(!rd_copy(base+offsets[j],bytes,32))supported=0;
            else for(int i=0;i<32;i++)if(bytes[i]!=prefixes[j][i])supported=0;
        }
        if(!supported)log_text("native menu detection: build differs; existing pause/menu fallback retained");
    }
    if(!supported)return;
    UINT64 ui=rd_pointer(base+0x71f5ef0),manager=ui?rd_pointer(ui+0x140):0;
    int state=-1;
    if(manager && readable(manager+0xf20,32)){
        typedef void (*Lock)(UINT64);
        ((Lock)(base+0x1933460))(manager+0xf20);
        state=sc_ui_menu_read(manager,rd_copy,(sc_ui_value)(base+0x19190b0),&cache);
        ((Lock)(base+0x1933d70))(manager+0xf20);
    }
    g_native_menu=state;
    if(state<0)next=now+2000;
    if(state!=previous){
        previous=state;
        log_text(state==1?"native game menu: active; stock controller inputs":state==0?"native game menu: closed; gameplay controls permitted":"native game menu: model unavailable; pause/menu fallback");
    }
}
