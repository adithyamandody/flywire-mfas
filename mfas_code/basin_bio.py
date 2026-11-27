"""Do the biological conclusions survive a change of basin?

usage: python3 -I basin_bio.py annotations.tsv label=order.csv [label=order.csv ...]

The diversity experiment showed that independent runs reach near-equal
scores while disagreeing on more than a million units of synaptic weight.
The obvious objection to reading biology off any one of them is that the
ordering is not unique. This asks the question directly: compute the median
position of each annotated class in every ordering, and see whether the
axis is the same one each time.

Pure stdlib.
"""
import csv
import sys

ann_path = sys.argv[1]
COL = 10 if "--super" in sys.argv else 11
specs = []
for a in sys.argv[2:]:
    if a.startswith("--"):
        continue
    label, _, path = a.partition("=")
    specs.append((label, path))

cls = {}
with open(ann_path, encoding="utf-8", errors="replace") as f:
    r = csv.reader(f, delimiter="\t")
    next(r)
    for row in r:
        if len(row) > COL and row[COL].strip():
            cls[row[1]] = row[COL].strip()

medians = {}
for label, path in specs:
    pos = {}
    with open(path, newline="") as f:
        r = csv.reader(f)
        next(r)
        for nid, o in r:
            pos[nid] = int(o)
    n = len(pos)
    groups = {}
    for nid, c in cls.items():
        p = pos.get(nid)
        if p is not None:
            groups.setdefault(c, []).append(100.0 * p / (n - 1))
    for c, v in groups.items():
        if len(v) < 50:
            continue
        v.sort()
        medians.setdefault(c, {})[label] = v[len(v) // 2]

labels = [l for l, _ in specs]
order = sorted(medians, key=lambda c: medians[c].get(labels[0], 0))
w = max(len(c) for c in order)
print(f"{'class':<{w}s} " + " ".join(f"{l:>9s}" for l in labels) + "    spread")
for c in order:
    row = medians[c]
    vals = [row.get(l) for l in labels]
    got = [v for v in vals if v is not None]
    spread = max(got) - min(got) if got else 0
    cells = " ".join(f"{v:>8.1f}%" if v is not None else "       --" for v in vals)
    print(f"{c:<{w}s} {cells}  {spread:>7.1f}")
print(f"\n{len(order)} classes of >=50 neurons. 'spread' is the largest "
      f"disagreement\nbetween any two of the {len(labels)} orderings about "
      f"where that class sits.")
