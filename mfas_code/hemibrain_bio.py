"""Do the FlyWire feedback findings hold in a second connectome? Pure stdlib.

usage: python3 -I hemibrain_bio.py hb_graph.csv hb_order.csv \
                traced-neurons.csv flywire_annotations.tsv

The ordering comes from the hemibrain connectome alone (a different fly, a
different imaging run, a different lab, reconstructed independently). The
*labels* -- super class and neurotransmitter -- come from FlyWire, carried
across by cell type: FlyWire's annotation table has a `hemibrain_type`
column, so every FlyWire neuron votes for the class and transmitter of the
hemibrain type it was matched to. Cell type is conserved between individuals;
the ordering is not borrowed from anywhere.
"""
import csv, sys

graph_path, order_path, neurons_path, annot_path = sys.argv[1:5]

# ---- our hemibrain ordering -------------------------------------------
pos = {}
with open(order_path, newline="") as f:
    r = csv.reader(f); next(r)
    for nid, o in r:
        pos[int(nid)] = int(o)
N = len(pos)

# ---- hemibrain body -> cell type --------------------------------------
btype = {}
with open(neurons_path, newline="") as f:
    r = csv.reader(f); next(r)
    for row in r:
        t = (row[1] or "").strip()
        if t:
            btype[int(row[0])] = t

# ---- FlyWire: hemibrain cell type -> class, transmitter ----------------
votes_sc, votes_nt, votes_cc = {}, {}, {}
with open(annot_path, newline="") as f:
    r = csv.DictReader(f, delimiter="\t")
    for row in r:
        ht = (row.get("hemibrain_type") or "").strip()
        if not ht:
            continue
        sc = (row.get("super_class") or "").strip()
        nt = (row.get("top_nt") or "").strip()
        cc = (row.get("cell_class") or "").strip()
        for t in ht.split(","):            # "MC61,MC64" -> votes for both
            t = t.strip()
            if not t:
                continue
            if sc: votes_sc.setdefault(t, {})[sc] = votes_sc.setdefault(t, {}).get(sc, 0) + 1
            if nt: votes_nt.setdefault(t, {})[nt] = votes_nt.setdefault(t, {}).get(nt, 0) + 1
            if cc: votes_cc.setdefault(t, {})[cc] = votes_cc.setdefault(t, {}).get(cc, 0) + 1

def winner(d, t):
    v = d.get(t)
    if not v: return None
    return max(v.items(), key=lambda kv: kv[1])[0]

sc_of, nt_of, cc_of = {}, {}, {}
for b, t in btype.items():
    s = winner(votes_sc, t); n = winner(votes_nt, t); c = winner(votes_cc, t)
    if s: sc_of[b] = s
    if n: nt_of[b] = n
    if c: cc_of[b] = c

print(f"hemibrain ordering: {N:,} neurons")
print(f"  with a hemibrain cell type   : {sum(1 for b in pos if b in btype):,}")
print(f"  matched to a FlyWire class   : {sum(1 for b in pos if b in sc_of):,}")
print(f"  matched to a transmitter     : {sum(1 for b in pos if b in nt_of):,}")

def pct(n): return 100.0 * pos[n] / (N - 1)
def g(d, n): return d.get(n, "(unmatched)")

def report_positions(d, title, minimum=30):
    groups = {}
    for n in pos:
        groups.setdefault(g(d, n), []).append(pct(n))
    rows = []
    for k, ps in groups.items():
        if len(ps) < minimum: continue
        ps.sort()
        q = lambda p: ps[int(p * (len(ps) - 1))]
        rows.append((q(.5), k, len(ps), q(.25), q(.75)))
    rows.sort()
    print(f"\n{title}  (0% = start of the order, 100% = end)")
    print(f"  {'group':<28}{'neurons':>9}{'median':>9}{'25-75%':>16}")
    for med, k, c, lo, hi in rows:
        print(f"  {k[:27]:<28}{c:>9,}{med:>8.1f}%{lo:>9.1f} -{hi:>6.1f}")

report_positions(sc_of, "WHERE EACH SUPER CLASS SITS")

# ---- forward / backward ------------------------------------------------
tot_w = back_w = 0
in_w, in_b, pair_w, pair_b, node_b = {}, {}, {}, {}, {}
nt_w, nt_b = {}, {}
with open(graph_path, newline="") as f:
    r = csv.reader(f); next(r)
    for s, t, w in r:
        s = int(s); t = int(t); w = int(w)
        if s not in pos or t not in pos: continue
        tot_w += w
        cs, ct = g(sc_of, s), g(sc_of, t)
        pair_w[(cs, ct)] = pair_w.get((cs, ct), 0) + w
        in_w[ct] = in_w.get(ct, 0) + w
        nts = g(nt_of, s)
        nt_w[nts] = nt_w.get(nts, 0) + w
        if pos[s] > pos[t]:
            back_w += w
            in_b[ct] = in_b.get(ct, 0) + w
            pair_b[(cs, ct)] = pair_b.get((cs, ct), 0) + w
            nt_b[nts] = nt_b.get(nts, 0) + w
            node_b[s] = node_b.get(s, 0) + w
            node_b[t] = node_b.get(t, 0) + w

print(f"\ntotal synaptic weight {tot_w:,};  pointing backward {back_w:,} "
      f"({100*back_w/tot_w:.2f}%)")

base = back_w / tot_w          # brain-wide backward rate, for comparing
                               # across two connectomes with different rates
print("\nFEEDBACK BY THE RECEIVING NEURON'S CLASS")
print(f"  {'super class':<28}{'weight':>13}{'backward':>13}{'share':>8}{'vs brain':>10}")
rows = [(in_b.get(k, 0) / v, k, v, in_b.get(k, 0))
        for k, v in in_w.items() if v >= tot_w / 1000]
for sh, k, v, b in sorted(rows, reverse=True):
    print(f"  {k[:27]:<28}{v:>13,}{b:>13,}{100*sh:>7.1f}%{sh/base:>9.2f}x")

print("\nPATHWAYS, by share of their weight pointing backward "
      f"(at least {tot_w//1000:,} weight)")
print(f"  {'from -> to':<46}{'all weight':>13}{'backward':>12}{'share back':>12}")
prs = [(v, k) for k, v in pair_w.items() if v >= tot_w / 1000]
for v, k in sorted(prs, key=lambda x: -pair_b.get(x[1], 0) / x[0]):
    b = pair_b.get(k, 0)
    print(f"  {(k[0][:21] + ' -> ' + k[1][:21]):<46}{v:>13,}{b:>12,}{100*b/v:>11.1f}%")

print("\nNEUROTRANSMITTER OF THE SENDING NEURON")
print(f"  {'transmitter':<16}{'weight':>13}{'of all':>9}{'backward':>13}{'share back':>12}{'vs brain':>10}")
for t, v in sorted(nt_w.items(), key=lambda kv: -kv[1]):
    if v < tot_w / 500: continue
    b = nt_b.get(t, 0)
    print(f"  {t[:15]:<16}{v:>13,}{100*v/tot_w:>8.1f}%{b:>13,}{100*b/v:>11.1f}%{(b/v)/base:>9.2f}x")

print("\n20 NEURONS CARRYING THE MOST FEEDBACK")
print(f"  {'cell type':<24}{'class':<20}{'pos':>7}{'backward weight':>17}")
for n, v in sorted(node_b.items(), key=lambda kv: -kv[1])[:20]:
    print(f"  {btype.get(n,'(none)')[:23]:<24}{g(sc_of,n)[:19]:<20}{pct(n):>6.1f}%{v:>17,}")
