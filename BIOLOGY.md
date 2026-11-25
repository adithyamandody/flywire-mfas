# What the ordering says about the fly brain

The MFAS challenge asks for one number. But the thing we optimised is a list of
136,648 neurons in order, and FlyWire publishes what every one of those neurons
*is*. Joining the two turns the number into anatomy.

Annotations: Schlegel et al. 2024, supplemental file 1 (FAFB v783), which
covers 136,635 of our 136,648 neurons (100.0%). Reproduce with:

```
python3 -I mfas_code/biology.py mfas_data/graph.csv best_solution.csv \
        fw_annot/neuron_annotations.tsv
```

Our ordering is `best_solution.csv` (35,551,569 forward weight, 84.82%).
Nothing biological went into producing it: the search only ever maximised
forward synaptic weight.

## 1. The ordering recovers the brain's input-to-output axis on its own

| flow class | neurons | median position |
|---|---|---|
| afferent (into the brain) | 16,811 | **7.8%** |
| intrinsic | 118,373 | 55.5% |
| efferent (out of the brain) | 1,451 | **99.0%** |

and by super class:

| super class | neurons | median position |
|---|---|---|
| ascending | 1,629 | 6.9% |
| sensory | 14,827 | 7.9% |
| visual_centrifugal | 524 | 16.9% |
| optic | 77,509 | 47.0% |
| visual_projection | 8,038 | 78.2% |
| central | 32,294 | 85.5% |
| descending | 1,276 | 98.9% |
| motor | 109 | **99.8%** |

Sensory neurons land in the first tenth, motor neurons in the last fifth of a
percent, and the visual pathway sits in between in its correct anatomical
order: optic lobe, then the projection neurons that leave it, then the central
brain. None of this was given to the search. It is the strongest evidence that
the ordering means something beyond its score — a list tuned only to maximise
a sum of edge weights reconstructs the direction information actually travels.

This is a validation, not a discovery: the anatomy is well known, and the
ordering recovers it *because* that is how the neurons are wired. The point is
that the recovery is independent.

## 2. The forward pathway is nearly pure; the feedback is on the return path

15.18% of all synaptic weight (6,360,572) points backward in our ordering.
That weight is not spread evenly. Comparing each pathway's backward weight
with its total weight:

| pathway | total weight | % pointing backward |
|---|---|---|
| optic → visual_projection | 3,649,881 | **2.4%** |
| visual_projection → central | 1,512,229 | **4.7%** |
| central → central | 12,523,698 | 16.3% |
| optic → optic | 17,120,781 | 17.9% |
| central → sensory | 157,439 | 27.3% |
| optic → visual_centrifugal | 120,639 | 37.6% |
| descending → central | 252,549 | 40.0% |
| central → visual_centrifugal | 215,069 | 43.9% |
| central → visual_projection | 273,590 | **46.9%** |
| visual_projection → optic | 582,607 | **53.1%** |
| visual_projection → visual_centrifugal | 93,345 | **75.0%** |

The brain's outbound route — optic lobe to projection neurons to central brain
— is almost perfectly feedforward: only 2.4% and 4.7% of that weight has to be
turned around. Every route running the other way is between a third and
three-quarters backward.

Taking both directions between each pair of classes sharpens this. Nine pairs
carry at least 20,000 weight each way, and in every one of them the clean
direction is between 0.8% and 8.7% backward while the return direction is
between 27.3% and 75.0% — the brain-wide 15.18% falls strictly between the
two every time:

| the clean direction | | the other way | |
|---|---|---|---|
| ascending → central | 0.8% | central → ascending | 54.2% |
| optic → visual_projection | 2.4% | visual_projection → optic | 53.1% |
| central → descending | 2.8% | descending → central | 40.0% |
| sensory → optic | 2.8% | optic → sensory | 35.2% |
| visual_projection → central | 4.7% | central → visual_projection | 46.9% |
| visual_centrifugal → optic | 5.0% | optic → visual_centrifugal | 37.6% |
| visual_centrifugal → visual_projection | 5.4% | visual_projection → visual_centrifugal | 75.0% |
| sensory → central | 6.7% | central → sensory | 27.3% |
| visual_centrifugal → central | 8.7% | central → visual_centrifugal | 43.9% |

Drawn as `figures/fig3_pathways.svg`.

Counted by sheer volume, 80% of all backward weight is local (optic → optic
48.1%, central → central 32.0%) — but those are also the two biggest blocks of
the connectome, and their backward *rates*, 17.9% and 16.3%, are close to the
brain-wide 15.18%. The structure is in the rates, not the totals.

By receiving class, the neurons most bathed in feedback are exactly the ones
anatomy calls feedback targets:

| receiving class | weight received | % of it backward |
|---|---|---|
| visual_centrifugal | 485,721 | **45.7%** |
| ascending | 129,985 | 31.3% |
| sensory | 234,730 | 26.8% |
| optic | 18,799,695 | 18.2% |
| central | 15,512,885 | 14.5% |
| visual_projection | 4,691,869 | 6.4% |
| descending | 1,810,599 | 3.1% |
| motor | 223,047 | **0.9%** |

Centrifugal neurons carry signals from the central brain back into the optic
lobe — that is what "centrifugal" means — and they are the single most
feedback-saturated class in the brain. Motor neurons, at the far end of the
line, receive almost nothing backward (0.9%): by the time the brain commits to
an action, the loops are behind it.

## 3. Feedback is disproportionately inhibitory

Splitting the same weight by the sending neuron's neurotransmitter:

| transmitter | share of all weight | % of its weight backward |
|---|---|---|
| acetylcholine (excitatory) | 56.8% | **10.6%** |
| dopamine (modulatory) | 1.3% | 17.6% |
| GABA (inhibitory) | 24.3% | **21.1%** |
| glutamate (inhibitory in flies, via GluCl) | 16.7% | **21.3%** |
| serotonin (modulatory) | 0.7% | **29.2%** |

Excitation runs forward. Inhibitory connections are **twice as likely** to run
backward as excitatory ones, and the modulators more likely still. Put
plainly: the fly brain mostly uses excitation to pass signals along and
inhibition to send them back — the signature of feedback loops that regulate,
normalise and gate rather than propagate.

## 4. The neurons carrying the most feedback are the ones named for it

Ranking individual neurons by backward weight, with no biological input, puts
these at the top:

| cell type | super class | position | backward weight |
|---|---|---|---|
| **APL** | central | 86.1% | 55,913 |
| **APL** | central | 85.8% | 48,211 |
| **CT1** | optic | 47.6% | 42,467 |
| **CT1** | optic | 50.2% | 41,217 |
| PVLP011 | central | 83.4% | 17,320 |
| Li32 | optic | 77.6% | 17,118 |

APL and CT1 — one of each per hemisphere, which is why each appears twice —
are the fly's two best-known giant inhibitory feedback neurons. APL spreads
through the whole mushroom body; Kenyon cells excite it and it inhibits them
back, and that loop is what keeps their odour code sparse. In this graph the
loop is direct and nearly complete: 5,176 of the 5,177 Kenyon cells synapse
onto an APL, and 5,166 get a synapse back (Liu & Davis, *Nat Neurosci* 2009;
Lin et al., *Nat Neurosci* 2014). CT1 reaches into two neuropils of the
optic lobe and visits each of their ~700 columns, covering the whole visual
field (Takemura et al., *eLife* 2017; Meier & Borst, *Curr Biol* 2019). Both
are GABAergic by immunostaining, both are textbook examples of wide-field
feedback. The optimisation found them without being told they exist.

Below them the list is local interneurons: Li30/32/33 (lobula intrinsic),
LPi12/13 (lobula plate intrinsic), Am1 (amacrine), Sm42 (serpentine medulla).
By feedback per neuron the densest classes are ME.LO (1,130 each) and the
antennal lobe local neurons ALLN (848 each) — the lateral inhibition that
implements gain control in the antennal lobe (Olsen & Wilson, *Nature*
2008).

## Does this hold in a second brain?

Partly, and we checked. The same pipeline run on Janelia's hemibrain — a
different fly, a different lab, a different reconstruction — reproduces the
axis, the feedforward purity of the inward route, the near-absence of feedback
onto output neurons, and APL at the top of the feedback ranking; across 2,831
shared cell types the feedback each type carries correlates at Spearman
rho 0.615. The glutamate result in section 3 does **not** replicate.
See [HEMIBRAIN.md](HEMIBRAIN.md).

## Caveats

- **Our ordering is one of many near-optimal ones.** Neurons with similar
  scores can swap without changing the total much, so an individual neuron's
  position is soft. The class-level patterns above involve thousands of
  neurons each and are not sensitive to that.
- **"Backward" means backward in this ordering.** Some backward weight is
  unavoidable — our cycle-packing bound proves at least 9.76% of all weight
  must point backward in *any* ordering (see the README) — but which specific
  edges get turned around depends on the ordering chosen.
- These are correlations in one connectome, from one female fly. They say what
  the wiring looks like, not what any circuit computes.
