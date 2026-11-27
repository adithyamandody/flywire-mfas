"""Build an MFAS graph from the Drosophila larva connectome (Winding 2023).

usage: python3 -I build_larva.py edges.csv nodes.csv out_graph.csv out_ann.tsv [minw]

The published larval connectome is a MULTIGRAPH: a pair of neurons can be
joined by up to four parallel edges, one per compartment pair -- 'ad'
(axo-dendritic), 'aa' (axo-axonic), 'da' (dendro-axonic), 'dd'
(dendro-dendritic) -- each weighted by its synapse count. The FlyWire and
hemibrain graphs we compare against count every synapse between two neurons
regardless of compartment, so to match them we SUM across all four types.
That choice is stated in the methods; splitting by compartment instead would
be a different and also defensible analysis.

Self-loops are dropped (they cannot point either way in an ordering) and
connections below `minw` synapses are discarded, default 2, matching the
threshold of the FlyWire challenge graph.

Output is the challenge's own CSV format, so every existing tool in this
repo reads it unchanged.

Pure stdlib.
"""
import csv
import sys

edges_path, nodes_path, out_graph, out_ann = sys.argv[1:5]
minw = int(sys.argv[5]) if len(sys.argv) > 5 else 2


def rows(path):
    """Yield data rows, skipping the '# col, col' comment header."""
    with open(path, newline="", encoding="utf-8", errors="replace") as f:
        for row in csv.reader(f):
            if not row or row[0].lstrip().startswith("#"):
                continue
            yield row


pair = {}
by_type = {}
self_loops = 0
for r in rows(edges_path):
    s, t, c, et = r[0], r[1], int(r[2]), r[3]
    by_type[et] = by_type.get(et, 0) + c
    if s == t:
        self_loops += c
        continue
    k = (s, t)
    pair[k] = pair.get(k, 0) + c

kept = {k: w for k, w in pair.items() if w >= minw}
tot = sum(kept.values())
nodes = set()
for s, t in kept:
    nodes.add(s)
    nodes.add(t)

with open(out_graph, "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["Source Node  ID", "Target Node ID", "Edge Weight"])
    for (s, t), x in sorted(kept.items(), key=lambda kv: (int(kv[0][0]), int(kv[0][1]))):
        w.writerow([s, t, x])

n_ann = 0
with open(out_ann, "w", newline="") as f:
    w = csv.writer(f, delimiter="\t")
    w.writerow(["index", "vids", "hemisphere", "cell_type", "annotations", "cluster"])
    for r in rows(nodes_path):
        if len(r) < 7:
            continue
        w.writerow([r[0], r[1], r[2], r[4], r[5], r[6]])
        n_ann += 1

print("synapses by compartment type:")
for et in sorted(by_type):
    print(f"  {et}: {by_type[et]:,}")
print(f"self-loop synapses dropped ... {self_loops:,}")
print(f"neuron pairs before min {minw} .. {len(pair):,}")
print(f"connections kept ............ {len(kept):,}")
print(f"neurons with an edge ........ {len(nodes):,}")
print(f"total weight ................ {tot:,}")
print(f"annotations written ......... {n_ann:,}")
