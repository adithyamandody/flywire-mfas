#!/bin/zsh
# TopoShuffle islands (ilstopo). Every round all islands start from the best
# order so far and run independently; the best island seeds the next round.
# Shuffling islands drift across the whole order, so they can't be merged.
# Islands 1-3: whole-order shuffles on 30% of iterations; 4-6: whole-order
# shuffles only; 7-9: 20,000-neuron windows on 50%; 10-12: 50,000-neuron
# windows on 50%. Hub moves stay on (HK 64, best-alt). Low priority (nice 10).
# usage: overnight6.sh END_EPOCH [ROUND_SECS]
cd "$HOME/connectome"
END=$1; PER=${2:-600}; TOTAL=41912141; NISL=12
CFG=("0.3 0 -1 1" "0.3 0 -1 1" "0.3 0 -1 1" "1 0 -1 1" "1 0 -1 1" "1 0 -1 1"
     "0.5 20000 20000 0" "0.5 20000 20000 0" "0.5 20000 20000 0"
     "0.5 50000 50000 0" "0.5 50000 50000 0" "0.5 50000 50000 0")
GRP=(whole30 whole30 whole30 whole100 whole100 whole100 w20k w20k w20k w50k w50k w50k)
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
    nice -n 10 mfas_code/ilstopo mfas_data/graph.csv runs/ts_r${round}_$i.csv $per $((round*1000+500+i)) runs/island_seed.csv \
      0 8 1 0 0 1 0.5 2 64 0.04 1 ${=CFG[$i]} > runs/ts_r${round}_$i.log 2>&1 &
  done
  wait
  typeset -A G; G=(); bi=0; bd=-1
  for i in $(seq 1 $NISL); do
    d=$(awk '/^DONE/{print $2}' runs/ts_r${round}_$i.log); [ -z "$d" ] && d=$gb
    d=$((d-gb)); g=${GRP[$i]}; [ -z "${G[$g]}" ] || [ $d -gt ${G[$g]} ] && G[$g]=$d
    [ $d -gt $bd ] && { bd=$d; bi=$i; }
  done
  log "round $round  seed=$gb  ->  $((gb+bd)) (+$bd, island $bi ${GRP[$bi]})   best per group: whole30 +${G[whole30]}  whole100 +${G[whole100]}  w20k +${G[w20k]}  w50k +${G[w50k]}"
  grep -h '!!' runs/ts_r${round}_*.log | head -3
  prev=$((round-1)); [ $prev -ge 1 ] && rm -f runs/ts_r${prev}_*.csv
  round=$((round+1))
done
set -- $(best_of); gb=$1; gf=$2
cp "$gf" best_solution.csv; echo "$gb" > best_score.txt
chk=$(python3 -I mfas_code/score.py mfas_data/graph.csv best_solution.csv | awk '/^FORWARD/{gsub(",","",$4); print $4}')
if [ "$chk" != "$gb" ]; then log "!! scorer says $chk, search says $gb: not committing"; exit 1; fi
pct=$(echo "scale=3; 100*$gb/$TOTAL" | bc)
python3 -I - "$gb" "$((round-1))" <<'PY'
import sys
gb, nr = int(sys.argv[1]), sys.argv[2]
p = "README.md"; s = open(p).read()
anchor = "| + TopoShuffle kicks, one core, 10 min | 35,505,143 | 84.71% |\n"
row = f"| + TopoShuffle island search ({nr} rounds) | {gb:,} | {100*gb/41912141:.2f}% |\n"
if anchor in s and row not in s: s = s.replace(anchor, anchor + row)
lines = [f"| **This repo, best so far** | **{gb}** | **{100*gb/41912141:.3f}%** |" if l.startswith("| **This repo, best so far**") else l for l in s.split("\n")]
open(p, "w").write("\n".join(lines))
PY
git add -A
git -c user.name="adithyamandody" -c user.email="madithyam16@gmail.com" \
  commit -q -m "TopoShuffle island search: $gb ($pct%)

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>" && git push -q origin master
log "DONE best=$gb ($pct%)"
