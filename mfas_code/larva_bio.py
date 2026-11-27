"""Does the larval ordering recover the same axis as the adult ones?

usage: python3 -I larva_bio.py larva_graph.csv larva_order.csv larva_ann.tsv

Reports, by cell type: where its neurons sit in the ordering, and how much
of its synaptic weight the ordering has to turn around. The adult result to
compare against is sensory early, descending neurons last, and the mushroom
body feedback cells carrying far more than their share of backward weight.

Pure stdlib.
"""
import csv
import sys

graph_path, order_path, ann_path = sys.argv[1], sys.argv[2], sys.argv[3]

pos = {}
with open(order_path, newline="") as f:
    r = csv.reader(f)
    next(r)
    for nid, o in r:
        pos[nid] = int(o)
N = len(pos)

ctype = {}
with open(ann_path, newline="", encoding="utf-8") as f:
    r = csv.reader(f, delimiter="\t")
    next(r)
    for row in r:
        if len(row) > 3 and row[3].strip():
            ctype[row[0]] = row[3].strip()

tot_w = back_w = 0
by_type_tot = {}
by_type_back = {}
neuron_back = {}
with open(graph_path, newline="") as f:
    r = csv.reader(f)
    next(r)
    for u, v, w in r:
        if u == v:
            continue
        w = int(w)
        tot_w += w
        backward = pos[u] >= pos[v]
        if backward:
            back_w += w
            neuron_back[u] = neuron_back.get(u, 0) + w
            neuron_back[v] = neuron_back.get(v, 0) + w
        for n in (u, v):
            t = ctype.get(n)
            if t:
                by_type_tot[t] = by_type_tot.get(t, 0) + w
                if backward:
                    by_type_back[t] = by_type_back.get(t, 0) + w

base = back_w / tot_w
print(f"neurons {N:,}   total weight {tot_w:,}   "
      f"backward {back_w:,} ({100*base:.2f}%)\n")


def quart(v):
    v.sort()
    n = len(v)
    return v[n // 4], v[n // 2], v[(3 * n) // 4]


groups = {}
for n, t in ctype.items():
    if n in pos:
        groups.setdefault(t, []).append(100.0 * pos[n] / (N - 1))

print(f"{'cell type':<14s} {'n':>5s} {'25%':>7s} {'median':>7s} {'75%':>7s}")
for t, v in sorted(groups.items(), key=lambda kv: quart(kv[1][:])[1]):
    if len(v) < 20:
        continue
    lo, md, hi = quart(v[:])
    print(f"{t:<14s} {len(v):>5,} {lo:>6.1f}% {md:>6.1f}% {hi:>6.1f}%")

print(f"\n{'cell type':<14s} {'weight':>10s} {'backward':>9s} {'vs brain':>9s}")
rows = []
for t, tw in by_type_tot.items():
    if tw < 5000:
        continue
    bw = by_type_back.get(t, 0)
    rows.append((bw / tw, t, tw, bw))
for rate, t, tw, bw in sorted(rows, reverse=True):
    print(f"{t:<14s} {tw:>10,} {100*rate:>8.1f}% {rate/base:>8.2f}x")

print("\ntop 10 neurons by backward weight:")
top = sorted(neuron_back.items(), key=lambda kv: -kv[1])[:10]
for n, w in top:
    print(f"  {w:>7,}  node {n:<6s} {ctype.get(n, '?'):<14s} "
          f"at {100.0*pos[n]/(N-1):.1f}%")
