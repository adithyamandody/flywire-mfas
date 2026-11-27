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
3. **TopoShuffle** (from the Rocket-Crane paper; tested 15 Nov, **works**):
   a random re-sort that keeps every forward edge forward, used as a kick
   that never lowers the score. After hub moves had stalled (+13 per core in
   10 minutes), whole-order shuffles gained +5,953. See the README.

---

# The 26 Nov entries (checked 27 Nov 2026)

Both top entries were posted on 26 November: ours at 35,551,497 (first) and
Jean-René Denoual's at 35,544,474 (second). Denoual's file was downloaded
and scored independently: it is exactly 35,544,474, as claimed. Still no
code, paper or method write-up from anyone in the top five.

## 1. The new #2 is the old #1 with 241 added

Denoual's 26 November file against Westfall's 8 November file:

| | weight |
|---|---|
| forward in both | 35,521,224 |
| forward only in Denoual | 23,250 |
| forward only in Westfall | 23,009 |
| net | **+241** |

They disagree on 0.06% of the graph. The new second place is Westfall's
ordering polished by 241 weight, which settles the question left open on
15 November: the top of the leaderboard is one lineage, not several methods
converging. There is no new method in this file to learn from.

## 2. Ours against theirs is an even split that we win narrowly

| | weight |
|---|---|
| forward in both | 34,733,434 |
| forward only in ours | 818,135 |
| forward only in theirs | 811,040 |
| backward in both | 5,549,532 |
| net | **+7,095** |

1.63M of weight is contested and we take 50.2% of it. **They capture
811,040 that we do not** — 1.9% of the whole graph. The same neurons head
both win lists (Li30, Li32, APL, Am1, Sm42, LT33, LHCENT3), so the two
searches are not disagreeing about the brain in general; they are making
opposite choices about where to put the same few thousand optic-lobe
interneurons, and each choice pays off somewhere.

## 3. Our own search has stopped exploring

All ten islands from the 19 November overnight run score 35,551,569, and
`split.py` finds **zero** disagreeing weight between any of them and
`best_solution.csv`. They are the same ordering. We have no second basin of
our own to compare against.

## What this is worth, and what we will not do

The file is not usable and would not help if it were: it is Westfall + 241.
The policy is unchanged — posted files are read for analysis and never used
as a starting point, and nothing derived from one is submitted.

The diagnostic is the real result, and it is a pointed one. An unrelated
search holds 811k of weight that ours misses, while ours has collapsed to a
single point and now gains about 15 per core per 4 minutes. Taken together
those say the remaining headroom is in **exploration, not polish**, and that
our island scheme defeats itself: islands that merge back together converge
to one answer and lose the diversity that made them worth running.

The experiment that follows, entirely on our own data: several runs from
genuinely different starting layouts — different SpringRank seeds, different
relaxation parameters, TopoShuffle-randomised starts — kept strictly apart
with no merging, each polished independently, and compared with `split.py`.
If two of our own runs disagree on hundreds of thousands of weight the way
ours and theirs do, the basin is not unique and a better one may be
reachable. If they all collapse to 35,551,569 again, the layout our pipeline
finds is forced, and the gap to the 90.24% ceiling is not something this
search will close.
