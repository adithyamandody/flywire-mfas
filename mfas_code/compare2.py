"""Where do two orderings differ? Pure stdlib.

usage: python3 -I compare2.py graph.csv orderA.csv orderB.csv

  * global shape: cut A into 20 equal segments, show where each segment's
    neurons sit in B (median and 10-90% range, as % of the order)
  * concentration: per-neuron "flip weight" = weight of its edges whose
    direction differs between A and B. How few neurons carry half of it?
    What are those neurons like (degree, synapse weight)?
"""
import csv, sys

def read_order(path):
    with open(path, newline="") as f:
        r = csv.reader(f); next(r)
        rows = sorted((int(o), int(n)) for n, o in r)
    return [n for _, n in rows]

g, a, b = sys.argv[1:4]
A = read_order(a); B = read_order(b); n = len(A)
idx = {x: i for i, x in enumerate(A)}
posB = [0] * n
for p, x in enumerate(B): posB[idx[x]] = p

flip = [0] * n; wdeg = [0] * n; deg = [0] * n
gainB = [0] * n          # flipped weight that B gets right and A gets wrong
tot = 0
with open(g, newline="") as f:
    r = csv.reader(f); next(r)
    for s, t, w in r:
        u = idx[int(s)]; v = idx[int(t)]; w = int(w); tot += w
        wdeg[u] += w; wdeg[v] += w; deg[u] += 1; deg[v] += 1
        x = u < v; y = posB[u] < posB[v]
        if x != y:
            flip[u] += w; flip[v] += w
            d = w if y else -w
            gainB[u] += d; gainB[v] += d

print("global shape: A split into 20 segments -> position in B (% of order)")
seg = n // 20
for k in range(20):
    ps = sorted(posB[k * seg:(k + 1) * seg if k < 19 else n])
    q = lambda p: 100 * ps[int(p * (len(ps) - 1))] / n
    print(f"  A {5*k:3d}-{5*k+5:3d}%  ->  B median {q(.5):5.1f}%   10-90% range {q(.1):5.1f} .. {q(.9):5.1f}")

F = sum(flip) // 2
order = sorted(range(n), key=lambda i: -flip[i])
acc = 0; cut = {}
for r_, i in enumerate(order, 1):
    acc += flip[i]
    for frac in (.25, .5, .75):
        if frac not in cut and acc >= frac * 2 * F: cut[frac] = r_
print(f"\nflipped weight {F:,}; neurons carrying 25% / 50% / 75% of it: "
      f"{cut[.25]:,} / {cut[.5]:,} / {cut[.75]:,}  (of {n:,})")

top = order[:cut[.5]]
rest = order[cut[.5]:]
med = lambda xs: sorted(xs)[len(xs) // 2]
print(f"those top neurons: median degree {med([deg[i] for i in top]):,}, "
      f"median synapse weight {med([wdeg[i] for i in top]):,}")
print(f"everyone else:     median degree {med([deg[i] for i in rest]):,}, "
      f"median synapse weight {med([wdeg[i] for i in rest]):,}")
print(f"share of all synapse weight touching the top group: "
      f"{100*sum(wdeg[i] for i in top)/(2*tot):.1f}%")

print("\n20 neurons with most flipped weight:")
print("   flipped   in-B-favour   degree   syn.weight   posA%   posB%")
for i in order[:20]:
    print(f"  {flip[i]:8,}   {gainB[i]:+10,}   {deg[i]:6,}   {wdeg[i]:9,}   "
          f"{100*i/n:5.1f}   {100*posB[i]/n:5.1f}")
