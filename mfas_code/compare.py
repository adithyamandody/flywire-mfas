"""Compare two neuron orderings of the FlyWire MFAS graph. Pure stdlib.

usage: python3 -I compare.py graph.csv orderA.csv orderB.csv

Reports, for A -> B:
  * score of each, and the weight of edges whose direction differs
  * how many neurons must move to turn A into B (n - LIS)
  * the shape of those moves: are moved neurons moved alone, or in blocks
    that were already adjacent in A and stay adjacent in B?
  * how far they move, and rank correlation of the two orders
"""
import csv, sys
from bisect import bisect_left

def read_order(path):
    with open(path, newline="") as f:
        r = csv.reader(f); next(r)
        rows = [(int(o), int(n)) for n, o in r]
    rows.sort()
    return [n for _, n in rows]

graph_path, a_path, b_path = sys.argv[1:4]
A = read_order(a_path); B = read_order(b_path)
idx = {n: i for i, n in enumerate(A)}
n = len(A)
assert len(B) == n and all(x in idx for x in B)
posA = list(range(n))                      # node i is A[i]
posB = [0] * n
for p, x in enumerate(B): posB[idx[x]] = p

# ---- edges -------------------------------------------------------------
fa = fb = tot = a_only = b_only = 0
na_only = nb_only = 0
with open(graph_path, newline="") as f:
    r = csv.reader(f); next(r)
    for s, t, w in r:
        u = idx[int(s)]; v = idx[int(t)]; w = int(w); tot += w
        x = posA[u] < posA[v]; y = posB[u] < posB[v]
        if x: fa += w
        if y: fb += w
        if x and not y: a_only += w; na_only += 1
        elif y and not x: b_only += w; nb_only += 1
print(f"score A {fa:,} ({100*fa/tot:.3f}%)   score B {fb:,} ({100*fb/tot:.3f}%)   B-A = {fb-fa:+,}")
print(f"edges forward in A only: {na_only:,} edges, weight {a_only:,}")
print(f"edges forward in B only: {nb_only:,} edges, weight {b_only:,}")
print(f"=> {100*(a_only+b_only)/tot:.2f}% of all synapse weight flips direction between them")

# ---- LIS: largest set of neurons already in B's relative order -----------
seq = [posA[idx[x]] for x in B]            # A-positions read in B's order
tails, tails_i, prev = [], [], [-1] * n
for i, v in enumerate(seq):
    k = bisect_left(tails, v)
    if k == len(tails): tails.append(v); tails_i.append(i)
    else: tails[k] = v; tails_i[k] = i
    prev[i] = tails_i[k - 1] if k else -1
keep = [False] * n
i = tails_i[-1] if tails_i else -1
while i >= 0: keep[i] = True; i = prev[i]
moved = n - len(tails)
print(f"\nneurons that stay in relative order (LIS): {len(tails):,}")
print(f"neurons that must move to turn A into B:  {moved:,} ({100*moved/n:.2f}%)")

# ---- shape: blocks of moved neurons adjacent in both orders -------------
blocks, cur = [], 0
for i in range(n):
    if keep[i]:
        if cur: blocks.append(cur); cur = 0
        continue
    if cur and not keep[i - 1] and seq[i] == seq[i - 1] + 1: cur += 1
    else:
        if cur: blocks.append(cur)
        cur = 1
if cur: blocks.append(cur)
bins = [(1, 1), (2, 3), (4, 10), (11, 100), (101, 10**9)]
print(f"moved neurons form {len(blocks):,} blocks (adjacent in A and still adjacent in B):")
for lo, hi in bins:
    c = sum(1 for b in blocks if lo <= b <= hi); m = sum(b for b in blocks if lo <= b <= hi)
    lab = f"{lo}" if lo == hi else (f"{lo}-{hi}" if hi < 10**9 else f">{lo-1}")
    print(f"   size {lab:>7}: {c:7,} blocks, {m:7,} neurons ({100*m/max(moved,1):5.1f}% of moved)")

# ---- displacement ---------------------------------------------------------
d = sorted(abs(posA[i] - posB[i]) for i in range(n))
dm = sorted(abs(seq[i] - i) for i in range(n) if not keep[i])
q = lambda a, p: a[min(len(a) - 1, int(p * len(a)))]
print(f"\n|posA - posB| over all neurons: median {q(d,.5):,}  p90 {q(d,.9):,}  p99 {q(d,.99):,}  max {d[-1]:,}")
if dm:
    print(f"   over moved neurons only:     median {q(dm,.5):,}  p90 {q(dm,.9):,}  p99 {q(dm,.99):,}")
mean = (n - 1) / 2
sxy = sum((posA[i] - mean) * (posB[i] - mean) for i in range(n))
sxx = sum((p - mean) ** 2 for p in range(n))
print(f"Spearman rank correlation: {sxy/sxx:.6f}")
