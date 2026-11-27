"""Draw real against degree-matched null recurrence.

usage: python3 -I fig_null.py nulls.tsv out.svg

TSV columns: name, conns_per_cell, real, null_mean, null_lo, null_hi, proved
Ratios are computed here, so the figure cannot disagree with its inputs.

Pure stdlib, no plotting library.
"""
import csv, sys
src, out = sys.argv[1], sys.argv[2]
INK="#1b1b1b"; GREY="#8a8a8a"; LIGHT="#d8d8d8"; REAL="#2d6a9f"; NULL="#b5562c"
F='font-family="Helvetica,Arial,sans-serif"'
rows=[]
with open(src, newline="", encoding="utf-8") as f:
    r=csv.reader(f, delimiter="\t"); next(r)
    for name,dens,real,nm,lo,hi,proved in r:
        rows.append(dict(name=name,dens=float(dens),real=float(real),
                         nm=float(nm),lo=float(lo),hi=float(hi),
                         proved=proved.strip()=="yes"))
XMAX=26.0; L,R=210,700; TOP,RH=92,62
H=TOP+RH*len(rows)+66; W=835
x=lambda v: L+(R-L)*v/XMAX
s=[f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">',
   f'<rect width="{W}" height="{H}" fill="white"/>']
s.append(f'<text x="24" y="30" {F} font-size="15" font-weight="bold" fill="{INK}">'
         'Every nervous system is less recurrent than its own degree sequence allows</text>')
s.append(f'<text x="24" y="50" {F} font-size="11.5" fill="{GREY}">'
         'Share of synaptic weight that must point backward, real brain against '
         'degree-matched random graphs at equal solver effort.</text>')
s.append(f'<rect x="{L}" y="62" width="13" height="9" fill="{REAL}"/>')
s.append(f'<text x="{L+19}" y="70" {F} font-size="11" fill="{INK}">real connectome</text>')
s.append(f'<rect x="{L+122}" y="62" width="13" height="9" fill="{NULL}" opacity="0.45"/>')
s.append(f'<text x="{L+141}" y="70" {F} font-size="11" fill="{INK}">degree-matched null</text>')
for g in range(0,int(XMAX)+1,5):
    s.append(f'<line x1="{x(g):.1f}" y1="{TOP-18}" x2="{x(g):.1f}" y2="{H-50}" stroke="{LIGHT}" stroke-width="1"/>')
    s.append(f'<text x="{x(g):.1f}" y="{H-32}" {F} font-size="11" fill="{GREY}" text-anchor="middle">{g}%</text>')
s.append(f'<text x="{(L+R)/2:.0f}" y="{H-12}" {F} font-size="12" fill="{INK}" text-anchor="middle">'
         'irreducible recurrence</text>')
for i,d in enumerate(rows):
    y=TOP+RH*i
    s.append(f'<text x="{L-12}" y="{y+2:.0f}" {F} font-size="12.5" fill="{INK}" text-anchor="end">{d["name"]}</text>')
    s.append(f'<text x="{L-12}" y="{y+18:.0f}" {F} font-size="10.5" fill="{GREY}" text-anchor="end">'
             f'{d["dens"]:.1f} conns/cell</text>')
    s.append(f'<rect x="{x(0):.1f}" y="{y-14}" width="{x(d["real"])-x(0):.1f}" height="12" fill="{REAL}"/>')
    s.append(f'<text x="{x(d["real"])+6:.1f}" y="{y-4:.0f}" {F} font-size="11" fill="{REAL}">{d["real"]:.2f}%</text>')
    s.append(f'<rect x="{x(0):.1f}" y="{y+4}" width="{x(d["nm"])-x(0):.1f}" height="12" fill="{NULL}" opacity="0.45"/>')
    s.append(f'<line x1="{x(d["lo"]):.1f}" y1="{y+10}" x2="{x(d["hi"]):.1f}" y2="{y+10}" stroke="{NULL}" stroke-width="2"/>')
    # the whisker can run past the bar end, so clear both or the label
    # sits under the line and a 5 reads as a 6
    nlabx = max(x(d["nm"]), x(d["hi"])) + 7
    s.append(f'<text x="{nlabx:.1f}" y="{y+14:.0f}" {F} font-size="11" fill="{NULL}">{d["nm"]:.2f}%</text>')
    rat=d["real"]/d["nm"]
    tag=f'{rat:.2f}×' + ("  proved" if d["proved"] else "")
    s.append(f'<text x="{R+26}" y="{y+5:.0f}" {F} font-size="12" font-weight="bold" fill="{INK}">{tag}</text>')
s.append('</svg>')
open(out,"w",encoding="utf-8").write("\n".join(s))
print(f"wrote {out}")
for d in rows:
    print(f"  {d['name']:<22s} {d['real']:>6.2f}% vs {d['nm']:>6.2f}%  ratio {d['real']/d['nm']:.2f}")
