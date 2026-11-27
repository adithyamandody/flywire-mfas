#!/bin/zsh
cd "$HOME/connectome"
END=$(( $(date +%s) + 27000 ))    # 7.5 hours from launch
TOTAL=41912141

best_of() {
  b=0; w=""
  for f in runs/*.log; do
    s=$(grep -o 'checkpoint [0-9]*' "$f" 2>/dev/null | tail -1 | awk '{print $2}')
    [ -n "$s" ] && [ "$s" -gt "$b" ] && { b=$s; w="runs/$(basename $f .log).csv"; }
  done
  echo "$b $w"
}

echo "[$(date)] supervisor up; waiting for the running batch"
while pgrep -f 'mfas_code/solve' > /dev/null 2>&1; do sleep 60; done

round=1
while :; do
  remain=$(( END - $(date +%s) ))
  [ $remain -lt 600 ] && break
  per=$remain; [ $per -gt 4800 ] && per=4800
  set -- $(best_of); gb=$1; gf=$2
  echo "[$(date)] round $round  from=$gf ($gb)  ${per}s"
  # half intensify from the global best, half explore from random
  for i in 1 2 3 4 5 6; do
    mfas_code/solve mfas_data/graph.csv runs/r${round}i${i}.csv $per $((round*137+i)) "$gf" > runs/r${round}i${i}.log 2>&1 &
  done
  for i in 7 8 9 10 11 12; do
    mfas_code/solve mfas_data/graph.csv runs/r${round}f${i}.csv $per $((round*137+i)) > runs/r${round}f${i}.log 2>&1 &
  done
  wait
  round=$(( round+1 ))
done

set -- $(best_of); gb=$1; gf=$2
cp "$gf" best_solution.csv; echo "$gb" > best_score.txt
pct=$(echo "scale=3; 100*$gb/$TOTAL" | bc)
sed -i '' -E "s#^\| \*\*This repo, best so far\*\*.*#| **This repo, best so far** | **$gb** | **$pct%** |#" README.md
git add -A
git -c user.name="adithyamandody" -c user.email="madithyam16@gmail.com" \
    commit -q -m "Overnight search: $gb ($pct%)" && git push -q origin master
echo "[$(date)] DONE  best=$gb ($pct%)"
