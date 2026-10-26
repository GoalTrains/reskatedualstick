from pathlib import Path
import re,subprocess,os
P=Path(__file__).resolve().parent
S=P.parents[1]/'src'
def own(name): return re.sub(r'^#include[^\n]*','', (S/name).read_text(),flags=re.M)
def function(name,s):
 a=s.index('static int '+name+'(');b=s.index('{',a);depth=1;i=b+1
 while depth:
  depth+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[a:i]+'\n'
stub='typedef struct { const char* name; int anim; int nollie; int count; float xy[24][2]; } sc_trick;\n#define SC_TRICK_COUNT 64\nstatic const sc_trick sc_tricks[64] = {0};\n'
frame='typedef unsigned long long np_address;\ntypedef struct {np_address selector,core,context,source;unsigned char basis,native_frame,mirrored;float position[3],forward[3];} nr_frame;\nstatic int nr_same(const nr_frame*a,const nr_frame*b){return a->selector==b->selector && a->core==b->core && a->context==b->context && a->source==b->source;}\n'
text=stub+own('physics_states.h')+own('translate.h')+own('manual_catch.h')+own('camera_dpad.h')+function('sc_camera_side_geometry',(S/'camera_side_read.h').read_text())+own('stance_track.h')+frame+own('xl_stance_agreement.h')+own('motion_stance.h')+own('fakie_input_guard.h')+own('grind_entry_plan.h')
(P/'source.c').write_text(text)
subprocess.run([os.environ.get('CLANG','clang'),'-E','-P','-x','c',str(P/'source.c'),'-o',str(P/'source.pp.c')],check=True)
print('Built pure v6.6 state source without game gesture data or native readers')
