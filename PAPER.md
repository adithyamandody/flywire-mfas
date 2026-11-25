# A maximally feedforward ordering of the *Drosophila* connectome recovers its processing hierarchy and replicates across two brains

**Adithya Mandody**, independent researcher.
Draft, 24 November 2026. Code and data: https://github.com/adithyamandody/flywire-mfas

---

## Abstract

A connectome is a directed, weighted graph; a brain is a device that turns
sensation into action. The simplest bridge between the two is to ask for the
ordering of neurons that makes the most synaptic weight point forward — the
weighted minimum feedback arc set problem. We compute a high-quality solution
for the FlyWire whole-brain connectome of adult *Drosophila* (136,648 neurons, 41,912,141
synapses), reaching 35,551,569 forward weight (84.824%), above the best
figures we are aware of in print, and we prove by cycle packing that no
ordering can exceed 90.242%, so the remaining gap is at most 5.4 points. The
resulting order, built with no biological input whatsoever, places sensory
neurons at the 7.9% mark and motor neurons at 99.8%, and lays the visual
pathway out in its correct anatomical sequence. Feedback is not uniform:
the outbound visual route is 2.4–4.7% backward while the return routes are
38–75% backward; connections from GABAergic, dopaminergic and serotonergic
neurons run backward more often than cholinergic ones; and the two neurons
carrying the most backward weight in the whole brain are APL and CT1, the
fly's textbook giant inhibitory feedback cells. We then repeat the entire
procedure on the independently reconstructed hemibrain connectome of a
different fly. The axis, the route asymmetry, the near-absence of feedback
onto output neurons, and APL's position at the top all reappear; the
glutamate result does not. Across the 2,831 cell types shared by the two
connectomes, how much feedback a type carries correlates at Spearman
ρ = 0.615 against a shuffled ceiling of 0.057. A single optimisation
objective, applied to wiring alone, recovers a hierarchy that two brains
agree on.

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
| weight forced backward in any ordering | 4,089,720 | 2,377,741 |
| upper bound | 37,822,421 (**90.242%**) | 10,165,208 (**81.043%**) |
| remaining gap | 5.42 points | **1.91 points** |

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
correct anatomical sequence. This is a validation rather than a discovery —
the anatomy is well known — but it is an independent one, and it is the
evidence that the order carries information beyond its objective value.

### 2.3 Feedforward routes are nearly pure; the feedback is on the return path

15.18% of all synaptic weight (6,360,572) points backward. That weight is
very unevenly distributed, and the clearest way to see it is to take each
pair of classes and look at both directions between them (Figure 3). There
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

### 2.4 Feedback is biased toward inhibitory and modulatory transmitters

Splitting the same weight by the sending neuron's predicted transmitter [4]:

| transmitter | share of weight | % of its weight backward |
|---|---|---|
| acetylcholine (excitatory) | 56.8% | **10.6%** |
| dopamine | 1.3% | 17.6% |
| GABA (inhibitory) | 24.3% | 21.1% |
| glutamate | 16.7% | 21.3% |
| serotonin | 0.7% | **29.2%** |

Cholinergic connections are the most feedforward class of connection in the
brain; serotonergic ones the least. Section 2.6 shows that this holds for
acetylcholine, GABA and serotonin in a second connectome but **not** for
glutamate.

### 2.5 The neurons carrying the most feedback are the ones named for it

Ranking individual neurons by the backward weight they carry, the top of the
list is APL (two copies, one per hemisphere, 55,913 and 48,211) followed by
CT1 (42,467 and 41,217). APL broadly innervates the whole mushroom body
[13]; Kenyon cells excite it and it inhibits them in return, and that
negative feedback loop is what keeps their odour code sparse [14]. In this
graph the loop is direct and nearly complete: 5,176 of the 5,177 Kenyon
cells synapse onto an APL and 5,166 receive a synapse back, which the
functional experiments on their own could not establish [14]. CT1 is a
single giant cell spanning one stratum of the medulla and one of the lobula,
with a separate arbour terminal in each column [16] — roughly 700 of them,
covering the whole visual field [15]. Both are GABAergic by immunostaining
rather than by the predicted labels of §2.4 — APL by anti-GABA [13], CT1 by
anti-GAD1 [16] — and both are standard examples of wide-field feedback. The
ordering puts them at the top of the feedback ranking without being given
any of this.

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
| serotonin | 1.92× | 1.79× |
| dopamine | 1.16× | 1.49× |
| glutamate | 1.40× | **0.95×** |

Four of the five transmitter results and all of the pathway results survive.
Glutamate acts as an inhibitory transmitter in the fly antennal lobe,
through the glutamate-gated chloride channel GluClα [20] — but that is
evidence from one circuit, not from the brain as a whole. It is 1.40× in
FlyWire and dead average in hemibrain; we therefore regard the "feedback is
inhibitory" claim as supported for GABA and the modulators and unsupported
for glutamate.

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

The strongest test uses every cell type at once (Figure 2). For each of the
2,831 cell types present in both connectomes with at least 1,000 synaptic
weight, we take the share of that type's weight the ordering must turn
around — one number per type per brain — and correlate the two lists:

| | Spearman ρ |
|---|---|
| backward **rate** (share of the type's own weight) | **0.615** |
| backward weight (absolute) | 0.598 |
| total weight (size control) | 0.602 |
| shuffled control, 200 draws: largest &#124;ρ&#124; observed | 0.057 |
| restricted to the 243 types with ≥10,000 weight in both | 0.628 |

The rate is already normalised by each type's own weight, so type size cannot
produce the correlation, and restricting to types whose rate is measured from
a large amount of weight raises it slightly. Agreement is visible at both
ends of the range: lLN1_a 44.2% vs 40.9%, APL 43.3% vs 46.4%, PVLP011 40.8%
vs 41.0%, down to DNp13 at 0.3% vs 0.3% and DNES2 at 0.0% vs 0.0%.

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
9.76% of all synaptic weight must point backward in any ordering whatsoever
(§2.1), and the ordering concentrates the realised backward weight on exactly
the pathways and cell types that neuroscience already calls feedback. The
method therefore behaves like a detector for recurrence, with a certificate
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

- **One ordering of many near-optimal ones.** Neurons with similar scores can
  be exchanged with little effect on the total, so an individual neuron's
  position is soft. The class- and type-level results involve thousands of
  neurons each and are not sensitive to this; single-neuron positions should
  not be over-read.
- **"Backward" means backward in this ordering.** Part of the backward weight
  is unavoidable and we bound it, but which particular edges get turned around
  depends on the ordering chosen.
- **Labels in the hemibrain analysis are FlyWire's**, transferred by cell
  type. The ordering is independently derived; the labelling is not.
- **Hemibrain is a cropped volume**: one hemisphere, no sensory input, no
  optic lobe. Several differences from FlyWire are attributable to this and
  are marked where we can tell.
- **Transmitter identities are predictions** [4], not measurements, at roughly
  94% accuracy per neuron. DPM is a case where the prediction and the
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
library. Figure 3 overlays the hemibrain rate only on routes whose two
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
which lines to pursue.

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
The author set the direction, decided which lines to pursue, and takes full
responsibility for this paper, including its claims, figures and references.
No AI system is an author, and none could be: accountability for a
scientific claim cannot be delegated to a model.

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
- **One sentence only you can verify.** The disclosure states what the
  author did: set the direction and chose which lines to pursue at each
  stage. That is what I can attest to. If you also read code, re-ran a
  check, or went to the sources yourself, add it — that is precisely the
  "demonstrable human accountability" the policies ask for, and it counts
  for more stated by you than inferred by me. Equally, do not let the
  sentence claim more review than happened; an overstated oversight claim
  is the one thing in this draft that a referee could not check and that
  would be worth retracting over.
- **Leaderboard status.** Our 35,551,569 exceeds the figures published by
  Bader et al. (35,459,266) and Vahidi (0.8461), and the top public
  leaderboard entry as of November 2026 (35,544,233). The challenge closed in
  October 2024; our submission of 35,551,497 was emailed on 18 November 2026
  and has not yet appeared on the leaderboard. Any "state of the art" claim
  should be phrased as a comparison to specific published numbers, as above,
  rather than as a leaderboard position.
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
- **Figures.** `figures/fig1_axis.svg`, `figures/fig2_crosscheck.svg` and
  `figures/fig3_pathways.svg`, all regenerated by `mfas_code/figures.py` from
  the orderings and the graphs. Figure 3 leaves the hemibrain comparison off
  any route involving the optic lobe or centrifugal neurons, because the
  hemibrain volume does not contain their partners; that exclusion is stated
  on the figure itself and should stay stated wherever it is reproduced.
