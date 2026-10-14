#ifndef SC_MENU_SCHEME_H
#define SC_MENU_SCHEME_H
/* Old INIs remain legacy choices until the user selects a complete layout. */
static int sc_menu_layout_kind(int front_back,int mode){return front_back?0:mode==3?1:2;}
static void sc_menu_choose_layout(int kind,int *front_back,int *mode){*front_back=kind==0;*mode=kind==0?1:3;}
static int sc_menu_next_layout(int kind,int direction){return kind==2?(direction<0?0:1):!kind;}
static int sc_menu_input_layout_kind(int front_back,int mode,int xl){return front_back?0:mode!=3?3:xl?2:1;}
static int sc_menu_next_input_layout(int kind,int direction){return kind==3?(direction<0?0:1):(kind+(direction<0?2:1))%3;}
static void sc_menu_choose_input_layout(int kind,int* front_back,int* mode,int* xl){sc_menu_choose_layout(kind==0?0:1,front_back,mode);*xl=kind==2;}
/* XL reverses canonical vertical foot directions only in switch, so the
 * physical sticks retain their normal-stance setup direction. */
static int sc_menu_xl_card_y(int xl,int switched){return xl && switched;}
static int sc_menu_left_front(int front_back,int goofy,int mode,int switched,int swapped){
 int left=front_back?1:!goofy;if(swapped)left=!left;if(!front_back && mode==3 && switched)left=!left;return left;
}
static int sc_menu_mirror_x(int front_back,int goofy,int flip_x){return ((!front_back && goofy)!=!!flip_x);}
#endif
