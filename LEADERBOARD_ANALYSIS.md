# What the leaderboard files tell us (15 Nov 2026)

None of the top five (Westfall, de Polavieja, Denoual, Hassan, Burton) has
published code, a paper or a method write-up. The challenge page does post
each entry's submission file, so we downloaded #1 and #2 and compared them
with ours. The files themselves are not in this repo (`lb_data/` is
gitignored): they are other people's work.

**Policy: we use posted files for analysis only.** We do not start our search
from them or submit anything derived from them.

Tools: `mfas_code/compare.py` (how different are two orders, and in what
shape) and `mfas_code/compare2.py` (where the differences sit).

## 1. Both files score exactly as claimed

Our independent scorer gives 35,544,233 (#1) and 35,543,204 (#2).

## 2. #1 and #2 are almost the same ordering

| | #2 vs #1 | ours vs #1 |
|---|---|---|
| synapse weight that points the other way | 0.16% | 4.18% |
| rank correlation (Spearman) | 0.997 | 0.836 |
| median distance a neuron sits from its #1 position | 152 | 8,262 |

Two independent methods almost never land this close, so #1 and #2 very
likely share an ancestor: one built on the other, or both on an earlier
posted file. Our ordering is in a different region of the search space.

## 3. The leader's file is not polished the way ours is

- Single-neuron sifting finds **zero** improving moves on it, the same as on
  ours.
- Our tabu ILS, started from it, finds +111 to +172 in 3 minutes per core.
  On our own file the same search now finds roughly +15 per core per 4
  minutes. Their solution came from a different kind of search, and their
  lead comes from a better global layout, not from finer local polish.
  (Those polished copies were deleted and never used.)

## 4. The difference is global, spread out, and centred on hub neurons

- Only 4,355 neurons (3%) are already in #1's relative order. The rest are
  re-interleaved **one at a time**: 95.5% of the moved neurons move alone.
  The diff is not made of blocks, which matches our earlier negative result
  on block moves.
- The broad flow agrees (our first 5% sits in their first ~5%, our last 5%
  in their last ~5%). But big slices of our middle are placed much earlier by
  them: for our 60–65% band, the 10th percentile of their position is 16.8%.
- Half of the flipped synapse weight touches just 5,054 neurons (3.7%). These
  are hubs: median degree 198 vs 52 for everyone else. The top 20 have 4,000
  to 8,000 connections each, and #1 puts some of them tens of percent away from
  where we put them (one sits at 99.6% in ours, 57.9% in theirs).

## What this means for us

The 97,584 gap is not something more polishing will close. Our pipeline
fixes the global layout early (relaxation, then local search), and every hour
after that refines the same basin. The leaders' basin differs mainly in where
the hub neurons sit and how everything else interleaves around them. A single
hub can't be moved on its own, because thousands of partners pin it in place.

Ideas this suggests:

1. **Hub moves** (tested 15 Nov, **works**): a hub plus its strongest
   wrong-side partners, moved together, gained up to +7,711 per core in 5
   minutes against +16 for the old search. A hub moved alone gained nothing.
   See the README.
2. **Better starting layouts**: several relaxation restarts that differ in
   how hubs are treated, judged after a short ILS, not by raw score.
3. **TopoShuffle** (from the Rocket-Crane paper): a random re-sort that keeps
   every forward edge forward, used as a kick that never lowers the score.
