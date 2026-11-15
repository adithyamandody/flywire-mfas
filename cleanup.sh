#!/bin/zsh
# Keeps best_solution.csv and the logs; removes the per-run order files.
cd "$HOME/connectome"
if pgrep -f 'mfas_code/(overnight|ils|ilsk2|merge|solve)' >/dev/null; then
  echo "search still running - not deleting anything. Try again after it finishes (11:20)."; exit 1
fi
before=$(du -sh runs 2>/dev/null | awk '{print $1}')
best=$(cat best_score.txt 2>/dev/null)
[ -s best_solution.csv ] || { echo "no best_solution.csv - not deleting anything"; exit 1; }
rm -f runs/*.csv
after=$(du -sh runs 2>/dev/null | awk '{print $1}')
echo "runs/ $before -> $after   (best_solution.csv kept, score $best)"
