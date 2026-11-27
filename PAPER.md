# A maximally feedforward ordering of the *Drosophila* connectome recovers its processing hierarchy and replicates across two brains

**Adithya Mandody**, independent researcher.
Draft, 25 November 2026. Code and data: https://github.com/adithyamandody/flywire-mfas

---

## Abstract

A connectome is a directed, weighted graph; a brain is a device that turns
sensation into action. The simplest bridge between the two is to ask for the
ordering of neurons that makes the most synaptic weight point forward — the
weighted minimum feedback arc set problem. We compute a high-quality solution
for the FlyWire whole-brain connectome of adult *Drosophila* (136,648 neurons, 41,912,141
synapses), reaching 35,551,569 forward weight (84.824%), above the best
figures we are aware of in print, and we prove by cycle packing that no
ordering can exceed 87.374%, so the remaining gap is at most 2.6 points. The
resulting order, built with no biological input whatsoever, places sensory
neurons at the 7.9% mark and motor neurons at 99.8%, and lays the visual
pathway out in its correct anatomical sequence — though we show that most of
that axis is also recoverable from a single degree statistic, and scope our
claims to what the optimisation adds beyond it. Feedback is not uniform:
GABAergic connections run backward twice as often as cholinergic ones, and
APL, the mushroom body's giant inhibitory neuron, turns 44% of its own
synaptic weight around against a brain-wide 15%. We then repeat the entire
procedure on the independently reconstructed hemibrain connectome of a
different fly. Compared like for like — central brain against central brain —
the axis, the route asymmetry, the near-absence of feedback onto output
neurons, APL's rate, and every transmitter result replicate. Across the
2,831 cell types shared by the two connectomes, how much feedback a type
carries correlates at Spearman ρ = 0.615, and at 0.607 after controlling
for type size. A single optimisation objective, applied to wiring alone,
produces an ordering that two independently reconstructed brains agree on at
cell-type resolution.

---

## 1. Introduction

Connectomes are now large enough that the problem has shifted from obtaining
them to reading them. One natural summary of a directed network is a linear
order on its nodes, chosen so that as much edge weight as possible runs
forward. For a brain this order is a candidate processing hierarchy: a
proposal about what is upstream of what, derived from wiring alone.

Formally, given a weighted directed graph $G=(V,E,w)$ we seek a bijection
$\pi: V \to \{1,\dots,n\}$ maximising

$$\sum_{(u,v)\in E,\; \pi(u)<\pi(v)} w(u,v).$$

This is the weighted linear ordering problem, equivalently the weighted
minimum feedback arc set problem, and it is NP-hard [11]. The FlyWire
Connectome Challenge [12] poses exactly this instance, on the whole-brain
reconstruction of the adult female *Drosophila* brain [1].

Two things make the exercise worth doing carefully. First, a heuristic
ordering is only interesting if it is close to the best possible one;
otherwise its structure may be an artifact of where the search stopped.
Second, a hierarchy extracted from one brain is a hypothesis about brains,
and a connectome is one animal. We address both: we certify how much of the
gap to optimality remains, and we repeat everything in a second, independently
acquired connectome.

**Related work.** Greedy orderings for feedback arc set go back to Eades, Lin
and Smyth [9]. SpringRank [8] produces continuous hierarchy scores for
directed networks and we use it as an initialiser. In connectomics, Varshney
et al. [10] laid out the *C. elegans* wiring diagram to reflect signal flow,
and Winding et al. [5] analysed feedforward and feedback pathways in the
larval *Drosophila* brain, reporting that 41% of neurons receive long-range
recurrent input. On this specific instance, Bader et al.'s Rocket-Crane [6]
reports 35,459,266 forward weight (84.60%) and Vahidi [7] reports a forward
ratio of 0.8461; both are algorithm papers and neither interprets the
resulting order biologically, nor bounds it from above.

Our contribution is (i) a better ordering, (ii) a verified upper bound that
says how much room is left, (iii) a biological read-out of the order, and
(iv) a replication of that read-out in a second connectome.

---

## 2. Results

### 2.1 An ordering with a certified optimality gap

Our solver reaches **35,551,569** forward weight on the FlyWire challenge
graph, 84.824% of the 41,912,141 total, against the challenge's own greedy
baseline of 29,023,882 (69.25%). Every number we report was re-scored by an
independent implementation written for the purpose.

Because the objective is a maximisation, a heuristic alone cannot say how
much is left. We obtain an upper bound by cycle packing. In any ordering,
every directed cycle contains at least one backward edge. Assign each
directed cycle $C$ a value $y_C \geq 0$ such that for each edge $e$,
$\sum_{C \ni e} y_C \leq w_e$. Then in **any** ordering the backward weight is
at least $\sum_C y_C$, and the forward weight is at most the total minus that
sum. A greedy packing on residual capacities — all 2-cycles, then 3-cycles,
then longer cycles found by bounded search — yields:

| | FlyWire | hemibrain |
|---|---|---|
| neurons | 136,648 | 21,737 |
| connections | 5,657,719 | 1,764,123 |
| total synaptic weight | 41,912,141 | 12,542,949 |
| our forward weight | 35,551,569 (**84.824%**) | 9,925,483 (**79.132%**) |
| weight forced backward in any ordering | 5,291,688 | 2,377,741 |
| upper bound | 36,620,453 (**87.374%**) | 10,165,208 (**81.043%**) |
| remaining gap | **2.55 points** | **1.91 points** |

Each packing is re-checked by a separate program that re-reads the graph,
confirms every packed cycle is a genuine directed cycle, and confirms that no
connection is used beyond its weight. For hemibrain this verified 669,576
cycles (264,098 of length 2, 289,939 of length 3, 115,539 longer) with zero
violations.

### 2.2 The ordering recovers the input-to-output axis

Joining the FlyWire order to the published annotations [2] — which cover
136,635 of our 136,648 neurons — gives Figure 1 and the medians below. The search optimised a sum
of edge weights and nothing else; it had no access to any of these labels.

| super class | neurons | median position |
|---|---|---|
| ascending | 1,629 | 6.9% |
| sensory | 14,827 | **7.9%** |
| visual_centrifugal | 524 | 16.9% |
| optic | 77,509 | 47.0% |
| visual_projection | 8,038 | 78.2% |
| central | 32,294 | 85.5% |
| descending | 1,276 | 98.9% |
| motor | 109 | **99.8%** |

Sensory neurons land in the first tenth of the order, motor neurons in the
last fifth of a percent, and the visual pathway sits between them in its
correct anatomical sequence. This is a validation rather than a discovery — the
anatomy is well known — and §2.8 shows that most of it does not require the
optimisation at all. What it establishes is that the ordering is not
nonsense; the claims that depend on the optimisation are set out there.

### 2.3 Feedforward routes are nearly pure; the feedback is on the return path

15.18% of all synaptic weight (6,360,572) points backward. That weight is
very unevenly distributed, and the clearest way to see it is to take each
pair of classes and look at both directions between them (Figure 2). There
are nine pairs of classes carrying at least 20,000 weight in both directions,
and in **every one of them** one direction is far below the brain-wide rate
and the other far above it:

| the clean direction | % backward | the other way | % backward |
|---|---|---|---|
| ascending → central | 0.8% | central → ascending | 54.2% |
| optic → visual_projection | 2.4% | visual_projection → optic | 53.1% |
| central → descending | 2.8% | descending → central | 40.0% |
| sensory → optic | 2.8% | optic → sensory | 35.2% |
| visual_projection → central | 4.7% | central → visual_projection | 46.9% |
| visual_centrifugal → optic | 5.0% | optic → visual_centrifugal | 37.6% |
| visual_centrifugal → visual_projection | 5.4% | visual_projection → visual_centrifugal | **75.0%** |
| sensory → central | 6.7% | central → sensory | 27.3% |
| visual_centrifugal → central | 8.7% | central → visual_centrifugal | 43.9% |

The clean directions span 0.8–8.7%, the return directions 27.3–75.0%, and
the brain-wide figure of 15.18% falls strictly between the two in all nine
cases. The ordering is not spreading its backward weight around; it is
putting almost all of it on one side of each route.

Connections within a class behave differently again. By sheer volume 80% of
all backward weight is local — optic→optic 48.1%, central→central 32.0% —
but these are the two largest blocks in the connectome and their backward
*rates*, 17.9% and 16.3%, sit close to the brain-wide figure. The structure
is in the rates between classes, not in the totals.

Sorting instead by the receiving neuron's class, the most feedback-saturated
class in the brain is visual_centrifugal (45.7% of the 485,721 weight it
receives), the neurons that by definition carry signals from the central
brain back into the optic lobe. At the other end, motor neurons receive 0.9%
backward: by the time the brain commits to an action, the loops are behind it.

### 2.4 Inhibitory connections run backward about twice as often

Splitting the same weight by the sending neuron's predicted transmitter [4]:

| transmitter | share of weight | % of its weight backward |
|---|---|---|
| acetylcholine (excitatory) | 56.8% | **10.6%** |
| GABA (inhibitory) | 24.3% | **21.1%** |
| glutamate | 16.7% | 21.3% |

Cholinergic connections are the most feedforward class in the brain and
GABAergic ones run backward at roughly twice that rate. Both replicate in a
second connectome (§2.6). Glutamate is treated separately there, because its
elevated rate turns out to be confined to one region.

**Dopamine and serotonin are omitted, and should be.** They appear in the
transmitter predictions as 1.3% and 0.7% of weight, with rates of 17.6% and
29.2%, and an earlier version of this paper reported them. They do not
survive inspection. Of the 5,177 Kenyon cells, 5,172 are *predicted*
dopaminergic, while the same annotation file's measured `known_nt` column
records acetylcholine for all 5,177; those cells carry roughly three quarters
of everything counted as dopaminergic weight. The serotonin row is dominated
the same way by a few dozen antennal lobe local neurons that the measured
column also calls cholinergic. At these abundances the rows are reporting a
known failure mode of the image-based classifier rather than a property of
the brain, and transferring the labels to hemibrain by cell type (§2.6)
reproduces the error rather than testing it. The acetylcholine and GABA rows
are unaffected: they rest on 81% of all synaptic weight, where prediction and
measurement agree.

### 2.5 One neuron turns around 44% of its own traffic

Ranked by the **share of its own synaptic weight** the ordering has to turn
around, against a brain-wide 15.18%, the standout is APL: 44.0% and 42.6%
for the two copies, one per hemisphere. CT1 follows the same way at 25.7%
and 25.0%.

Rank by *absolute* backward weight instead and the same four cells lead, but
that ranking is substantially a size ranking — CT1 and APL are the four
largest neurons in the brain by total synaptic weight, and 64 of the top 100
by backward weight are also in the top 100 by size. The rate is the measure
that means something, and by it APL is exceptional (2.9× the brain) while
CT1 is merely elevated (1.7×). APL broadly innervates the whole mushroom body
[13]; Kenyon cells excite it and it inhibits them in return, and that
negative feedback loop is what keeps their odour code sparse [14]. In this
graph the loop is direct and nearly complete: 5,176 of the 5,177 Kenyon
cells synapse onto an APL and 5,166 receive a synapse back, which the
functional experiments on their own could not establish [14]. CT1 is a
single giant cell spanning one stratum of the medulla and one of the lobula,
with a separate arbour terminal in each column [16] — roughly 700 of them,
covering the whole visual field [15]. Both are GABAergic by immunostaining
rather than by the predicted labels of §2.4 — APL by anti-GABA [13], CT1 by
anti-GAD1 [16]. APL is a textbook wide-field feedback neuron; we do not
claim CT1 is one, and neither [15] nor [16] describes it that way — its
characterised role is as an amacrine input to the T4/T5 motion detectors.
What the ordering shows is that it carries an elevated share of backward
weight, which is a statement about position in a hierarchy, not about its
physiological role.

Below them the list is local interneurons: lobula intrinsic (Li30/32/33),
lobula plate intrinsic (LPi12/13), amacrine (Am1), serpentine medulla
(Sm42). By feedback per neuron the densest classes are ME.LO (1,130 each)
and antennal lobe local neurons (848 each) — the lateral inhibition that
implements gain control in the antennal lobe [17].

### 2.6 Replication in an independent connectome

We repeated the entire pipeline on the hemibrain connectome [3]: a different
animal, a different laboratory, a different reconstruction, 21,737 traced
non-cropped neurons and 1,764,123 connections of at least 2 synapses. Three
independent solver runs landed within 0.02 points of each other and merging
them gained nothing, so the search is converged; the result is within 1.91
points of the certified ceiling (§2.1).

Labels are carried from FlyWire by cell type, through the `hemibrain_type`
column of the FlyWire annotations; 15,710 of 21,737 neurons (72%) could be
matched. **The labels are borrowed; the ordering is not.** Cell type is
conserved between individuals, while each neuron's position is decided by
hemibrain's own wiring.

Because the two connectomes have different brain-wide backward rates (15.18%
vs 20.87%), rates are compared relative to each brain's own:

| | FlyWire | hemibrain |
|---|---|---|
| descending neurons, median position | 98.9% | 97.2% |
| the way in (visual_projection → central) | 0.31× | **0.30×** |
| the way back (central → visual_projection) | 3.09× | 2.30× |
| feedback received by output neurons | 0.20× | **0.13×** |
| acetylcholine | 0.70× | 0.83× |
| GABA | 1.39× | 1.20× |
| glutamate, whole brain vs central brain | 1.40× | 0.95× |
| glutamate, **central brain both sides** | **0.97×** | **0.95×** |

Every pathway result and both reliable transmitter results survive.
Glutamate looked like the exception and is not. An earlier version of this
paper recorded it as 1.40× in FlyWire against 0.95× in hemibrain and called
it a failed replication. That comparison was not like for like: every other
row sets FlyWire's **whole** brain against hemibrain's **central** brain.
Restricted to the central brain on both sides, FlyWire gives 0.97× against
hemibrain's 0.95× — agreement, not disagreement.

The 1.40× is real but regional. Within the optic lobe, glutamatergic
connections run backward at 27.0% against that region's own 16.0%, a local
ratio of 1.69×; within the central brain they sit at 15.8% against 16.3%.
Glutamate's backward bias is an optic-lobe phenomenon, and hemibrain
contains no optic lobe, so it could not have shown it either way. This also
fits what is known physiologically: glutamate's inhibitory action in the fly
is established in the antennal lobe through the glutamate-gated chloride
channel GluClα [20], evidence from particular circuits rather than from the
brain as a whole.

Ranking neurons by backward weight in each brain separately:

| cell type | FlyWire rank (of 136,648) | hemibrain rank (of 21,737) |
|---|---|---|
| **APL** | **1** | **1** |
| PVLP011 | 5 | 3 |
| AVLP080 | 14 | 5 |
| DPM | 143 | 2 |
| oviIN | 148 | 4 |
| LHCENT3 | 85 | 20 |
| CT1 | 3 | 8,643 |

Every type here except CT1 is in the top 0.11% of both brains. In hemibrain
APL carries 105,167 units of backward weight, 2.6× the next neuron and about
a thousand times the median. Second to it is DPM, the mushroom body's other
wide-field cell: one neuron per hemisphere, projecting over all of the lobes
[18], whose output is required during the consolidation of middle-term
olfactory memory [19]. CT1 is the explicable exception: it is a single
enormous cell spanning the medulla and lobula [15], and the hemibrain volume
contains only a cropped fragment of it.

Two further differences follow from what each volume contains rather than
from the orderings. Hemibrain's traced set holds just 2 olfactory receptor
neurons — the rest are severed at the block boundary — so there is no
sensory class to place. And centrifugal neurons move from 16.9% in FlyWire to
91.4% in hemibrain because the optic lobe they feed back into is outside the
hemibrain volume; both orderings place them immediately upstream of their
targets, and the volumes disagree about where those targets are.

### 2.7 Agreement at cell-type resolution

The strongest test uses every cell type at once (Figure 3). For each of the
2,831 cell types present in both connectomes with at least 1,000 synaptic
weight, we take the share of that type's weight the ordering must turn
around — one number per type per brain — and correlate the two lists:

| | Spearman ρ |
|---|---|
| backward **rate** (share of the type's own weight) | **0.615** |
| backward weight (absolute) | 0.598 |
| total weight (size control) | 0.602 |
| **partial ρ, controlling for type size** | **0.607** |
| shuffled control, 200 draws: largest &#124;ρ&#124; observed | 0.057 |
| restricted to the 243 types with ≥10,000 weight in both | 0.628 |

The rate is already normalised by each type's own weight, so type size cannot
produce the correlation, and restricting to types whose rate is measured from
a large amount of weight raises it slightly. Agreement is visible at both
ends of the range: lLN1_a 44.2% vs 40.9%, APL 43.3% vs 46.4%, PVLP011 40.8%
vs 41.0%, down to DNp13 at 0.3% vs 0.3% and DNES2 at 0.0% vs 0.0%.

### 2.8 How much of this needs the optimisation?

A near-optimal ordering is expensive to compute, so it is worth asking what
of the above a cheap one would also give. Order every neuron by the share of
its own synaptic weight that is outgoing — out / (in + out) — descending.
That is one arithmetic operation per neuron, no search of any kind, and it
scores 71.91% forward against our 84.824%. Yet:

| class | optimised | out-share sort |
|---|---|---|
| ascending | 6.9% | 5.2% |
| sensory | **7.9%** | **7.2%** |
| optic | 47.0% | 49.3% |
| visual_projection | 78.2% | 79.0% |
| **central** | **85.5%** | **60.6%** |
| descending | 98.9% | 94.6% |
| motor | **99.8%** | **98.7%** |

Every class lands within about two points of the optimised position except
one. The cause is local and unmysterious: 5,476 of 14,827 sensory neurons
have no inputs at all in this graph and 79 of 109 motor neurons have no
outputs, so a statistic that reads only each neuron's own in/out balance
already places the two ends of the brain correctly.

**The axis of §2.2 is therefore not evidence that the optimisation found
something a simpler statistic could not.** What the optimisation adds is the
central-brain row — placing 32,294 central neurons after the visual
projection neurons that drive them, a 25-point correction — together with
13 points of objective value and the tighter rates everything downstream is
measured against. Readers taking hierarchy off a connectome ordering should
run this baseline; a result that the out-share sort reproduces is a result
about degree, not about hierarchy.

The cell-type agreement of §2.7 is the result here that has no such cheap
explanation, because it compares two independently reconstructed brains
rather than two orderings of one.

---

## 3. Discussion

A single scalar objective over wiring — point as much synaptic weight forward
as possible — reproduces the fly brain's gross functional organisation, and
two brains agree on the result at the resolution of individual cell types.
That is a stronger statement than "connectomes are hierarchical". It says the
hierarchy is recoverable without any functional data, any anatomy, or any
prior, and that what it recovers is reproducible across animals.

The most interesting structure is in what refuses to be ordered. Backward
weight under an optimal ordering is a lower bound on recurrence: it is the
part of the wiring that no hierarchy can explain away. In FlyWire at least
12.63% of all synaptic weight must point backward in any ordering whatsoever
(§2.1), and the ordering concentrates the realised backward weight on
pathways and cells that neuroscience already calls feedback. The method
therefore behaves like a detector for recurrence, with a certificate
attached.

The hemibrain result sharpens this. Its ordering reaches only 79.13% against
FlyWire's 84.82%, but it is within 1.91 points of its own proved ceiling
rather than 5.42. The lower figure is not a weaker search: the central brain,
with its sensory input severed and no optic lobe to supply easy feedforward
weight, simply is more recurrent. Feedforward share is thus a property of a
volume that can be measured and compared, provided the bound is computed
alongside it.

---

## 4. Limitations

- **One ordering of many near-optimal ones**, and we have measured what that
  costs. Six orderings built from different starting layouts, with identical
  search settings and no merging, land within 0.06% of each other in score
  while disagreeing about the direction of roughly 1.2 million units of
  weight — 2.4% of the graph. Across all six, sensory neurons sit at
  7.8 ± 0.1% and motor neurons at 99.8 ± 0.1%, so the axis of §2.2 does not
  depend on which of them is chosen. Finer classes are not all so stable:
  Kenyon cells and antennal lobe projection neurons move by about twenty
  points between basins, so the mushroom body's position in the hierarchy is
  not determined by this method and nothing here rests on it. Single-neuron
  positions should not be over-read at all.
- **"Backward" means backward in this ordering.** Part of the backward weight
  is unavoidable and we bound it, but which particular edges get turned around
  depends on the ordering chosen.
- **Labels in the hemibrain analysis are FlyWire's**, transferred by cell
  type. The ordering is independently derived; the labelling is not.
- **Hemibrain is a cropped volume**: one hemisphere, no sensory input, no
  optic lobe. Several differences from FlyWire are attributable to this and
  are marked where we can tell.
- **Much of the axis does not need the optimisation.** A one-line degree
  statistic reproduces every class position except the central brain's
  (§2.8). Claims resting on the axis alone are claims about degree.
- **Transmitter identities are predictions** [4], not measurements, at roughly
  94% accuracy per neuron. Where a prediction is systematically wrong for a
  large population the error does not average out: 5,172 of 5,177 Kenyon
  cells are predicted dopaminergic against a measured acetylcholine, which is
  why §2.4 reports only the transmitters carrying 81% of weight and drops the
  two rare ones. Transferring labels to a second connectome by cell type
  (§2.6) propagates such errors rather than testing them. DPM is a case where the prediction and the
  immunostaining recorded in the annotations [2] disagree: predicted
  dopaminergic, measured serotonergic and GABAergic.
- **One position per neuron.** Each neuron is a single node here. CT1's
  terminals have been shown to act as independent functional units [15], so
  for a cell like it one position summarises hundreds of local circuits
  rather than describing one.
- Both connectomes are of female flies, and all results are correlational:
  they describe wiring, not computation.

---

## 5. Methods

**Data.** FlyWire FAFB v783 connectome and challenge graph [1, 12]; FlyWire
annotations, supplemental file 1 of [2]; hemibrain v1.2 traced adjacency
export [3], CC-BY 4.0. The hemibrain graph keeps connections of at least 2
synapses between traced, non-cropped neurons, 2 being the smallest weight
present in the FlyWire challenge graph, so the two graphs are filtered alike.

**Solver.** A continuous SpringRank solution [8] provides the initial order.
It is then improved by iterated local search over a sifting neighbourhood
(remove a neuron, reinsert it at its best position, computed in
O(deg log deg) by prefix scan over its neighbours sorted by position).
Iterations kick a set of neurons, freeze them while a repair cascade runs over
everything that could have changed, then release them; losing iterations are
undone exactly from a move log. Four move types are mixed: single-neuron
kicks, hub moves (a high-degree neuron plus its strongest wrong-side partners
moved together), topological-shuffle kicks following Bader et al. [6] (re-sort
a window along a random topological order of the pairs it currently gets
right, which cannot lose weight), and group moves (an anchor and its strongest
partners lifted out and reinserted as one block at the jointly best position).
Parallel searches run on disjoint regions of the order and are combined by an
edge-space merge: the child keeps each pair the base gets right unless an
island broke it, adds each pair an island fixed, topologically sorts those
constraints, and is kept only if it beats the base.

**Upper bound.** Greedy cycle packing on residual capacities: all 2-cycles,
then a sweep over every in/out pair at each node for 3-cycles, then longer
cycles by bounded search to depth 6. Every pushed cycle is written out and
re-verified independently (§2.1).

**Annotation join.** Neurons are joined to annotations by root ID (FlyWire) or
body ID (hemibrain). For hemibrain, class and transmitter are transferred from
FlyWire by majority vote over the FlyWire neurons carrying each
`hemibrain_type`; comma-separated multi-type entries contribute a vote to each
component for class transfer, and are excluded entirely from the cell-type
correlation of §2.7.

**Statistics.** Spearman rank correlation with average ranks for ties. The
null is 200 random permutations of one side.

**Figures.** Drawn directly to SVG by `mfas_code/figures.py` with no plotting
library. Figure 2 overlays the hemibrain rate only on routes whose two
endpoint classes are both inside the hemibrain volume; routes defined by
optic-lobe partners are not comparable between the two datasets and are shown
for FlyWire alone.

**Code and data.** All analysis code is pure-stdlib Python or C with no
external dependencies, in the repository above. Both orderings
(`best_solution.csv`, `hemibrain_solution.csv`) are included, along with the
scorer and bound verifier used to check every figure quoted here.

**Use of AI assistance.** This work was carried out with Claude (Anthropic)
throughout. Claude wrote the solver (`springrank.c`, `ilsg.c`, `tmerge.c`),
designed and implemented the cycle-packing bound (`bound3.c`), wrote every
analysis script in `mfas_code/`, ran the experiments, produced the figures
and drafted this manuscript; several of the analyses were its own
proposals, among them the hemibrain replication (§2.6) and the cell-type
correlation (§2.7). The author set the direction and decided at each stage
which lines to pursue, and did not rely on Claude's account of its own
results: the author ran `score.py` and `check_bound.py`, read the solver and
the analysis code, read the cited sources, and checked the figures and the
numbers quoted here against the underlying data.

Because that is a larger role than a declaration alone can qualify, the
central claims are built so as not to rest on trust in the process that
produced them. The score of §2.1 is recomputed from the published challenge
graph by `score.py`, a pure-stdlib script that re-reads the graph and the
ordering from disk and shares no code with the C solver. The bound is
re-verified by `check_bound.py`, which re-reads the graph and every cycle in
the packing, confirms each is a real directed cycle and checks the capacity
constraint connection by connection; a bound that could not be re-verified
this way was discarded rather than reported, which happened once. Both
checks run on public data in minutes. The biological results join published
annotations [2,4] to the ordering by identifier, and the principal ones
replicate in an independently collected connectome (§2.6).

Fabricated citation is a known failure mode of language models. Every
reference here was therefore checked against the PubMed or Crossref record
for the work, and the claims resting on each checked against the paper's
full text wherever it was reachable.

---

## 6. AI use declaration

Generative AI was used as a research tool in this work, not only as writing
assistance. Claude (Anthropic) wrote the solver and the analysis code,
designed and implemented the cycle-packing bound, ran the experiments,
produced the figures, proposed several of the analyses and drafted this
manuscript; §5 gives the detail and the checks that are independent of it.
The author set the direction and decided which lines to pursue; ran the
scorer and the bound verifier; read the solver and the analysis code; read
the cited sources; checked the figures and quoted numbers against the data;
and takes full responsibility for this paper, including its claims, figures
and references. No AI system is an author, and none could be:
accountability for a scientific claim cannot be delegated to a model.

---

## References

1. Dorkenwald S, et al. Neuronal wiring diagram of an adult brain. *Nature* **634**, 124–138 (2024).
2. Schlegel P, et al. Whole-brain annotation and multi-connectome cell typing of *Drosophila*. *Nature* **634**, 139–152 (2024).
3. Scheffer LK, et al. A connectome and analysis of the adult *Drosophila* central brain. *eLife* **9**, e57443 (2020).
4. Eckstein N, et al. Neurotransmitter classification from electron microscopy images at synaptic sites in *Drosophila melanogaster*. *Cell* **187**(10), 2574–2594 (2024).
5. Winding M, et al. The connectome of an insect brain. *Science* **379**, eadd9330 (2023).
6. Bader DA, Ellis-Joyce J, Both G-J, Turaga SC, Asoori Sriram H, Chinthalapudi S, Du Z. Rocket-Crane algorithm for the feedback arc set problem. *ASONAM 2025*; *Social Network Analysis and Mining* **15**(1) (2025).
7. Vahidi S. Feedforward ordering in neural connectomes via feedback arc minimization. arXiv:2506.13799 (2025).
8. De Bacco C, Larremore DB, Moore C. A physical model for efficient ranking in networks. *Science Advances* **4**, eaar8260 (2018).
9. Eades P, Lin X, Smyth WF. A fast and effective heuristic for the feedback arc set problem. *Information Processing Letters* **47**(6), 319–323 (1993).
10. Varshney LR, Chen BL, Paniagua E, Hall DH, Chklovskii DB. Structural properties of the *Caenorhabditis elegans* neuronal network. *PLoS Computational Biology* **7**(2), e1001066 (2011).
11. Karp RM. Reducibility among combinatorial problems. In Miller RE, Thatcher JW (eds), *Complexity of Computer Computations*, 85–103. Plenum Press (1972).
12. FlyWire Connectome MFAS Challenge. https://codex.flywire.ai/app/mfas_challenge?dataset=fafb
13. Liu X, Davis RL. The GABAergic anterior paired lateral neuron suppresses and is suppressed by olfactory learning. *Nature Neuroscience* **12**(1), 53–59 (2009).
14. Lin AC, Bygrave AM, de Calignon A, Lee T, Miesenböck G. Sparse, decorrelated odor coding in the mushroom body enhances learned odor discrimination. *Nature Neuroscience* **17**(4), 559–568 (2014).
15. Meier M, Borst A. Extreme compartmentalization in a *Drosophila* amacrine cell. *Current Biology* **29**(9), 1545–1550.e2 (2019).
16. Takemura S-Y, Nern A, Chklovskii DB, Scheffer LK, Rubin GM, et al. The comprehensive connectome of a neural substrate for 'ON' motion detection in *Drosophila*. *eLife* **6**, e24394 (2017).
17. Olsen SR, Wilson RI. Lateral presynaptic inhibition mediates gain control in an olfactory circuit. *Nature* **452**(7190), 956–960 (2008).
18. Waddell S, Armstrong JD, Kitamoto T, Kaiser K, Quinn WG. The *amnesiac* gene product is expressed in two neurons in the *Drosophila* brain that are critical for memory. *Cell* **103**(5), 805–813 (2000).
19. Keene AC, Stratmann M, Keller A, Perrat PN, Vosshall LB, Waddell S. Diverse odor-conditioned memories require uniquely timed dorsal paired medial neuron output. *Neuron* **44**(3), 521–533 (2004).
20. Liu WW, Wilson RI. Glutamate is an inhibitory neurotransmitter in the *Drosophila* olfactory system. *Proceedings of the National Academy of Sciences* **110**(25), 10294–10299 (2013).

---

## Notes for the author, not for the draft

- **Citations.** All twenty references were checked against the PubMed or
  Crossref record for the work — title, full author list, journal, volume,
  issue, pages and year — rather than from memory. PMIDs for the ones added
  here: [13] 19043409, [14] 24561998, [15] 31031119, [16] 28432786,
  [17] 18344978, [18] 11114336, [19] 15504331, [20] 23729809. [5], [10] and
  [11] were re-checked and the last two corrected (missing article number;
  missing editors and publisher).
- **How far each was read.** [13], [14], [16], [17] and [20] were read in
  full text, and every claim resting on them was checked against the passage
  that supports it. [15], [18] and [19] have no reachable free full text —
  *Current Biology* serves a bot check, and the 2000 *Cell* and 2004
  *Neuron* papers have no PMC deposit — so those three are confirmed only to
  the level of their abstracts. Each claim attached to them is stated in its
  abstract directly, but anyone with library access should confirm them from
  the PDFs.
- **Two claims were corrected, not just cited**, when the sources turned out
  not to support them. CT1 was described as normalising visual input: Meier
  & Borst show that it covers every column and that its terminals are
  independent, not that it normalises, so the text now says what they show.
  And the inhibitory role of glutamate is established in the antennal lobe,
  not brain-wide — which is itself a reason not to have expected the
  glutamate result to generalise.
- **The oversight sentence is yours, and you set it.** §5 and §6 state that
  you ran the scorer and the bound verifier, read the solver and the
  analysis code, read the cited sources, and checked the figures and quoted
  numbers against the data — confirmed by you on 25 November 2026. That is
  the "demonstrable human accountability" the policies ask for, and it is
  the strongest part of the disclosure precisely because it is specific. It
  is also the one claim in the paper a referee cannot check, so keep it
  true: if a later revision adds material you have not been through
  yourself, narrow the sentence to match before submitting.
- **Leaderboard status**, checked 27 November 2026. The submission emailed
  on 18 November was posted on 26 November and now stands first:
  *Adithya Mandody, 35,551,497 (84.824%)*. Second is a new entry of the same
  date, Jean-René Denoual at 35,544,474 (84.807%), which displaced the
  35,544,233 that stood when this draft was written; the margin is 7,023.
  Our 35,551,569 also exceeds the figures published by Bader et al.
  (35,459,266) and Vahidi (0.8461). The challenge itself closed in October
  2024 and the board is now an open "beat the top score" list, so a "state
  of the art" claim should still be phrased as a comparison to specific
  published numbers rather than as a leaderboard position — a position can
  change the week after a paper is posted, as it just did.
- **The listed score is not our best.** The board shows 35,551,497, the
  figure emailed on 18 November; the paper reports 35,551,569, found after
  that email. The difference is 72 weight and no claim depends on it, but a
  reader who checks the board against the paper will see two numbers under
  one name. Submissions are unlimited and only the best is shown, so sending
  the improved ordering would close that gap before the preprint goes up.
- **Disclosure.** Now written, as Methods §5 ("Use of AI assistance") plus a
  standalone §6 declaration. That is the structure Springer Nature asks for
  — a description in Methods and an AI use declaration at the end — and it
  also satisfies ICMJE, who want research use in the Methods and writing
  help in the acknowledgements; this work is both, so both are covered.
  Nature Portfolio's current policy is risk-tiered, and the tiers matter
  here: AI "contributing to reasoning", with verification, oversight and
  demonstrable human accountability, is amber (permitted with care), while
  "using an LLM to generate core research reasoning without disclosure" and
  "generating hypotheses, analyses or conclusions and presenting them as
  human-derived" are red (not permitted). The disclosure is what keeps this
  work on the permitted side, so it should not be trimmed to a sentence in
  the acknowledgements to make a word limit. Two things remain
  venue-specific: most journals also want the statement at submission, in
  the cover letter, so repeat it there; and if a venue has no numbered
  section for it, §6 can move into the acknowledgements unchanged.
- **Figures.** `figures/fig1_axis.svg`, `figures/fig2_pathways.svg` and
  `figures/fig3_crosscheck.svg`, all regenerated by `mfas_code/figures.py`
  from the orderings and the graphs. Figures are numbered in order of first
  citation, so the pathway figure is 2 and the cross-check is 3. Figure 2
  leaves the hemibrain comparison off
  any route involving the optic lobe or centrifugal neurons, because the
  hemibrain volume does not contain their partners; that exclusion is stated
  on the figure itself and should stay stated wherever it is reproduced.
