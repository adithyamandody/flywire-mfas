"""Build an MFAS graph from the hemibrain v1.2 traced adjacency export.

usage: python3 -I build_hemibrain.py traced-neurons.csv traced-total-connections.csv out_graph.csv [minw]

Keeps connections of at least `minw` synapses (default 2, matching the
minimum weight in the FlyWire challenge graph) between two traced,
non-cropped neurons. Output is in the challenge's own CSV format so every
tool we already have -- solver, scorer, bound -- reads it unchanged.
"""
import csv, sys

neurons_path, conns_path, out_path = sys.argv[1:4]
minw = int(sys.argv[4]) if len(sys.argv) > 4 else 2

traced = set()
with open(neurons_path, newline="") as f:
    r = csv.reader(f); next(r)
    for row in r:
        traced.add(int(row[0]))

kept = dropped_w = dropped_untraced = self_loops = 0
tot = 0
with open(conns_path, newline="") as f, open(out_path, "w", newline="") as g:
    r = csv.reader(f); next(r)
    g.write("Source Node ID,Target Node ID,Edge Weight\n")
    for a, b, w in r:
        a = int(a); b = int(b); w = int(w)
        if a == b:
            self_loops += 1; continue
        if w < minw:
            dropped_w += 1; continue
        if a not in traced or b not in traced:
            dropped_untraced += 1; continue
        g.write(f"{a},{b},{w}\n"); kept += 1; tot += w

nodes = set()
with open(out_path, newline="") as f:
    r = csv.reader(f); next(r)
    for a, b, w in r:
        nodes.add(a); nodes.add(b)

print(f"traced neurons in export : {len(traced):,}")
print(f"connections kept         : {kept:,}  (weight {tot:,})")
print(f"  dropped, below minw={minw} : {dropped_w:,}")
print(f"  dropped, untraced end  : {dropped_untraced:,}")
print(f"  self loops dropped     : {self_loops:,}")
print(f"neurons with an edge     : {len(nodes):,}")
