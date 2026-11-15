#!/bin/zsh
# Hub-move islands (ilsh), merged every round. Islands vary partners-moved
# (HK 64/128/200) and hub target (1-6 best-alt, 7-12 mixed with random window).
# Strict islands only move what they improve, so their diffs from the seed
# are small and can be transplanted into the best island. Merge never does
# worse than the best island (it starts there and keeps only gains).
cd "$HOME/connectome"
END=$1; NISL=${2:-12}; PER=${3:-300}; TOTAL=41912141; HKS=(64 128 200)
log() { echo "[$(date '+%H:%M:%S')] $*"; }
best_of() {
  b=0; w=""
  for f in runs/*.log; do
    c="runs/$(basename $f .log).csv"; [ -s "$c" ] || continue
    s=$(grep -o 'checkpoint [0-9]*' "$f" 2>/dev/null | tail -1 | awk '{print $2}')
    [ -n "$s" ] && [ "$s" -gt "$b" ] && { b=$s; w=$c; }
  done; echo "$b $w"
}
round=1
while :; do
  remain=$(( END - $(date +%s) )); [ $remain -lt 120 ] && break
  per=$PER; [ $per -gt $remain ] && per=$remain
  set -- $(best_of); gb=$1; gf=$2
  cp "$gf" runs/island_seed.csv; cp "$gf" best_solution.csv; echo "$gb" > best_score.txt
  for i in $(seq 1 $NISL); do
    hk=${HKS[$(( (i-1)%3+1 ))]}; tg=1; [ $i -gt 6 ] && tg=2
    mfas_code/ilsh mfas_data/graph.csv runs/sm_r${round}_$i.csv $per $((round*1000+i)) runs/island_seed.csv 0 8 1 0 0 1 0.5 2 $hk 0.04 $tg > runs/sm_r${round}_$i.log 2>&1 &
  done
  wait
  mfas_code/merge mfas_data/graph.csv runs/mg_r${round}.csv 20 runs/island_seed.csv runs/sm_r${round}_*.csv > runs/mg_r${round}.log 2>&1
  m=$(grep '^MERGED' runs/mg_r${round}.log)
  log "round $round  seed=$gb  ->  $m"
  prev=$((round-1)); [ $prev -ge 1 ] && rm -f runs/sm_r${prev}_*.csv runs/mg_r${prev}.csv
  round=$((round+1))
done
set -- $(best_of); gb=$1; gf=$2
cp "$gf" best_solution.csv; echo "$gb" > best_score.txt
pct=$(echo "scale=3; 100*$gb/$TOTAL" | bc)
sed -i '' -E "s#^\| \*\*This repo, best so far\*\*.*#| **This repo, best so far** | **$gb** | **$pct%** |#" README.md
git add -A
git -c user.name="adithyamandody" -c user.email="madithyam16@gmail.com" \
  commit -q -m "Hub-move island search: $gb ($pct%)

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>" && git push -q origin master
log "DONE best=$gb ($pct%)"
