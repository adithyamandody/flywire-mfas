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
| + slow ILS, 12 cores, 50 min | 35,326,977 | 84.29% |
| + fast worklist ILS, 12 islands | 35,340,935 | 84.32% |
| + tabu after kick | 35,357,901 | 84.36% |
| **This repo, best so far** | **35357901** | **84.362%** |
| Leaderboard top (Nov 2026) | 35,544,233 | 84.81% |

## What works

**Sifting** (`mfas.c`) — remove a neuron, reinsert it at its best position.
For neuron v the gain of sliding past u is w(u,v) − w(v,u), so only v's
neighbours matter: sort them by position, prefix-scan both directions, take
the best. O(deg log deg) per neuron.

**Continuous relaxation** (`relax.c`, `relax2.c`) — each neuron gets a real
position; backward weight is relaxed into a soft penalty and optimised with
Adam under an annealed temperature. Finds a good basin from a random start
in ~1 minute.

**Fast iterated local search** (`ils.c`) — the main engine. Kick 1–8 random
neurons to new positions (window 2k–60k or anywhere), then re-sift *only*
what could have changed: the kicked neurons, then transitively the
neighbours of anything that moves. Moving a neuron only alters its own
neighbours' options, so this reaches the same local optimum as a full sweep
at a tiny fraction of the cost. Every move is logged; a losing iteration is
undone exactly. ~330 iterations/s on one core vs ~0.4/s for full re-sweeps.
Head-to-head over 4 minutes from the same converged start: +5,248 vs +1,843.

**Tabu after kick** (`ilst.c`, now `ils.c`) — the kicked neurons are frozen
while the repair cascade runs, then released for a final repair. Without
this, nearly every iteration ended at exactly zero: the repair simply moved
the kicked neuron back to its old position, which was its best position
before the kick. Freezing forces the neighbours to adapt to the new
arrangement. Same seed, same 5 minutes: +12,698 vs +2,090 — 6x the rate,
with fewer iterations (28k vs 100k) each doing real work.

**Island model** (`overnight3.sh`) — 12 tabu-ILS islands from the global best in
15-minute rounds; each round restarts every island from the best found.

## What did not work (measured, not guessed)

- **Contiguous block moves** (`solve2.c`, multilevel sifting of runs of
  2–512 consecutive neurons). Found +3,000 from a converged optimum in 30 s,
  then stalled; slower than ILS overall. Every gain came from blocks of 2–5;
  blocks ≥96 never helped in ~1,700 tries. The groups that must move together
  are graph-defined, not contiguous in the order.
- **Consensus crossover** (`xover.py`, order by mean rank across 2, 4 or 8
  good solutions). Every child polished to 84.21–84.22%, below every parent.
  Good local optima here are further apart than a "big valley" assumes.
- **Loss shape in the relaxation.** Clamped-linear, logistic and arctan
  losses all land in basins of the same quality (83.7–84.0% after sifting).
- **Targeted backward-edge kicks** (`ilsb.c`) — force one end of a heavy
  backward edge past the other. 5 min from the same seed: +5,657 vs +8,008
  for random kicks; a 50/50 mix was in between.
- **Simulated annealing** (`ilssa.c`) at T = 3 and 15: indistinguishable from
  T = 0. Only 21–38 losing iterations in ~100k were small enough to accept;
  kicks that lose, lose big.
- **Longer relaxation** — 30k steps (5x) with or without periodic re-ranking
  lands in the same basins as 6k steps.
- **SpringRank initialisation** (`springrank.c`, least-squares potentials
  via preconditioned CG). 77.8% raw, 83.76% after sifting — below a random
  relaxation start.

## Prior work

- Vahidi & Koutis, *Feedforward Ordering in Neural Connectomes via Feedback
  Arc Minimization*, arXiv:2506.13799 — pair moves along backward edges,
  exhaustive permutation of small SCCs and of group windows; 35,462,925.
- Vahidi & Koutis, *Interval-Based Refinement Algorithms for Large-Scale
  Weighted Feedback Arc Set*, SSRN 6221201 (Feb 2026).
- TuragaLab/connectome_reordering — relaxation in JAX + Monte Carlo swaps
  (2024 winner, 35,435,948).
- CIBR-Okubo-Lab/min_feedback_final — ILS over the insertion neighbourhood
  (35,374,656).

## Build and run

```
cc -O3 -march=native -o ils mfas_code/ils.c
./ils graph.csv out.csv 1800 1 start_order.csv
python3 mfas_code/score.py graph.csv out.csv
```

`score.py` is an independent pure-stdlib scorer; it reproduces the official
baseline figure exactly and verifies every result here.

## Data

Not committed (233 MB). Fetch with:

```
curl -L -o graph.csv.gz "https://codex.flywire.ai/app/mfas_challenge?download=connectome_graph"
curl -L -o benchmark.csv.gz "https://codex.flywire.ai/app/mfas_challenge?download=benchmark"
gunzip graph.csv.gz benchmark.csv.gz
```
