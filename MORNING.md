# Overnight report — Sunday 15 Nov 2026

**Live number:** run `~/connectome/status.sh`. The search stops at 11:20 and
commits its final result to GitHub by itself.

## Where it stood at 05:25

| | Forward weight | % |
|---|---|---|
| When you went to sleep (03:55) | 35,309,952 | 84.25% |
| **05:25, independently verified** | **35,402,292** | **84.47%** |
| 2024 runner-up (CIBR) — passed ~04:55 | 35,374,656 | 84.40% |
| 2024 winner (Turaga lab) — next | 35,435,948 | 84.55% |
| Vahidi & Koutis (published method) | 35,463,823 | 84.61% |
| Leaderboard top | 35,544,233 | 84.81% |

## What happened

The gap was never compute. Vahidi & Koutis reached 84.61% in Python on
Colab; my C code on 12 cores was stuck at 84.26%. So the night went into
finding better *moves*, testing each idea head-to-head from the same start.

**Three things worked, and they stack:**

1. **Worklist ILS.** After a kick, only re-examine neurons that could have
   changed (the kicked ones, then neighbours of anything that moves).
   800x more iterations per second; 2.8x the improvement rate.
2. **Tabu after kick.** The search was spending nearly every iteration
   undoing its own kick — the repair moved the kicked neuron straight back.
   Freezing kicked neurons during repair: 6x the improvement rate again.
3. **Merging islands.** 12 cores were delivering ~1.14 cores of progress —
   each round kept one island and threw away eleven islands' gains. Islands
   that accept only strict improvements leave small, precise diffs, which
   can be transplanted into the best island.

**Nine things did not, all measured:** contiguous block moves, consensus
crossover, relaxation loss shape, 5x longer relaxation, SpringRank
initialisation, targeted backward-edge kicks, simulated annealing, smart
kick targets, kick-node locking. Recorded in README.md so nobody repeats them.

The recurring lesson: on this problem, *random* perturbation beat every
*targeted* one I tried. Diversity mattered more than plausibility.

## Honest assessment

**Merging works at scale.** At 12 islands, round 1's merged result was 2.9x
the best single island (+5,949 vs +2,043), round 2's was 1.8x (+5,442 vs
+2,986). Net: about 1.5x faster than the plateau islands it replaced.

**But gains are decaying** — before merging, the improvement rate was roughly
halving every 20 minutes. Extrapolating, the night most likely ends around
**35.44–35.50M**: somewhere between 6th and 10th on the board, level with or
past the 2024 winner and the published Vahidi–Koutis method.

**Beating 35,544,233 tonight is unlikely with these methods.** That would
need a deeper basin or a qualitatively different neighbourhood, and nobody
has published what the October–November leaders did.

**Nobody knows where the ceiling is.** 519,895 reciprocal neuron pairs force
2.8M of weight backward no matter what, which caps any ordering at
39,096,838 (93.28%). That bound is loose — longer cycles force more — so it
can't tell us whether 35.54M is near-optimal or far from it.

## Still open for you

- **The BANC proofreading form** — https://flywire.ai/banc_access — three
  fields, two minutes. Nothing overnight touches it; it's yours to send.
- `cleanup.sh` removes the per-run files if you want the disk back.
