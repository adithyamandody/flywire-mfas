#!/bin/zsh
cd "$HOME/connectome"
G=mfas_data/graph.csv
run() {  # name steps loss tau1 rerank
  mfas_code/relax2 $G runs/rx_$1.csv $2 11 $3 2.0 $4 0.02 0.0008 $5 > runs/rx_$1.log 2>&1
  mfas_code/mfas $G runs/rx_$1.csv runs/rx_$1_sift.csv 300 3 >> runs/rx_$1.log 2>&1
}
run A_hinge_6k    6000 0 0.0015 0 &
run B_atan_6k     6000 1 0.0015 0 &
run C_logit_6k    6000 2 0.0015 0 &
run D_atan_30k   30000 1 0.0005 0 &
run E_atan_30k_rr  30000 1 0.0005 1000 &
run F_logit_30k_rr 30000 2 0.0005 1000 &
wait
for f in runs/rx_*.log; do
  raw=$(grep '^RAW' $f | awk '{print $2}')
  fin=$(grep '^wrote' $f | grep -o 'final=[0-9]*' | cut -d= -f2)
  echo "$(basename $f .log)  raw=$raw  sifted=$fin"
done
