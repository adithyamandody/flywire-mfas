# Audit findings, 27 November 2026

An adversarial audit of `PAPER.md` and `RECURRENCE_PAPER.md` was run by an
independent agent that was given the files, the tools and the mechanics, but
not our conclusions. Everything below I then re-ran myself. **Nothing here is
relayed on trust.** Neither paper is postable until these are addressed.

## Confirmed: the cross-species trend has four independent problems

`RECURRENCE_PAPER.md` claims irreducible recurrence "increases monotonically
with size" across worm → larva → adult central brain (3.81% → 11.79% →
18.96%). Four things attack that, and they compound.

### 1. The dataset effect is nearly half the biological signal

Restricting our FlyWire ordering to FlyWire's own central brain gives a valid
ordering of that subgraph, hence a ceiling for the same tissue hemibrain
covers:

| | neurons | backward |
|---|---|---|
| FlyWire, super_class = central | 32,383 | **16.27%** (ceiling) |
| FlyWire, central + VP + VC + descending | 42,248 | 15.38% (ceiling) |
| hemibrain, same tissue | 21,737 | **18.96%** (floor) |

A floor of 18.96% against a ceiling of 16.27% for the same tissue in the
same species. The two reconstructions disagree by **at least 2.7 points** —
against a larva-to-hemibrain gap of 5.85 points. Roughly half the "biological"
difference is attributable to reconstruction and segmentation, not biology.

### 2. Threshold sensitivity exceeds the between-species differences

Re-scoring the existing orderings on the same graphs thresholded higher:

| | ≥2 | ≥3 | ≥5 | ≥10 |
|---|---|---|---|---|
| larva | 13.11% | 11.45% | 9.12% | 5.89% |
| hemibrain | 20.87% | 19.64% | 18.31% | 17.07% |

The larva moves 7.2 points across this range, hemibrain only 3.8. The
datasets do not respond alike, so holding the threshold "fixed" at a nominal
synapse count does not make them comparable — it fixes a number, not a
meaning. The larva at ≥3 is already below its own certified ≥2 floor.

### 3. Half the worm's weight cannot be in a cycle at all

Weight outside any non-trivial strongly connected component can never be
backward in any ordering, so it dilutes the denominator for free:

| | weight inside an SCC |
|---|---|
| *C. elegans* ≥2 | **49.2%** |
| *C. elegans* ≥1 | 69.9% |
| larva | 95.0% |
| hemibrain | 99.0% |

The worm's 3.81% and hemibrain's 18.96% are fractions of structurally
different denominators. Normalised to in-component weight they read about
7.7%, 12.4% and 19.2% — the direction survives, but "roughly fivefold"
becomes about 2.5-fold.

### 4. "297 cells" includes a pseudo-node for all body-wall muscle

`LegacyBodyWallMuscles` is one node standing for the entire musculature. It
receives 1,399 of 6,875 units — **20.3% of all weight** — and sends nothing,
so it is a pure sink and accounts for most of the non-SCC weight above.
Remove it and the same certified floor of 262 reads **4.78%, not 3.81%**.

The 3.81% is not wrong — it is exact for the graph as defined — but
`RECURRENCE.md` calling it "a theorem about the animal" overstates what the
graph is. It is a theorem about a composite reconstruction that includes a
modelling convenience.

## Confirmed: two problems in the companion paper

### 5. The dopamine and serotonin results are classifier artefacts

`PAPER.md` §2.4 and its abstract report that dopaminergic and serotonergic
connections run backward more than cholinergic ones.

Of the 5,177 Kenyon cells, **5,172 are predicted dopaminergic**. The same
annotation file's measured `known_nt` column says acetylcholine for all
5,177. Those cells carry roughly three-quarters of everything counted as
dopamine weight. The serotonin row is dominated the same way by antennal
lobe local neurons the same column calls cholinergic.

GABA and acetylcholine are unaffected and stand. The hemibrain "replication"
of these rows is not independent — labels are transferred from FlyWire by
cell type, carrying the same errors across.

### 6. A trivial sort reproduces most of the axis

Ordering neurons by out-weight ÷ (in + out), with no optimisation at all,
scores 71.91% forward against our 84.82%, and yet:

| class | optimised | degree-ratio sort |
|---|---|---|
| sensory | 7.9% | 7.2% |
| ascending | 6.9% | 5.2% |
| visual_projection | 78.2% | 79.0% |
| descending | 98.9% | 94.6% |
| motor | 99.8% | 98.7% |
| **central** | **85.5%** | **60.6%** |

So §2.2's "it is the evidence that the order carries information beyond its
objective value" is wrong as written. The axis is largely a consequence of
each neuron's in/out ratio. The optimisation's distinctive contribution is
the central-brain row — placing central after visual projection — plus
tighter rates. The paper needs this baseline, and its claims scoped to what
exceeds it.

## Confirmed: our own bookkeeping

- **The FlyWire bound was time-truncated.** `runs/bound3c.log`: the 3-cycle
  pass stopped "after 108374 of 136648 nodes". `BOUND.md` argues the 5.42
  gap is the relaxation rather than our implementation, on the strength of a
  0.031-point experiment, while our own log shows roughly 1.4 points left on
  the table. A complete packing is running.
- **`certificates/README.md` had three errors** when written: a cycle count
  of 361 for a 342-line file, the wrong ordering named for the 262 proof
  (the right one was gitignored, so the proof could not be reproduced from
  the repo), and a determinism claim generalised from a packing that
  finishes in two seconds. Corrected in d195f7b.
- **`build_worm.py` mislabels motor neurons as muscle.** `role()` tests for
  the substring "muscle" before "motor neuron", against a string that
  includes classifications such as "innervates dorsal muscles". 74 ventral
  cord motor neurons are labelled muscle, so the "muscle 82.5%" row in
  `RECURRENCE_PAPER.md` §3.6 is motor neurons.

## Not yet verified by me

All three are now checked. The glutamate region confound
(glutamate 30.9% backward in the optic lobe against 15.8% in the central
brain, which would make the FlyWire/hemibrain disagreement an optic-lobe
effect rather than a failed replication); the claim that the top-feedback
ranking is a size ranking and that CT1 is not a feedback cell; and a
possible prior result on feedback-arc minimisation in the 297-node worm
network (arXiv:1612.05347) bearing on the priority claim.

## What survives

- The method: a two-sided certified measurement, with the asymmetry that a
  weak search or weak packing widens the interval but cannot make a false
  claim.
- The *C. elegans* exactness — floor meets ceiling at 262 — as a statement
  about that graph.
- GABA and acetylcholine in §2.4.
- The type-level correlation of §2.7: ρ = 0.615, and 0.611 after controlling
  for type size, so it is not a size effect.
- The six-basin stability result.
- larva < hemibrain, which survives removing the larval periphery.
