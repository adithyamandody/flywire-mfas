# A third connectome: the Drosophila larva

Run on 27 November 2026, on the larval brain of Winding et al. 2023 — a
different animal, a different developmental stage, a different laboratory
and a different reconstruction from either of the two adult connectomes we
had used. The pipeline was unchanged.

## The graph

The published larval connectome is a multigraph: each pair of neurons can
be joined by up to four parallel edges, one per compartment pair — `ad`
(axo-dendritic), `aa`, `da`, `dd` — each weighted by its synapse count.
FlyWire and hemibrain count every synapse between two neurons regardless of
compartment, so to match them `mfas_code/build_larva.py` **sums across all
four types**. Splitting by compartment would be a different and also
defensible analysis; this one is stated so it can be repeated or disputed.

| | |
|---|---|
| neurons with an edge | 2,923 |
| connections (≥2 synapses) | 58,489 |
| total weight | 300,177 |
| synapses by type | ad 234,958 · aa 90,800 · dd 20,415 · da 6,438 |

## The ordering, and how close to optimal it is

| | weight | % |
|---|---|---|
| our ordering, forward | 260,837 | **86.894%** |
| certified upper bound | 264,783 | 88.209% |
| gap | 3,946 | **1.315 points** |

Both numbers were re-derived independently: `score.py` re-reads the graph
and the ordering, `check_bound.py` re-reads the graph and all 14,554 packed
cycles and confirms every one is a real directed cycle with no connection
used beyond its weight.

**At least 11.79% of the larval brain's synaptic weight must point backward
in any ordering whatsoever.** That is not a statement about our search. It
is a property of the wiring.

## The axis appears again

Median position of each cell type, 0% first to 100% last:

| cell type | n | median |
|---|---|---|
| sensory | 403 | **8.5%** |
| ascending | 46 | 13.3% |
| PN (projection) | 206 | 27.1% |
| LN (local) | 110 | 34.6% |
| PN-somato | 152 | 37.4% |
| KC (Kenyon cells) | 144 | 38.6% |
| MBON (MB output) | 48 | 44.4% |
| LHN (lateral horn) | 202 | 55.5% |
| MB-FFN (feedforward) | 54 | 56.4% |
| DN-SEZ | 164 | 57.6% |
| pre-DN-SEZ | 102 | 62.6% |
| pre-DN-VNC | 475 | 65.2% |
| MB-FBN (feedback) | 108 | 73.6% |
| RGN (ring gland) | 53 | 77.2% |
| MBIN (MB input) | 28 | 78.8% |
| CN (convergence) | 100 | 79.6% |
| DN-VNC (descending) | 182 | **94.4%** |

Sensory first at 8.5%, descending-to-nerve-cord last at 94.4%. The adult
whole brain gave 7.9% and 99.8% for the equivalent classes. The olfactory
pathway comes out in its textbook order — sensory → PN → KC → MBON — and
the premotor neurons (pre-DN-VNC, 65.2%) are placed ahead of the motor
output they drive (DN-VNC, 94.4%), without being told that either class
exists.

## What carries the feedback

Backward rate by class, against the brain-wide 13.11%:

| cell type | weight | backward | vs brain |
|---|---|---|---|
| MBIN | 17,580 | 34.7% | **2.65×** |
| KC | 58,716 | 24.0% | 1.83× |
| LN | 48,586 | 18.1% | 1.38× |
| MB-FBN | 45,265 | 13.8% | 1.05× |
| … | | | |
| PN | 64,481 | 6.8% | 0.52× |
| DN-VNC | 18,428 | 6.5% | 0.50× |

MBINs — the modulatory neurons that deliver reinforcement signals into the
mushroom body — carry over two and a half times their share of backward
weight, the highest of any class. That is the same shape as the adult
result, where dopaminergic and serotonergic connections ran backward far
more often than cholinergic ones: the reinforcement signal travels against
the direction of the feedforward sweep. Local neurons are second, as the
antennal lobe local neurons were in the adult, and the ten neurons carrying
the most backward weight are eight LNs and two MBINs.

A caution worth stating: MB-FBN is *named* for providing feedback and sits
at only 1.05×. The naming reflects anatomy within the mushroom body; the
ordering is measuring something else — share of a class's total synaptic
weight that the global hierarchy has to turn around.

## Irreducible recurrence across three connectomes

Irreducible recurrence is bounded on both sides: from below by the cycle
packing, and from above by any ordering we can exhibit, since an achievable
backward rate proves the minimum is no larger.

| connectome | neurons | floor (certified) | ceiling (exhibited) | width |
|---|---|---|---|---|
| larva, whole brain | 2,923 | **11.79%** | **13.11%** | 1.31 |
| adult central brain (hemibrain) | 21,737 | **18.96%** | **20.87%** | 1.91 |
| adult whole brain (FlyWire) | 136,648 | 9.76% | 15.18% | 5.42 |

The floors are not equally sharp. The larval and hemibrain bounds are tight
— 1.3 and 1.9 points — so those floors are close to the real thing. The
FlyWire bound is loose at 5.42 points, so its true floor is somewhere above
9.76% and we do not yet know where. **Tightening it is the single thing
that would make this table say something firm.**

The controlled comparison is between the two volumes that both exclude
optic lobes: the whole larval brain and the adult central brain. **Their
intervals do not overlap** — the adult's floor of 18.96% is above the
larva's ceiling of 13.11% — so the adult central brain is more irreducibly
recurrent than the entire larval brain, and that conclusion holds whatever
the truth turns out to be inside each interval. The adult volume is about
seven times larger. That is one comparison on two brains of one species,
nowhere near enough to claim a trend, but it is the first time the question
has been asked with certificates on both sides, and it survives being
stated as conservatively as the evidence allows.

See `BOUND.md` for why the FlyWire interval is the wide one and what it
would take to narrow it.

Next points on the curve: *C. elegans* (302 neurons, a different phylum) and
MICrONS mouse visual cortex (~75,000 neurons, mammalian).
