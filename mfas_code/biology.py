"""What does our feedforward ordering say about the fly brain? Pure stdlib.

usage: python3 -I biology.py graph.csv order.csv neuron_annotations.tsv

The ordering is just a list of neurons. Joined to FlyWire's published
annotations it can be asked real questions:

  1. does the order run from sensing to action, as information flow should?
  2. which kinds of neuron carry the synapses that point backward -- the
     feedback the ordering cannot remove?

Annotations: Schlegel et al. 2024, supplemental file 1 (FAFB v783).
"""
import csv, sys

graph_path, order_path, annot_path = sys.argv[1:4]

# ---- our ordering ------------------------------------------------------
pos = {}
with open(order_path, newline="") as f:
    r = csv.reader(f); next(r)
    for nid, o in r:
        pos[int(nid)] = int(o)
N = len(pos)

# ---- published annotations --------------------------------------------
ann = {}
with open(annot_path, newline="") as f:
    r = csv.DictReader(f, delimiter="\t")
    for row in r:
        try:
            rid = int(row["root_id"])
        except (ValueError, KeyError, TypeError):
            continue
        ann[rid] = row

have = [n for n in pos if n in ann]
print(f"our ordering: {N:,} neurons;  annotations cover {len(have):,} of them "
      f"({100*len(have)/N:.1f}%)")

def field(n, key):
    a = ann.get(n)
    if not a: return "(unannotated)"
    v = (a.get(key) or "").strip()
    return v if v else "(blank)"

def pct(n):            # position as a percentage of the order
    return 100.0 * pos[n] / (N - 1)

def report_positions(key, title, minimum=30):
    groups = {}
    for n in pos:
        groups.setdefault(field(n, key), []).append(pct(n))
    rows = []
    for g, ps in groups.items():
        if len(ps) < minimum: continue
        ps.sort()
        q = lambda p: ps[int(p * (len(ps) - 1))]
        rows.append((q(.5), g, len(ps), q(.25), q(.75)))
    rows.sort()
    print(f"\n{title}  (earliest first; 0% = start of the order, 100% = end)")
    print(f"  {'group':<28}{'neurons':>9}{'median':>9}{'25-75%':>16}")
    for med, g, k, lo, hi in rows:
        print(f"  {g[:27]:<28}{k:>9,}{med:>8.1f}%{lo:>9.1f} -{hi:>6.1f}")

report_positions("flow", "WHERE EACH FLOW CLASS SITS")
report_positions("super_class", "WHERE EACH SUPER CLASS SITS")

# ---- edges: forward or backward ---------------------------------------
tot_w = back_w = 0
out_w, out_b = {}, {}      # by super class of the source
in_w, in_b = {}, {}        # by super class of the target
pair_b = {}                # backward weight by (source class -> target class)
pair_w = {}                # and total weight, so the share can be judged
node_b = {}                # backward weight per neuron
with open(graph_path, newline="") as f:
    r = csv.reader(f); next(r)
    for s, t, w in r:
        s = int(s); t = int(t); w = int(w)
        if s not in pos or t not in pos: continue
        tot_w += w
        cs = field(s, "super_class"); ct = field(t, "super_class")
        pair_w[(cs, ct)] = pair_w.get((cs, ct), 0) + w
        out_w[cs] = out_w.get(cs, 0) + w
        in_w[ct] = in_w.get(ct, 0) + w
        if pos[s] > pos[t]:
            back_w += w
            out_b[cs] = out_b.get(cs, 0) + w
            in_b[ct] = in_b.get(ct, 0) + w
            pair_b[(cs, ct)] = pair_b.get((cs, ct), 0) + w
            node_b[s] = node_b.get(s, 0) + w
            node_b[t] = node_b.get(t, 0) + w

print(f"\ntotal synaptic weight {tot_w:,};  pointing backward {back_w:,} "
      f"({100*back_w/tot_w:.2f}%)")

def report_share(wtot, wback, title):
    rows = [(wback.get(g, 0) / v, g, v, wback.get(g, 0))
            for g, v in wtot.items() if v >= tot_w / 1000]
    rows.sort(reverse=True)
    print(f"\n{title}")
    print(f"  {'super class':<28}{'weight':>13}{'backward':>13}{'share':>8}")
    for sh, g, v, b in rows:
        print(f"  {g[:27]:<28}{v:>13,}{b:>13,}{100*sh:>7.1f}%")

report_share(out_w, out_b, "FEEDBACK BY THE SENDING NEURON'S CLASS")
report_share(in_w, in_b, "FEEDBACK BY THE RECEIVING NEURON'S CLASS")

print("\nBIGGEST BACKWARD FLOWS between classes")
print(f"  {'from -> to':<46}{'all weight':>13}{'backward':>12}{'of all back':>12}{'share back':>11}")
for (a, b), v in sorted(pair_b.items(), key=lambda kv: -kv[1])[:12]:
    tw = pair_w[(a, b)]
    print(f"  {(a[:21] + ' -> ' + b[:21]):<46}{tw:>13,}{v:>12,}{100*v/back_w:>11.1f}%{100*v/tw:>10.1f}%")

# ---- neurotransmitter: is the feedback inhibitory? ---------------------
nt_w, nt_b = {}, {}
with open(graph_path, newline="") as f:
    r = csv.reader(f); next(r)
    for a, b, w in r:
        a = int(a); b = int(b); w = int(w)
        if a not in pos or b not in pos: continue
        t = field(a, "top_nt")
        nt_w[t] = nt_w.get(t, 0) + w
        if pos[a] > pos[b]: nt_b[t] = nt_b.get(t, 0) + w
print("\nNEUROTRANSMITTER OF THE SENDING NEURON")
print(f"  {'transmitter':<16}{'weight':>13}{'of all':>9}{'backward':>13}{'of all back':>13}{'share back':>12}")
for t, v in sorted(nt_w.items(), key=lambda kv: -kv[1]):
    if v < tot_w / 500: continue
    b = nt_b.get(t, 0)
    print(f"  {t[:15]:<16}{v:>13,}{100*v/tot_w:>8.1f}%{b:>13,}{100*b/back_w:>12.1f}%{100*b/v:>11.1f}%")

# ---- which cell classes carry the feedback -----------------------------
cls_b, cls_n = {}, {}
for n in pos:
    c = field(n, "cell_class")
    cls_b[c] = cls_b.get(c, 0) + node_b.get(n, 0)
    cls_n[c] = cls_n.get(c, 0) + 1
rows = sorted(((v, c, cls_n[c]) for c, v in cls_b.items()), reverse=True)[:15]
print("\nCELL CLASSES CARRYING THE MOST FEEDBACK  (weight counted at both ends)")
print(f"  {'cell class':<28}{'neurons':>9}{'backward weight':>18}{'per neuron':>12}")
for v, c, k in rows:
    print(f"  {c[:27]:<28}{k:>9,}{v:>18,}{v/k:>12,.0f}")

print("\n20 NEURONS CARRYING THE MOST FEEDBACK")
print(f"  {'cell type':<24}{'super class':<20}{'pos':>7}{'backward weight':>17}")
for n, v in sorted(node_b.items(), key=lambda kv: -kv[1])[:20]:
    print(f"  {field(n,'cell_type')[:23]:<24}{field(n,'super_class')[:19]:<20}"
          f"{pct(n):>6.1f}%{v:>17,}")
