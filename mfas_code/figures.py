"""Figures for the paper. Pure stdlib; writes SVG directly.

usage: python3 -I figures.py fw_order.csv annotations.tsv \
                             hb_order.csv traced-neurons.csv \
                             pertype.tsv out_dir
"""
import csv, sys, math

fw_order, annot, hb_order, hb_neurons, pertype, out_dir = sys.argv[1:7]

INK = "#1b1b1b"; GREY = "#8a8a8a"; LIGHT = "#d8d8d8"
FW = "#2d6a9f"; HB = "#b5562c"
FONT = 'font-family="Helvetica,Arial,sans-serif"'

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

# ---------- figure 1: where each class sits in each ordering ----------
fwp = load_order(fw_order)
fw_cls = {}
with open(annot, newline="") as f:
    r = csv.DictReader(f, delimiter="\t")
    for row in r:
        try: rid = int(row["root_id"])
        except Exception: continue
        sc = (row.get("super_class") or "").strip()
        if sc and rid in fwp: fw_cls.setdefault(sc, []).append(100 * fwp[rid] / (len(fwp) - 1))

# hemibrain classes, via the FlyWire hemibrain_type -> super_class transfer
votes = {}
with open(annot, newline="") as f:
    r = csv.DictReader(f, delimiter="\t")
    for row in r:
        ht = (row.get("hemibrain_type") or "").strip()
        sc = (row.get("super_class") or "").strip()
        if not ht or not sc: continue
        for t in ht.split(","):
            t = t.strip()
            if t: votes.setdefault(t, {})[sc] = votes.setdefault(t, {}).get(sc, 0) + 1
hbp = load_order(hb_order)
hb_cls = {}
with open(hb_neurons, newline="") as f:
    r = csv.reader(f); next(r)
    for row in r:
        b = int(row[0]); t = (row[1] or "").strip()
        if b not in hbp or t not in votes: continue
        sc = max(votes[t].items(), key=lambda kv: kv[1])[0]
        hb_cls.setdefault(sc, []).append(100 * hbp[b] / (len(hbp) - 1))

order = sorted(fw_cls, key=lambda k: quartiles(fw_cls[k][:])[1])
order = [k for k in order if len(fw_cls[k]) >= 30]

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
    s.append(f'<text x="{L-10}" y="{y0+4:.0f}" {FONT} font-size="12" fill="{INK}" text-anchor="end">{g}</text>')
    for vals, col, dy in ((fw_cls.get(g), FW, -5), (hb_cls.get(g), HB, 7)):
        if not vals or len(vals) < 30: continue
        lo, md, hi = quartiles(vals[:])
        yy = y0 + dy
        s.append(f'<line x1="{x(lo):.1f}" y1="{yy}" x2="{x(hi):.1f}" y2="{yy}" stroke="{col}" stroke-width="3" opacity="0.45"/>')
        s.append(f'<circle cx="{x(md):.1f}" cy="{yy}" r="4" fill="{col}"/>')
        s.append(f'<text x="{x(md)+8:.1f}" y="{yy+4}" {FONT} font-size="10" fill="{col}">{md:.0f}%</text>')
s.append('</svg>')
open(f"{out_dir}/fig1_axis.svg", "w").write("\n".join(s))

# ---------- figure 2: per cell type, the two connectomes agree ----------
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
        s.append(f'<text x="{px(a)+7:.1f}" y="{py(b)+4:.1f}" {FONT} font-size="11" fill="{HB}">{name}</text>')
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
print(f"fig1: {len(order)} classes;  fig2: {len(pts):,} points, rho {rho:.3f}")
