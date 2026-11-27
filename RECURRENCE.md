# Irreducible recurrence across four nervous systems

*Working synthesis, 27 November 2026.*

## The quantity

For a connectome, order the neurons however you like; some synaptic weight
will still point backward. The minimum of that over **all** orderings is a
property of the wiring, not of any algorithm — the share of a nervous
system's connectivity that no feedforward hierarchy can explain. Call it
irreducible recurrence.

It is bounded on both sides, and both sides are certificates:

* **Floor** — a cycle packing. Assign each directed cycle a value so that
  the cycles through any connection never exceed its weight; every cycle
  must contain a backward edge, so the packed total is forced backward in
  any ordering. Re-verified by re-reading the graph and every packed cycle.
* **Ceiling** — any ordering we can exhibit. If an ordering achieves a
  backward rate, the minimum is no larger. Re-verified by an independent
  scorer.

No theory is needed to make the measurement, and nothing in it depends on
the search being good — a weak search only widens the interval.

## Four measurements

All at the same ≥2-synapse threshold used by the FlyWire challenge graph.

| nervous system | cells | floor | ceiling | width |
|---|---|---|---|---|
| *C. elegans*, whole | 297 | **3.81%** | **3.81%** | **0.00** |
| *Drosophila* larva, whole brain | 2,923 | 11.79% | 13.11% | 1.31 |
| *Drosophila* adult, central brain | 21,737 | 18.96% | 20.87% | 1.91 |
| *Drosophila* adult, whole brain | 136,648 | 9.76% | 15.18% | 5.42 |

**The worm is solved, not bounded.** Its packing forces exactly 262 units
backward and our ordering achieves exactly 262. The interval has zero width:
irreducible recurrence in the *C. elegans* chemical connectome is 3.81%, and
that is a theorem about the animal rather than an estimate.

## The pattern

Across the three whole-or-central volumes that exclude a large feedforward
sensory periphery, irreducible recurrence rises monotonically with size and
**the intervals do not overlap**:

```
    297 cells   C. elegans            3.81%
  2,923 cells   larval brain         11.79 – 13.11%
 21,737 cells   adult central brain  18.96 – 20.87%
```

A 73-fold range in cell count, two phyla, three independent reconstructions
by three laboratories, and a fivefold increase in the fraction of wiring
that no hierarchy can account for.

The adult **whole** brain sits apart at 9.76–15.18%, below the central brain
it contains. That is not a contradiction: the whole-brain figure includes
the optic lobes, which are large, highly feedforward, and dilute the rate.
We found the same thing when hemibrain's rates were compared with FlyWire's.

## The axis, in all four

Each ordering was computed from wiring alone, with no labels. Medians:

| | first | … | last |
|---|---|---|---|
| *C. elegans* | sensory 24.5% | interneuron 42.1%, motor 62.3% | muscle 82.5% |
| larva | sensory 8.5% | PN 27.1%, KC 38.6% | DN-VNC 94.4% |
| adult | sensory 7.9% | optic 47.0%, central 85.5% | motor 99.8% |

Input first, output last, every time, in a worm and in a fly at two life
stages. In the fly this is reproducible across independent basins too:
six different orderings place sensory at 7.8 ± 0.1% and motor at
99.8 ± 0.1%.

## What would undo this

* **n = 4, and three of them are *Drosophila*.** This is not a phylogenetic
  analysis and no trend should be fitted to it. Mammalian cortex (MICrONS)
  is the point that would matter.
* **Size and complexity are conflated**, and so are size and reconstruction
  completeness. A bigger volume is also a more recently traced one.
* **Threshold sensitivity is real.** At ≥1 synapse rather than ≥2, the worm
  reads 6.29–6.41% instead of 3.81%. Weak connections carry a
  disproportionate share of the irreducible recurrence, so every comparison
  must hold the threshold fixed — as this table does — and the absolute
  numbers are threshold-relative.
* **Gap junctions must be excluded.** They are electrically symmetric, so
  they appear as reciprocal pairs no ordering can resolve and manufacture
  recurrence out of connections that have no direction to be wrong about.
  The widely used catalogue copy of the *C. elegans* connectome merges them
  with chemical synapses and does not document what its edge directions
  mean; 29% of its reciprocal pairs carry exactly equal weights. It was
  discarded for this reason and the typed White et al. source used instead.
* **The FlyWire interval is wide**, and `BOUND.md` shows exact local solves
  will not narrow it — that needs a different relaxation.

## Why it is worth doing

Recurrent processing, global workspace and integrated information theories
disagree about almost everything except that feedforward sweeps are not
enough. None of them has had a way to ask how much of a given nervous
system's wiring is irreducibly recurrent, with a certificate, at whole-brain
scale. This is that measurement, and it is now four points long.

The honest claim is structural: this is a property of wiring. Consciousness
is the motivation for caring about the quantity, never a finding here — a
fly is not conscious in any settled sense, and a worm certainly is not.
