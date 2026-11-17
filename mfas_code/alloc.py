"""Re-allocate regional islands by where gains were found. Pure stdlib.

usage: python3 -I alloc.py CFG LOGPREFIX HEATFILE NREG
  CFG        island config used last round (fields 18/19 = REGLO REGHI; 0 0 = global)
  LOGPREFIX  e.g. runs/v9_r7_  (island i's log is LOGPREFIX<i>.log)
Keeps an exponential moving average of gain per unit of order length in 200
bins, then cuts the order into NREG regions of equal heat (hot stretches get
narrow regions, i.e. more islands per neuron). Global islands are kept.
Writes the new config to CFG (atomically)."""
import json, os, sys
cfg, prefix, heatf, nreg = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
NB, FLOOR, MINW = 200, 0.02, 0.004
lines = [l.split() for l in open(cfg) if l.strip() and not l.startswith("#")]
try: heat = json.load(open(heatf))
except Exception: heat = [1.0] * NB
obs, cnt = [0.0] * NB, [0] * NB
for i, f in enumerate(lines, 1):
    lo, hi = float(f[17]), float(f[18])
    if hi <= lo: continue
    try:
        L = open(f"{prefix}{i}.log").read().split("\n")
        st = int(L[0].split()[1]); dn = int([x for x in L if x.startswith("DONE")][0].split()[1])
    except Exception: continue
    dens = (dn - st) / (hi - lo)              # gain per unit of order length
    for b in range(NB):
        c = (b + 0.5) / NB
        if lo <= c < hi: obs[b] += dens; cnt[b] += 1
mean_obs = [obs[b] / cnt[b] for b in range(NB) if cnt[b]]
avg = sum(mean_obs) / len(mean_obs) if mean_obs else 1.0
for b in range(NB):
    if cnt[b]: heat[b] = 0.5 * heat[b] + 0.5 * obs[b] / cnt[b]
top = max(heat) or 1.0
h = [max(x, FLOOR * top) for x in heat]
H = sum(h); cuts = [0.0]; acc = 0.0; k = 1
for b in range(NB):
    a0 = acc; acc += h[b]
    while k < nreg and acc >= k * H / nreg:
        frac = (k * H / nreg - a0) / h[b]
        x = (b + frac) / NB
        if x - cuts[-1] >= MINW: cuts.append(x)
        k += 1
cuts.append(1.0)
tmpl = next((f for f in lines if float(f[18]) > float(f[17])), lines[0])
out = ["# GROUP HUBP TOPOP TOPOL TOPOJ TOPOACC GP GK GMODE GHUB SP SK SMODE SHUB IMP IML IMMODE REGLO REGHI"]
for a, b in zip(cuts, cuts[1:]):
    out.append(" ".join([f"r{100*a:.1f}-{100*b:.1f}"] + tmpl[1:17] + [f"{a:.5f}", f"{b:.5f}"]))
out += [" ".join(f) for f in lines if float(f[18]) <= float(f[17])]
json.dump(heat, open(heatf, "w"))
open(cfg + ".tmp", "w").write("\n".join(out) + "\n"); os.replace(cfg + ".tmp", cfg)
print("regions:", " ".join(f"{100*a:.1f}" for a in cuts[1:-1]))
