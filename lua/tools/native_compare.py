from pathlib import Path
import subprocess,sys,json
src=Path(sys.argv[1]).resolve();out=Path(sys.argv[2]).resolve();clang=sys.argv[3];out.mkdir(exist_ok=True)
selected=[p.stem for p in (out.parent/'test').glob('*.luau')]
for name in selected:
 text=(src/'test'/(name+'.c')).read_text().replace('#include "../src/translate.h"','')
 wrapper='#include <stdio.h>\n#include "'+(src/'src/translate.h').as_posix()+'"\n'
 wrapper+='static sc_pad audit_translate(const sc_config*c,sc_state*s,sc_pad p,double now){sc_pad o=sc_translate(c,s,p,now);printf("VECTOR %d %d %d %d %d %d %d %d %d %d %d %d\\n",o.buttons,o.lt,o.rt,o.lx,o.ly,o.rx,o.ry,s->fired,s->grind_pop,s->d_land_hold,s->sw_lat,s->pump_r3);return o;}\n#define sc_translate audit_translate\n'
 text=text.replace('../src/',(src/'src').as_posix()+'/')
 path=out/(name+'.c');path.write_text(wrapper+text)
 exe=out/(name+'.exe')
 r=subprocess.run([clang,'-std=c11','-O0',str(path),'-o',str(exe)],capture_output=True,text=True)
 if r.returncode:raise RuntimeError(r.stderr)
 r=subprocess.run([str(exe)],capture_output=True,text=True,cwd=src)
 if r.returncode:raise RuntimeError(name+' '+r.stdout+r.stderr)
 vectors=[[int(x) for x in line.split()[1:]] for line in r.stdout.splitlines() if line.startswith('VECTOR ')]
 (out/(name+'.json')).write_text(json.dumps(vectors))
 print(name,len(vectors))
