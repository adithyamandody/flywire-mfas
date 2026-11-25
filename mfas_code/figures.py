"""Figures for the paper. Pure stdlib; writes SVG directly.

usage: python3 -I figures.py fw_graph.csv fw_order.csv annotations.tsv \
                             hb_graph.csv hb_order.csv traced-neurons.csv \
                             pertype.tsv out_dir

fig1  where each super class sits in each of the two orderings
fig2  per cell type, the backward rate in one connectome against the other
fig3  for every pair of classes, the two directions between them
"""
import csv, sys, math

fw_graph, fw_order, annot, hb_graph, hb_order, hb_neurons, pertype, out_dir = sys.argv[1:9]

INK = "#1b1b1b"; GREY = "#8a8a8a"; LIGHT = "#d8d8d8"
FW = "#2d6a9f"; HB = "#b5562c"
FONT = 'font-family="Helvetica,Arial,sans-serif"'
esc = lambda s: s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")

def load_order(p):
    pos = {}
    with open(p, newline="") as f:
        r = csv.reader(f); next(r)
        for n, o in r: pos[int(n)] = int(o)
    return pos

def quartiles(vals):
    vals.sort()
    q = lambda p: vals[int(p * (len(vals) - 1))]
    return q(.25), q(.5), q(.75)

# ---- labels -------------------------------------------------------------
fw_cls_of = {}                      # FlyWire root id -> super class
hb_votes = {}                       # hemibrain cell type -> super class votes
with open(annot, newline="") as f:
    r = csv.DictReader(f, delimiter="\t")
    for row in r:
        sc = (row.get("super_class") or "").strip()
        if not sc: continue
        try: fw_cls_of[int(row["root_id"])] = sc
        except Exception: pass
        for t in (row.get("hemibrain_type") or "").split(","):
            t = t.strip()
            if t: hb_votes.setdefault(t, {})[sc] = hb_votes.setdefault(t, {}).get(sc, 0) + 1
hb_cls_of = {}                      # hemibrain body id -> super class
with open(hb_neurons, newline="") as f:
    r = csv.reader(f); next(r)
    for row in r:
        t = (row[1] or "").strip()
        if t in hb_votes:
            hb_cls_of[int(row[0])] = max(hb_votes[t].items(), key=lambda kv: kv[1])[0]

fwp, hbp = load_order(fw_order), load_order(hb_order)

def class_positions(pos, cls_of):
    out = {}
    for n, p in pos.items():
        c = cls_of.get(n)
        if c: out.setdefault(c, []).append(100.0 * p / (len(pos) - 1))
    return out

def pair_rates(graph, pos, cls_of):
    """total and backward weight for every ordered pair of classes."""
    tot, back, T, B = {}, {}, 0, 0
    with open(graph, newline="") as f:
        r = csv.reader(f); next(r)
        for s, t, w in r:
            s = int(s); t = int(t); w = int(w)
            if s not in pos or t not in pos: continue
            T += w
            bw = pos[s] > pos[t]
            if bw: B += w
            a, b = cls_of.get(s), cls_of.get(t)
            if not a or not b: continue
            tot[(a, b)] = tot.get((a, b), 0) + w
            if bw: back[(a, b)] = back.get((a, b), 0) + w
    return tot, back, T, B

# ======================= figure 1 ========================================
fw_cls, hb_cls = class_positions(fwp, fw_cls_of), class_positions(hbp, hb_cls_of)
order = [k for k in sorted(fw_cls, key=lambda k: quartiles(fw_cls[k][:])[1]) if len(fw_cls[k]) >= 30]

W, H = 760, 40 + 34 * len(order) + 56
L, R = 168, W - 30
x = lambda v: L + (R - L) * v / 100.0
s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">',
     f'<rect width="{W}" height="{H}" fill="white"/>']
for g in range(0, 101, 25):
    s.append(f'<line x1="{x(g):.1f}" y1="34" x2="{x(g):.1f}" y2="{H-50}" stroke="{LIGHT}" stroke-width="1"/>')
    s.append(f'<text x="{x(g):.1f}" y="{H-32}" {FONT} font-size="11" fill="{GREY}" text-anchor="middle">{g}%</text>')
s.append(f'<text x="{(L+R)/2:.0f}" y="{H-12}" {FONT} font-size="12" fill="{INK}" text-anchor="middle">'
         'position in the ordering (0% = first, 100% = last)</text>')
s.append(f'<text x="{L}" y="20" {FONT} font-size="12" fill="{FW}">FlyWire (136,648 neurons)</text>')
s.append(f'<text x="{L+220}" y="20" {FONT} font-size="12" fill="{HB}">hemibrain (21,737 neurons)</text>')
for i, g in enumerate(order):
    y0 = 48 + 34 * i
    s.append(f'<text x="{L-10}" y="{y0+4:.0f}" {FONT} font-size="12" fill="{INK}" text-anchor="end">{esc(g)}</text>')
    for vals, col, dy in ((fw_cls.get(g), FW, -5), (hb_cls.get(g), HB, 7)):
        if not vals or len(vals) < 30: continue
        lo, md, hi = quartiles(vals[:])
        yy = y0 + dy
        s.append(f'<line x1="{x(lo):.1f}" y1="{yy}" x2="{x(hi):.1f}" y2="{yy}" stroke="{col}" stroke-width="3" opacity="0.45"/>')
        s.append(f'<circle cx="{x(md):.1f}" cy="{yy}" r="4" fill="{col}"/>')
        s.append(f'<text x="{x(md)+8:.1f}" y="{yy+4}" {FONT} font-size="10" fill="{col}">{md:.0f}%</text>')
s.append('</svg>')
open(f"{out_dir}/fig1_axis.svg", "w").write("\n".join(s))

# ======================= figure 2 ========================================
pts = []
with open(pertype) as f:
    r = csv.DictReader(f, delimiter="\t")
    for row in r:
        a = int(row["fw_total"]); b = int(row["fw_backward"])
        c = int(row["hb_total"]); d = int(row["hb_backward"])
        pts.append((100 * b / a, 100 * d / c, row["cell_type"]))

def spearman(xs, ys):
    def ranks(v):
        o = sorted(range(len(v)), key=lambda i: v[i]); r = [0.0] * len(v); i = 0
        while i < len(o):
            j = i
            while j + 1 < len(o) and v[o[j + 1]] == v[o[i]]: j += 1
            for k in range(i, j + 1): r[o[k]] = (i + j) / 2.0 + 1
            i = j + 1
        return r
    a, b = ranks(xs), ranks(ys); n = len(a)
    ma, mb = sum(a) / n, sum(b) / n
    num = sum((p - ma) * (q - mb) for p, q in zip(a, b))
    da = math.sqrt(sum((p - ma) ** 2 for p in a)); db = math.sqrt(sum((q - mb) ** 2 for q in b))
    return num / (da * db)
rho = spearman([p[0] for p in pts], [p[1] for p in pts])

S = 470; M = 62
W2 = S + M + 28; H2 = S + M + 34
px = lambda v: M + S * min(v, 60) / 60.0
py = lambda v: M + S - S * min(v, 60) / 60.0
s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W2}" height="{H2}" viewBox="0 0 {W2} {H2}">',
     f'<rect width="{W2}" height="{H2}" fill="white"/>']
for g in range(0, 61, 15):
    s.append(f'<line x1="{px(g):.1f}" y1="{M}" x2="{px(g):.1f}" y2="{M+S}" stroke="{LIGHT}" stroke-width="1"/>')
    s.append(f'<line x1="{M}" y1="{py(g):.1f}" x2="{M+S}" y2="{py(g):.1f}" stroke="{LIGHT}" stroke-width="1"/>')
    s.append(f'<text x="{px(g):.1f}" y="{M+S+18}" {FONT} font-size="11" fill="{GREY}" text-anchor="middle">{g}%</text>')
    s.append(f'<text x="{M-8}" y="{py(g)+4:.1f}" {FONT} font-size="11" fill="{GREY}" text-anchor="end">{g}%</text>')
s.append(f'<line x1="{px(0)}" y1="{py(0)}" x2="{px(60)}" y2="{py(60)}" stroke="{GREY}" stroke-width="1" stroke-dasharray="4 3"/>')
for a, b, _ in pts:
    s.append(f'<circle cx="{px(a):.1f}" cy="{py(b):.1f}" r="2.1" fill="{INK}" opacity="0.28"/>')
for a, b, name in pts:
    if name in ("APL", "PVLP011", "DNp13", "lLN1_a", "CT1"):
        s.append(f'<circle cx="{px(a):.1f}" cy="{py(b):.1f}" r="4" fill="{HB}"/>')
        s.append(f'<text x="{px(a)+7:.1f}" y="{py(b)+4:.1f}" {FONT} font-size="11" fill="{HB}">{esc(name)}</text>')
s.append(f'<text x="{M+S/2:.0f}" y="{H2-6}" {FONT} font-size="12" fill="{INK}" text-anchor="middle">'
         'share of the cell type’s weight pointing backward — FlyWire</text>')
s.append(f'<text x="16" y="{M+S/2:.0f}" {FONT} font-size="12" fill="{INK}" text-anchor="middle" '
         f'transform="rotate(-90 16 {M+S/2:.0f})">same share — hemibrain</text>')
s.append(f'<text x="{M+10}" y="{M+20}" {FONT} font-size="13" fill="{INK}">'
         f'{len(pts):,} cell types shared by both connectomes</text>')
s.append(f'<text x="{M+10}" y="{M+38}" {FONT} font-size="13" fill="{INK}">'
         f'Spearman ρ = {rho:.3f}  (shuffled control: 0.057)</text>')
s.append('</svg>')
open(f"{out_dir}/fig2_crosscheck.svg", "w").write("\n".join(s))

# ======================= figure 3 ========================================
fw_tot, fw_back, fw_T, fw_B = pair_rates(fw_graph, fwp, fw_cls_of)
hb_tot, hb_back, hb_T, hb_B = pair_rates(hb_graph, hbp, hb_cls_of)
MINW = 20000
OUTSIDE = {"optic", "visual_centrifugal"}   # classes the hemibrain volume cuts off

def rate(tot, back, k):
    return 100.0 * back.get(k, 0) / tot[k] if k in tot and tot[k] else None

pairs = []                       # unordered class pairs, both directions present
seen = set()
for (a, b), v in fw_tot.items():
    if a == b or (b, a) not in fw_tot: continue
    if v < MINW or fw_tot[(b, a)] < MINW: continue
    key = tuple(sorted((a, b)))
    if key in seen: continue
    seen.add(key)
    ra, rb = rate(fw_tot, fw_back, (a, b)), rate(fw_tot, fw_back, (b, a))
    lo = (a, b, ra) if ra <= rb else (b, a, rb)      # the clean direction
    hi = (b, a, rb) if ra <= rb else (a, b, ra)      # and the other one
    pairs.append((lo, hi))
pairs.sort(key=lambda p: p[0][2])

RH, GAP, TOP = 19, 14, 84
W3 = 880; L3 = 318; R3 = W3 - 118
H3 = TOP + len(pairs) * (2 * RH + GAP) + 54
MAXP = 80.0
bx = lambda v: L3 + (R3 - L3) * min(v, MAXP) / MAXP
s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W3}" height="{H3}" viewBox="0 0 {W3} {H3}">',
     f'<rect width="{W3}" height="{H3}" fill="white"/>']
for g in range(0, int(MAXP) + 1, 20):
    s.append(f'<line x1="{bx(g):.1f}" y1="{TOP-10}" x2="{bx(g):.1f}" y2="{H3-46}" stroke="{LIGHT}" stroke-width="1"/>')
    s.append(f'<text x="{bx(g):.1f}" y="{H3-28}" {FONT} font-size="11" fill="{GREY}" text-anchor="middle">{g}%</text>')
brain = 100.0 * fw_B / fw_T
s.append(f'<line x1="{bx(brain):.1f}" y1="{TOP-10}" x2="{bx(brain):.1f}" y2="{H3-46}" '
         f'stroke="{INK}" stroke-width="1" stroke-dasharray="5 3" opacity="0.7"/>')
s.append(f'<text x="{bx(brain):.1f}" y="{TOP-16}" {FONT} font-size="11" fill="{INK}" text-anchor="middle">'
         f'whole brain {brain:.1f}%</text>')
s.append(f'<text x="24" y="24" {FONT} font-size="13" fill="{INK}">'
         'Between any two classes of neuron, one direction is clean and the other is not</text>')
s.append(f'<text x="24" y="42" {FONT} font-size="11.5" fill="{GREY}">'
         'bars: share of that route\u2019s synaptic weight pointing backward in the FlyWire ordering'
         f' \u00b7 <tspan fill="{HB}">\u25c6 hemibrain</tspan>, for routes with both ends inside that volume</text>')
s.append(f'<text x="24" y="56" {FONT} font-size="11.5" fill="{GREY}">'
         'hemibrain excludes the optic lobe, so routes defined by optic-lobe partners are not comparable'
         ' and carry no diamond</text>')
s.append(f'<text x="{(L3+R3)/2:.0f}" y="{H3-8}" {FONT} font-size="12" fill="{INK}" text-anchor="middle">'
         'share of the route’s weight pointing backward</text>')
for i, (lo, hi) in enumerate(pairs):
    y0 = TOP + i * (2 * RH + GAP)
    s.append(f'<line x1="18" y1="{y0+2}" x2="18" y2="{y0+2*RH-4}" stroke="{LIGHT}" stroke-width="2"/>')
    for j, (a, b, r) in enumerate((lo, hi)):
        yy = y0 + j * RH
        s.append(f'<text x="{L3-12}" y="{yy+12}" {FONT} font-size="12" fill="{INK}" text-anchor="end">'
                 f'{esc(a)} → {esc(b)}</text>')
        s.append(f'<rect x="{L3}" y="{yy+3}" width="{max(bx(r)-L3, 1.2):.1f}" height="12" fill="{FW}" '
                 f'opacity="{0.5 if j == 0 else 1.0}"/>')
        hr = rate(hb_tot, hb_back, (a, b))
        # hemibrain excludes the optic lobe, so any route defined by optic-lobe
        # partners (optic, visual_centrifugal) is not comparable and is left off
        show_hb = (hr is not None and hb_tot.get((a, b), 0) >= MINW
                   and hb_tot.get((b, a), 0) >= MINW
                   and a not in OUTSIDE and b not in OUTSIDE)
        labx = bx(r) + 6
        if show_hb:
            s.append(f'<path d="M {bx(hr):.1f} {yy+3.5} l 5.5 5.5 l -5.5 5.5 l -5.5 -5.5 z" fill="{HB}"/>')
            labx = max(bx(r), bx(hr) + 6) + 7
        s.append(f'<text x="{labx:.1f}" y="{yy+13}" {FONT} font-size="11" fill="{FW}">{r:.1f}%</text>')
        if show_hb:
            s.append(f'<text x="{labx+32:.1f}" y="{yy+13}" {FONT} font-size="11" fill="{HB}">{hr:.1f}%</text>')
s.append('</svg>')
open(f"{out_dir}/fig3_pathways.svg", "w").write("\n".join(s))
print(f"fig1: {len(order)} classes | fig2: {len(pts):,} types, rho {rho:.3f} | fig3: {len(pairs)} class pairs")
