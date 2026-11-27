# Cycle-packing certificates

Each file is a proof. A line reads `weight id1 id2 … idk`, meaning the
directed cycle id1 → id2 → … → idk → id1 carries that weight in the packing.

If every line is a genuine directed cycle of the graph, and no connection is
used by the packing beyond its own weight, then **every ordering of that
connectome must point at least the summed weight backward** — because every
directed cycle contains at least one backward edge whatever the ordering.

Check one yourself:

```
python3 -I mfas_code/check_bound.py mfas_data/worm_graph_w2.csv certificates/celegans_min2_cycles.txt
```

It re-reads the graph and every line, and must report zero lines that are not
real cycles and zero connections used beyond their weight.

| file | graph | cycles | forces backward | of total |
|---|---|---|---|---|
| `celegans_min2_cycles.txt` | `mfas_data/worm_graph_w2.csv` | 97 | 262 | 6,875 (**3.81%**) |
| `celegans_min1_cycles.txt` | `mfas_data/worm_graph.csv` | 361 | 500 | 7,943 (6.29%) |
| `larva_cycles.txt` | `mfas_data/larva_graph.csv` | 14,554 | 35,394 | 300,177 (11.79%) |

The *C. elegans* certificate is 97 cycles. For that connectome the floor it
proves (262) equals the backward weight of the ordering in
`worm_solution.csv`, so the two bounds meet and the quantity is known
exactly rather than bracketed. A determined reader could check that one by
hand.

## The two large ones

The FlyWire and hemibrain packings are not stored here — they run to
millions of cycles. They do not need to be: `bound3c` is **deterministic**,
so the same command reproduces the identical packing, which can then be
checked with the same verifier.

```
CYCLEFILE=cycles.txt ./mfas_code/bound3c mfas_data/graph.csv 600 6
CYCLEFILE=cycles.txt ./mfas_code/bound3c mfas_data/hemibrain_graph.csv 600 6
```

Determinism was confirmed by running the larval packing twice and comparing:
identical cycle counts and an identical bound of 35,394 both times.
