"""Split the disagreement between two orderings into who wins what.

usage: python3 -I split.py graph.csv ours.csv theirs.csv [annotations.tsv]

Every edge lands in one of four buckets: forward in both, forward only in
ours, forward only in theirs, backward in both. The two middle buckets are
the whole story -- their difference is exactly the score gap, and their
sizes say whether the other ordering knows anything we don't.

Pure stdlib. Reads the other party's file as data and nothing else: this is
analysis, never a starting point.
"""
import csv
import sys

graph_path, a_path, b_path = sys.argv[1], sys.argv[2], sys.argv[3]
ann_path = sys.argv[4] if len(sys.argv) > 4 else None


def load(p):
    pos = {}
    with open(p, newline="") as f:
        r = csv.reader(f)
        next(r)
        for nid, o in r:
            pos[nid] = int(o)
    return pos


A, B = load(a_path), load(b_path)

both = a_only = b_only = neither = 0
a_win = {}
b_win = {}
with open(graph_path, newline="") as f:
    r = csv.reader(f)
    next(r)
    for u, v, w in r:
        if u == v:
            continue
        w = int(w)
        fa = A[u] < A[v]
        fb = B[u] < B[v]
        if fa and fb:
            both += w
        elif fa:
            a_only += w
            a_win[u] = a_win.get(u, 0) + w
            a_win[v] = a_win.get(v, 0) + w
        elif fb:
            b_only += w
            b_win[u] = b_win.get(u, 0) + w
            b_win[v] = b_win.get(v, 0) + w
        else:
            neither += w

tot = both + a_only + b_only + neither
print(f"total weight .................. {tot:,}")
print(f"forward in both ............... {both:,}")
print(f"forward ONLY in ours .......... {a_only:,}")
print(f"forward ONLY in theirs ........ {b_only:,}")
print(f"backward in both .............. {neither:,}")
print(f"net (ours - theirs) ........... {a_only - b_only:,}")
print(f"ours = {both + a_only:,}   theirs = {both + b_only:,}")
print()
print(f"They capture {b_only:,} that we do not. If that weight were free to")
print(f"take, the ceiling of a merge would be {both + a_only + b_only:,};")
print("a cycle in the graph is what stops any of it being free.")
print()


def top(d, label, n=12):
    names = {}
    if ann_path:
        with open(ann_path, encoding="utf-8", errors="replace") as f:
            r = csv.reader(f, delimiter="\t")
            next(r)
            for row in r:
                if len(row) > 15 and row[1] in d:
                    names[row[1]] = (row[14] or row[11] or "?")[:28]
    items = sorted(d.items(), key=lambda kv: -kv[1])[:n]
    half = sum(d.values()) / 2.0
    run = 0
    k = 0
    for _, w in sorted(d.items(), key=lambda kv: -kv[1]):
        run += w
        k += 1
        if run >= half:
            break
    print(f"{label}: {len(d):,} neurons touched; "
          f"half the weight sits on {k:,} of them")
    for nid, w in items:
        print(f"    {w:>8,}  {nid}  {names.get(nid, '')}")
    print()


top(b_win, "weight only THEY get")
top(a_win, "weight only WE get")
