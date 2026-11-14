#!/bin/zsh
# Island-model fast ILS until END (epoch secs). Every round restarts all
# islands from the global best; ILS never drops below its seed, so the best
# is always in the current round and older round files can be deleted.
cd "$HOME/connectome"
END=$1; NISL=${2:-12}; TOTAL=41912141
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
  remain=$(( END - $(date +%s) )); [ $remain -lt 300 ] && break
  per=$remain; [ $per -gt 1800 ] && per=1800
  set -- $(best_of); gb=$1; gf=$2
  cp "$gf" runs/island_seed.csv; cp "$gf" best_solution.csv; echo "$gb" > best_score.txt
  log "round $round  seed=$gb ($gf)  ${per}s x $NISL"
  for i in $(seq 1 $NISL); do
    mfas_code/ils mfas_data/graph.csv runs/isl_r${round}_$i.csv $per $((round*1000+i)) runs/island_seed.csv > runs/isl_r${round}_$i.log 2>&1 &
  done
  wait
  prev=$((round-1)); [ $prev -ge 1 ] && rm -f runs/isl_r${prev}_*.csv
  round=$((round+1))
done
set -- $(best_of); gb=$1; gf=$2
cp "$gf" best_solution.csv; echo "$gb" > best_score.txt
pct=$(echo "scale=3; 100*$gb/$TOTAL" | bc)
sed -i '' -E "s#^\| \*\*This repo, best so far\*\*.*#| **This repo, best so far** | **$gb** | **$pct%** |#" README.md
git add -A
git -c user.name="adithyamandody" -c user.email="madithyam16@gmail.com" \
  commit -q -m "Overnight island search: $gb ($pct%)

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>" && git push -q origin master
log "DONE best=$gb ($pct%)"
