/* A stronger lower bound on forced-backward weight: exact MFAS on disjoint
 * induced subgraphs.
 *
 * Why it is valid. Any global ordering induces an ordering on every vertex
 * subset S, so the backward weight inside G[S] is at least the minimum over
 * ALL orderings of S -- the exact minimum feedback arc set of G[S]. Disjoint
 * subsets have disjoint internal edge sets, so those minima add, and the sum
 * is a valid lower bound on the backward weight of any ordering of G.
 *
 * Why it beats cycle packing. Greedy packing is one feasible solution to the
 * fractional cycle packing LP, so on the same edges it can never exceed the
 * exact answer. Inside a dense cluster the gap is wide: exact MFAS accounts
 * for all overlapping cycles at once, while greedy spends an edge's capacity
 * on whichever cycle reached it first.
 *
 * usage: clusterbound graph.csv [K] [out_clusters.txt]
 *        clusterbound --selftest
 *
 * K = maximum cluster size (default 14); cost is O(2^K * K^2) per cluster.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ---------------------------------------------------------- exact MFAS --- */
/* f[S] = max forward weight when exactly the vertices of S are placed first.
 * Placing v last within S collects every edge from S\{v} into v. */
static long long exact_backward(const long long *W, int n)
{
    if (n <= 1) return 0;
    size_t full = (size_t)1 << n;
    long long *f = malloc(full * sizeof(long long));
    if (!f) { fprintf(stderr, "oom\n"); exit(1); }
    long long total = 0;
    for (int i = 0; i < n * n; i++) total += W[i];
    f[0] = 0;
    for (size_t S = 1; S < full; S++) {
        long long best = -1;
        for (int v = 0; v < n; v++) {
            if (!((S >> v) & 1)) continue;
            size_t T = S ^ ((size_t)1 << v);
            long long gain = 0;
            for (int u = 0; u < n; u++)
                if ((T >> u) & 1) gain += W[u * n + v];
            long long cand = f[T] + gain;
            if (cand > best) best = cand;
        }
        f[S] = best;
    }
    long long fw = f[full - 1];
    free(f);
    return total - fw;
}

/* greedy 2- then 3-cycle packing on the same matrix: the comparison baseline */
static long long greedy_backward(const long long *W0, int n)
{
    long long *W = malloc((size_t)n * n * sizeof(long long));
    memcpy(W, W0, (size_t)n * n * sizeof(long long));
    long long forced = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            long long a = W[i * n + j], b = W[j * n + i];
            long long m = a < b ? a : b;
            if (m > 0) { W[i * n + j] -= m; W[j * n + i] -= m; forced += m; }
        }
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            for (int k = 0; k < n; k++) {
                if (k == i || k == j) continue;
                long long a = W[i * n + j], b = W[j * n + k], c = W[k * n + i];
                long long m = a < b ? a : b; if (c < m) m = c;
                if (m > 0) {
                    W[i * n + j] -= m; W[j * n + k] -= m; W[k * n + i] -= m;
                    forced += m;
                }
            }
        }
    free(W);
    return forced;
}

/* ------------------------------------------------------------ selftest --- */
static long long bb_best;
static void bb_rec(const long long *W, int n, int *perm, int d, char *used,
                   long long fwd)
{
    if (d == n) {
        long long tot = 0;
        for (int i = 0; i < n * n; i++) tot += W[i];
        if (tot - fwd < bb_best) bb_best = tot - fwd;
        return;
    }
    for (int v = 0; v < n; v++) {
        if (used[v]) continue;
        long long gain = 0;
        for (int i = 0; i < d; i++) gain += W[perm[i] * n + v];
        used[v] = 1; perm[d] = v;
        bb_rec(W, n, perm, d + 1, used, fwd + gain);
        used[v] = 0;
    }
}

static int selftest(void)
{
    srandom(12345);
    for (int trial = 0; trial < 400; trial++) {
        int n = 3 + (int)(random() % 6);            /* 3..8 */
        long long *W = calloc((size_t)n * n, sizeof(long long));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (i != j && (random() % 3)) W[i * n + j] = random() % 20;
        int perm[10]; char used[10] = {0};
        bb_best = 1LL << 60;
        bb_rec(W, n, perm, 0, used, 0);
        long long e = exact_backward(W, n), g = greedy_backward(W, n);
        if (e != bb_best) {
            printf("FAIL n=%d dp=%lld brute=%lld\n", n, e, bb_best);
            free(W); return 1;
        }
        if (g > e) {
            printf("FAIL greedy %lld > exact %lld (n=%d)\n", g, e, n);
            free(W); return 1;
        }
        free(W);
    }
    printf("selftest OK: 400 random graphs, the DP equals brute-force\n"
           "minimum feedback arc set every time, and greedy cycle packing\n"
           "never exceeds it.\n");
    return 0;
}

/* -------------------------------------------------------------- graph ---- */
typedef struct { uint64_t id; int idx; } Slot;
static Slot *tab; static size_t tabn;
static uint64_t *ids; static int np;

static int lookup(uint64_t id)
{
    size_t h = (size_t)(id * 1125899906842597ULL) & (tabn - 1);
    for (;;) {
        if (!tab[h].id) { tab[h].id = id; tab[h].idx = np; ids[np] = id; return np++; }
        if (tab[h].id == id) return tab[h].idx;
        h = (h + 1) & (tabn - 1);
    }
}

typedef struct { int u, v; long long w; } E;
static long long *g_rec;
static int cmp_rec(const void *A, const void *B)
{
    long long a = g_rec[*(const size_t *)A], b = g_rec[*(const size_t *)B];
    return a < b ? 1 : a > b ? -1 : 0;
}

int main(int argc, char **argv)
{
    if (argc > 1 && !strcmp(argv[1], "--selftest")) return selftest();
    if (argc < 2) { fprintf(stderr, "usage: %s graph.csv [K] [out.txt]\n", argv[0]); return 1; }
    int K = argc > 2 ? atoi(argv[2]) : 14;
    const char *outp = argc > 3 ? argv[3] : NULL;
    if (K < 3) K = 3;
    if (K > 20) K = 20;

    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }
    size_t cap = 1 << 20, ne = 0;
    E *es = malloc(cap * sizeof(E));
    tabn = 1 << 19; tab = calloc(tabn, sizeof(Slot));
    ids = malloc(500000 * sizeof(uint64_t));
    char line[256];
    long long total = 0;
    if (!fgets(line, sizeof line, f)) return 1;
    while (fgets(line, sizeof line, f)) {
        unsigned long long a, b; long long w;
        if (sscanf(line, "%llu,%llu,%lld", &a, &b, &w) != 3) continue;
        if (a == b) continue;
        int u = lookup(a), v = lookup(b);
        if (ne == cap) { cap *= 2; es = realloc(es, cap * sizeof(E)); }
        es[ne].u = u; es[ne].v = v; es[ne].w = w; ne++;
        total += w;
    }
    fclose(f);
    fprintf(stderr, "graph: %d nodes, %zu edges, %lld weight\n", np, ne, total);

    /* directed out-CSR and in-CSR */
    int *od = calloc(np, sizeof(int)), *idg = calloc(np, sizeof(int));
    for (size_t i = 0; i < ne; i++) { od[es[i].u]++; idg[es[i].v]++; }
    int *oo = malloc((np + 1) * sizeof(int)), *io = malloc((np + 1) * sizeof(int));
    oo[0] = io[0] = 0;
    for (int i = 0; i < np; i++) { oo[i+1] = oo[i] + od[i]; io[i+1] = io[i] + idg[i]; }
    int *oa = malloc((size_t)oo[np] * sizeof(int)); long long *ow = malloc((size_t)oo[np] * sizeof(long long));
    int *ia = malloc((size_t)io[np] * sizeof(int)); long long *iw = malloc((size_t)io[np] * sizeof(long long));
    int *fo = calloc(np, sizeof(int)), *fi = calloc(np, sizeof(int));
    for (size_t i = 0; i < ne; i++) {
        int u = es[i].u, v = es[i].v;
        oa[oo[u] + fo[u]] = v; ow[oo[u] + fo[u]] = es[i].w; fo[u]++;
        ia[io[v] + fi[v]] = u; iw[io[v] + fi[v]] = es[i].w; fi[v]++;
    }

    /* seed order: strongest reciprocal pairs first */
    g_rec = calloc(ne, sizeof(long long));
    for (size_t i = 0; i < ne; i++) {
        int u = es[i].u, v = es[i].v; long long back = 0;
        for (int k = oo[v]; k < oo[v+1]; k++) if (oa[k] == u) { back = ow[k]; break; }
        g_rec[i] = es[i].w < back ? es[i].w : back;
    }
    size_t *ord = malloc(ne * sizeof(size_t));
    for (size_t i = 0; i < ne; i++) ord[i] = i;
    qsort(ord, ne, sizeof(size_t), cmp_rec);

    char *used = calloc(np, 1);
    int *pos = malloc(np * sizeof(int));
    for (int i = 0; i < np; i++) pos[i] = -1;
    int *cl = malloc(K * sizeof(int));
    long long *W = malloc((size_t)K * K * sizeof(long long));
    long long sum_exact = 0, sum_greedy = 0, intra_w = 0;
    int nclust = 0, nnodes = 0;
    FILE *out = outp ? fopen(outp, "w") : NULL;

    for (size_t oi = 0; oi < ne; oi++) {
        size_t i = ord[oi];
        if (g_rec[i] <= 0) break;
        int u = es[i].u, v = es[i].v;
        if (used[u] || used[v]) continue;
        int n = 0;
        cl[n++] = u; cl[n++] = v; used[u] = used[v] = 1;
        pos[u] = 0; pos[v] = 1;
        /* gain of adding x = total directed weight between x and the cluster,
         * computed in O(deg x) using the membership array rather than a scan
         * over cluster members. */
        while (n < K) {
            int bestx = -1; long long bestg = 0; int seen = 0;
            for (int ci = 0; ci < n && seen < 4096; ci++) {
                int c = cl[ci];
                for (int side = 0; side < 2; side++) {
                    int lo = side ? io[c] : oo[c], hi = side ? io[c+1] : oo[c+1];
                    const int *arr = side ? ia : oa;
                    for (int k = lo; k < hi; k++) {
                        int x = arr[k];
                        if (used[x]) continue;
                        if (++seen > 4096) break;
                        long long g = 0;
                        for (int kk = oo[x]; kk < oo[x+1]; kk++)
                            if (pos[oa[kk]] >= 0) g += ow[kk];
                        for (int kk = io[x]; kk < io[x+1]; kk++)
                            if (pos[ia[kk]] >= 0) g += iw[kk];
                        if (g > bestg) { bestg = g; bestx = x; }
                    }
                }
            }
            if (bestx < 0) break;
            pos[bestx] = n; cl[n++] = bestx; used[bestx] = 1;
        }
        if (n < 3) { for (int a = 0; a < n; a++) pos[cl[a]] = -1; continue; }
        for (int a = 0; a < n; a++) pos[cl[a]] = a;
        memset(W, 0, (size_t)n * n * sizeof(long long));
        for (int a = 0; a < n; a++)
            for (int k = oo[cl[a]]; k < oo[cl[a]+1]; k++) {
                int b = pos[oa[k]];
                if (b >= 0) W[a * n + b] += ow[k];
            }
        for (int a = 0; a < n; a++) pos[cl[a]] = -1;

        long long e = exact_backward(W, n), g = greedy_backward(W, n), tw = 0;
        for (int a = 0; a < n * n; a++) tw += W[a];
        sum_exact += e; sum_greedy += g; intra_w += tw;
        nclust++; nnodes += n;
        if (out) {
            fprintf(out, "%lld %d", e, n);
            for (int a = 0; a < n; a++) fprintf(out, " %llu", (unsigned long long)ids[cl[a]]);
            fprintf(out, "\n");
        }
        if (nclust % 500 == 0)
            fprintf(stderr, "  %d clusters, exact %lld vs greedy %lld\n",
                    nclust, sum_exact, sum_greedy);
    }
    if (out) fclose(out);

    printf("clusters ................... %d covering %d nodes\n", nclust, nnodes);
    printf("weight inside clusters ..... %lld  (%.2f%% of graph)\n",
           intra_w, 100.0 * intra_w / total);
    printf("greedy packing on that ..... %lld\n", sum_greedy);
    printf("EXACT on that .............. %lld   (+%lld, %.1f%% more)\n",
           sum_exact, sum_exact - sum_greedy,
           sum_greedy ? 100.0 * (sum_exact - sum_greedy) / sum_greedy : 0.0);
    printf("\nforced backward (clusters only) %lld  = %.3f%% of all weight\n",
           sum_exact, 100.0 * sum_exact / total);
    printf("upper bound on forward from clusters alone: %lld (%.3f%%)\n",
           total - sum_exact, 100.0 * (total - sum_exact) / total);
    return 0;
}
