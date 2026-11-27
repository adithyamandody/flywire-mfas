"""Build a single-file, print-ready manuscript from PAPER.md.

usage: python3 -I build_preprint.py PAPER.md figures_dir out_dir

Writes out_dir/manuscript.html (open it and print to PDF -- bioRxiv's
preferred route is one PDF containing text and figures) and
out_dir/abstract.txt (plain text, for pasting into the submission form).

Everything from the "Notes for the author" heading onward is dropped: that
section is working notes and must never reach a submitted manuscript.

Pure stdlib. No pandoc, no LaTeX, no plotting library.
"""
import os
import re
import sys

src_path, fig_dir, out_dir = sys.argv[1], sys.argv[2], sys.argv[3]
manifest = sys.argv[4] if len(sys.argv) > 4 else None

# ----------------------------------------------------------------- math ----
# Every math span in the paper, mapped by hand. A span that is not in this
# table is a build error rather than a guess: silently mangling an equation
# is worse than failing.
MATH = {
    r"G=(V,E,w)":
        "<i>G</i>&nbsp;=&nbsp;(<i>V</i>,&nbsp;<i>E</i>,&nbsp;<i>w</i>)",
    r"\pi: V \to \{1,\dots,n\}":
        "<i>π</i>:&nbsp;<i>V</i>&nbsp;→&nbsp;{1,&nbsp;…,&nbsp;<i>n</i>}",
    r"\sum_{(u,v)\in E,\; \pi(u)<\pi(v)} w(u,v).":
        "∑<sub>(<i>u</i>,<i>v</i>)∈<i>E</i>, <i>π</i>(<i>u</i>)&lt;<i>π</i>(<i>v</i>)</sub>"
        " <i>w</i>(<i>u</i>,<i>v</i>)",
    r"C": "<i>C</i>",
    r"y_C \geq 0": "<i>y</i><sub><i>C</i></sub>&nbsp;≥&nbsp;0",
    r"e": "<i>e</i>",
    r"\sum_{C \ni e} y_C \leq w_e":
        "∑<sub><i>C</i> ∋ <i>e</i></sub> <i>y</i><sub><i>C</i></sub>"
        "&nbsp;≤&nbsp;<i>w</i><sub><i>e</i></sub>",
    r"\sum_C y_C":
        "∑<sub><i>C</i></sub> <i>y</i><sub><i>C</i></sub>",
}

# -------------------------------------------------------------- captions ---
# Written from the drawing code in figures.py, not from the rendered image.
FIGURES = {
    1: ("fig1_axis.svg",
        "<b>Figure 1. The ordering recovers the brain's input-to-output axis.</b> "
        "Each row is an annotated class of neuron, ordered by its median position "
        "in the FlyWire ordering. The bar spans the interquartile range of the "
        "positions of that class's neurons and the dot marks the median, labelled "
        "with its value. Blue, FlyWire (136,648 neurons); orange, hemibrain "
        "(21,737). A class is drawn only where it has at least 30 neurons in that "
        "connectome. The axis is position in the ordering, 0% first to 100% last. "
        "Neither ordering was given any class label."),
    3: ("fig3_crosscheck.svg",
        "<b>Figure 3. The same cell types carry the same feedback in two brains.</b> "
        "Each point is a cell type present in both connectomes with at least 1,000 "
        "synaptic weight. The axes give the share of that type's own weight the "
        "ordering has to turn around, in FlyWire (horizontal) and hemibrain "
        "(vertical); both are clipped at 60%. The dashed line is equality, not a "
        "fit. Five types are labelled for orientation. The two orderings were "
        "computed independently, one per connectome."),
    2: ("fig2_pathways.svg",
        "<b>Figure 2. Between any two classes, one direction is clean and the other "
        "is not.</b> The nine pairs of classes carrying at least 20,000 weight in "
        "both directions, sorted by the cleaner direction. For each pair, the pale "
        "bar is that direction and the solid bar is the return direction; the "
        "length is the share of that direction's weight pointing backward in the "
        "ordering. The dashed line is the brain-wide rate, 15.2%, which falls "
        "between the two in all nine cases. Orange diamonds give the hemibrain "
        "value, shown only for routes whose endpoint classes both lie inside the "
        "hemibrain volume; the excluded routes are defined by optic-lobe partners, "
        "which hemibrain does not contain."),
}

# A different paper supplies its own figures through a manifest:
# one row per figure, "number<TAB>file<TAB>caption". Without one the map
# above is used, which belongs to the first paper.
if manifest:
    import csv as _csv
    FIGURES = {}
    with open(manifest, newline="", encoding="utf-8") as _f:
        _r = _csv.reader(_f, delimiter="\t")
        next(_r)
        for _num, _file, _cap in _r:
            FIGURES[int(_num)] = (_file, _cap)

# ------------------------------------------------------------ conversion ---
ENTITY = re.compile(r"&(#\d+|#x[0-9a-fA-F]+|[a-zA-Z]+);")


def esc(t):
    """Escape for HTML, leaving existing character entities intact."""
    keep = {}

    def stash(m):
        k = "\x01%d\x01" % len(keep)
        keep[k] = m.group(0)
        return k

    t = ENTITY.sub(stash, t)
    t = t.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
    for k, v in keep.items():
        t = t.replace(k, v)
    return t


def inline(t):
    """Inline markup on one chunk of text."""
    slots = []

    def stash_math(m):
        body = m.group(1)
        if body not in MATH:
            raise SystemExit("unmapped math span: %r\nadd it to MATH" % body)
        slots.append(MATH[body])
        return "\x00%d\x00" % (len(slots) - 1)

    t = re.sub(r"\$\$(.+?)\$\$", stash_math, t, flags=re.S)
    t = re.sub(r"\$([^$]+)\$", stash_math, t)

    def stash_code(m):
        slots.append("<code>" + esc(m.group(1)) + "</code>")
        return "\x00%d\x00" % (len(slots) - 1)

    t = re.sub(r"`([^`]+)`", stash_code, t)

    t = esc(t)
    t = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", r'<a href="\2">\1</a>', t)
    t = re.sub(r"\*\*(.+?)\*\*", r"<b>\1</b>", t, flags=re.S)
    t = re.sub(r"(?<![\w*])\*([^*\n]+)\*(?![\w*])", r"<i>\1</i>", t)
    t = re.sub(r"\x00(\d+)\x00", lambda m: slots[int(m.group(1))], t)
    return t


def table(lines):
    rows = [[c.strip() for c in ln.strip().strip("|").split("|")] for ln in lines]
    body = [r for r in rows if not re.match(r"^:?-{2,}:?$", r[0] or "-")]
    head, rest = body[0], body[1:]
    out = ['<table><thead><tr>']
    out += ["<th>%s</th>" % inline(c) for c in head]
    out.append("</tr></thead><tbody>")
    for r in rest:
        out.append("<tr>" + "".join("<td>%s</td>" % inline(c) for c in r) + "</tr>")
    out.append("</tbody></table>")
    return "".join(out)


def figure_block(n):
    name, caption = FIGURES[n]
    svg = open(os.path.join(fig_dir, name), encoding="utf-8").read()
    svg = re.sub(r"<\?xml[^>]*\?>", "", svg).strip()
    svg = re.sub(r'(<svg\b[^>]*?)\swidth="[^"]*"', r"\1", svg, count=1)
    svg = re.sub(r'(<svg\b[^>]*?)\sheight="[^"]*"', r"\1", svg, count=1)
    return '<figure id="fig%d">%s<figcaption>%s</figcaption></figure>' % (n, svg, caption)


def convert(md):
    blocks = re.split(r"\n\s*\n", md)
    html, placed = [], set()
    for b in blocks:
        b = b.strip("\n")
        if not b.strip():
            continue
        lines = b.split("\n")
        if re.match(r"^-{3,}$", lines[0].strip()) and len(lines) == 1:
            html.append("<hr/>")
            continue
        m = re.match(r"^(#{1,3})\s+(.*)$", lines[0])
        if m and len(lines) == 1:
            lvl = len(m.group(1))
            html.append("<h%d>%s</h%d>" % (lvl, inline(m.group(2)), lvl))
            continue
        if lines[0].lstrip().startswith("```"):
            body_lines = [ln for ln in lines
                          if not ln.lstrip().startswith("```")]
            html.append("<pre>%s</pre>" % esc("\n".join(body_lines)))
            continue
        if lines[0].lstrip().startswith("|"):
            html.append(table(lines))
            continue
        if re.match(r"^\s*-\s+", lines[0]):
            items, cur = [], []
            for ln in lines:
                if re.match(r"^\s*-\s+", ln):
                    if cur:
                        items.append(" ".join(cur))
                    cur = [re.sub(r"^\s*-\s+", "", ln)]
                else:
                    cur.append(ln.strip())
            if cur:
                items.append(" ".join(cur))
            html.append("<ul>" + "".join("<li>%s</li>" % inline(i) for i in items) + "</ul>")
            continue
        if re.match(r"^\d+\.\s+", lines[0]):
            items, cur = [], []
            for ln in lines:
                if re.match(r"^\d+\.\s+", ln):
                    if cur:
                        items.append(" ".join(cur))
                    cur = [re.sub(r"^\d+\.\s+", "", ln)]
                else:
                    cur.append(ln.strip())
            if cur:
                items.append(" ".join(cur))
            html.append("<ol>" + "".join("<li>%s</li>" % inline(i) for i in items) + "</ol>")
            continue
        if b.strip().startswith("$$"):
            html.append('<p class="disp">%s</p>' % inline(b.strip()))
            continue

        para = inline(" ".join(ln.strip() for ln in lines))
        html.append("<p>%s</p>" % para)
        # drop each figure in after the paragraph that first calls for it
        for n in sorted(FIGURES):
            if n not in placed and re.search(r"Figure %d\b" % n, b):
                html.append(figure_block(n))
                placed.add(n)
    missing = set(FIGURES) - placed
    if missing:
        raise SystemExit("figures never placed: %s" % sorted(missing))
    return "\n".join(html)


CSS = """
pre { font: 9.5pt/1.45 'SF Mono', Menlo, Consolas, monospace;
      background: #f6f6f4; padding: .7em .9em; border-radius: 3px;
      overflow-x: auto; break-inside: avoid; margin: 1em 0 1.2em; }
@page { size: letter; margin: 18mm 17mm; }
html { -webkit-print-color-adjust: exact; print-color-adjust: exact; }
body { font: 10.5pt/1.5 Georgia, 'Times New Roman', serif; color: #111;
       max-width: 46em; margin: 0 auto; padding: 2em 1.2em 4em; background: #fff; }
h1 { font-size: 17pt; line-height: 1.25; margin: 0 0 .5em; }
h2 { font-size: 12.5pt; margin: 1.8em 0 .5em; break-after: avoid; }
h3 { font-size: 11pt; margin: 1.4em 0 .4em; break-after: avoid; }
p { margin: 0 0 .75em; text-align: justify; hyphens: auto; }
p.disp { text-align: center; font-size: 12pt; margin: 1.1em 0; }
.byline { font-size: 10.5pt; margin-bottom: .2em; }
.meta { color: #555; font-size: 9.5pt; margin-bottom: 1.4em; }
table { border-collapse: collapse; margin: 1em 0 1.3em; font-size: 9.5pt;
        width: 100%; break-inside: avoid; }
th, td { border-bottom: 1px solid #ddd; padding: 4px 8px; text-align: left;
         vertical-align: top; }
thead th { border-bottom: 1.2px solid #888; font-weight: bold; }
code { font: 9.5pt/1.4 'SF Mono', Menlo, Consolas, monospace; }
figure { margin: 1.4em 0 1.6em; break-inside: avoid; page-break-inside: avoid; }
figure svg { width: 100%; height: auto; display: block; }
figcaption { font-size: 9pt; line-height: 1.45; color: #222; margin-top: .5em;
             text-align: left; }
ol, ul { margin: 0 0 .9em; padding-left: 1.4em; }
li { margin-bottom: .35em; }
ol { font-size: 9.8pt; }
hr { border: 0; border-top: 1px solid #ddd; margin: 1.6em 0; }
a { color: #1a4f7a; text-decoration: none; word-break: break-word; }
@media print { body { padding: 0; max-width: none; } a { color: #111; } }
"""

raw = open(src_path, encoding="utf-8").read()

cut = raw.find("## Notes for the author")
if cut < 0:
    raise SystemExit("could not find the author-notes heading; refusing to "
                     "build in case the notes would be included")
body_md = raw[:cut].rstrip().rstrip("-").rstrip()

lines = body_md.split("\n")
title = lines[0].lstrip("# ").strip()
plain_title = title.replace("*", "")
plain_title = title.replace("*", "")
byline = lines[2].strip()
meta = lines[3].strip()
rest_md = "\n".join(lines[4:])

# abstract, as plain text for the submission form
am = re.search(r"## Abstract\n(.*?)\n## ", body_md, flags=re.S)
if not am:
    raise SystemExit("could not find the abstract")
abstract = am.group(1).strip()
abstract = re.sub(r"\n\s*-{3,}\s*$", "", abstract).strip()
plain = re.sub(r"\*\*(.+?)\*\*", r"\1", abstract)
plain = re.sub(r"(?<![\w*])\*([^*\n]+)\*(?![\w*])", r"\1", plain)
plain = re.sub(r"`([^`]+)`", r"\1", plain)
plain = "\n\n".join(" ".join(p.split()) for p in re.split(r"\n\s*\n", plain))

os.makedirs(out_dir, exist_ok=True)
open(os.path.join(out_dir, "abstract.txt"), "w", encoding="utf-8").write(plain + "\n")

doc = """<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>%s</title><style>%s</style></head><body>
<h1>%s</h1>
<p class="byline">%s</p>
<p class="meta">%s</p>
%s
</body></html>
""" % (esc(plain_title), CSS, inline(title), inline(byline), inline(meta), convert(rest_md))

out = os.path.join(out_dir, "manuscript.html")
open(out, "w", encoding="utf-8").write(doc)

words = len(re.sub(r"<[^>]+>", " ", doc).split())
print("wrote %s  (%d bytes, ~%d words incl. captions)" % (out, len(doc), words))
print("wrote %s  (%d words)" % (os.path.join(out_dir, "abstract.txt"), len(plain.split())))
