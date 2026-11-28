# Does any of it hold in a second brain?

> **Correction, 27 November 2026.** The transmitter table below is
> superseded; see `AUDIT.md` and `PAPER.md` 2.6.
>
> - **Glutamate does replicate.** Comparing central brain to central brain
>   rather than whole brain to central brain, FlyWire gives 0.97x against
>   hemibrain's 0.95x. The "does not hold" verdict was an artefact of
>   comparing different volumes.
> - **The serotonin and dopamine rows are not evidence.** Both are dominated
>   by neurons the FlyWire annotations themselves measure as cholinergic, and
>   because hemibrain's labels are transferred from FlyWire by cell type, the
>   agreement between the two columns reproduces the error rather than
>   testing it.
> - The FlyWire floor quoted anywhere as 9.76% is superseded by a verified
>   12.63%.

[BIOLOGY.md](BIOLOGY.md) reports what our FlyWire ordering says about the fly
brain. Every one of those claims comes from a single ordering of a single
connectome. The obvious question is whether a second fly, imaged by a
different lab, reconstructed by a different pipeline, gives the same answers.

So we ran the whole thing again on the **hemibrain**: Janelia's
dense reconstruction of the central brain of a different female *Drosophila*
(Scheffer et al. 2020, v1.2 traced adjacency export, CC-BY 4.0). Nothing is
shared with FlyWire except the species.

## The second ordering

| | FlyWire | hemibrain |
|---|---|---|
| neurons | 136,648 | 21,737 |
| connections | 5,657,719 | 1,764,123 |
| total synaptic weight | 41,912,141 | 12,542,949 |
| our forward weight | 35,551,576 | 9,925,483 |
| **forward share** | **84.82%** | **79.13%** |

Built with `build_hemibrain.py`, keeping connections of at least 2 synapses
between traced, non-cropped neurons — 2 being the smallest weight that appears
in the FlyWire challenge graph, so the two graphs are filtered alike. The
ordering came from the same solver (`springrank` to start, then `ilsg`), with
no biological input of any kind. Three independent runs landed on 79.13%,
79.13% and 79.11%, and merging them gained nothing, so the search is
converged, not merely stopped.

Labels — super class and neurotransmitter — are carried over from FlyWire by
cell type, through the `hemibrain_type` column of the FlyWire annotations.
**The labels are borrowed; the ordering is not.** Cell type is conserved
between individuals, so a type that is GABAergic in one fly is GABAergic in
the other; where each neuron ends up in the order is decided by hemibrain's
own wiring alone.

```
python3 -I mfas_code/build_hemibrain.py \
    data_hemibrain/exported-traced-adjacencies-v1.2/traced-neurons.csv \
    data_hemibrain/exported-traced-adjacencies-v1.2/traced-total-connections.csv \
    mfas_data/hemibrain_graph.csv 2
python3 -I mfas_code/hemibrain_bio.py mfas_data/hemibrain_graph.csv \
    hemibrain_solution.csv \
    data_hemibrain/exported-traced-adjacencies-v1.2/traced-neurons.csv \
    fw_annot/neuron_annotations.tsv
python3 -I mfas_code/crosscheck.py mfas_data/graph.csv best_solution.csv \
    fw_annot/neuron_annotations.tsv mfas_data/hemibrain_graph.csv \
    hemibrain_solution.csv \
    data_hemibrain/exported-traced-adjacencies-v1.2/traced-neurons.csv
```

## 1. The axis: recovered again, in the shape of the volume

| super class | neurons | median position |
|---|---|---|
| visual_projection | 3,580 | 20.2% |
| central | 11,631 | 57.2% |
| visual_centrifugal | 125 | 91.4% |
| descending | 249 | **97.2%** |

Descending neurons — the brain's output to the nerve cord — land at 97.2%,
against 99.0% in FlyWire. Visual projection neurons, which are what carries
visual information *into* this volume, land early. The axis is there again.

Two differences are the volume's doing, not the ordering's:

- **The sensory end is missing.** Hemibrain's traced, non-cropped set contains
  exactly 2 olfactory receptor neurons; the rest are severed at the edge of
  the imaged block and excluded. There is no sensory class to place.
- **Centrifugal neurons flip from early (16.9%) to late (91.4%).** They carry
  signals from the central brain back into the optic lobe — and in hemibrain
  the optic lobe is outside the volume, so their targets are simply absent.
  With nothing left to send to, they look like output neurons. In FlyWire,
  where their targets are present and sit early, they sit early too. Both
  orderings put them immediately upstream of their targets; the volumes
  disagree about where the targets are.

## 2. Feedforward purity: holds, almost number for number

Brain-wide, 20.87% of hemibrain's weight points backward (FlyWire: 15.18%).
The second column of each pair below is the rate relative to that brain-wide
figure, which is what makes two connectomes comparable:

| | FlyWire | | hemibrain | |
|---|---|---|---|---|
| | rate | vs brain | rate | vs brain |
| visual_projection → central (the way in) | 4.7% | 0.31x | 6.2% | **0.30x** |
| central → visual_projection (the way back) | 46.9% | 3.09x | 48.0% | 2.30x |
| weight *received* by descending neurons | 3.1% | 0.20x | 2.8% | **0.13x** |

The route carrying information inward is 0.31x the brain-wide backward rate in
FlyWire and 0.30x in hemibrain. The return route is three to twenty times
dirtier in both. Output neurons receive almost no feedback in either brain.
This is the strongest of the FlyWire findings and it survives intact.

## 3. Inhibitory feedback: three transmitters of five

| transmitter | FlyWire (vs brain) | hemibrain (vs brain) | |
|---|---|---|---|
| acetylcholine (excitatory) | 0.70x | 0.83x | holds |
| GABA (inhibitory) | 1.39x | 1.20x | holds |
| serotonin (modulatory) | 1.92x | 1.79x | holds |
| dopamine (modulatory) | 1.16x | 1.49x | same direction, stronger |
| glutamate (inhibitory in flies) | 1.40x | **0.95x** | **does not hold** |
| octopamine | (below threshold) | 0.13x | — |

Acetylcholine is the most feedforward transmitter in both brains, serotonin
the most feedback-heavy in both, GABA above average in both. Glutamate,
inhibitory in flies through GluCl, was 1.40x in FlyWire and is 0.95x — dead
average — in hemibrain. So the headline from BIOLOGY.md, "inhibitory
connections are twice as likely to run backward", is **half confirmed**:
it is solid for GABA and for the modulators, and not supported for glutamate
once a second brain is asked.

## 4. The named neurons: APL again, first, and alone

Ranking every neuron by the weight it carries backward, in each brain
independently:

| cell type | FlyWire rank | hemibrain rank | what it is |
|---|---|---|---|
| **APL** | **1** (of 136,648) | **1** (of 21,737) | giant GABAergic feedback neuron of the mushroom body |
| PVLP011 | 5 | 3 | ventrolateral protocerebrum |
| AVLP080 | 14 | 5 | anterior ventrolateral protocerebrum |
| DPM | 143 | 2 | the other mushroom-body-wide feedback neuron |
| oviIN | 148 | 4 | oviposition inhibitory neuron |
| LHCENT3 | 85 | 20 | lateral horn centrifugal neuron |
| CT1 | 3 | 8,643 | giant optic-lobe feedback neuron |

APL is the single neuron that carries the most feedback in the fly brain, and
two orderings built from two different brains with no biological input both
put it first. In hemibrain it carries 105,167 units of backward weight — 2.6x
the next neuron and roughly a thousand times the median.

Every type in that table except CT1 is in the top 0.11% of both brains. CT1 is
the exception for a plain reason: it is a single enormous cell spanning every
column of the medulla and lobula, and hemibrain holds only a cropped fragment
of it, so there is almost nothing of it left to rank.

## 5. The test that is hardest to get right by accident

The four findings above are each a handful of numbers. The strongest check is
to compare **every cell type at once**. For each of the 2,831 cell types that
appear in both connectomes with at least 1,000 synaptic weight, take the share
of that type's weight the ordering has to turn around — one number per type
per brain — and correlate the two lists.

```
backward RATE  (share of the type's weight turned around) : 0.615
backward WEIGHT (absolute)                                : 0.598
total weight  -- the size control                         : 0.602
shuffled control (200 draws): largest |rho| seen 0.057, 95th percentile 0.039
restricted to the 243 types with >= 10,000 weight in both: 0.628
```

Spearman ρ = **0.615** across 2,831 cell types. Shuffling one side 200 times
never produced |ρ| above 0.057. The rate is already normalised by each type's
own weight, so size cannot produce it, and restricting to the 243 types whose
rate is measured from at least 10,000 weight in both brains raises it slightly
to 0.628.

The agreement is visible at both ends of the list:

| cell type | FlyWire | hemibrain |
|---|---|---|
| lLN1_a | 44.2% | 40.9% |
| APL | 43.3% | 46.4% |
| PVLP011 | 40.8% | 41.0% |
| AVLP077 | 39.6% | 38.6% |
| … | | |
| DNp13 | 0.3% | 0.3% |
| DNES2 | 0.0% | 0.0% |

Two solvers, two brains, no shared information, and they agree on which cell
types sit in loops.

## What is genuinely different: hemibrain is harder to order

79.13% forward against FlyWire's 84.82%. Part of that is what the volume
contains: FlyWire includes the optic lobe, 77,509 neurons of largely
feedforward visual processing, which is easy weight to point the right way.
Hemibrain is central brain — the recurrent part — with its sensory inputs cut
off, so almost everything in it is in a loop with something else.

## How much is left on the table

The same cycle-packing argument used on FlyWire (see the README) applies here:
assign each directed cycle a weight so that the cycles through any one
connection never exceed that connection's weight, and the sum is weight that
*must* point backward in any ordering whatsoever.

```
cycles packed: 264,098 of length 2, 289,939 of length 3, 115,539 longer
connections used beyond their weight: 0   (must be 0)
weight forced backward in any ordering  2,377,741
UPPER BOUND on forward weight          10,165,208  (81.043%)
```

Independently re-checked with `check_bound.py`, which re-reads the graph and
all 669,576 packed cycles, confirms every line is a real directed cycle and
that no connection is over-used, and arrives at the same figure.

| | our ordering | proved ceiling | gap |
|---|---|---|---|
| FlyWire | 84.824% | 87.374% | 2.55 points |
| hemibrain | **79.132%** | **81.043%** | **1.91 points** |

So hemibrain's lower feedforward share is not our search falling short. Its
ordering is within 1.91 points of anything that could ever exist, against 5.42
for FlyWire. The central brain, with its sensory input cut away, really is
that recurrent — there is almost nothing left to win there.

## Caveats

- **The labels are FlyWire's.** Class and transmitter are carried across by
  cell type. That is legitimate — cell type is conserved, and the transmitter
  predictions come from FlyWire's own images, not from any ordering — but it
  means the *labelling* is not independently replicated, only the *ordering*.
  15,710 of 21,737 hemibrain neurons (72%) could be matched this way.
- **Hemibrain is a cropped volume.** No sensory input, no optic lobe, one
  hemisphere. Several of the differences above are that, and are marked as
  such where we can tell.
- **One ordering of many near-optimal ones**, in both brains. Class- and
  type-level patterns involve thousands of neurons and are not sensitive to
  that; a single neuron's position is soft.
- Both connectomes are of female flies, and both are correlational: they say
  what the wiring looks like, not what any circuit computes.

Data: Scheffer et al. 2020, hemibrain v1.2 (Janelia, CC-BY 4.0); Schlegel et
al. 2024, FlyWire annotations (FAFB v783). `hemibrain_solution.csv` is our
ordering of the hemibrain graph.
