#!/bin/zsh
cd "$HOME/connectome"
for s in 21 22 23 24; do
  mfas_code/solve mfas_data/graph.csv runs/fresh_$s.csv 6600 $s > runs/fresh_$s.log 2>&1 &
done
wait
