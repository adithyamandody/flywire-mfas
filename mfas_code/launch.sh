#!/bin/zsh
cd "$HOME/connectome"
B=7200
G=mfas_data/graph.csv
S=mfas_code/solve
# 4 fresh searches (random relaxation start)
for s in 1 2 3 4; do
  $S $G runs/fresh_$s.csv $B $s > runs/fresh_$s.log 2>&1 &
done
# 4 resuming from the best hybrid chain (35,155,515)
for s in 5 6 7 8; do
  $S $G runs/chain_$s.csv $B $s mfas_data/out_chain.csv > runs/chain_$s.log 2>&1 &
done
# 4 resuming from the sifted benchmark (34,943,313)
for s in 9 10 11 12; do
  $S $G runs/sift_$s.csv $B $s mfas_data/out_sift.csv > runs/sift_$s.log 2>&1 &
done
wait
