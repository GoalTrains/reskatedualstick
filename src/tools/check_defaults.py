import json,importlib.util,sys
sp=importlib.util.spec_from_file_location('m',sys.argv[1]+'/model.py'); m=importlib.util.module_from_spec(sp); sp.loader.exec_module(m)
G=json.load(open(sys.argv[1]+'/gestures.json'))
bad=0
for l in open(sys.argv[2]):
    r=json.loads(l); p=[tuple(x) for x in r['path']]
    if r['move'].startswith('manual'): p=p[int(1150/8.333):]
    hits=[n for n in G if m.matches(p,G[n])]
    best=sorted(hits, key=lambda n:-max(pp['prio'] for pp in G[n]['patterns']))
    ok=any(e in hits for e in r['expect'].split('|')); bad+=not ok
    print('OK  ' if ok else 'FAIL', '%-34s'%r['move'], 'fired', '%-18s'%(r.get('fired') or '-'), '->', best[:5])
print(bad,'failure(s)'); sys.exit(bad>0)
