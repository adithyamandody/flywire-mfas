# Is the recurrence trend a density artefact? A degree-preserving null

**Question.** Irreducible recurrence (minimum backward weight over all
orderings) is 3.8% / 13.1% / 20.9% / 15.2% in worm / larva / hemibrain /
FlyWire. Connections per cell are 4.4 / 20.0 / 81.2 / 41.4. Does anything
remain of the "rises with nervous-system size" claim once each brain is
compared with random graphs that have its own degree statistics?

**Short answer.** Mostly no. Degree-matched random graphs reproduce the whole
pattern, including the non-monotone step (hemibrain above FlyWire), from
density alone. Every real brain comes out *less* recurrent than its null, not
more. What is left after normalising is a real/null ratio of about 0.64, 0.67,
0.90, 0.79 — not constant, but not ordered by size either (it is ordered by
density), and only the worm's ratio is proved rather than estimated.

## Method

`mfas_code/rewire.py` (pure stdlib; `--selftest` passes). Double-edge swaps:
pick a->b and c->d at random, replace with a->d and c->b, reject if that makes
a self-loop or an existing connection. 10 swap attempts per connection
(82% accepted in the worm, 96–99% elsewhere; 98–100% of targets moved). Each
connection keeps its weight and its source. Reciprocal pairs are not forbidden.

After writing each null the script re-reads the file and asserts: in-degree and
out-degree of every node identical to the original, weight multiset identical,
out-strength of every node identical, no self-loops, no duplicates. All 15
nulls passed. Nulls are reproducible: `rewire.py graph.csv out.csv --seed i`,
seeds 1..5 (worm, larva), 1..3 (hemibrain), 1..2 (FlyWire).

What the null does **not** preserve: in-strength per node, and any pairing
between weight and target. It destroys reciprocity almost entirely:

| system | weight on reciprocated connections, real | same, nulls |
|---|---|---|
| worm | 9.88% | 2.1–3.6% |
| larva | 20.55% | 1.5–1.7% |
| hemibrain | 41.94% | 1.70–1.75% |
| FlyWire | 24.86% | 0.45% |

Floors: `bound3c graph secs 6`, every packing re-verified by `check_bound.py`
(0 non-cycles, 0 over-used connections in every case). Budgets 120 s (worm,
larva), 900 s (hemibrain), 1800 s (FlyWire). Ceilings: `springrank` then
`ilsg` with the standard tuning arguments, seed 1, budgets 120 s (worm), 600 s
(larva), 1800 s (hemibrain), 2400 s (FlyWire); every ordering re-scored by
`score.py`. The **real graphs were re-run through the identical pipeline and
budget**, so real and null ceilings are like for like.

## Results

Percent of total synaptic weight forced backward. Null columns are mean
[min–max] over replicates.

| system | conns/cell | real floor | real ceiling | null floor | null ceiling | nulls |
|---|---|---|---|---|---|---|
| worm | 4.4 | 3.81% | 3.81% | 4.93% [4.64–5.24] | 5.93% [5.57–6.31] | 5 |
| larva | 20.0 | 11.79% | 13.06% | 12.42% [12.38–12.44] | 19.43% [19.36–19.52] | 5 |
| hemibrain | 81.2 | 18.94% | 20.84% | 13.81% [13.79–13.85] | 23.10% [23.05–23.17] | 3 |
| FlyWire | 41.4 | 12.63% | 15.18% (15.46% at equal effort) | 5.77% [5.74–5.79] | 19.64% [19.63–19.64] | 2 |

Raw counts (backward weight; total weight 6,875 / 300,177 / 12,542,949 /
41,912,141):

| system | real floor | real ceiling | null floors | null ceilings |
|---|---|---|---|---|
| worm | 262 | 262 | 337, 360, 320, 359, 319 | 407, 428, 383, 434, 387 |
| larva | 35,394 | 39,213 | 37,318, 37,336, 37,235, 37,151, 37,299 | 58,484, 58,126, 58,595, 58,327, 58,150 |
| hemibrain | 2,376,234 | 2,614,328 | 1,730,548, 1,737,090, 1,729,380 | 2,893,363, 2,905,998, 2,891,427 |
| FlyWire | 5,291,688 | 6,360,572 (best_solution.csv); 6,478,433 (this pipeline, 2400 s) | 2,427,019, 2,406,913 | 8,231,820, 8,227,576 |

### Ratio real / null

| system | cells | conns/cell | **point estimate** (ceiling / ceiling, equal effort) | **proved interval** (real floor / max null ceiling … real ceiling / min null floor) |
|---|---|---|---|---|
| worm | 297 | 4.4 | 0.64 [0.60–0.68 across nulls] | 0.60 … 0.82 |
| larva | 2,923 | 20.0 | 0.67 [0.669–0.675] | 0.60 … 1.06 |
| hemibrain | 21,737 | 81.2 | 0.90 [0.900–0.904] | 0.82 … 1.51 |
| FlyWire | 136,648 | 41.4 | 0.79 (0.77 using the best known real ordering) | 0.64 … 2.64 |

## What is proved and what is estimated

* **Worm: proved.** Every worm null's floor (4.64–5.24%) exceeds the worm's
  exact value (3.81%). The worm is less recurrent than its own degree sequence
  demands, by a factor between 0.60 and 0.82.
* **Larva, hemibrain, FlyWire: estimated, not proved.** The efficiency shortcut
  (null floor above real ceiling) does not fire. Cycle packing is a weak
  certificate on the nulls: rewiring removes nearly all 2-cycles, the packing
  then has to use long cycles that spend several connections per unit of
  bound, and floor and ceiling end up 7–14 points apart (against 0–2.6 for the
  real graphs). The larva floor is converged, not time-limited (the search finishes
  in 2 s; informal probes with more passes or greater depth did not move it
  materially). The hemibrain and FlyWire
  null floors hit their time limits and are certainly loose.
* So for the three larger systems the comparison rests on ceilings: the same
  solver, same initialiser, same budget, on real and null. Under that
  comparison the real graph is lower in every case, by 2.1 to 6.4 points, with
  replicate spread of 0.01–0.16 points (0.74 in the worm). The solver was
  nearly flat by the end of the larva, hemibrain and FlyWire runs (the last 20%
  of the budget improved the score by <0.03 points in each); a 1500 s second-seed run on one larva null
  gave 58,420 against 58,484 at 600 s. On FlyWire, where weeks of effort exist
  for the real graph, the equal-effort run is 0.28 points above the best known
  ordering — that is the size of the solver slack to keep in mind.
* The only way the point estimates could be badly wrong is if the solver is
  several points worse on random graphs than on real ones. The worm null, where
  the bracket is tightest, puts the truth within about one point of the ceiling.
  I think the estimates are sound, but they are not certificates.

## Reading

1. **The cross-system pattern is a density pattern.** Null recurrence is
   5.9% / 19.4% / 23.1% / 19.6% at 4.4 / 20.0 / 81.2 / 41.4 connections per
   cell: monotone in density, and it reproduces the feature that breaks the
   size story (hemibrain, 21.7k cells, above FlyWire, 136.6k cells). The real
   values 3.8 / 13.1 / 20.8 / 15.2 track it. Randomly wired graphs with these
   degree sequences would have produced the draft's table.
2. **Real brains are less recurrent than chance, in all four.** Despite 3–55×
   more reciprocal weight than their nulls, every connectome needs less
   backward weight than a random graph of the same degrees. Local reciprocity
   is high; global loop structure is low. That is a statement about wiring and
   the null does not explain it away — but it is the opposite sign from
   "brains accumulate recurrence".
3. **The residual does not follow size.** Ratios in size order: 0.64, 0.67,
   0.90, 0.79. Not monotone. In density order (worm, larva, FlyWire,
   hemibrain) they are monotone: 0.64, 0.67, 0.79, 0.90 — the denser the
   graph, the closer the brain sits to its null. Four points, one of them a
   truncated subvolume (hemibrain), and a solver whose slack may itself grow
   with density: this is a hint, not a finding.
4. **Verdict on the draft claim.** "Irreducible recurrence rises with
   nervous-system size" is not supported once density is controlled. The rise
   is what density predicts, the one non-monotone point is what density
   predicts, and the density-normalised quantity is not ordered by size.

## By-products

* The like-for-like reruns produced slightly better real orderings than the
  ones in the repo root: larva 39,213 backward (was 39,340; 13.06% vs 13.11%)
  and hemibrain 2,614,328 (was 2,617,466; 20.84% vs 20.87%). Both confirmed by
  `score.py`. Saved as `runs/nullmodel/larva_real.ord.csv` and
  `runs/nullmodel/hemi_real.ord.csv`; the root solution files are untouched.
* Hemibrain real floor from this 900 s run is 2,376,234 (18.94%), verified;
  the 18.96% in the draft table is from a different run that I did not
  re-check.
* **Real FlyWire floor.** The 1800 s `bound3c` run done here for parity packs
  5,291,688 units (12.63%), verified by `check_bound.py` with 0 non-cycles and
  0 over-used connections. That is well above the 9.76% in the draft table
  (bracket 12.63–15.18% instead of 9.76–15.18%). The packing is saved as
  `runs/nullmodel/fly_real_cycles.txt` (git-ignored).
* The two FlyWire nulls have the same reciprocal-pair count (11,340). Checked:
  the graphs are different (35,536 shared connections out of 5.66M;
  reciprocated weight 188,810 vs 187,230). Coincidence.

## Reproduce

    python3 -I mfas_code/rewire.py --selftest
    python3 -I mfas_code/rewire.py mfas_data/larva_graph.csv null.csv --seed 1
    CYCLEFILE=cyc.txt ./mfas_code/bound3c null.csv 120 6
    python3 -I mfas_code/check_bound.py null.csv cyc.txt
    ./mfas_code/springrank null.csv sr.csv 0.01 3000
    ./mfas_code/ilsg null.csv ord.csv 600 1 sr.csv 0 8 1 0 0 0 0 2 64 0.04 1 0.3 0 -1 1 0.3 32 1 0.3 0 12 1 0.3 0 2000 1 0 0
    python3 -I mfas_code/score.py null.csv ord.csv

Scorer and checker outputs for every number above are in `runs/nullmodel/`
(git-ignored): `*.check`, `*.score`, `*.rewire.log`.
