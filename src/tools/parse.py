import re, json, glob, os, sys
out = {}
for f in sorted(glob.glob(sys.argv[1] + '/gesture_fliptrick_*.txt')):
    t = open(f).read()
    name = os.path.basename(f)[len('gesture_fliptrick_'):-4]
    nollie = 'IsNollie = true' in t
    pats = []
    body = t.split('Patterns [',1)[1].split('NameHash',1)[0]
    for chunk in re.split(r'\n    \[\d+\] \{', body)[1:]:
        mode = int(re.search(r'Mode = (\d)', chunk).group(1))
        pts = [(float(x), float(y)) for x, y in re.findall(r'x = (-?[\d.e-]+)\s+y = (-?[\d.e-]+)', chunk)]
        tol = float(re.search(r'DistTolerance = ([\d.]+)', chunk).group(1))
        prio = int(re.search(r'Priority = (\d)', chunk).group(1))
        arcs = [dict(a=float(a), b=float(b), mn=float(mn), mx=float(mx)) for mn, mx, a, b in
                re.findall(r'MinMagnitude = ([\d.]+)\s+MaxMagnitude = ([\d.]+)\s+StartAngle = ([\d.]+)\s+EndAngle = ([\d.]+)', chunk)]
        if not arcs:
            arcs = [dict(a=float(a), b=float(b)) for a, b in re.findall(r'StartAngle = ([\d.]+)\s+EndAngle = ([\d.]+)', chunk)]
        pats.append(dict(mode=mode, pts=pts, tol=tol, prio=prio, arcs=arcs))
    anim = re.search(r'AntEnumeration.*?Value = (\d+)', t, re.S)
    out[name] = dict(nollie=nollie, anim=int(anim.group(1)) if anim else None, patterns=pats)
json.dump(out, open(sys.argv[2], 'w'), indent=1)
for n, g in out.items():
    pm = [p for p in g['patterns'] if p['mode'] == 0]
    am = [p for p in g['patterns'] if p['mode'] == 1]
    first = ' '.join('(%.2f,%.2f)' % p for p in pm[0]['pts']) if pm else ''
    arcs = ' | '.join('/'.join('%g-%g' % (a['a'], a['b']) for a in p['arcs']) for p in am)
    print('%-22s anim=%-3s N=%d pts=%d arcs=%d  %s  ARCS: %s' % (n, g['anim'], g['nollie'], len(pm), len(am), first, arcs))
