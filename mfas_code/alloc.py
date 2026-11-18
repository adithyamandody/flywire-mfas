"""Re-allocate regional islands by where gains were found. Pure stdlib.

usage: python3 -I alloc.py CFG LOGPREFIX HEATFILE NREG
  CFG        island config used last round (fields 18/19 = REGLO REGHI; 0 0 = global)
  LOGPREFIX  e.g. runs/v9_r7_  (island i's log is LOGPREFIX<i>.log)
Keeps an exponential moving average of gain per unit of order length in 200
bins, then cuts the order into NREG regions of equal heat (hot stretches get
narrow regions, i.e. more islands per neuron). Global islands are kept.
Also re-tunes the move mix (TOPOP, GP; the rest is ordinary kicks) for all
islands: time goes to move types in proportion to their gain per second over
the last round (at least 5% each), smoothed with the previous mix.
Writes the new config to CFG (atomically)."""
import json, os, re, sys
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
        x = max(x, cuts[-1] + MINW)
        if x <= 1.0 - MINW: cuts.append(x)
        k += 1
cuts.append(1.0)
tmpl = next((f for f in lines if float(f[18]) > float(f[17])), lines[0])
out = ["# GROUP HUBP TOPOP TOPOL TOPOJ TOPOACC GP GK GMODE GHUB SP SK SMODE SHUB IMP IML IMMODE REGLO REGHI"]
for a, b in zip(cuts, cuts[1:]):
    out.append(" ".join([f"r{100*a:.1f}-{100*b:.1f}"] + tmpl[1:17] + [f"{a:.5f}", f"{b:.5f}"]))
out += [" ".join(f) for f in lines if float(f[18]) <= float(f[17])]
# ---- move mix: time share ~ gain/s, probability = share / cost per try
pat = {"n": r"^normal kicks: tries (\d+) wins \d+ gain (-?\d+) time (\d+)s",
       "t": r"^topo kicks: tries (\d+) wins .* gain (-?\d+) \(shuffle alone -?\d+\) time (\d+)s",
       "g": r"^group moves: tries (\d+) wins .* gain (-?\d+) time (\d+)s"}
agg = {k: [0, 0, 0] for k in pat}
for i in range(1, len(lines) + 1):
    try: txt = open(f"{prefix}{i}.log").read()
    except Exception: continue
    for k, rx in pat.items():
        m = re.search(rx, txt, re.M)
        if m:
            for j in range(3): agg[k][j] += int(m.group(j + 1))
mix = None
if all(agg[k][0] > 0 and agg[k][2] > 0 for k in agg):
    rate = {k: max(agg[k][1], 0) / agg[k][2] for k in agg}
    cost = {k: agg[k][2] / agg[k][0] for k in agg}
    tot = sum(rate.values()) or 1.0
    share = {k: 0.05 + 0.85 * rate[k] / tot for k in agg}
    pr = {k: share[k] / cost[k] for k in agg}; z = sum(pr.values())
    pr = {k: v / z for k, v in pr.items()}
    oldt, oldg = float(tmpl[2]), float(tmpl[6])
    newt = round(0.5 * oldt + 0.5 * pr["t"], 4); newg = round(0.5 * oldg + 0.5 * pr["g"], 4)
    mix = (newt, newg)
    out = [o if o.startswith("#") else " ".join(o.split()[:2] + [str(newt)] + o.split()[3:6] + [str(newg)] + o.split()[7:]) for o in out]
    print("rates/s: " + " ".join(f"{k}={rate[k]:.2f}" for k in "ntg") + f"  -> TOPOP {newt} GP {newg}")
json.dump(heat, open(heatf, "w"))
open(cfg + ".tmp", "w").write("\n".join(out) + "\n"); os.replace(cfg + ".tmp", cfg)
print("regions:", " ".join(f"{100*a:.1f}" for a in cuts[1:-1]))
