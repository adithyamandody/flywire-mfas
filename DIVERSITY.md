# Is there more than one basin? (27 November 2026)

Prompted by the leaderboard analysis: an unrelated search holds 811k of
weight ours misses, while all ten islands of our own overnight run had
collapsed to one identical ordering. Either the layout our pipeline finds is
forced, or our search had simply stopped exploring.

## The experiment

Six arms, **identical search settings**, different starting layouts and
seeds, 30 minutes each, run in parallel and **never merged** — the merge
step is what destroyed the diversity of the islands. Five started from
SpringRank layouts differing in `alpha` and in how far the solver was run;
the sixth started from our committed best and is the control.

`mfas_code/diversity.sh` runs it; `mfas_code/divcmp.py` loads the graph once
and compares every pair.

## Result: many basins, and they are near-equal

| arm | start | forward | % |
|---|---|---|---|
| sr-a.01 | α=0.01, 3000 iters | 35,446,229 | 84.573% |
| sr-a.001 | α=0.001 | 35,421,119 | 84.513% |
| sr-a.1 | α=0.1 | 35,437,120 | 84.551% |
| sr-i200 | α=0.01, 200 iters | 35,433,639 | 84.543% |
| sr-i20k | α=0.01, 20000 iters | 35,431,921 | 84.539% |
| ours (control) | our best | 35,551,576 | 84.824% |

The five fresh arms span 25,110 weight — 0.06%. Pairwise, they disagree on
**about a million units of weight each time**:

| pair | contested | net |
|---|---|---|
| sr-a.01 vs sr-a.001 | 1,164,938 | 25,110 |
| sr-a.01 vs sr-a.1 | 1,191,423 | 9,109 |
| sr-i200 vs sr-i20k | 991,416 | 1,718 |
| sr-a.001 vs sr-i20k | 1,006,978 | −10,802 |

Runs that finish within a few thousand of each other disagree about the
direction of 2.4% of the entire brain. **The landscape is not one funnel.**
It holds many near-equal arrangements that differ enormously in structure,
which also explains the leaderboard: the 811k that the Westfall/Denoual
lineage captures and we miss is not special, it is one more basin.

The control arm gained **7 weight in 30 minutes** (35,551,569 → 35,551,576),
confirming our own basin is exhausted. We have not promoted it to
`best_solution.csv`: updating the paper, the README and the leaderboard
entry for 7 weight would create more inconsistency than it removes. The
ordering is in `runs/div/out6.csv` if it is ever wanted.

Also worth noting: 30 minutes from a cold start reaches 84.54%, and days of
compute bought 0.28 points beyond that. Diminishing returns here are severe.

## The objection this raises, and the answer

If many near-equal orderings disagree on a million units of weight, why
should anyone believe biology read off one of them? Asked directly —
median position of each super class, in all six orderings:

| class | a.01 | a.001 | a.1 | i200 | i20k | ours | spread |
|---|---|---|---|---|---|---|---|
| sensory | 7.9% | 7.8% | 7.8% | 7.7% | 7.8% | 7.9% | **0.1** |
| ascending | 7.4% | 7.6% | 7.2% | 7.3% | 7.4% | 7.1% | 0.5 |
| visual_centrifugal | 13.9% | 15.0% | 18.4% | 15.1% | 15.1% | 16.9% | 4.5 |
| optic | 45.7% | 45.8% | 46.0% | 46.2% | 45.9% | 47.0% | 1.3 |
| visual_projection | 77.5% | 77.7% | 78.4% | 77.5% | 77.7% | 78.1% | 0.9 |
| central | 86.4% | 86.6% | 86.1% | 86.5% | 86.4% | 85.5% | 1.1 |
| descending | 99.0% | 97.6% | 98.3% | 99.0% | 98.9% | 99.0% | 1.5 |
| motor | 99.8% | 99.9% | 99.8% | 99.8% | 99.8% | 99.8% | **0.1** |

**Six orderings that disagree about 2.4% of the brain's wiring place sensory
neurons at 7.8 ± 0.1% and motor neurons at 99.8 ± 0.1%.** The axis is not an
artifact of which basin the search lands in. The paper's limitation — that
class-level results involve thousands of neurons and are insensitive to the
particular ordering — is now measured rather than asserted.

## Where it is *not* stable

At finer granularity some classes move a great deal:

| cell class | fresh arms | ours | spread |
|---|---|---|---|
| ocellar | 11.7–48.0% | 44.6% | 36.3 |
| Kenyon_Cell | 17.2–18.8% | 39.5% | 22.3 |
| ALPN | 15.6–16.8% | 37.2% | 21.6 |

Our basin places Kenyon cells and antennal lobe projection neurons about
twenty points later than every fresh arm does. Nothing says which is right —
the fresh arms score 0.28 points lower, and the true optimum is unknown —
but it means **the mushroom body's position in the hierarchy is not
determined by this method**, and no claim should rest on it. The paper makes
none: its mushroom body statements are about APL's connectivity and the
weight it carries backward, both of which are properties of the wiring
rather than of a position in the order.

## What to do with this

The headroom is in exploration, not polish, and now we know its shape:
restarts that are kept apart produce genuinely different solutions of
near-equal quality. A campaign that ran many such arms for a long time and
never merged them would sample the basin structure properly. Whether that
yields more than tens of thousands of weight is unknown, and the honest
expectation from the history is "one more step of 6k–45k, not more".
