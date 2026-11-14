#!/bin/zsh
# Keeps best_solution.csv and the logs; removes the per-run order files.
cd "$HOME/connectome"
before=$(du -sh runs 2>/dev/null | awk '{print $1}')
best=$(cat best_score.txt 2>/dev/null)
[ -s best_solution.csv ] || { echo "no best_solution.csv - not deleting anything"; exit 1; }
rm -f runs/*.csv
after=$(du -sh runs 2>/dev/null | awk '{print $1}')
echo "runs/ $before -> $after   (best_solution.csv kept, score $best)"
