"""Consensus crossover: order nodes by their mean rank across several solutions."""
import csv, sys, random
out, files = sys.argv[1], sys.argv[2:]
acc = {}
for fn in files:
    with open(fn, newline="") as f:
        r = csv.reader(f); next(r)
        for nid, o in r:
            acc[nid] = acc.get(nid, 0) + int(o)
random.seed(1)
nodes = sorted(acc, key=lambda n: (acc[n], random.random()))
with open(out, "w") as f:
    f.write("Node ID,Order\n")
    for i, n in enumerate(nodes):
        f.write(f"{n},{i}\n")
print(f"{out}: consensus of {len(files)} solutions")
