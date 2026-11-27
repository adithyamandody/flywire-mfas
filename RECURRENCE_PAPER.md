# Nervous systems are wired less recurrently than their own degree sequences require

**Adithya Mandody**, independent researcher.
Draft, 27 November 2026. Code and data: https://github.com/adithyamandody/flywire-mfas

---

## Abstract

Order the neurons of a connectome however you like and some synaptic weight
still points backward. The minimum over all orderings is a property of the
wiring, not of any algorithm — the share of connectivity that no feedforward
hierarchy can account for. We measure it with certificates on both sides, a
cycle packing below and an exhibited ordering above, in four connectomes
spanning two phyla. In *Caenorhabditis elegans* the two bounds meet exactly,
at 3.81%. We first asked whether this quantity grows with nervous-system
size, and report that **it does not**: degree-matched random graphs reproduce
the entire pattern, including its non-monotone step, from network density
alone. What survives that control is a different result, pointing the
opposite way. Compared against random graphs carrying its own degree sequence
under the same solver effort, **every nervous system we measured is less
irreducibly recurrent than chance**, by ratios of 0.64, 0.67, 0.79 and 0.90.
Zhao and Zhou reported this direction for the 297-node *C. elegans* network
in 2016 [16], against a weaker null; we confirm it with a degree-preserving
null, by weight rather than arc count, with certificates, and on connectomes
orders of magnitude larger.
This holds although the real connectomes carry three to fifty-five times more
weight on reciprocated connections than their nulls do — the property that
most directly forces backward weight. Real nervous systems are therefore
arranged so that their recurrence is unusually resolvable into a hierarchy,
not unusually abundant.

---

## 1. Introduction

Theories of consciousness disagree about nearly everything except one
structural commitment: a feedforward sweep is not sufficient. Recurrent
processing theory locates awareness in reentrant signalling [10]; the global
neuronal workspace requires ignition sustained by long-range loops [11];
integrated information theory makes irreducibility to independent parts
definitional [12]. Each asserts that recurrence matters.

The measurement that bears on this has a precedent. Zhao and Zhou [16]
computed near-minimal feedback arc sets for a set of real directed networks,
including the 297-node *C. elegans* network available in 2016, and compared
each against an ensemble in which arc directions were randomised. They found
real networks markedly feedback-scarce: the worm had 70 feedback arcs where
randomisation predicted 405, some 34 standard deviations below chance. **The
qualitative result in §3.3 below is theirs**, and we say so.

Three things have changed since. Whole-brain connectomes now exist at up to
500 times that scale and carry synapse counts, so the question can be asked
by weight rather than by arc count. The feedback arc set can be **bracketed
by certificates** rather than approached heuristically, which turns "nearly
optimal" into "provably within". And a direction-randomised ensemble is a
weak null here, because reversing arcs destroys the degree sequence along
with everything else; a degree-preserving null is the harder test, and the
one we use.

This paper reports that measurement in four connectomes, and the result of
subjecting it to the obvious control. It is a companion to work on a single
connectome [14], which established the ordering method on the adult
*Drosophila* brain.

## 2. The measurement

Given a weighted directed graph and an ordering of its nodes, the backward
weight is the total on edges pointing against the order. Minimising it is the
weighted minimum feedback arc set problem, which is NP-hard [7] and normally
approached by greedy heuristics in the lineage of Eades et al. [8]. We do not
solve it; we bracket it.

**Floor.** Assign each directed cycle a value so that the cycles through any
edge never exceed its weight. Every directed cycle contains at least one
backward edge in every ordering, so the packed total is forced backward
always. A packing is a certificate: re-read the graph, check each line is a
real cycle, check no edge is over-used.

**Ceiling.** Any ordering we exhibit achieves some backward rate, and the
minimum cannot exceed it. Also a certificate, checkable by re-scoring.

Failure is one-sided. A weak search or a weak packing widens the interval but
cannot produce a false claim, only a vague one.

## 3. Results

### 3.1 Four nervous systems

All at a threshold of two synapses per connection; §4 shows why that choice
is not innocent.

| nervous system | cells | conns/cell | floor | ceiling |
|---|---|---|---|---|
| *C. elegans*, whole [5] | 297 | 4.4 | **3.81%** | **3.81%** |
| *D.* larva, whole brain [4] | 2,923 | 20.0 | 11.79% | 13.06% |
| *D.* adult, central brain [3] | 21,737 | 81.2 | 18.94% | 20.84% |
| *D.* adult, whole brain [1] | 136,648 | 41.4 | 12.63% | 15.18% |

In *C. elegans* the packing forces exactly 262 units backward and an ordering
achieves exactly 262. The bounds meet: irreducible recurrence in the worm's
chemical connectome is 3.81%, with no residual uncertainty. That certificate
is 97 cycles and can be checked by hand.

### 3.2 The raw numbers track density, not size

Our initial hypothesis was that this quantity grows with nervous-system size.
It does not. The fourth row already breaks it — the adult whole brain is six
times larger than the central brain inside it and reads lower — and the
column that orders the rows is connections per cell, not cells.

The decisive test is a degree-preserving null. We rewired each connectome by
repeated double-edge swaps, preserving every node's in-degree, out-degree and
out-strength and the multiset of edge weights, asserted by re-reading each
generated file. Measured the same way, with the same solver effort:

| system | conns/cell | real ceiling | null ceiling | nulls |
|---|---|---|---|---|
| worm | 4.4 | 3.81% | 5.93% [5.57–6.31] | 5 |
| larva | 20.0 | 13.06% | 19.43% [19.36–19.52] | 5 |
| FlyWire | 41.4 | 15.46% | 19.64% [19.63–19.64] | 2 |
| hemibrain | 81.2 | 20.84% | 23.10% [23.05–23.17] | 3 |

**The nulls reproduce the original pattern from density alone**, including
the non-monotone step that broke the size story: random graphs with
hemibrain's degree sequence are more recurrent than random graphs with
FlyWire's, exactly as the real ones are, though hemibrain is the smaller
brain. Any claim that this quantity scales with nervous-system size is
unsupported by these data, and we withdraw it.

### 3.3 Against its own degree sequence, every brain is less recurrent

The same table, read down its last two columns, gives the result that does
survive (Figure 1):

| system | real / null |
|---|---|
| worm | **0.64** |
| larva | **0.67** |
| FlyWire | **0.79** |
| hemibrain | **0.90** |

Every real nervous system is less irreducibly recurrent than random graphs
built from its own degrees. The deficit ranges from 10% to 36%.

**This direction is not new.** It is Zhao and Zhou's feedback scarcity [16],
found in 2016 on the *C. elegans* network of the day. What is new is how
hard the test is. Their null randomised arc directions, which destroys the
degree sequence; ours preserves every node's in-degree, out-degree and
out-strength, so each null graph is a close relative of the real one. Their
effect is correspondingly larger — 70 feedback arcs against 405 — and ours
smaller but much harder to explain away. We also measure weight rather than
arc count, and the result holds on three connectomes that did not exist when
they wrote.

### 3.4 And it holds against the reciprocity they carry

This is not a consequence of real brains having less of the structure that
forces backward weight. They have far more of it. Weight on reciprocated
connections, real against null:

| system | real | nulls |
|---|---|---|
| worm | 9.9% | 2.1–3.6% |
| larva | 20.6% | 1.5–1.7% |
| FlyWire | 24.9% | 0.45% |
| hemibrain | 41.9% | 1.7–1.8% |

A reciprocated pair is a 2-cycle and forces backward weight directly. Real
connectomes carry three to fifty-five times more of it and still finish with
less irreducible recurrence than their nulls. Whatever arrangement achieves
that has to overcome the reciprocity rather than avoid it.

### 3.5 How much of this is proved

Only the worm comparison is settled by certificates: every null's floor
(4.64–5.24%) exceeds the worm's exact ceiling of 3.81%, so no ordering of any
null can be as feedforward as the real worm.

For the other three the comparison is between ceilings at equal solver
effort — the same initialiser, search and time budget on real and null alike.
That is an estimate, not a proof. It would be wrong only if the solver were
systematically several points worse on random graphs than on real ones;
replicate spread is 0.16 points or less outside the worm, and the runs were
near-flat at their budgets. The floor shortcut is unavailable here because
rewiring destroys reciprocity (§3.4), and cycle packing is a weak certificate
on a graph with few 2-cycles.

### 3.6 The orderings recover each animal's axis

Computed from wiring alone; the class labels used to read the result off are
published annotations [2,5] and were never available to the solver. Median
positions:

| | first | middle | last |
|---|---|---|---|
| *C. elegans* | sensory 27.0% | interneuron 51.7% | motor 76.4%, body-wall muscle 100% |
| *D.* larva | sensory 8.5% | PN 27.1%, Kenyon 38.6% | descending 94.4% |
| *D.* adult | sensory 7.9% | optic 47.0%, central 85.5% | motor 99.8% |

In the adult this is stable across independent solutions: six orderings built
from different starting layouts, disagreeing about 2.4% of the graph's
weight, place sensory at 7.8 ± 0.1% and motor at 99.8 ± 0.1% [14].

We note what this does *not* show. An ordering by each neuron's out-share of
its own weight, with no optimisation at all, recovers most of this axis [14].
The axis confirms the orderings are not nonsense; it is not evidence that the
optimisation found something a simpler statistic could not.

## 4. Limitations

- **Dataset effects are nearly half the between-system differences.**
  Restricting our FlyWire ordering to FlyWire's own central brain gives a
  16.27% ceiling for the tissue where hemibrain certifies an 18.94% floor.
  Two reconstructions of the same tissue in the same species disagree by at
  least 2.7 points. Any cross-system difference smaller than about 3 points
  should be treated as indistinguishable from reconstruction method.
- **The threshold is not a neutral choice.** Re-scoring at higher synapse
  thresholds, the larva runs 13.06% → 5.89% between ≥2 and ≥10 while
  hemibrain runs 20.84% → 17.07%. The datasets do not respond alike, so
  fixing a nominal synapse count fixes a number rather than a meaning. The
  real/null comparison is more robust to this than the raw values, because
  the null is rebuilt from whatever graph the threshold produces.
- **The denominators differ structurally.** Weight outside any strongly
  connected component can never be backward. That is 50.8% of the worm, 5.0%
  of the larva and 1.0% of hemibrain, so the raw percentages are fractions of
  very different things.
- **"297 cells" is not 297 neurons.** The *C. elegans* source lumps the
  entire musculature into one node, which receives 20.3% of all weight as a
  pure sink. Removing it, the same certified floor of 262 reads 4.78% rather
  than 3.81%. The 3.81% is exact for the graph as defined; the graph includes
  a modelling convenience.
- **Four systems, three of them *Drosophila*.** No trend should be fitted to
  this. The real/null ratio happens to be monotone in density and is not
  monotone in size, but with four points that is an observation, not a
  finding. Mammalian cortex [13] is the point that would matter.
- **This is wiring, not dynamics.** Nothing here measures what any circuit
  computes.

## 5. What this does and does not say about consciousness

It supplies the theories of §1 with a quantity whose importance they assert
but which has been measured only once before, on a 297-node network with a
weaker null and without certificates [16]: how much of a nervous system's
connectivity is irreducible to any hierarchy, at whole-organism scale.

The direction of the result is worth stating carefully, because it is not the
one we expected. These nervous systems are not unusually recurrent for their
wiring statistics — they are unusually *resolvable*, carrying more
reciprocity than chance while still admitting a more feedforward arrangement
than chance allows. If recurrence is the substrate those theories require,
then whatever makes it special is not its quantity, since by that measure
brains have less of it than random graphs with the same degrees.

We make no claim that any of these animals is conscious. A fly is not
conscious in any settled sense and a worm certainly is not. This is a
property of wiring, and a measurement is a precondition for addressing such
questions rather than an answer to one.

## 6. Methods

**Graphs.** FlyWire from the challenge distribution [1,15]; hemibrain v1.2
traced non-cropped neurons [3]; larva from the published multigraph [4],
summing the four compartment-pair edge types; *C. elegans* from the typed
White et al. reconstruction [5,6] as distributed by the OpenWorm Connectome
Toolbox, a composite over animals, **chemical synapses only**. Gap junctions
are electrically symmetric and appear as reciprocal pairs no ordering can
resolve, manufacturing recurrence from connections that have no direction to
be wrong about; a catalogue copy of this connectome that merges them, and
whose documentation states its edge directions are undocumented, was
discarded after 29% of its reciprocal pairs were found to carry exactly equal
weights. Self-loops dropped, connections below two synapses discarded.

**Nulls.** `mfas_code/rewire.py`, double-edge swaps at ten attempts per
connection, rejecting self-loops and duplicates, weights travelling with
their source. Each generated file is re-read and asserted to have identical
in- and out-degree per node, identical out-strength per node, and an
identical weight multiset. Reciprocal pairs are not forbidden. Seeds are
recorded and the nulls are reproducible.

**Orderings and bounds.** SpringRank initialisation [9] then iterated local
search [14]; greedy cycle packing over 2-cycles, 3-cycles and longer cycles
by bounded search. Real and null graphs were run through identical pipelines
and budgets so the ceilings are comparable.

**Verification.** Every ordering re-scored by an independent implementation
that re-reads graph and ordering from disk and shares no code with the
solver; every packing re-verified cycle by cycle. The *C. elegans*, larval
and FlyWire packings are in `certificates/`.

**A correction to [14].** The FlyWire floor reported there as 9.76% came from
a packing whose 3-cycle sweep hit its time limit after 108,374 of 136,648
nodes. A complete run packs 5,291,688 units, verified at 1,513,396 cycles
with no over-used connection: the floor is 12.63% and the bracket is
[12.63%, 15.18%].

## 7. AI use declaration

Generative AI was used as a research tool in this work, not only as writing
assistance. Claude (Anthropic) wrote the solver, the bound, the null-model
code and the analysis, ran the experiments, proposed the framing of
irreducible recurrence as a measurable quantity, and drafted this manuscript.
It also ran the adversarial audit and the null-model test that overturned
this paper's original thesis. The author set the direction, decided which
lines to pursue, ran the verification scripts, read the code and the cited
sources, checked the figures and quoted numbers against the data, and takes
full responsibility for this paper, including its claims and references. No
AI system is an author, and none could be: accountability for a scientific
claim cannot be delegated to a model.

---

## References

1. Dorkenwald S, et al. Neuronal wiring diagram of an adult brain. *Nature* **634**, 124–138 (2024).
2. Schlegel P, et al. Whole-brain annotation and multi-connectome cell typing of *Drosophila*. *Nature* **634**, 139–152 (2024).
3. Scheffer LK, et al. A connectome and analysis of the adult *Drosophila* central brain. *eLife* **9**, e57443 (2020).
4. Winding M, et al. The connectome of an insect brain. *Science* **379**, eadd9330 (2023).
5. White JG, Southgate E, Thomson JN, Brenner S. The structure of the nervous system of the nematode *Caenorhabditis elegans*. *Philosophical Transactions of the Royal Society B* **314**(1165), 1–340 (1986).
6. Varshney LR, Chen BL, Paniagua E, Hall DH, Chklovskii DB. Structural properties of the *Caenorhabditis elegans* neuronal network. *PLoS Computational Biology* **7**(2), e1001066 (2011).
7. Karp RM. Reducibility among combinatorial problems. In Miller RE, Thatcher JW (eds), *Complexity of Computer Computations*, 85–103. Plenum Press (1972).
8. Eades P, Lin X, Smyth WF. A fast and effective heuristic for the feedback arc set problem. *Information Processing Letters* **47**(6), 319–323 (1993).
9. De Bacco C, Larremore DB, Moore C. A physical model for efficient ranking in networks. *Science Advances* **4**, eaar8260 (2018).
10. Lamme VAF. Towards a true neural stance on consciousness. *Trends in Cognitive Sciences* **10**(11), 494–501 (2006).
11. Mashour GA, Roelfsema P, Changeux J-P, Dehaene S. Conscious processing and the global neuronal workspace hypothesis. *Neuron* **105**(5), 776–798 (2020).
12. Tononi G, Boly M, Massimini M, Koch C. Integrated information theory: from consciousness to its physical substrate. *Nature Reviews Neuroscience* **17**(7), 450–461 (2016).
13. MICrONS Consortium. Functional connectomics spanning multiple areas of mouse visual cortex. *Nature* **640**(8058), 435–447 (2025).
14. Mandody A. A maximally feedforward ordering of the *Drosophila* connectome recovers its processing hierarchy and replicates across two brains (companion manuscript, 2026).
15. FlyWire Connectome MFAS Challenge. https://codex.flywire.ai/app/mfas_challenge?dataset=fafb
16. Zhao J-H, Zhou H-J. Feedback arcs and node hierarchy in directed networks. *Chinese Physics B* **26**(7), 078901 (2017). arXiv:1612.05347.

---

## Notes for the author, not for the draft

- **This paper's thesis was replaced, not patched.** The original claimed
  irreducible recurrence rises with nervous-system size. A degree-preserving
  null showed the whole pattern follows from density, so that claim is
  withdrawn in §3.2 rather than hedged. The surviving result is the opposite
  sign and better controlled. Do not let it drift back; the size framing is
  not supported.
- **Only the worm comparison is proved.** §3.5 says so. If a referee presses
  on the other three, the answer is more solver budget on the nulls, not a
  stronger claim.
- **Priority: checked, and it bit.** Zhao & Zhou 2017 [16] ran near-minimal
  feedback arc sets on real directed networks including the 297-node worm,
  compared them against direction-randomised ensembles, and reported exactly
  the feedback-scarcity direction this paper found. The novelty claim is
  narrowed accordingly in the abstract, §1, §3.3 and §5: what is ours is the
  certificate, the weighting, the degree-preserving null and the scale — not
  the direction of the effect. Their worm network is also the merged
  gap-junction dataset this paper rejects in Methods, which is worth a
  sentence if a referee raises it.
- **Carried from the audit, still to verify:** the glutamate region confound,
  and the claim that the companion paper's top-feedback ranking is really a
  size ranking. Both affect [14], not this paper.
