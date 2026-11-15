"""Upper bound on forward weight from 2-cycles: each pair contributes at most max(w_uv, w_vu)."""
import csv, sys
w = {}
with open(sys.argv[1], newline="") as f:
    r = csv.reader(f); next(r)
    for a, b, x in r:
        w[(a, b)] = w.get((a, b), 0) + int(x)
total = sum(w.values()); forced = 0; recip_pairs = 0
for (a, b), x in w.items():
    if a < b and (b, a) in w:
        recip_pairs += 1
        forced += min(x, w[(b, a)])
print(f"total weight            {total:,}")
print(f"reciprocal pairs        {recip_pairs:,}")
print(f"weight forced backward  {forced:,}  (the smaller side of every reciprocal pair)")
print(f"2-cycle upper bound     {total-forced:,}  ({100*(total-forced)/total:.2f}%)")
