#!/bin/zsh
# Islands (ilsg: hub moves + TopoShuffle + group moves) from the best order so
# far, then tmerge folds every island into the best one (mode 3: keep what the
# base gets right unless an island broke it, add what an island fixed; never
# keeps a loss). Island settings come from runs/islands.cfg (one line per
# island: GROUPNAME TOPOP TOPOL TOPOJ TOPOACC GP GK GMODE GHUB), re-read every
# round so the mix can change while it runs. Commits every COMMIT_EVERY rounds.
# usage: overnight7.sh END_EPOCH [ROUND_SECS] [COMMIT_EVERY]
cd "$HOME/connectome"
END=$1; PER=${2:-300}; CEV=${3:-24}; TOTAL=41912141
log() { echo "[$(date '+%H:%M:%S')] $*"; }
best_of() {
  b=0; w=""
  for f in runs/*.log; do
    c="runs/$(basename $f .log).csv"; [ -s "$c" ] || continue
    s=$(grep -o 'checkpoint [0-9]*' "$f" 2>/dev/null | tail -1 | awk '{print $2}')
    [ -n "$s" ] && [ "$s" -gt "$b" ] && { b=$s; w=$c; }
  done; echo "$b $w"
}
save() {  # verify with the independent scorer, then commit and push
  local msg="$1"
  set -- $(best_of); local gb=$1 gf=$2
  cp "$gf" best_solution.csv; echo "$gb" > best_score.txt
  local chk=$(python3 -I mfas_code/score.py mfas_data/graph.csv best_solution.csv | awk '/^FORWARD/{gsub(",","",$4); print $4}')
  if [ "$chk" != "$gb" ]; then log "!! scorer says $chk, search says $gb: not committing"; return 1; fi
  local pct=$(echo "scale=3; 100*$gb/$TOTAL" | bc)
  sed -i '' -E "s#^\| \*\*This repo, best so far\*\*.*#| **This repo, best so far** | **$gb** | **$pct%** |#" README.md
  git add -A
  git -c user.name="adithyamandody" -c user.email="madithyam16@gmail.com" \
    commit -q -m "$msg: $gb ($pct%)

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>" && git push -q origin master && log "saved $gb ($pct%) and pushed"
}
round=1
while :; do
  remain=$(( END - $(date +%s) )); [ $remain -lt 120 ] && break
  per=$PER; [ $per -gt $remain ] && per=$remain
  set -- $(best_of); gb=$1; gf=$2
  cp "$gf" runs/island_seed.csv; cp "$gf" best_solution.csv; echo "$gb" > best_score.txt
  CFG=("${(@f)$(grep -v '^#' runs/islands.cfg | grep .)}"); n=${#CFG}
  for i in $(seq 1 $n); do
    a=(${=CFG[$i]})
    nice -n 10 mfas_code/ilsg mfas_data/graph.csv runs/ts_r${round}_$i.csv $per $((round*1000+700+i)) runs/island_seed.csv \
      0 8 1 0 0 1 0.5 2 64 0.04 1 ${a[2,-1]} > runs/ts_r${round}_$i.log 2>&1 &
  done
  wait
  typeset -A G; G=(); bi=0; bd=-1; L=()
  for i in $(seq 1 $n); do
    d=$(awk '/^DONE/{print $2}' runs/ts_r${round}_$i.log); [ -z "$d" ] && continue
    L+=("$d runs/ts_r${round}_$i.csv"); d=$((d-gb)); a=(${=CFG[$i]}); g=${a[1]}
    { [ -z "${G[$g]}" ] || [ $d -gt ${G[$g]} ]; } && G[$g]=$d
    [ $d -gt $bd ] && { bd=$d; bi=$i; }
  done
  L=($(printf '%s\n' $L | sort -rn | awk '{print $2}'))
  mfas_code/tmerge mfas_data/graph.csv runs/tm_r${round}.csv 3 1 2 runs/island_seed.csv $L > runs/tm_r${round}.log 2>&1
  m=$(awk '/^TMERGED/{print $2}' runs/tm_r${round}.log); [ -z "$m" ] && m=$((gb+bd))
  gs=""; for g in ${(ko)G}; do gs="$gs $g +${G[$g]}"; done
  log "round $round  seed=$gb  best island +$bd ($bi)  merged $m (+$((m-gb)))  |$gs"
  grep -h '!!' runs/ts_r${round}_*.log runs/tm_r${round}.log | head -3
  prev=$((round-1)); [ $prev -ge 1 ] && rm -f runs/ts_r${prev}_*.csv runs/tm_r${prev}.csv
  [ $((round % CEV)) -eq 0 ] && save "Overnight search, round $round"
  round=$((round+1))
done
save "Overnight search (merge + group moves), $((round-1)) rounds"
log "DONE"
