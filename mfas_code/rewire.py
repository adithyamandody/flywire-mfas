"""Degree-preserving random rewiring of a weighted directed graph. Pure stdlib.

usage: python3 -I rewire.py graph.csv out.csv [--seed S] [--swaps-per-edge K]
       python3 -I rewire.py --selftest

Null model: repeated double-edge swaps. Pick two connections a->b and c->d
uniformly at random and replace them with a->d and c->b. A swap is rejected if
it would create a self-loop (a==d or c==b) or a connection that already exists.
Each connection keeps its weight and its source, only the target moves, so

  * every node's in-degree and out-degree is exactly preserved,
  * the multiset of connection weights is exactly preserved,
  * every node's out-strength (summed outgoing weight) is exactly preserved
    (in-strength is NOT: weights travel with the source end).

Reciprocal pairs (a->b together with b->a) are NOT forbidden; they arise at the
rate the degree sequence dictates, which is the point of the null.

K swap ATTEMPTS per connection are made (default 10). After writing, the output
file is read back and the invariants above are asserted against the input.
"""
import csv, random, sys
from collections import Counter


def read_graph(path):
    with open(path, newline="") as f:
        r = csv.reader(f)
        header = next(r)
        src, dst, wt = [], [], []
        for a, b, w in r:
            src.append(int(a)); dst.append(int(b)); wt.append(int(w))
    return header, src, dst, wt


def rewire(src, dst, wt, seed=1, swaps_per_edge=10):
    """Return (new_dst, accepted, attempts). src and wt are unchanged."""
    m = len(src)
    ids = sorted(set(src) | set(dst))
    idx = {v: i for i, v in enumerate(ids)}
    n = len(ids)
    s = [idx[v] for v in src]
    d = [idx[v] for v in dst]
    present = set()
    for i in range(m):
        if s[i] == d[i]:
            raise ValueError("input has a self-loop")
        present.add(s[i] * n + d[i])
    if len(present) != m:
        raise ValueError("input has duplicate connections")
    rnd = random.Random(seed).random
    attempts = swaps_per_edge * m
    accepted = 0
    add = present.add; rem = present.remove
    for _ in range(attempts):
        i = int(rnd() * m); j = int(rnd() * m)
        a = s[i]; c = s[j]; b = d[i]; e = d[j]
        if a == e or c == b or b == e:
            continue
        k1 = a * n + e; k2 = c * n + b
        if k1 in present or k2 in present:
            continue
        rem(a * n + b); rem(c * n + e); add(k1); add(k2)
        d[i] = e; d[j] = b
        accepted += 1
    return [ids[v] for v in d], accepted, attempts


def check(src0, dst0, wt0, src1, dst1, wt1):
    """Assert the null keeps the invariants. Raises AssertionError otherwise."""
    assert len(src0) == len(src1), "connection count changed"
    assert Counter(src0) == Counter(src1), "out-degree sequence changed"
    assert Counter(dst0) == Counter(dst1), "in-degree sequence changed"
    assert sorted(wt0) == sorted(wt1), "weight multiset changed"
    os0 = Counter(); os1 = Counter()
    for a, w in zip(src0, wt0): os0[a] += w
    for a, w in zip(src1, wt1): os1[a] += w
    assert os0 == os1, "out-strength changed"
    assert all(a != b for a, b in zip(src1, dst1)), "self-loop created"
    assert len(set(zip(src1, dst1))) == len(src1), "duplicate connection created"


def stats(src, dst, wt):
    """(reciprocal pairs, weight on reciprocated connections)"""
    w = {(a, b): x for a, b, x in zip(src, dst, wt)}
    pairs = 0; rw = 0
    for (a, b), x in w.items():
        if (b, a) in w:
            rw += x
            if a < b: pairs += 1
    return pairs, rw


def selftest():
    rng = random.Random(12345)
    for trial in range(20):
        n = rng.randrange(8, 60)
        m = rng.randrange(n, min(n * (n - 1), 6 * n))
        es = set()
        while len(es) < m:
            a = rng.randrange(n); b = rng.randrange(n)
            if a != b: es.add((a * 1000003 + 7, b * 1000003 + 7))   # sparse ids
        es = sorted(es)
        src = [a for a, _ in es]; dst = [b for _, b in es]
        wt = [rng.randrange(2, 50) for _ in es]
        nd, acc, att = rewire(src, dst, wt, seed=trial, swaps_per_edge=10)
        check(src, dst, wt, src, nd, wt)
        assert att == 10 * m
        if trial == 0:
            assert nd != dst, "nothing moved"
            nd2, _, _ = rewire(src, dst, wt, seed=trial, swaps_per_edge=10)
            assert nd2 == nd, "not deterministic for a fixed seed"
    # the checker itself must catch a broken null
    src = [1, 2, 3]; dst = [2, 3, 1]; wt = [2, 3, 4]
    for bad in ([2, 3, 2], [1, 3, 2]):          # in-degree change; self-loop
        try:
            check(src, dst, wt, src, bad, wt)
        except AssertionError:
            pass
        else:
            raise SystemExit("selftest FAILED: checker accepted a bad null")
    try:
        check(src, dst, wt, src, dst, [2, 3, 5])
    except AssertionError:
        pass
    else:
        raise SystemExit("selftest FAILED: checker accepted changed weights")
    # a 3-ring admits no legal swap: must come back unchanged
    nd, acc, _ = rewire(src, dst, wt, seed=0)
    assert acc == 0 and nd == dst
    print("selftest OK: 20 random graphs, degrees/weights/out-strength preserved, "
          "no self-loops or duplicates, checker rejects bad nulls")


def main(argv):
    if "--selftest" in argv:
        selftest(); return
    seed = 1; k = 10; pos = []
    i = 1
    while i < len(argv):
        if argv[i] == "--seed": seed = int(argv[i + 1]); i += 2
        elif argv[i] == "--swaps-per-edge": k = int(argv[i + 1]); i += 2
        else: pos.append(argv[i]); i += 1
    if len(pos) != 2:
        sys.exit(__doc__)
    header, src, dst, wt = read_graph(pos[0])
    nd, acc, att = rewire(src, dst, wt, seed, k)
    with open(pos[1], "w", newline="") as f:
        f.write(",".join(header) + "\n")
        f.write("".join(f"{a},{b},{w}\n" for a, b, w in zip(src, nd, wt)))
    # verify from the file actually written, not from memory
    _, s1, d1, w1 = read_graph(pos[1])
    check(src, dst, wt, s1, d1, w1)
    moved = sum(1 for x, y in zip(dst, d1) if x != y)
    p0, r0 = stats(src, dst, wt); p1, r1 = stats(s1, d1, w1)
    tot = sum(wt)
    print(f"{pos[1]}: seed {seed}, {len(src):,} connections, {att:,} attempts, "
          f"{acc:,} accepted ({100*acc/att:.1f}%), {100*moved/len(src):.1f}% of targets differ")
    print(f"  verified: in/out degree sequences identical, weight multiset identical, "
          f"out-strength identical, no self-loops, no duplicates")
    print(f"  reciprocal pairs {p0:,} -> {p1:,}; weight on reciprocated connections "
          f"{100*r0/tot:.2f}% -> {100*r1/tot:.2f}%")


if __name__ == "__main__":
    main(sys.argv)
