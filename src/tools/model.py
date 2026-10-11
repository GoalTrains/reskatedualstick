import math

def ang(x, y):  # recognizer angle in degrees
    a = math.degrees(math.atan2(x, y))
    return a % 360

def in_arc(t, a, b):
    return a <= t <= b if a <= b else (t >= a or t <= b)

def match_points(path, p):
    k = 0
    for x, y in path:
        px, py = p['pts'][k]
        if math.hypot(x - px, y - py) <= p['tol']:
            k += 1
            if k == len(p['pts']):
                return True
    return False

def match_arcs(path, p):
    k = 0
    for x, y in path:
        m = math.hypot(x, y)
        a = p['arcs'][k]
        mn, mx = a.get('mn', 0.5), a.get('mx', 1.0)
        if mn <= m <= mx + 1e-6 and in_arc(ang(x, y), a['a'], a['b']):
            k += 1
            if k == len(p['arcs']):
                return True
    return False

def matches(path, g):
    for p in g['patterns']:
        if p['mode'] == 0 and p['pts'] and match_points(path, p):
            return True
        if p['mode'] == 1 and p['arcs'] and match_arcs(path, p):
            return True
    return False

