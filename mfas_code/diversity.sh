#!/bin/zsh
# Does our search have more than one basin?
#
# Six arms, identical search settings, different starting layouts and seeds,
# NEVER merged. Five start from SpringRank layouts that differ in alpha and
# in how far the solver is run; the sixth starts from our current best and is
# the control -- it says how far the basin we are already in can still move.
#
# usage: diversity.sh [SECONDS_PER_ARM]
cd "$HOME/connectome" || exit 1
D=runs/div
mkdir -p $D
SECS=${1:-1800}
log() { echo "[$(date '+%H:%M:%S')] $*" }

log "building six starting layouts"
mfas_code/springrank mfas_data/graph.csv $D/s1.csv 0.01  3000  > $D/s1.init.log 2>&1
mfas_code/springrank mfas_data/graph.csv $D/s2.csv 0.001 3000  > $D/s2.init.log 2>&1
mfas_code/springrank mfas_data/graph.csv $D/s3.csv 0.1   3000  > $D/s3.init.log 2>&1
mfas_code/springrank mfas_data/graph.csv $D/s4.csv 0.01  200   > $D/s4.init.log 2>&1
mfas_code/springrank mfas_data/graph.csv $D/s5.csv 0.01  20000 > $D/s5.init.log 2>&1
cp best_solution.csv $D/s6.csv
for i in 1 2 3 4 5 6; do
  s=$(python3 -I mfas_code/score.py mfas_data/graph.csv $D/s$i.csv | awk '/^FORWARD/{print $NF}')
  log "  start $i: $s"
done

log "running six arms for ${SECS}s each, in parallel, no merging"
for i in 1 2 3 4 5 6; do
  nice -n 10 mfas_code/ilsg mfas_data/graph.csv $D/out$i.csv $SECS $((9100 + i * 37)) $D/s$i.csv \
    0 8 1 0 0 0 \
    0 2 64 0.04 1 \
    0.3 0 -1 1 \
    0.3 32 1 0.3 \
    0 12 1 0.3 \
    0 2000 1 \
    0 0 > $D/run$i.log 2>&1 &
done
wait
log "arms finished"

log "independently scoring every arm"
for i in 1 2 3 4 5 6; do
  s=$(python3 -I mfas_code/score.py mfas_data/graph.csv $D/out$i.csv | awk '/^FORWARD/{print $NF}')
  log "  arm $i: $s"
done

log "pairwise comparison"
python3 -I mfas_code/divcmp.py mfas_data/graph.csv \
  sr-a.01=$D/out1.csv sr-a.001=$D/out2.csv sr-a.1=$D/out3.csv \
  sr-i200=$D/out4.csv sr-i20k=$D/out5.csv ours=$D/out6.csv \
  > $D/RESULT.txt 2>&1
cat $D/RESULT.txt
log "done; summary in $D/RESULT.txt"
