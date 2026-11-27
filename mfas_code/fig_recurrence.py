"""Draw the irreducible-recurrence intervals.

usage: python3 -I fig_recurrence.py measurements.tsv out.svg

The TSV carries one row per nervous system: name, cells, total weight,
weight forced backward by the verified cycle packing, and the backward
weight of the exhibited ordering. Floor and ceiling are computed from
those, so the figure cannot disagree with the numbers it was given.

Pure stdlib, no plotting library.
"""
import csv
import sys

src, out = sys.argv[1], sys.argv[2]

INK = "#1b1b1b"; GREY = "#8a8a8a"; LIGHT = "#d8d8d8"
BAR = "#2d6a9f"; DIL = "#b5562c"
FONT = 'font-family="Helvetica,Arial,sans-serif"'


def esc(s):
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


rows = []
with open(src, newline="", encoding="utf-8") as f:
    r = csv.reader(f, delimiter="\t")
    next(r)
    for name, cells, total, forced, achieved, note in r:
        total, forced, achieved = int(total), int(forced), int(achieved)
        rows.append({
            "name": name, "cells": int(cells),
            "lo": 100.0 * forced / total,
            "hi": 100.0 * achieved / total,
            "note": note,
        })

XMAX = 24.0
L, R = 212, 760
TOP, RH = 78, 52
H = TOP + RH * len(rows) + 74
W = 820
x = lambda v: L + (R - L) * v / XMAX

s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">',
     f'<rect width="{W}" height="{H}" fill="white"/>']

s.append(f'<text x="24" y="30" {FONT} font-size="15" font-weight="bold" fill="{INK}">'
         'How much of a nervous system no hierarchy can explain</text>')
s.append(f'<text x="24" y="50" {FONT} font-size="11.5" fill="{GREY}">'
         'Each bar spans a proven floor and an achieved ceiling. '
         'Connections of at least two synapses.</text>')

for g in range(0, int(XMAX) + 1, 5):
    s.append(f'<line x1="{x(g):.1f}" y1="{TOP - 14}" x2="{x(g):.1f}" y2="{H - 56}" '
             f'stroke="{LIGHT}" stroke-width="1"/>')
    s.append(f'<text x="{x(g):.1f}" y="{H - 38}" {FONT} font-size="11" fill="{GREY}" '
             f'text-anchor="middle">{g}%</text>')
s.append(f'<text x="{(L + R) / 2:.0f}" y="{H - 16}" {FONT} font-size="12" fill="{INK}" '
         f'text-anchor="middle">irreducible recurrence '
         '(share of synaptic weight backward in every possible ordering)</text>')

for i, d in enumerate(rows):
    yy = TOP + RH * i
    col = DIL if d["note"] == "diluted" else BAR
    s.append(f'<text x="{L - 12}" y="{yy + 2:.0f}" {FONT} font-size="12.5" fill="{INK}" '
             f'text-anchor="end">{esc(d["name"])}</text>')
    s.append(f'<text x="{L - 12}" y="{yy + 18:.0f}" {FONT} font-size="10.5" fill="{GREY}" '
             f'text-anchor="end">{d["cells"]:,} cells</text>')
    w = x(d["hi"]) - x(d["lo"])
    if w < 3:
        cx = x(d["lo"])
        s.append(f'<path d="M {cx:.1f} {yy - 7} l 8 8 l -8 8 l -8 -8 z" fill="{col}"/>')
        lab = f'{d["lo"]:.2f}%  exact'
        labx = cx + 14
    else:
        s.append(f'<rect x="{x(d["lo"]):.1f}" y="{yy - 7}" width="{w:.1f}" height="14" '
                 f'rx="2" fill="{col}" opacity="0.5"/>')
        for v in (d["lo"], d["hi"]):
            s.append(f'<line x1="{x(v):.1f}" y1="{yy - 9}" x2="{x(v):.1f}" y2="{yy + 9}" '
                     f'stroke="{col}" stroke-width="2.5"/>')
        lab = f'{d["lo"]:.2f} – {d["hi"]:.2f}%'
        labx = x(d["hi"]) + 10
    s.append(f'<text x="{labx:.1f}" y="{yy + 4:.0f}" {FONT} font-size="11.5" fill="{col}">{lab}</text>')
    if d["note"] == "diluted":
        s.append(f'<text x="{L - 12}" y="{yy + 32:.0f}" {FONT} font-size="9.5" fill="{DIL}" '
                 f'text-anchor="end">includes the optic lobes</text>')

s.append('</svg>')
open(out, "w", encoding="utf-8").write("\n".join(s))
print(f"wrote {out} ({len(rows)} systems)")
