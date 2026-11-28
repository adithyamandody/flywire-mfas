# Can the bound be tightened? (27 November 2026)

> **Correction, 28 November 2026. The conclusion below is wrong, and the
> way it was wrong is the point.** This document argued that the 5.42-point
> FlyWire gap was the relaxation rather than our implementation, on the
> strength of a 0.031-point experiment. It was our implementation: the
> packing behind the 90.242% ceiling came from a run whose 3-cycle sweep hit
> its time limit after 108,374 of 136,648 nodes. A complete run packs
> 5,291,688 units — verified at 1,513,396 cycles with no over-used
> connection — giving a floor of 12.63% and a ceiling of 87.374%. The gap
> is 2.55 points, not 5.42, and 2.87 of the "irreducible" gap was simply a
> timeout. The finding below about exact local solves still stands; the
> inference drawn from it does not.
>
> *The original text follows unchanged, including its now-superseded
> numbers. It is kept because the experiment it reports is sound and
> the inference it drew is the instructive part.*

The FlyWire gap — 84.824% achieved against a 90.242% ceiling — is the
largest open number in this work, and the cross-species recurrence
comparison is only as good as it. This asks whether the gap is our search
falling short or our bound being weak, and answers it in one direction.

## A strictly stronger bound, and why it is valid

Any global ordering induces an ordering on every vertex subset S. So the
backward weight inside G[S] is at least the **exact minimum feedback arc
set** of G[S], minimised over all orderings of S. Disjoint subsets have
disjoint internal edge sets, so those exact minima add, and the sum is a
valid lower bound on the backward weight of any ordering of the whole graph.

On the same edges this can only beat cycle packing. Greedy packing is one
feasible solution to the fractional cycle packing LP; exact MFAS accounts
for every overlapping cycle at once, while greedy spends an edge's capacity
on whichever cycle reached it first.

`mfas_code/clusterbound.c` grows disjoint clusters of up to K nodes around
the strongest reciprocal pairs and solves each exactly by subset DP —
`f[S] = max forward weight with exactly S placed first`, O(2^K · K²).

**It is checked against brute force.** `clusterbound --selftest` builds 400
random weighted digraphs of 3–8 nodes, computes the minimum feedback arc set
by enumerating every permutation, and confirms the DP matches exactly on all
400 — and that greedy packing never exceeds it.

## Result: greedy was not the problem

| | larva | FlyWire |
|---|---|---|
| clusters | 178 | 7,888 |
| nodes covered | 2,447 | 94,190 |
| weight inside clusters | 21.66% | 11.87% |
| greedy packing on that weight | 7,538 | 603,107 |
| **exact on that weight** | **7,680** | **616,142** |
| improvement | +1.9% | +2.2% |

Exact beats greedy by about two per cent, consistently, on both connectomes.
Applied to FlyWire that is **+13,035 weight, or 0.031 points** on a gap of
5.42. It closes about half a per cent of the problem.

The reason is visible in the same table: clusters small enough to solve
exactly hold only 11.87% of the graph's weight. Most of the brain's
recurrence is not inside tight local groups where exactness pays — it is
spread across long-range structure, where cycle packing is all we have, and
where greedy is already close to the best any packing can do.

**So the looseness is not in the implementation. It is in the relaxation.**
Anything in the cycle-packing family — better greedy, exact local solves,
fractional refinement — looks to be near its ceiling at around ten per cent.
Beating it needs a different relaxation entirely: the triangle-inequality LP
for linear ordering under cutting planes, or something semidefinite. That is
a research project, not an afternoon.

## The reframing this forces, which is more useful than the bound

We have been quoting one side of a two-sided thing. The *minimum* backward
weight over all orderings — the irreducible recurrence — is bounded from
below by the packing, and from **above** by any ordering we can actually
exhibit, because an achievable backward rate proves the minimum is no larger.

| connectome | floor (certified) | ceiling (exhibited) | width |
|---|---|---|---|
| larva, whole brain | 11.79% | 13.11% | **1.31** |
| adult central brain (hemibrain) | 18.96% | 20.87% | **1.91** |
| adult whole brain (FlyWire) | 12.63% | 15.18% | 2.55 |

Stated this way, two of the three are already tight, and the comparative
claim survives: the adult central brain's irreducible recurrence is at least
18.96%, which is above the larval brain's ceiling of 13.11%. **Those two
intervals do not overlap**, so the adult central brain is more irreducibly
recurrent than the whole larval brain — and that conclusion does not depend
on closing the FlyWire gap at all.

Where in its interval does FlyWire's truth sit? The diversity experiment is
indirect evidence. Six independent searches from different starting layouts
all finished between 84.51% and 84.82% forward, none of them anywhere near
the 90.24% the bound permits. If orderings with far less backward weight
existed, a search that samples many basins might be expected to find one.
That is suggestive, not proof — but it points at the truth lying near the
ceiling, meaning our ordering is close to optimal and the bound is what is
loose.

## What was not done

No attempt was made at the LP relaxation proper. The negative result here is
specific: *exact local solves do not close this gap*. It does not prove no
tractable bound does.
