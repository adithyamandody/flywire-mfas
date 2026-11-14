"""Score an MFAS solution: sum of forward edge weights. Pure stdlib."""
import csv, sys

graph_path, order_path = sys.argv[1], sys.argv[2]

pos = {}
with open(order_path, newline="") as f:
    r = csv.reader(f)
    next(r)
    for nid, o in r:
        pos[nid] = int(o)

fwd = bwd = total = self_loops = missing = 0
nodes = set()
with open(graph_path, newline="") as f:
    r = csv.reader(f)
    next(r)
    for a, b, w in r:
        w = int(w)
        total += w
        nodes.add(a); nodes.add(b)
        if a == b:
            self_loops += w
            continue
        pa = pos.get(a); pb = pos.get(b)
        if pa is None or pb is None:
            missing += 1
            continue
        if pa < pb: fwd += w
        else:      bwd += w

print(f"nodes in graph ........ {len(nodes):,}")
print(f"nodes in order ........ {len(pos):,}")
print(f"total weight .......... {total:,}")
print(f"self-loop weight ...... {self_loops:,}")
print(f"edges w/ missing node . {missing:,}")
print(f"FORWARD weight ........ {fwd:,}")
print(f"backward weight ....... {bwd:,}")
print(f"percentage ............ {100*fwd/total:.2f}%")
