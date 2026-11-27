# How much of a nervous system is irreducibly recurrent? A certified measurement in four connectomes

**Adithya Mandody**, independent researcher.
Draft, 27 November 2026. Code and data: https://github.com/adithyamandody/flywire-mfas

---

## Abstract

Order the neurons of a connectome however you like and some synaptic weight
will still point backward. The minimum of that quantity over all possible
orderings is a property of the wiring rather than of any algorithm: the
fraction of a nervous system's connectivity that no feedforward hierarchy
can account for. We call it irreducible recurrence, and we measure it with
certificates on both sides — a cycle packing that lower-bounds it and an
explicit ordering that upper-bounds it — in four connectomes spanning two
phyla and a 460-fold range in size. In *Caenorhabditis elegans* the two
bounds meet: the packing forces exactly 262 units of synaptic weight
backward and an ordering achieves exactly 262, so irreducible recurrence in
the worm's chemical connectome is 3.81%, exactly. The whole brain of a
*Drosophila* larva lies in [11.79%, 13.11%], and the adult *Drosophila*
central brain in [18.96%, 20.87%]. These three intervals do not overlap and
increase monotonically with size, across 297 to 21,737 cells and three
independent reconstructions. Every ordering was computed from wiring alone,
and every one recovers the animal's known input-to-output axis — sensory
cells first, muscle or motor neurons last — which is evidence that the
orderings are not arbitrary members of a large near-optimal set. We report
two methodological traps that invert the result if missed: electrical gap
junctions are symmetric and manufacture apparent recurrence, and the
synapse-count threshold moves the worm's figure from 3.81% to 6.4%.

---

## 1. Introduction

Theories of consciousness disagree about nearly everything except one
structural commitment: a feedforward sweep is not sufficient. Recurrent
processing theory locates awareness in reentrant signalling rather than the
initial sweep [10]; the global neuronal workspace requires broadcast and
ignition that depend on long-range reentrant loops [11]; integrated
information theory makes irreducibility to independent parts the definition
of the quantity it proposes [12]. Each asserts that recurrence matters.

None has been accompanied by a measurement of how much recurrence a nervous
system actually has, in the sense of wiring that no hierarchy can explain
away. Connectomes now make that question answerable. It is not a question
about dynamics or about consciousness; it is a question about a directed
graph, and it has an exact answer that can be certified.

This paper reports that measurement in four connectomes. It is a companion
to work on a single connectome [14], which established the ordering method
and its biological interpretation on the adult *Drosophila* brain; here the
object of interest is the bound rather than the ordering, and the comparison
across animals rather than within one.

## 2. The measurement

Given a weighted directed graph *G* = (*V*, *E*, *w*) and a bijection
π: *V* → {1 … *n*}, the backward weight is the sum of *w*(*u*,*v*) over
edges with π(*u*) > π(*v*). Minimising it is the weighted minimum feedback
arc set problem, which is NP-hard [7]. We are not trying to solve it. We are
trying to bracket its optimum.

**Lower bound (floor).** Assign every directed cycle *C* a value
*y_C* ≥ 0 such that for each edge *e*, the cycles through it satisfy
Σ *y_C* ≤ *w_e*. Every directed cycle must contain at least one backward
edge in any ordering, so the backward weight is at least Σ *y_C* in every
ordering. A packing is a certificate: it can be checked by re-reading the
graph, confirming each packed cycle is a genuine directed cycle, and
confirming no edge is used beyond its weight.

**Upper bound (ceiling).** Any ordering we can exhibit achieves some
backward rate, and the minimum cannot exceed what is achieved. A heuristic
ordering is therefore also a certificate, checkable by re-scoring it.

The two together bracket irreducible recurrence. Note the asymmetry of
failure: a weak search or a weak packing only widens the interval. Neither
can produce a false claim, only a vague one.

## 3. Results

### 3.1 Four nervous systems

All measurements use a threshold of two synapses per connection, matching
the FlyWire challenge graph; §3.5 shows why holding it fixed matters.

| nervous system | cells | connections | weight | floor | ceiling | width |
|---|---|---|---|---|---|---|
| *C. elegans*, whole [5] | 297 | 1,318 | 6,875 | **3.81%** | **3.81%** | **0.00** |
| *D.* larva, whole brain [4] | 2,923 | 58,489 | 300,177 | 11.79% | 13.11% | 1.31 |
| *D.* adult, central brain [3] | 21,737 | 1,764,123 | 12,542,949 | 18.96% | 20.87% | 1.91 |
| *D.* adult, whole brain [1] | 136,648 | 5,657,719 | 41,912,141 | 9.76% | 15.18% | 5.42 |

### 3.2 The worm is solved, not bounded

In *C. elegans* the cycle packing forces exactly 262 units of synaptic
weight backward, and our ordering achieves exactly 262. The bounds meet.
Irreducible recurrence in the chemical connectome of the adult hermaphrodite
is **3.81%**, with no residual uncertainty: no ordering of those 297 cells
can do better, and one that attains it is exhibited.

This is, as far as we are aware, the first nervous system for which the
quantity is known rather than estimated.

### 3.3 It rises with size, and the intervals do not overlap

Among the three volumes that are whole nervous systems or central brains —
excluding the adult whole brain, for the reason in §3.4 — the measurement
increases monotonically and the intervals are disjoint (Figure 1):

| | nervous system | irreducible recurrence |
|---|---|---|
| 297 cells | *C. elegans* | **3.81%** |
| 2,923 cells | larval brain | 11.79 – 13.11% |
| 21,737 cells | adult central brain | 18.96 – 20.87% |

A 73-fold range in cell count, two phyla, three laboratories, three
reconstruction methods, and a roughly fivefold increase in the share of
wiring that no hierarchy explains. Because the intervals are disjoint, the
ordering of the three does not depend on where within each interval the
truth lies.

### 3.4 The whole adult brain reads lower than the central brain it contains

The adult whole brain sits at [9.76%, 15.18%], below the central brain that
is a subset of it. This is dilution rather than contradiction: the whole
brain includes the optic lobes, roughly 57% of its neurons, which are large,
repetitive and strongly feedforward. The same effect appears within a single
dataset: in the adult, the outbound visual route is a few per cent backward
while the return routes are tens of per cent [14]. A measurement of this
quantity is therefore a measurement of a *volume*, and comparisons must say
which volume they mean.

### 3.5 Two ways to get this badly wrong

**Gap junctions.** Electrical synapses are symmetric. A gap junction
appears in a directed graph as a pair of edges of identical weight in
opposite directions, which no ordering can ever make forward. Including them
manufactures irreducible recurrence out of connections that have no
direction to be wrong about. The widely used catalogue copy of the *C.
elegans* connectome merges gap junctions with chemical synapses, and its own
documentation states that the meaning of its edge directions is not
recorded; 29% of its reciprocal pairs carry exactly equal weights, which is
the signature. We discarded it and used the typed White et al. source [5],
chemical synapses only.

**Threshold.** At a one-synapse threshold rather than two, the worm reads
[6.29%, 6.41%] instead of 3.81%. Weak connections carry a disproportionate
share of the irreducible recurrence. Absolute values of this quantity are
therefore threshold-relative, and no comparison across connectomes is
meaningful unless the threshold is held fixed, as it is in §3.1.

### 3.6 The orderings are not arbitrary

A near-optimal ordering is not unique, which raises the question of whether
anything read from one is an artifact of which one. Two checks.

First, each ordering recovers the animal's known axis, from wiring alone,
with no labels supplied:

| | first | middle | last |
|---|---|---|---|
| *C. elegans* | sensory 24.5% | interneuron 42.1%, motor 62.3% | muscle 82.5% |
| *D.* larva | sensory 8.5% | PN 27.1%, Kenyon cells 38.6% | descending 94.4% |
| *D.* adult | sensory 7.9% | optic 47.0%, central 85.5% | motor 99.8% |

Second, in the adult we built six orderings from different starting layouts
with no merging between them. They disagree about the direction of some 1.2
million units of weight — 2.4% of the graph — while scoring within 0.06% of
each other. Across all six, sensory neurons sit at 7.8 ± 0.1% and motor
neurons at 99.8 ± 0.1%. The axis is a property of the connectome, not of the
particular ordering found. Finer classes are not all so stable: Kenyon cells
move by about twenty points between orderings, and no claim here rests on
the position of any individual class.

## 4. Limitations

- **Four points, three of them *Drosophila*.** This is not a phylogenetic
  analysis and no trend should be fitted to it. The informative next point
  is mammalian cortex [13].
- **Size is confounded** with complexity, with developmental stage, and with
  reconstruction completeness: larger volumes are also more recently and
  more thoroughly traced.
- **The adult whole-brain interval is wide** at 5.42 points. Exact minimum
  feedback arc set on disjoint induced subgraphs — a strictly stronger bound
  than cycle packing on the same edges — improves it by only 0.031 points,
  so the limitation is the relaxation rather than its implementation.
  Narrowing it needs a different relaxation entirely.
- **"Whole brain" differs between animals.** The larval brain excludes the
  ventral nerve cord; the hemibrain is one hemisphere of a central brain;
  the *C. elegans* figure covers the somatic nervous system including
  muscle targets. We compare what each dataset contains and say so.
- **This is wiring, not dynamics.** Nothing here measures what any circuit
  computes, and a structural measure of recurrence does not establish that
  the recurrence is functionally used.

## 5. What this does and does not say about consciousness

It gives the theories in §1 a quantity they have asserted the importance of
but never had measured: how much of a nervous system's connectivity is
irreducible to any hierarchy, with a certificate, at whole-organism scale.
That it rises steeply and monotonically with nervous system size, across two
phyla, is the kind of structural fact those theories would predict.

It does not show that any of these animals is conscious, and we make no such
claim. A fly is not conscious in any settled sense and a worm is certainly
not. What we report is a property of wiring. Whether that property is the
substrate those theories require is exactly the open question, and a
measurement is a precondition for addressing it rather than an answer to it.

## 6. Methods

**Graphs.** FlyWire from the challenge distribution [1,15]; hemibrain v1.2
traced non-cropped neurons [3]; larva from the published multigraph [4],
summing the four compartment-pair edge types (`ad`, `aa`, `da`, `dd`) so
that weights mean the same thing they do in the other datasets; *C. elegans*
from the typed White et al. reconstruction [5,6], chemical synapses only.
Self-loops dropped, connections below two synapses discarded.

**Orderings.** A SpringRank initialisation [9] followed by iterated local
search over a sifting neighbourhood with hub moves and topological-shuffle
kicks. The search is not the contribution and is described in [14].

**Bounds.** Greedy cycle packing on residual capacities: all 2-cycles, then
3-cycles, then longer cycles by bounded search. Every packing is written out
and re-verified by a separate program that re-reads the graph, checks each
cycle is a genuine directed cycle, and checks no connection is used beyond
its weight. A bound that fails this check is not reported.

**Verification.** Every ordering is re-scored by an independent pure-stdlib
implementation that re-reads the graph and ordering from disk and shares no
code with the solver. Both checks run on public data in minutes.

## 7. AI use declaration

Generative AI was used as a research tool in this work, not only as writing
assistance. Claude (Anthropic) wrote the solver, the bound and the analysis
code, ran the experiments, proposed the framing of irreducible recurrence as
a measurable quantity, and drafted this manuscript. The author set the
direction, decided which lines to pursue, ran the verification scripts, read
the code and the cited sources, checked the figures and quoted numbers
against the data, and takes full responsibility for this paper, including
its claims and references. No AI system is an author, and none could be:
accountability for a scientific claim cannot be delegated to a model.

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

---

## Notes for the author, not for the draft

- **Citations.** All fifteen checked against the PubMed or Crossref record.
  PMIDs for the ones new to this paper: [5] 22462104, [10] 16997611,
  [11] 32135090, [13] 40205214; [12] confirmed via Crossref
  (10.1038/nrn.2016.44) because PubMed phrase search returns the wrong
  article. [14] is the companion paper and needs its preprint DOI once
  posted.
- **The companion paper should go first or alongside.** This one leans on it
  for the method and for the six-basin robustness result.
- **Figure.** `figures/fig4_recurrence.svg`, the four intervals.
- **The mouse point is the one that matters.** MICrONS needs CAVE
  registration and real extraction work. Until it exists, the trend is three
  usable points and should be described as suggestive, which §3.3 and §4 do.
- **Do not let the consciousness framing grow.** §1 and §5 are the whole of
  it on purpose. The measurement stands without the motivation; the
  motivation does not stand without the measurement.
