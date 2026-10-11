#ifndef SC_MENU_CONTROLS_H
#define SC_MENU_CONTROLS_H
/* Called with new presses only. Holding a bumper never repeats or reaches Skate. */
static int sc_controls_bumper_direction(unsigned down) {
    unsigned shoulders=down & 0x0300;
    return shoulders==0x0100 ? -1 : shoulders==0x0200 ? 1 : 0;
}
static int sc_controls_next_page(int page,int direction,int count,int per_page) {
    if(count<=0 || per_page<=0)return 0;
    int pages=(count+per_page-1)/per_page;
    return (page+(direction<0?-1:direction>0?1:0)+pages)%pages;
}
static int sc_menu_fit_scale(int requested,int width,int height,int base_width,int base_height) {
    int fit_w=(width-80)*100/base_width,fit_h=(height-80)*100/base_height;
    if(requested>fit_w)requested=fit_w;
    if(requested>fit_h)requested=fit_h;
    return requested<25?25:requested;
}
#endif
