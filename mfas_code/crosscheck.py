"""Do the two connectomes agree on WHICH CELL TYPES carry the feedback?

usage: python3 -I crosscheck.py fw_graph.csv fw_order.csv annotations.tsv \
                                hb_graph.csv hb_order.csv traced-neurons.csv

Two orderings, each produced from its own connectome with no biological
input and no knowledge of the other. For every cell type present in both,
compare how much of its synaptic weight the ordering has to turn around.
Names are aligned through FlyWire's `hemibrain_type` column; only FlyWire
neurons matched to exactly one hemibrain type are used.

Reported with Spearman rank correlation, plus a size control: cell types
with more synapses carry more of everything, so the rate (backward weight
as a share of that type's own weight) is the test that size cannot explain.
"""
import csv, sys

fw_graph, fw_order, annot, hb_graph, hb_order, hb_neurons = sys.argv[1:7]

def load_order(p):
    pos = {}
    with open(p, newline="") as f:
        r = csv.reader(f); next(r)
        for n, o in r: pos[int(n)] = int(o)
    return pos

def weights_by_type(graph, pos, type_of):
    """total and backward weight per cell type; an edge counts for the type
    of its source and the type of its target (so a type is credited for the
    feedback it sends and the feedback it receives, as in BIOLOGY.md)."""
    tot, back = {}, {}
    with open(graph, newline="") as f:
        r = csv.reader(f); next(r)
        for s, t, w in r:
            s = int(s); t = int(t); w = int(w)
            if s not in pos or t not in pos: continue
            bw = pos[s] > pos[t]
            for n in (s, t):
                k = type_of.get(n)
                if not k: continue
                tot[k] = tot.get(k, 0) + w
                if bw: back[k] = back.get(k, 0) + w
    return tot, back

# FlyWire: root_id -> hemibrain type (unambiguous matches only)
fw_type = {}
with open(annot, newline="") as f:
    r = csv.DictReader(f, delimiter="\t")
    for row in r:
        ht = (row.get("hemibrain_type") or "").strip()
        if not ht or "," in ht: continue
        try: fw_type[int(row["root_id"])] = ht
        except Exception: pass

# hemibrain: bodyId -> type
hb_type = {}
with open(hb_neurons, newline="") as f:
    r = csv.reader(f); next(r)
    for row in r:
        t = (row[1] or "").strip()
        if t: hb_type[int(row[0])] = t

fw_tot, fw_back = weights_by_type(fw_graph, load_order(fw_order), fw_type)
hb_tot, hb_back = weights_by_type(hb_graph, load_order(hb_order), hb_type)

MIN = 1000
shared = [k for k in fw_tot if k in hb_tot and fw_tot[k] >= MIN and hb_tot[k] >= MIN]
print(f"cell types in FlyWire (unambiguous match) : {len(fw_tot):,}")
print(f"cell types in hemibrain                   : {len(hb_tot):,}")
print(f"shared, with at least {MIN:,} weight each  : {len(shared):,}")

def spearman(xs, ys):
    def ranks(v):
        order = sorted(range(len(v)), key=lambda i: v[i])
        r = [0.0] * len(v); i = 0
        while i < len(order):            # average ranks within ties
            j = i
            while j + 1 < len(order) and v[order[j + 1]] == v[order[i]]: j += 1
            avg = (i + j) / 2.0 + 1
            for k in range(i, j + 1): r[order[k]] = avg
            i = j + 1
        return r
    a, b = ranks(xs), ranks(ys)
    n = len(a)
    ma = sum(a) / n; mb = sum(b) / n
    num = sum((x - ma) * (y - mb) for x, y in zip(a, b))
    da = sum((x - ma) ** 2 for x in a) ** 0.5
    db = sum((y - mb) ** 2 for y in b) ** 0.5
    return num / (da * db) if da and db else float("nan")

x_rate = [fw_back.get(k, 0) / fw_tot[k] for k in shared]
y_rate = [hb_back.get(k, 0) / hb_tot[k] for k in shared]
x_back = [fw_back.get(k, 0) for k in shared]
y_back = [hb_back.get(k, 0) for k in shared]
x_size = [fw_tot[k] for k in shared]
y_size = [hb_tot[k] for k in shared]

print(f"\nSpearman rank correlation across those {len(shared):,} cell types")
print(f"  backward RATE  (share of the type's weight turned around) : {spearman(x_rate, y_rate):.3f}")
print(f"  backward WEIGHT (absolute)                                : {spearman(x_back, y_back):.3f}")
print(f"  total weight  -- the size control                         : {spearman(x_size, y_size):.3f}")

rows = sorted(shared, key=lambda k: -(fw_back.get(k, 0) / fw_tot[k]))
# how big a correlation would chance give? shuffle one side, 200 times
import random
random.seed(7)
null = []
for _ in range(200):
    yy = y_rate[:]; random.shuffle(yy)
    null.append(abs(spearman(x_rate, yy)))
null.sort()
print(f"  shuffled control (200 draws): largest |rho| seen {null[-1]:.3f}, "
      f"95th percentile {null[int(.95*len(null))]:.3f}")

# the same correlation on types whose rate is measured from a lot of weight
STRICT = 10000
sub = [i for i, k in enumerate(shared) if fw_tot[k] >= STRICT and hb_tot[k] >= STRICT]
print(f"  restricted to the {len(sub):,} types with >= {STRICT:,} weight in both: "
      f"{spearman([x_rate[i] for i in sub], [y_rate[i] for i in sub]):.3f}")

print(f"\nMOST FEEDBACK-HEAVY CELL TYPES IN FLYWIRE, and where hemibrain puts them")
print(f"  {'cell type':<16}{'FlyWire rate':>14}{'hemibrain rate':>16}")
for k in rows[:12]:
    print(f"  {k[:15]:<16}{100*fw_back.get(k,0)/fw_tot[k]:>13.1f}%{100*hb_back.get(k,0)/hb_tot[k]:>15.1f}%")
print(f"  {'...':<16}")
for k in rows[-8:]:
    print(f"  {k[:15]:<16}{100*fw_back.get(k,0)/fw_tot[k]:>13.1f}%{100*hb_back.get(k,0)/hb_tot[k]:>15.1f}%")
