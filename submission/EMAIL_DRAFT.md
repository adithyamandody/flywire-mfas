# Re-submission email — draft, not sent

**To:** arie@princeton.edu
**Subject:** MFAS challenge — improved solution, 35,551,576 (Adithya Mandody)
**Attach:** `submission/mfas_submission_adithya_35551576.csv.gz` (971 KB)

---

Hi Arie,

Thank you for verifying and posting my earlier entry. I have an improved
solution and wondered if you would be willing to update it.

**Score:** 35,551,576 forward weight (84.824% of 41,912,141), up from the
35,551,497 currently listed. Attached as a gzipped CSV in the same Node ID,
Order format.

**How the improvement was achieved.** A SpringRank initialisation followed
by iterated local search over a sifting neighbourhood. Three things did most
of the work beyond a standard ILS:

- *Incremental repair.* After a kick, only the neurons whose best position
  could have changed are re-sifted — the kicked ones, then transitively the
  neighbours of anything that moves. That is roughly 800x the iteration
  throughput of full re-sweeps and reaches the same local optimum.
- *Tabu after kick.* Freezing the kicked neurons while the repair cascade
  runs, then releasing them. Without it, repair simply moves each kicked
  neuron back to where it started.
- *Hub moves.* Relocating a high-degree neuron together with its strongest
  wrong-side partners. Hubs cannot move alone, because thousands of partners
  pin them in place.

Parallel islands are merged by transplanting each island's moved neurons
into the best one and keeping a transplant only if the total rises, so the
merge never does worse than its best input. I also used the TopoShuffle kick
from Bader et al.'s Rocket-Crane paper, which never lowers the score.

**An upper bound, which may be of more interest than the score.** Packing
edge-disjoint directed cycles gives a certificate that at least 5,291,688
units of weight must point backward in *any* ordering, so no ordering can
exceed 36,620,453 — 87.374%. The packing is 1,513,396 cycles and is
machine-checkable: a separate program re-reads the graph, confirms every
line is a genuine directed cycle and that no connection is used beyond its
weight. That puts the current leaderboard within 2.55 points of optimal, and
bounds how much is left for everyone.

Happy for my name to continue appearing on the leaderboard as before.

All the code, both orderings and the bound verifier are public at
https://github.com/adithyamandody/flywire-mfas — the scorer and the
certificate checker run on the published data in a few minutes, if it is
useful for verification.

Best,
Adithya Mandody

---

## Optional closing line, once the preprints are posted

> I have also written the work up: [paper 1 title + DOI] on the ordering and
> what it recovers about the brain's organisation, and [paper 2 title + DOI]
> on using the bound to measure irreducible recurrence across four
> connectomes. Both cite the challenge as the source of the instance.

## Notes

- **Do not send until you are happy with it.** I have not sent anything.
- The attachment is verified: it decompresses to 136,648 rows and re-scores
  at exactly 35,551,576.
- If you would rather wait and send once per improvement, the papers' claims
  do not depend on the leaderboard being current — but the board showing a
  different number from the papers is the thing worth closing.
