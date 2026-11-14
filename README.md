# FlyWire Minimum Feedback Arc Set

Feedforward ordering of the FlyWire FAFB connectome: 136,648 neurons,
5,657,719 weighted edges, 41,912,141 synapses. Order the neurons so the
summed weight of forward-pointing edges is maximised.

Challenge: https://codex.flywire.ai/app/mfas_challenge?dataset=fafb

## Results

| | Forward weight | % |
|---|---|---|
| Official baseline (greedy, `sfas`) | 29,023,882 | 69.25% |
| Sifting local search alone | 34,943,313 | 83.37% |
| Relaxation + sifting | 35,155,515 | 83.88% |
| **This repo, best so far** | **35301793** | **84.228%** |
| Leaderboard top (Nov 2026) | 35,544,233 | 84.81% |

## Method

Two solvers, used together.

**Continuous relaxation** (`relax.c`) — each neuron gets a real-valued
position. Backward edge weight is relaxed into a piecewise-linear soft
penalty, optimised with Adam under a temperature schedule that anneals the
penalty from smooth to sharp. Positions are renormalised each step. Sorting
by final position gives a discrete order. Finds the global basin.

**Sifting local search** (`mfas.c`) — repeatedly removes each neuron and
reinserts it at its best position. For neuron v the gain of sliding past u
is w(u,v) - w(v,u), so only v's neighbours matter: sort them by position,
prefix-scan, take the best insertion point. O(deg log deg) per node,
O(E log) per sweep. Converges from the baseline to 83.37% in 11 seconds.

**`solve.c`** combines them and adds iterated local search — perturb by
relocating random neurons, or re-relax from the current order at low
temperature, then re-sift. Checkpoints to disk on every improvement.

## Build and run

```
cc -O3 -march=native -ffast-math -o solve solve.c -lm
./solve graph.csv out.csv 7200 1
python3 score.py graph.csv out.csv
```

`score.py` is an independent pure-stdlib scorer; it reproduces the
official baseline figure exactly and verifies every result here.

## Data

Not committed (233 MB). Fetch with:

```
curl -L -o graph.csv.gz "https://codex.flywire.ai/app/mfas_challenge?download=connectome_graph"
curl -L -o benchmark.csv.gz "https://codex.flywire.ai/app/mfas_challenge?download=benchmark"
gunzip graph.csv.gz benchmark.csv.gz
```
