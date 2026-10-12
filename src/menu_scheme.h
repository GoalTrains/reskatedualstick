#ifndef SC_MENU_SCHEME_H
#define SC_MENU_SCHEME_H
/* Old INIs remain legacy choices until the user selects a complete layout. */
static int sc_menu_layout_kind(int front_back,int mode){return front_back?0:mode==3?1:2;}
static void sc_menu_choose_layout(int kind,int *front_back,int *mode){*front_back=kind==0;*mode=kind==0?1:3;}
static int sc_menu_next_layout(int kind,int direction){return kind==2?(direction<0?0:1):!kind;}
static int sc_menu_left_front(int front_back,int goofy,int mode,int switched,int swapped){
 int left=front_back?1:!goofy;if(swapped)left=!left;if(!front_back && mode==3 && switched)left=!left;return left;
}
/* Preserve established normal-stance sideways diagrams. True Foot changes
 * physical stick ownership without adding another regular switch mirror. */
static int sc_menu_mirror_x(int front_back,int goofy,int flip_x){return ((!front_back && goofy)!=!!flip_x);}
#endif
