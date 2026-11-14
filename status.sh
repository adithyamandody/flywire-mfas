#!/bin/zsh
cd "$HOME/connectome"
TOP=35544233; TOTAL=41912141
best=0; who=""
for f in runs/*.log; do
  s=$(grep -o 'checkpoint [0-9]*' "$f" 2>/dev/null | tail -1 | awk '{print $2}')
  [ -n "$s" ] && [ "$s" -gt "$best" ] && { best=$s; who=$(basename "$f" .log); }
done
alive=$(pgrep -f 'mfas_code/solve' | wc -l | tr -d ' ')
echo "solvers running : $alive"
echo "best score      : $best   ($(echo "scale=3; 100*$best/$TOTAL" | bc)%)   [$who]"
echo "leaderboard top : $TOP   (84.808%)"
if [ "$best" -gt "$TOP" ]; then
  echo ">>> AHEAD OF THE LEADERBOARD by $((best-TOP)) <<<"
else
  echo "gap to beat     : $((TOP-best))"
fi
echo "best file       : $HOME/connectome/runs/$who.csv"
