"""Compare many orderings of the same graph at once.

usage: python3 -I divcmp.py graph.csv label=order.csv [label=order.csv ...]

Loads the graph once, then for every pair reports how much weight each one
gets forward that the other does not. Two runs that found the same basin
disagree on nothing; two runs in different basins disagree on a lot even
when they score the same.

Pure stdlib.
"""
import array
import csv
import sys

graph_path = sys.argv[1]
specs = []
for a in sys.argv[2:]:
    label, _, path = a.partition("=")
    specs.append((label, path))

idx = {}
U, V, W = array.array("i"), array.array("i"), array.array("i")
with open(graph_path, newline="") as f:
    r = csv.reader(f)
    next(r)
    for u, v, w in r:
        if u == v:
            continue
        iu = idx.get(u)
        if iu is None:
            iu = idx[u] = len(idx)
        iv = idx.get(v)
        if iv is None:
            iv = idx[v] = len(idx)
        U.append(iu)
        V.append(iv)
        W.append(int(w))
N = len(idx)
E = len(U)
total = sum(W)
print(f"graph: {N:,} nodes, {E:,} edges, {total:,} weight\n")

orders = {}
for label, path in specs:
    pos = array.array("i", bytes(4 * N))
    seen = 0
    with open(path, newline="") as f:
        r = csv.reader(f)
        next(r)
        for nid, o in r:
            i = idx.get(nid)
            if i is not None:
                pos[i] = int(o)
                seen += 1
    if seen != N:
        print(f"!! {label}: {seen:,} of {N:,} nodes present")
    fwd = 0
    for k in range(E):
        if pos[U[k]] < pos[V[k]]:
            fwd += W[k]
    orders[label] = (pos, fwd)
    print(f"{label:>12s}  {fwd:>12,}  {100.0*fwd/total:7.3f}%   {path}")

print()
labels = [l for l, _ in specs]
print(f"{'pair':>26s} {'contested':>12s} {'A only':>10s} {'B only':>10s} {'net':>9s}")
for i in range(len(labels)):
    for j in range(i + 1, len(labels)):
        la, lb = labels[i], labels[j]
        pa, _ = orders[la]
        pb, _ = orders[lb]
        a_only = b_only = 0
        for k in range(E):
            u, v, w = U[k], V[k], W[k]
            fa = pa[u] < pa[v]
            fb = pb[u] < pb[v]
            if fa != fb:
                if fa:
                    a_only += w
                else:
                    b_only += w
        tag = f"{la} vs {lb}"
        note = "  IDENTICAL" if (a_only == 0 and b_only == 0) else ""
        print(f"{tag:>26s} {a_only+b_only:>12,} {a_only:>10,} {b_only:>10,} "
              f"{a_only-b_only:>9,}{note}")
