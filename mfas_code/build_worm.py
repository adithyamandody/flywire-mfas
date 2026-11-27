"""Build an MFAS graph from the C. elegans connectome (White et al. 1986).

usage: python3 -I build_worm.py white_1986_whole.csv all_cell_info.csv \
                                out_graph.csv out_ann.tsv [minw]

Source: the OpenWorm Connectome Toolbox copy of the White et al. 1986
reconstruction, which labels every edge `chemical` or `electrical`.

**Only chemical synapses are used.** Gap junctions are electrical and
symmetric: a gap junction appears as a pair of edges of identical weight in
both directions, which no ordering can ever make forward. Including them
would manufacture irreducible recurrence out of a connection that has no
direction to be wrong about. The catalogue copy of this connectome that
merges the two -- and whose own description says "the meaning of the edge
direction is not documented" -- is unusable for this measurement; 29% of
its reciprocal pairs carry exactly equal weights, which is the signature.

Self-loops are dropped. Connections below `minw` synapses are discarded;
default 1, because this reconstruction is hand-curated rather than
automatically segmented, so a single synapse is evidence rather than noise.
Pass 2 to check the result is not an artefact of that choice.

Pure stdlib.
"""
import csv
import sys

edges_path, cells_path, out_graph, out_ann = sys.argv[1:5]
minw = int(sys.argv[5]) if len(sys.argv) > 5 else 1

pair = {}
kinds = {}
self_loops = 0
with open(edges_path, encoding="utf-8", errors="replace") as f:
    r = csv.reader(f, delimiter="\t")
    header = next(r)
    for row in r:
        if len(row) < 4:
            continue
        pre, post, kind, n = row[0].strip(), row[1].strip(), row[2].strip(), int(row[3])
        kinds[kind] = kinds.get(kind, 0) + n
        if kind != "chemical":
            continue
        if pre == post:
            self_loops += n
            continue
        pair[(pre, post)] = pair.get((pre, post), 0) + n

kept = {k: v for k, v in pair.items() if v >= minw}
nodes = sorted({x for k in kept for x in k})
idx = {name: i for i, name in enumerate(nodes)}
tot = sum(kept.values())

with open(out_graph, "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["Source Node  ID", "Target Node ID", "Edge Weight"])
    for (a, b), v in sorted(kept.items(), key=lambda kv: (idx[kv[0][0]], idx[kv[0][1]])):
        w.writerow([idx[a], idx[b], v])


def role(t, cl):
    s = (t + " " + cl).lower()
    if "muscle" in s:
        return "muscle"
    if "motor neuron" in s:
        return "motor"
    if "interneuron" in s:
        return "interneuron"
    if ("sensory" in s or "amphid" in s or "cephalic" in s
            or "mechanosens" in s or "chemosens" in s or "phasmid" in s):
        return "sensory"
    return "other"


info = {}
with open(cells_path, encoding="utf-8", errors="replace") as f:
    r = csv.reader(f)
    next(r)
    for row in r:
        if len(row) >= 5:
            info[row[0].strip()] = (row[1].strip(), row[4].strip())

n_lab = 0
with open(out_ann, "w", newline="") as f:
    w = csv.writer(f, delimiter="\t")
    w.writerow(["index", "name", "role", "type", "classification"])
    for name in nodes:
        t, cl = info.get(name, ("", ""))
        rl = role(t, cl) if t or cl else "other"
        if rl != "other":
            n_lab += 1
        w.writerow([idx[name], name, rl, t, cl])

print("synapses by edge kind in source:")
for k in sorted(kinds):
    print(f"  {k}: {kinds[k]:,}")
print(f"self-loop synapses dropped ... {self_loops:,}")
print(f"chemical pairs before min {minw} . {len(pair):,}")
print(f"connections kept ............. {len(kept):,}")
print(f"cells with an edge ........... {len(nodes):,}")
print(f"total weight ................. {tot:,}")
print(f"cells given a role ........... {n_lab:,}")
