#!/bin/zsh
# Head-to-head: hub moves alone vs hub moves + TopoShuffle kicks.
# Waits for the hub-move search (overnight5.sh) to finish, then runs
# 6 variants x 2 seeds = 12 runs of SECS seconds each, at low priority.
# usage: zsh topo_test.sh [SECS]
SECS=${1:-600}
HERE=${0:A:h}; REPO=${REPO:-~/connectome}; OUT=${OUT:-$REPO/runs/h2h}
mkdir -p $OUT
caffeinate -i -w $$ &!
[ -n "$NOWAIT" ] || while pgrep -f 'overnight5.sh' >/dev/null; do sleep 20; done
cp $REPO/best_solution.csv $OUT/start.csv
echo "start $(date +%T) from $(cat $REPO/best_score.txt)" > $OUT/summary.txt
# name : TOPOP TOPOL TOPOJ TOPOACC   (all: hub moves HUBP .5, HK 64, best-alt targets)
V=( "A_control:0 2000 2000 0"
    "B_w2k_strict:0.25 2000 2000 0"
    "C_w20k_strict:0.25 20000 20000 0"
    "D_w2k_plateau:0.25 2000 2000 1"
    "E_whole_plateau:0.02 0 -1 1"
    "F_w20k_mild:0.25 20000 200 0" )
for v in $V; do name=${v%%:*}; args=${v#*:}
  for s in 1 2; do
    nice -n ${NICE:-10} $HERE/ilstopo $REPO/mfas_data/graph.csv $OUT/${name}_$s.csv $SECS $((700+s)) $OUT/start.csv \
      0 8 1 0 0 1 0.5 2 64 0.04 1 ${=args} > $OUT/${name}_$s.log 2>&1 &
  done
done
wait
for f in $OUT/*_[12].log; do
  st=$(awk '/^start /{print $2}' $f); dn=$(awk '/^DONE /{print $2}' $f)
  printf "%-20s gain %+7d   %s\n" ${${f:t}%.log} $((dn-st)) "$(grep -E '^(topo kicks|hub kicks total)' $f | sed -E 's/ time .*->/ ->/' | tr '\n' ' ')" >> $OUT/summary.txt
  grep -H '!!' $f >> $OUT/summary.txt
done
echo "end $(date +%T)" >> $OUT/summary.txt
