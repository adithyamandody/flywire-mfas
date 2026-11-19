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
| + 12 islands, 45 min | 35,402,138 | 84.47% |
| + merged strict islands | 35,413,529 | 84.49% |
| + 12 islands overnight (88 rounds) | 35,446,649 | 84.57% |
| + hub moves, one core, 5 min | 35,454,360 | 84.59% |
| + hub-move island search (47 rounds) | 35,499,190 | 84.70% |
| + TopoShuffle kicks, one core, 10 min | 35,505,143 | 84.71% |
| + TopoShuffle island search (14 rounds) | 35,512,621 | 84.73% |
| + overnight 18 Nov: island merge, regional islands, faster solver | 35,551,497 | 84.82% |
| **This repo, best so far** | **35551569** | **84.824%** |
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

**Merged strict islands** (`overnight4.sh`, `merge.c`) — 12 tabu-ILS islands
in 4-minute rounds. Keeping only the best island wasted ~90% of the compute:
islands gained +21k–26k each, the best only 14% above the mean. Islands that
accept *strictly* improving iterations (`ilsk.c`, strict mode) leave small
diffs from the seed — the neurons outside the longest common subsequence,
found by LIS in O(N log N). `merge` transplants each island's moved neurons
into the best island next to the same predecessor, scores every move
exactly, repairs, and keeps the transplant only if the total rises (falling
back to chunks of 20 if the whole island is rejected). It never does worse
than the best island. At 12 islands: merged result 2.9x and 1.8x the best
single island in the first two rounds.

Plateau islands (accepting zero-gain iterations) cannot be merged this way:
they drift across ~113k of 136k neurons, so their diff is the whole order.

**Hub moves** (`ilsh.c`) — found by comparing with the leader's posted file
(see [LEADERBOARD_ANALYSIS.md](LEADERBOARD_ANALYSIS.md)): the layouts differ
most around hub neurons, which single moves can't shift because thousands of
partners pin them. A hub move picks a top-4% neuron by degree, moves it to one
of its best alternative positions (or a random window), and its HK strongest
partners left on the wrong side follow it, placed right next to it.
Everything moved is frozen while the rest repairs, then released.

5-minute head-to-head from the same start (35,446,649), 3 seeds each:

| kick | gain per core |
|---|---|
| current search (random kicks only) | +14, +23, +12 |
| hub alone | +14, +8, +11 |
| hub + 16 partners | +669, +18, +395 |
| hub + 64 partners | **+7,711**, +547, +487 |

The hub has to move together with its neighbourhood; moving it alone does
nothing. Best-alternative targets beat random windows. Once hub moves open a
new region, ordinary random kicks start winning again there too.

**TopoShuffle kicks** (`ilstopo.c`, idea from Bader et al. 2025,
"Rocket-Crane"): re-sort a stretch of the order, or all of it, along a random
topological order of the pairs it currently gets right. Every right pair stays
right, so the shuffle alone never loses weight and sometimes fixes a wrong
pair for free. The usual repair then runs on everything that moved. Each
shuffle lands on a different order with the same or better score, where
single moves that were blocked before can win again.

10-minute head-to-head from the end of the hub-move search (35,499,190),
where hub moves alone had stalled, 2 seeds each, hub moves on in every run
(`topo_test.sh`):

| kick mix | gain per core |
|---|---|
| hub moves only | +13, +2 |
| + 25% shuffles of a 2,000-neuron window | +1,475, +1,397 |
| + same, also keeping equal-score results | +1,697, +1,650 |
| + 25% shuffles of a 20,000 window, mild randomness | +1,144, +1,172 |
| + 25% shuffles of a 20,000 window | +4,584, +5,317 |
| + 2% shuffles of the whole order | **+5,953**, +5,899 |

Bigger and more random shuffles do better: 175 of 178 whole-order shuffles
won. Most of the gain comes early (+4,215 of +5,953 in the first 2.3
minutes), so the shuffles release a batch of improvements rather than set a
new steady rate; by the end they still gained about +3 per second per core.
Islands that shuffle the whole order can't be merged (their diff is the whole
order), so take the best island instead.

**Overnight, 18 Nov: a merge that works for shuffling islands, regional
islands, a 2.5x faster solver.** Starting from 35,512,621 at 03:30, each step
below was kept only if the next rounds gained more than the ones before it:

| change | from | best by then |
|---|---|---|
| `tmerge`: merge in edge space (below) | 03:40 | 35,512,928 |
| group moves (`ilsg`), merge after every 5-minute round | 03:39 | 35,518,277 |
| hub moves off (0 gain, ~40% of island time), 14 islands | 04:10 | 35,521,958 |
| regional islands: each kicks only inside its slice of the order | 04:31 | 35,525,762 |
| `alloc.py` re-draws the slices every round from where gains came | 04:47 | 35,531,627 |
| move mix rebalanced (TopoShuffle had 75% of time at 0.05/s) | 05:33 | 35,538,007 |
| partner lists kept sorted instead of sorted per evaluation | 06:41 | 35,539,506 |
| islands keep equal-score kicks (non-strict) | 07:32 | 35,551,497 at 11:00 |

- **`tmerge`** folds every island into the best one. The child keeps each
  pair the base gets right unless the island broke it (right in the round's
  seed, wrong in the island), and adds each pair the island fixed (wrong in
  the seed, right in the island); a topological sort of those constraints
  (cycles broken at the smallest key) plus sifting gives the child, kept only
  if it beats the base. Shuffling islands differ from the seed in nearly every
  position but in few pairs, which is why the old position-based merge failed
  on them. First test: best island +130 over the seed, merged +307.
- **Group moves**: an anchor and its K strongest partners are lifted out,
  keeping their order, and put back as one block where the block is best as a
  whole (exact: only edges leaving the group change). Members are frozen
  while the rest repairs.
- **Regional islands**: with every island searching the whole order, the
  merge kept ~34% of what islands found; with disjoint slices, ~64%. Gains
  were concentrated in the last ~15% of the order, so `alloc.py` keeps a heat
  map of gain per unit of order and cuts the order into 12 slices of equal
  heat each round (plus 2 whole-order islands). It also re-tunes how often
  each move type runs from its gain per second.
- **Sorting partner lists was 78% of island time.** A move shifts the
  neurons in between by one place, which keeps their relative order, so only
  the moved neuron's entry in each partner list needs relocating. 2.5x more
  iterations, and on the real graph the result of every evaluation matched
  the old code (zero mismatches).
- **Equal-score kicks**: the edge-space merge doesn't care how far an island
  drifts on a plateau, so islands now keep kicks that tie. Per-round gains
  went from ~+120 to ~+400.

Tried tonight and dropped (measured on the real graph):

- **Exact slot permutations** (DP over subsets: best way to permute up to 16
  strongly linked neurons among the positions they already hold): 5 gains in
  ~490,000 tries. Single-insertion optima are already optimal under these.
- **Optimal re-interleaving** (exact DP merging a set back into the rest,
  keeping both orders, O(|set| x N)): 2,000-neuron stretches +524 in 4 minutes
  from a fresh seed, ~0.1/s afterwards; larger stretches, random sets,
  label-propagation clusters and hub neighbourhoods ~0.
- **Guided TopoShuffle** (keys pulled toward wrong-side partners): the
  shuffle itself gained 0, worse than random keys.
- **Annealing** (T=3) in islands: +5 vs +80 in 150 s; kicks of up to 16
  neurons: +48 vs +80; trying two child constructions in the merge: +329 vs
  +325 for twice the time.

## Upper bound

How feedforward can this brain possibly be? Our ordering proves at least
84.82% is reachable; the question is how much of the remaining gap is even
achievable by anyone.

In any ordering every directed cycle has at least one backward connection.
Give each cycle a weight so that the cycles through any one connection never
use more than that connection's weight; then the backward weight is at least
the sum of the cycle weights, whatever the ordering. `bound3.c` builds such a
packing greedily on residual capacities: 2-cycles first (2 connections spent
per unit of bound), then 3-cycles, then longer ones found by bounded search in
what is left.

| packing | forced backward | upper bound | |
|---|---|---|---|
| reciprocal pairs only (`bound2.py`) | 2,815,303 | 39,096,838 | 93.283% |
| + all 3-cycles | 4,600,109 | 37,312,032 | 89.024% |
| + longer cycles (best run) | 4,697,043 | 37,215,098 | 88.793% |
| **verified packing** (995,074 cycles) | **4,089,720** | **37,822,421** | **90.242%** |

`check_bound.py` re-reads the graph and the dumped packing and checks every
line is a real directed cycle and that no connection is over-used — so the
90.242% row is proved from the graph file alone, not from trusting `bound3.c`.
The 88.793% run was faster because it did not dump its cycles; re-running it
with the dump is what it takes to verify that row too.

So the answer lies in **84.82% – 90.24%**, down from a 93.28% ceiling: the
window on what any ordering could achieve is about a third narrower than the
reciprocal-pair bound makes it look.

What did not work: repeating the 3-cycle sweep with a fresh random node order
adds nothing (the sweep already visits every in/out pair at every node, so one
pass saturates). Capping the search for longer cycles at 20,000 visited
neurons per attempt found half as many as the uncapped search.

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
- **Smart kick targets** — sending kicked neurons to one of their 5 best
  alternative positions instead of a random one: +4,692 vs +9,998.
- **Kick-node locking** (each neuron moves at most once per repair) and
  kick sizes 5/16/32 — within noise of, or worse than, kick size ≤ 8.
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

## What the leaders' posted files tell us

The top five have published nothing, but their submission files are posted.
Comparing #1 with ours (analysis only; we never seed from them): #1 and #2 are
near-identical orderings, while ours differs globally. 4.2% of synapse weight
points the other way, concentrated on hub neurons, and their file still yields
to our ILS quickly. The gap is the global layout, not polish. Details and next
ideas: [LEADERBOARD_ANALYSIS.md](LEADERBOARD_ANALYSIS.md).

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
