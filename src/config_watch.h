#ifndef SC_CONFIG_WATCH_H
#define SC_CONFIG_WATCH_H
typedef struct {int seen,stance;unsigned long long written,size;} sc_config_watch;
/* Failed metadata reads retry the full loader. A new native stance must still
 * reach AutoStance even when the INI itself has not changed. */
static int sc_config_needs_reload(sc_config_watch*s,int valid,unsigned long long written,
                                  unsigned long long size,int stance){
 if(!valid){s->seen=0;return 1;}
 int changed=!s->seen || s->written!=written || s->size!=size || s->stance!=stance;
 s->seen=1;s->written=written;s->size=size;s->stance=stance;return changed;
}
#endif
