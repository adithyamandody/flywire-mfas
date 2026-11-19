"""Check a cycle packing, and so the upper bound it proves. Pure stdlib.

usage: python3 -I check_bound.py graph.csv cycles.txt

cycles.txt has one cycle per line: "weight id1 id2 ... idk", meaning the
directed cycle id1 -> id2 -> ... -> idk -> id1 carries that weight.

The check: every line must be a real directed cycle in the graph, and the
weights through any one connection must not exceed that connection's weight.
If both hold then, in ANY ordering of the neurons, every cycle has at least one
backward connection, so the backward weight is at least the sum of the cycle
weights -- and the forward weight is at most total minus that sum.
"""
import csv, sys

graph_path, cyc_path = sys.argv[1], sys.argv[2]

w = {}
total = 0
with open(graph_path, newline="") as f:
    r = csv.reader(f); next(r)
    for s, t, x in r:
        x = int(x); total += x
        k = (int(s) << 64) | int(t)
        w[k] = w.get(k, 0) + x
print(f"graph: {len(w):,} connections, total weight {total:,}")

used = {}
ncyc = 0
packed = 0
by_len = {}
bad_edge = bad_weight = 0
with open(cyc_path) as f:
    for line in f:
        p = line.split()
        if len(p) < 3:
            bad_edge += 1; continue
        push = int(p[0]); nodes = [int(x) for x in p[1:]]
        if push <= 0 or len(set(nodes)) != len(nodes):
            bad_edge += 1; continue
        ok = True
        for i in range(len(nodes)):
            k = (nodes[i] << 64) | nodes[(i + 1) % len(nodes)]
            if k not in w:
                ok = False; break
        if not ok:
            bad_edge += 1; continue
        for i in range(len(nodes)):
            k = (nodes[i] << 64) | nodes[(i + 1) % len(nodes)]
            used[k] = used.get(k, 0) + push
        ncyc += 1; packed += push
        by_len[len(nodes)] = by_len.get(len(nodes), 0) + 1

for k, u in used.items():
    if u > w[k]:
        bad_weight += 1

print(f"cycles read: {ncyc:,}")
print("  by length: " + ", ".join(f"{L}:{n:,}" for L, n in sorted(by_len.items())))
print(f"lines that are not real cycles: {bad_edge:,}   (must be 0)")
print(f"connections used beyond their weight: {bad_weight:,}   (must be 0)")
print(f"connections touched: {len(used):,}")

if bad_edge or bad_weight:
    print("\nPACKING INVALID - the bound below is not proved")
    sys.exit(1)

print(f"\nweight forced backward in any ordering  {packed:,}")
print(f"UPPER BOUND on forward weight           {total-packed:,}  ({100*(total-packed)/total:.3f}%)")
