#include <assert.h>
#include <stdio.h>
#include "../src/config_watch.h"
int main(void){
 sc_config_watch s={0};
 assert(sc_config_needs_reload(&s,1,100,200,0));
 for(int i=0;i<1000;i++)assert(!sc_config_needs_reload(&s,1,100,200,0));
 assert(sc_config_needs_reload(&s,1,101,200,0)); /* same-size edit */
 assert(sc_config_needs_reload(&s,1,101,201,0)); /* size edit */
 assert(sc_config_needs_reload(&s,1,101,201,1)); /* AUTO stance */
 assert(!sc_config_needs_reload(&s,1,101,201,1));
 assert(sc_config_needs_reload(&s,1,101,201,-1)); /* lost native source */
 assert(sc_config_needs_reload(&s,0,0,0,-1)); /* deleted/inaccessible */
 assert(sc_config_needs_reload(&s,0,0,0,-1));
 assert(sc_config_needs_reload(&s,1,101,201,-1)); /* recreated */
 assert(!sc_config_needs_reload(&s,1,101,201,-1));
 puts("PASS: unchanged INI avoids profile rereads; same-size/size edits, native AutoStance changes, missing/recreated file and metadata failures reload");
}
