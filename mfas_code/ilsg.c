/* ilsg: ilstopo + group moves. With probability GP an iteration takes an
   anchor neuron (a hub with probability GHUB) and its K strongest partners
   (GMODE 1: any side, 2: only those on the wrong side), K random in 1..GK,
   lifts them out keeping their relative order, and puts them back as one
   contiguous block at the position that is best for the block as a whole
   (exact: only edges leaving the group change). Members are frozen while the
   rest repairs, then released.
   Slot DP (probability SP): take k (4..SK, max 16) neurons (SMODE 1: an anchor
   and its strongest partners, 2: its strongest wrong-side partners, 3: a run
   of consecutive positions) and find, exactly by dynamic programming over
   subsets, the best way to permute them among the positions they already
   occupy. Applied only if it gains.
   Re-interleave (probability IMP): take a set C (IMMODE 1: a stretch of IML
   consecutive positions, 2: IML random neurons) and merge it back into the
   rest optimally (IMMODE 3: a cluster from label propagation on synapse
   weight, size 30..IML; 4: a hub and its IML strongest partners), keeping the order inside C and inside the rest (exact DP
   over C's neurons x cut points in the rest, O(|C| N)). Applied only if it
   gains.
   Regions (REGLO < REGHI, fractions of the order): kicks, group anchors and
   TopoShuffle windows all start inside positions [REGLO*N, REGHI*N), so
   islands given different regions find non-overlapping improvements and the
   merge can keep more of them. The TopoShuffle window is then the region
   (widened to 10,000 neurons around it if narrower).
   usage: ilsg ... TOPOP TOPOL TOPOJ TOPOACC GP GK GMODE GHUB SP SK SMODE SHUB
               IMP IML IMMODE REGLO REGHI
   (ilstopo header follows)
   ilstopo: ilsh + TopoShuffle kicks. With probability TOPOP an iteration
   re-sorts a window of TOPOL positions (0 = the whole order) along a random
   topological order of the pairs the window currently gets right. Every right
   pair stays right, so the shuffle alone never loses weight; then the usual
   repair runs on everything that moved. TOPOJ is the jitter added to each
   neuron's old position when picking the next ready neuron (0 = no change,
   >= TOPOL = close to uniformly random). TOPOACC 0: keep only if the
   iteration gains; 1: also keep ties (walk the plateau).
   usage: ilstopo graph out secs seed init T Kmax tabu kmode lockonce strict
               HUBP HUBMODE HK HUBFRAC HUBTGT TOPOP TOPOL TOPOJ TOPOACC
   (ilsh header follows)
   ilsh: ilsk + hub moves. With probability HUBP an iteration kicks one hub
   (top HUBFRAC of neurons by degree) instead of K random neurons.
   HUBMODE 1: the hub moves alone.  HUBMODE 2: its HK strongest partners that
   end up on the wrong side follow it (placed right next to it), so the hub
   moves together with its neighbourhood. HUBTGT 0 random window, 1 one of its
   5 best alternative positions, 2 either. Everything kicked is frozen while
   the rest repairs, then released.
   usage: ilsh graph out secs seed init T Kmax tabu kmode lockonce strict
               HUBP HUBMODE HK HUBFRAC HUBTGT
   (original header follows)
   ils: fast iterated local search. After a kick, only the kicked neurons and
   (transitively) the neighbours of anything that moves are re-examined, so an
   iteration costs ~milliseconds instead of a full sweep. Moves are logged and
   undone exactly when an iteration loses weight.
   usage: ils graph.csv out.csv secs seed init.csv                         */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+1e-9*t.tv_nsec;}
static void isort64(int64_t*a,int n){for(int i=1;i<n;i++){int64_t x=a[i];int j=i-1;while(j>=0&&a[j]>x){a[j+1]=a[j];j--;}a[j+1]=x;}}
static void qs64(int64_t*a,int n){while(n>24){int64_t x=a[0],y=a[n/2],z=a[n-1],p;if(x>y){int64_t t=x;x=y;y=t;}if(y>z)y=z;if(x>y)y=x;p=y;
 int i=0,j=n-1;while(i<=j){while(a[i]<p)i++;while(a[j]>p)j--;if(i<=j){int64_t t=a[i];a[i]=a[j];a[j]=t;i++;j--;}}
 if(j+1<n-i){qs64(a,j+1);a+=i;n-=i;}else{qs64(a+i,n-i);n=j+1;}}isort64(a,n);}
#define HBITS 19
#define HSIZE (1u<<HBITS)
static uint64_t*hkey;static int32_t*hval;static int nnode=0;static uint64_t*id_of;
static inline uint64_t mix(uint64_t z){z+=0x9e3779b97f4a7c15ULL;z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;z=(z^(z>>27))*0x94d049bb133111ebULL;return z^(z>>31);}
static int32_t intern(uint64_t id){uint32_t h=mix(id)&(HSIZE-1);while(hval[h]>=0){if(hkey[h]==id)return hval[h];h=(h+1)&(HSIZE-1);}hkey[h]=id;hval[h]=nnode;id_of[nnode]=id;return nnode++;}
static int32_t lookup(uint64_t id){uint32_t h=mix(id)&(HSIZE-1);while(hval[h]>=0){if(hkey[h]==id)return hval[h];h=(h+1)&(HSIZE-1);}return -1;}
static char* slurp(const char*p,size_t*l){FILE*f=fopen(p,"rb");if(!f){perror(p);exit(1);}fseek(f,0,SEEK_END);long n=ftell(f);fseek(f,0,SEEK_SET);char*b=malloc(n+1);if(fread(b,1,n,f)!=(size_t)n)exit(1);b[n]=0;fclose(f);*l=n;return b;}
static inline uint64_t rdu64(char**p){uint64_t v=0;char*q=*p;while(*q>='0'&&*q<='9'){v=v*10+(*q-'0');q++;}*p=q;return v;}

static int32_t*src,*dst,*wt;static int64_t M=0,total=0;static int N;
static int64_t*adj,*astart;static int32_t*alen;static int maxdeg=0;
static int32_t*ord,*pos;static int64_t*sbuf;
static int32_t*wq;static uint8_t*inq,*tabu;static int32_t*locked;static int nlocked=0,LOCKONCE=0,phase1=0;static int qh=0,qt=0,qn=0;      /* ring worklist */
static int32_t*lv,*lf,*lt;static int nlog=0,logcap=0;                 /* undo log */
static uint64_t rs;static inline uint64_t rnd(void){rs^=rs<<13;rs^=rs>>7;rs^=rs<<17;return rs;}

static int64_t exact_score(void){int64_t f=0;for(int64_t e=0;e<M;e++)if(pos[src[e]]<pos[dst[e]])f+=wt[e];return f;}
static inline void shift_to(int32_t v,int32_t to){int32_t pv=pos[v];
  if(to>pv){memmove(ord+pv,ord+pv+1,(size_t)(to-pv)*4);for(int i=pv;i<to;i++)pos[ord[i]]=i;}
  else if(to<pv){memmove(ord+to+1,ord+to,(size_t)(pv-to)*4);for(int i=to+1;i<=pv;i++)pos[ord[i]]=i;}
  ord[to]=v;pos[v]=to;}
static inline void move_logged(int32_t v,int32_t to){
  if(nlog==logcap){logcap=logcap?logcap*2:4096;lv=realloc(lv,logcap*4);lf=realloc(lf,logcap*4);lt=realloc(lt,logcap*4);}
  lv[nlog]=v;lf[nlog]=pos[v];lt[nlog]=to;nlog++;shift_to(v,to);}
static void undo_all(void){for(int i=nlog-1;i>=0;i--)shift_to(lv[i],lf[i]);nlog=0;}
static inline void push(int32_t v){if(!inq[v]){inq[v]=1;wq[qt]=v;qt=(qt+1)%N;qn++;}}
static inline void push_nbrs(int32_t v){int64_t s=astart[v];for(int k=0;k<alen[v];k++)push((int32_t)(adj[s+k]>>32));}
/* exact gain of moving v to position q */
static int64_t gain_to(int32_t v,int32_t q){int32_t p=pos[v];int64_t s=astart[v],g=0;
  if(q>p){for(int k=0;k<alen[v];k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t pu=pos[u];if(pu>p&&pu<=q)g+=(int32_t)(adj[s+k]&0xffffffffLL);}}
  else if(q<p){for(int k=0;k<alen[v];k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t pu=pos[u];if(pu>=q&&pu<p)g-=(int32_t)(adj[s+k]&0xffffffffLL);}}
  return g;}
/* best insertion for v: returns gain, writes target */
static int64_t best_ins(int32_t v,int32_t*tgt){
  int n=alen[v];*tgt=pos[v];if(!n)return 0;int64_t s=astart[v];int32_t pv=pos[v];
  for(int k=0;k<n;k++){int32_t u=(int32_t)(adj[s+k]>>32);sbuf[k]=((int64_t)pos[u]<<32)|(uint32_t)(int32_t)(adj[s+k]&0xffffffffLL);}
  qs64(sbuf,n);int lo=0,hi=n;while(lo<hi){int m=(lo+hi)>>1;if((int32_t)(sbuf[m]>>32)<pv)lo=m+1;else hi=m;}
  int64_t best=0,acc=0;
  for(int k=lo;k<n;k++){acc+=(int32_t)(sbuf[k]&0xffffffffLL);if(acc>best){best=acc;*tgt=(int32_t)(sbuf[k]>>32);}}
  acc=0;for(int k=lo-1;k>=0;k--){acc-=(int32_t)(sbuf[k]&0xffffffffLL);if(acc>best){best=acc;*tgt=(int32_t)(sbuf[k]>>32);}}
  return best;}
/* a random pick among v's 5 best insertion points other than where it is */
static int32_t smart_target(int32_t v){
  int n=alen[v];if(!n)return -1;int64_t s=astart[v];int32_t pv=pos[v];
  for(int k=0;k<n;k++){int32_t u=(int32_t)(adj[s+k]>>32);sbuf[k]=((int64_t)pos[u]<<32)|(uint32_t)(int32_t)(adj[s+k]&0xffffffffLL);}
  qs64(sbuf,n);int lo=0,hi=n;while(lo<hi){int m=(lo+hi)>>1;if((int32_t)(sbuf[m]>>32)<pv)lo=m+1;else hi=m;}
  int64_t bg[5];int32_t bt[5];int nb=0;int64_t acc=0;
  #define CAND(T,G) do{int32_t t_=(T);int64_t g_=(G);if(t_<pv-1||t_>pv+1){ \
     if(nb<5){bg[nb]=g_;bt[nb]=t_;nb++;}else{int w=0;for(int i=1;i<5;i++)if(bg[i]<bg[w])w=i;if(g_>bg[w]){bg[w]=g_;bt[w]=t_;}}}}while(0)
  for(int k=lo;k<n;k++){acc+=(int32_t)(sbuf[k]&0xffffffffLL);CAND((int32_t)(sbuf[k]>>32),acc);}
  acc=0;for(int k=lo-1;k>=0;k--){acc-=(int32_t)(sbuf[k]&0xffffffffLL);CAND((int32_t)(sbuf[k]>>32),acc);}
  if(!nb)return -1;return bt[rnd()%(uint64_t)nb];}
static int64_t drain(int logged){int64_t g=0;
  while(qn){int32_t v=wq[qh];qh=(qh+1)%N;qn--;inq[v]=0;if(tabu[v])continue;int32_t t;int64_t b=best_ins(v,&t);
    if(b>0){if(logged)move_logged(v,t);else shift_to(v,t);g+=b;push_nbrs(v);if(phase1&&LOCKONCE&&!tabu[v]){tabu[v]=1;locked[nlocked++]=v;}}}
  return g;}
/* ---- TopoShuffle ---- */
static int32_t*tseg,*tnew,*tdeg;static int64_t*theap;static int thn=0,ta=0,tL=0;
static inline void hpush(int64_t x){int i=thn++;while(i){int p=(i-1)>>1;if(theap[p]<=x)break;theap[i]=theap[p];i=p;}theap[i]=x;}
static inline int64_t hpop(void){int64_t top=theap[0],x=theap[--thn];int i=0;
  for(;;){int c=2*i+1;if(c>=thn)break;if(c+1<thn&&theap[c+1]<theap[c])c++;if(theap[c]>=x)break;theap[i]=theap[c];i=c;}theap[i]=x;return top;}
/* sum over neurons v in [a,a+L) of the net weight of partners inside the
   window that sit before v. Twice the window's score, up to a constant. */
static int64_t win_sum(int a,int L){int64_t S=0;int b=a+L;
  for(int i=a;i<b;i++){int32_t v=ord[i];int64_t s=astart[v];
    for(int k=0;k<alen[v];k++){int32_t pu=pos[(int32_t)(adj[s+k]>>32)];if(pu>=a&&pu<i)S+=(int32_t)(adj[s+k]&0xffffffffLL);}}
  return S;}
/* re-sort [a,a+L) along a random topological order of its right pairs;
   returns the exact score change (always >= 0) */
static int64_t topo_shuffle(int a,int L,int J){
  int b=a+L;int64_t S0=win_sum(a,L);ta=a;tL=L;
  for(int i=a;i<b;i++){int32_t v=ord[i];tseg[i-a]=v;int c=0;int64_t s=astart[v];
    for(int k=0;k<alen[v];k++){int32_t pu=pos[(int32_t)(adj[s+k]>>32)];if(pu>=a&&pu<i&&(int32_t)(adj[s+k]&0xffffffffLL)>0)c++;}
    tdeg[v]=c;}
  thn=0;for(int i=a;i<b;i++){int32_t v=ord[i];if(!tdeg[v])hpush(((int64_t)((i-a)+(int)(rnd()%(uint64_t)(J+1)))<<32)|(uint32_t)v);}
  int no=0;
  while(thn){int32_t v=(int32_t)(hpop()&0xffffffffLL);tnew[no++]=v;int32_t pv=pos[v];int64_t s=astart[v];
    for(int k=0;k<alen[v];k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t pu=pos[u];
      if(pu>pv&&pu<b&&(int32_t)(adj[s+k]&0xffffffffLL)<0&&--tdeg[u]==0)hpush(((int64_t)((pu-a)+(int)(rnd()%(uint64_t)(J+1)))<<32)|(uint32_t)u);}}
  if(no!=L){fprintf(stderr,"!! topo: sorted %d of %d\n",no,L);exit(2);}
  for(int i=0;i<L;i++){ord[a+i]=tnew[i];pos[tnew[i]]=a+i;}
  int64_t dS=win_sum(a,L)-S0;if(dS&1)fprintf(stderr,"!! topo: odd change %lld\n",(long long)dS);
  if(dS<0)fprintf(stderr,"!! topo: shuffle lost %lld\n",(long long)dS);
  return dS/2;}
static void topo_restore(void){for(int i=0;i<tL;i++){ord[ta+i]=tseg[i];pos[tseg[i]]=ta+i;}}

/* ---- group moves ---- */
static int32_t*gmem;static uint8_t*ing;static int64_t*gext;static long gdirect=0;
static void place_after(int32_t*mem,int ng,int32_t L){for(int i=0;i<ng;i++){int32_t v=mem[i];int32_t to=pos[v]<pos[L]?pos[L]:pos[L]+1;move_logged(v,to);L=v;}}
static void place_before(int32_t*mem,int ng,int32_t F){for(int i=ng-1;i>=0;i--){int32_t v=mem[i];int32_t to=pos[v]<pos[F]?pos[F]-1:pos[F];move_logged(v,to);F=v;}}
/* returns exact gain of the block move; members (sorted by position) in gmem[0..*ngp) */
static int64_t group_move(int32_t v,int K,int GMODE,int*ngp){
  int ng=0;gmem[ng++]=v;ing[v]=1;int n=alen[v];int64_t s=astart[v];int m=0;
  for(int k=0;k<n;k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t w=(int32_t)(adj[s+k]&0xffffffffLL);
    if(GMODE==2&&!((w>0&&pos[u]>pos[v])||(w<0&&pos[u]<pos[v])))continue;sbuf[m++]=((int64_t)(w>0?w:-w)<<32)|(uint32_t)u;}
  qs64(sbuf,m);for(int i=m-1;i>=0&&ng<=K;i--){int32_t u=(int32_t)(sbuf[i]&0xffffffffLL);if(!ing[u]){ing[u]=1;gmem[ng++]=u;}}
  int ne=0;int64_t Fcur=0,F=0;
  for(int i=0;i<ng;i++){int32_t b=gmem[i];int64_t sb=astart[b];int32_t pb=pos[b];
    for(int k=0;k<alen[b];k++){int32_t u=(int32_t)(adj[sb+k]>>32);if(ing[u])continue;int32_t w=(int32_t)(adj[sb+k]&0xffffffffLL);int32_t pu=pos[u];
      gext[ne++]=((int64_t)pu<<32)|(uint32_t)w;if((w>0&&pu<pb)||(w<0&&pu>pb))Fcur+=(w>0?w:-w);if(w<0)F+=-w;}}
  int64_t d=0;
  if(ne){qs64(gext,ne);int64_t best=F;int bj=-1;
    for(int j=0;j<ne;){int32_t p=(int32_t)(gext[j]>>32);while(j<ne&&(int32_t)(gext[j]>>32)==p){int32_t w=(int32_t)(gext[j]&0xffffffffLL);F+=w;j++;}
      if(F>best){best=F;bj=j-1;}}
    /* sort members by position, then place */
    for(int i=0;i<ng;i++)sbuf[i]=((int64_t)pos[gmem[i]]<<32)|(uint32_t)gmem[i];qs64(sbuf,ng);for(int i=0;i<ng;i++)gmem[i]=(int32_t)(sbuf[i]&0xffffffffLL);
    if(bj<0)place_before(gmem,ng,ord[(int32_t)(gext[0]>>32)]);else place_after(gmem,ng,ord[(int32_t)(gext[bj]>>32)]);
    d=best-Fcur;}
  for(int i=0;i<ng;i++)ing[gmem[i]]=0;*ngp=ng;return d;}

/* ---- slot DP ---- */
#define SKMAX 16
static int32_t*sinw;static int64_t*sf;static int8_t*sbv;static int8_t*sid;
static int64_t sext[SKMAX][SKMAX];static int32_t swm[SKMAX][SKMAX];static int32_t snode[SKMAX],sslot[SKMAX];
static long sdtry=0,sdwin=0,sdless=0,sdeq=0;static int64_t sdgain=0;
static int64_t slot_dp(int32_t v,int k,int SMODE,int*np){
  int n=0;
  if(SMODE==3){int a=pos[v]-k/2;if(a<0)a=0;if(a+k>N)a=N-k;for(int i=0;i<k;i++){snode[n]=ord[a+i];sid[snode[n]]=(int8_t)n;n++;}}
  else{snode[n]=v;sid[v]=0;n++;int dn=alen[v];int64_t s=astart[v];int m=0;
    for(int q=0;q<dn;q++){int32_t u=(int32_t)(adj[s+q]>>32);int32_t w=(int32_t)(adj[s+q]&0xffffffffLL);
      if(SMODE==2&&!((w>0&&pos[u]>pos[v])||(w<0&&pos[u]<pos[v])))continue;sbuf[m++]=((int64_t)(w>0?w:-w)<<32)|(uint32_t)u;}
    qs64(sbuf,m);for(int i=m-1;i>=0&&n<k;i--){int32_t u=(int32_t)(sbuf[i]&0xffffffffLL);if(sid[u]<0){sid[u]=(int8_t)n;snode[n++]=u;}}}
  *np=n;sdtry++;if(n<3){for(int i=0;i<n;i++)sid[snode[i]]=-1;return 0;}
  for(int i=0;i<n;i++)sslot[i]=pos[snode[i]];
  for(int i=1;i<n;i++){int32_t x=sslot[i];int j=i-1;while(j>=0&&sslot[j]>x){sslot[j+1]=sslot[j];j--;}sslot[j+1]=x;}
  int64_t cur=0;
  for(int i=0;i<n;i++){int32_t b=snode[i];for(int j=0;j<n;j++)swm[i][j]=0;int64_t sb=astart[b];int m=0;int64_t tw=0;
    for(int q=0;q<alen[b];q++){int32_t u=(int32_t)(adj[sb+q]>>32);int32_t w=(int32_t)(adj[sb+q]&0xffffffffLL);int j=sid[u];
      if(j>=0){swm[i][j]=w;}else{sbuf[m++]=((int64_t)pos[u]<<32)|(uint32_t)w;tw+=w;}}
    qs64(sbuf,m);int64_t acc=0;int p=0;
    for(int j=0;j<n;j++){while(p<m&&(int32_t)(sbuf[p]>>32)<sslot[j]){acc+=(int32_t)(sbuf[p]&0xffffffffLL);p++;}sext[i][j]=2*acc-tw;}
    int r=0;while(sslot[r]!=pos[b])r++;cur+=sext[i][r];}
  for(int i=0;i<n;i++)for(int j=0;j<n;j++)if(i!=j&&pos[snode[j]]<pos[snode[i]])cur+=swm[i][j];
  int full=(1<<n)-1;
  for(int i=0;i<n;i++){int32_t*t=sinw+((size_t)i<<SKMAX);t[0]=0;for(int mk=1;mk<=full;mk++){int lb=__builtin_ctz(mk);t[mk]=t[mk&(mk-1)]+swm[i][lb];}}
  for(int mk=0;mk<=full;mk++)sf[mk]=INT64_MIN;sf[0]=0;
  for(int mk=0;mk<full;mk++){if(sf[mk]==INT64_MIN)continue;int c=__builtin_popcount(mk);
    for(int i=0;i<n;i++){if(mk>>i&1)continue;int64_t val=sf[mk]+sext[i][c]+sinw[((size_t)i<<SKMAX)+mk];int nm=mk|1<<i;if(val>sf[nm]){sf[nm]=val;sbv[nm]=(int8_t)i;}}}
  int64_t g=0;if(sf[full]<cur)sdless++;else if(sf[full]==cur)sdeq++;
  if(sf[full]>cur){int mk=full;int32_t nn[SKMAX];for(int c=n-1;c>=0;c--){int i=sbv[mk];nn[c]=snode[i];mk^=1<<i;}
    for(int c=0;c<n;c++){ord[sslot[c]]=nn[c];pos[nn[c]]=sslot[c];}g=(sf[full]-cur)/2;sdwin++;sdgain+=g;}
  for(int i=0;i<n;i++)sid[snode[i]]=-1;return g;}

/* ---- optimal re-interleaving ---- */
static int32_t*imA,*imB,*imIdx,*imG,*imCk,*imSeg,*imC,*imNew;static uint8_t*imIn;static int64_t*imD;static long imtry=0,imwin=0;static int64_t imgain=0;
/* one DP step: G <- prefixmax(G) + step function of node a's partners in B */
static void im_step(int32_t*G,int nb,int32_t a){
  int64_t s=astart[a];for(int k=0;k<alen[a];k++){int32_t u=(int32_t)(adj[s+k]>>32);if(imIn[u])continue;imD[imIdx[u]+1]+=(int32_t)(adj[s+k]&0xffffffffLL);}
  int32_t run=INT32_MIN;int64_t acc=0;
  for(int c=0;c<=nb;c++){acc+=imD[c];imD[c]=0;if(G[c]>run)run=G[c];G[c]=run+(int32_t)acc;}
  imD[nb+1]=0;}
static int32_t*clst,*clnodes;static int32_t*hubs_g;static int nhub_g=0;static int ncl=0;
static void build_clusters(int lo,int hi){int32_t*lab=malloc(N*4);int32_t*ordv=malloc(N*4);int64_t*ws=calloc(N,8);int32_t*touched=malloc(N*4);
  for(int i=0;i<N;i++){lab[i]=i;ordv[i]=i;}
  for(int it=0;it<20;it++){for(int i=N-1;i>0;i--){int j=(int)(rnd()%(uint64_t)(i+1));int32_t t=ordv[i];ordv[i]=ordv[j];ordv[j]=t;}long ch=0;
    for(int ii=0;ii<N;ii++){int32_t v=ordv[ii];int nt=0;int64_t s=astart[v];
      for(int k=0;k<alen[v];k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t w=(int32_t)(adj[s+k]&0xffffffffLL);int32_t l=lab[u];if(!ws[l])touched[nt++]=l;ws[l]+=w>0?w:-w;}
      int32_t bl=lab[v];int64_t bw=-1;for(int t=0;t<nt;t++){int32_t l=touched[t];if(ws[l]>bw||(ws[l]==bw&&l==lab[v])){bw=ws[l];bl=l;}ws[l]=0;}
      if(bl!=lab[v]){lab[v]=bl;ch++;}}
    if(ch<N/1000)break;}
  int32_t*cnt=calloc(N,4);for(int v=0;v<N;v++)cnt[lab[v]]++;int32_t*cid=malloc(N*4);int nc=0;int64_t tot=0;
  for(int l=0;l<N;l++){cid[l]=-1;if(cnt[l]>=lo&&cnt[l]<=hi){cid[l]=nc++;tot+=cnt[l];}}
  clst=calloc(nc+1,4);for(int v=0;v<N;v++)if(cid[lab[v]]>=0)clst[cid[lab[v]]+1]++;for(int c=0;c<nc;c++)clst[c+1]+=clst[c];
  clnodes=malloc((size_t)(tot+1)*4);int32_t*fp=malloc((nc+1)*4);memcpy(fp,clst,(nc+1)*4);for(int v=0;v<N;v++){int c=cid[lab[v]];if(c>=0)clnodes[fp[c]++]=v;}
  ncl=nc;fprintf(stderr,"clusters: %d of size %d..%d covering %lld neurons\n",nc,lo,hi,(long long)tot);
  free(lab);free(ordv);free(ws);free(touched);free(cnt);free(cid);free(fp);}
static int64_t reinterleave(int L,int MODE){
  int na=0,nb=0;
  if(MODE==3||MODE==4){
    if(MODE==3){if(!ncl)return 0;int c=(int)(rnd()%(uint64_t)ncl);for(int i=clst[c];i<clst[c+1];i++)imIn[clnodes[i]]=1;}
    else{int32_t h=hubs_g[rnd()%(uint64_t)nhub_g];imIn[h]=1;int n=alen[h];int64_t s=astart[h];int m=0;
      for(int k=0;k<n;k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t w=(int32_t)(adj[s+k]&0xffffffffLL);sbuf[m++]=((int64_t)(w>0?w:-w)<<32)|(uint32_t)u;}
      qs64(sbuf,m);for(int i=m-1;i>=0&&m-1-i<L;i--)imIn[(int32_t)(sbuf[i]&0xffffffffLL)]=1;}
    for(int i=0;i<N;i++){int32_t v=ord[i];if(imIn[v])imA[na++]=v;else{imIdx[v]=nb;imB[nb++]=v;}}}
  else if(MODE==1){int a=(int)(rnd()%(uint64_t)(N-L+1));for(int i=0;i<N;i++){int32_t v=ord[i];if(i>=a&&i<a+L){imA[na++]=v;imIn[v]=1;}else{imIdx[v]=nb;imB[nb++]=v;}}}
  else{for(int i=0;i<L;i++){int32_t v=(int32_t)(rnd()%(uint64_t)N);imIn[v]=1;}
    for(int i=0;i<N;i++){int32_t v=ord[i];if(imIn[v])imA[na++]=v;else{imIdx[v]=nb;imB[nb++]=v;}}}
  imtry++;
  /* current cut of each A node = number of B nodes before it */
  int64_t curv=0;{int cb=0;for(int i=0;i<N;i++){int32_t v=ord[i];if(!imIn[v]){cb++;continue;}imC[0]=cb;
      int64_t s=astart[v];for(int k=0;k<alen[v];k++){int32_t u=(int32_t)(adj[s+k]>>32);if(imIn[u])continue;if(imIdx[u]<cb)curv+=(int32_t)(adj[s+k]&0xffffffffLL);}}}
  int CK=1;while(CK*CK<na)CK++;int nck=(na+CK-1)/CK;size_t W=(size_t)nb+1;
  int32_t*G=imG;for(size_t c=0;c<W;c++)G[c]=0;
  for(int k=0;k<na;k++){if(k%CK==0)memcpy(imCk+(size_t)(k/CK)*W,G,W*4);im_step(G,nb,imA[k]);}
  int32_t best=INT32_MIN;int cbest=0;for(int c=0;c<=nb;c++)if(G[c]>best){best=G[c];cbest=c;}
  int64_t g=(int64_t)best-curv;
  if(g>0){ /* trace back: c_k for k=na-1..0 */
    int cnext=cbest;
    for(int sgi=nck-1;sgi>=0;sgi--){int k0=sgi*CK,k1=k0+CK<na?k0+CK:na;
      memcpy(imSeg,imCk+(size_t)sgi*W,W*4);
      for(int k=k0;k<k1-1;k++){memcpy(imSeg+(size_t)(k-k0+1)*W,imSeg+(size_t)(k-k0)*W,W*4);im_step(imSeg+(size_t)(k-k0+1)*W,nb,imA[k]);}
      for(int k=k1-1;k>=k0;k--){imC[k]=cnext; /* G_k (before node k) is imSeg[k-k0]; pick c'<=cnext maximizing it */
        int32_t*Gk=imSeg+(size_t)(k-k0)*W;int32_t bv=INT32_MIN;int bc=cnext;for(int c=cnext;c>=0;c--)if(Gk[c]>bv){bv=Gk[c];bc=c;}cnext=bc;}}
    int ia=0,ib=0,o=0;while(ia<na||ib<nb){if(ia<na&&imC[ia]<=ib)imNew[o++]=imA[ia++];else imNew[o++]=imB[ib++];}
    for(int i=0;i<N;i++){ord[i]=imNew[i];pos[imNew[i]]=i;}imwin++;imgain+=g;}
  for(int k=0;k<na;k++)imIn[imA[k]]=0;
  return g>0?g:0;}

static void dump(const char*path,int64_t sc){char tmp[1024];snprintf(tmp,sizeof tmp,"%s.tmp",path);
  FILE*f=fopen(tmp,"w");fprintf(f,"Node ID,Order\n");for(int i=0;i<N;i++)fprintf(f,"%llu,%d\n",(unsigned long long)id_of[ord[i]],i);fclose(f);rename(tmp,path);
  fprintf(stderr,"   [checkpoint %lld (%.4f%%)]\n",(long long)sc,100.0*sc/total);}

static const int WIN[]={2000,8000,20000,60000,0}; /* 0 = anywhere */
#define NW 5
int main(int argc,char**argv){
  if(argc<6){fprintf(stderr,"usage: %s graph out secs seed init\n",argv[0]);return 1;}
  double budget=atof(argv[3]);rs=strtoull(argv[4],0,10)*0x9E3779B97F4A7C15ULL+7;double t0=now();
  hkey=malloc(HSIZE*8);hval=malloc(HSIZE*4);for(uint32_t i=0;i<HSIZE;i++)hval[i]=-1;id_of=malloc(400000*8);
  size_t gl;char*g=slurp(argv[1],&gl);char*p=g;while(*p&&*p!='\n')p++;p++;
  int64_t cap=8000000;src=malloc(cap*4);dst=malloc(cap*4);wt=malloc(cap*4);
  while(*p){if(*p<'0'||*p>'9'){p++;continue;}uint64_t a=rdu64(&p);p++;uint64_t b=rdu64(&p);p++;uint64_t w=rdu64(&p);
    while(*p&&*p!='\n')p++;if(*p)p++;src[M]=intern(a);dst[M]=intern(b);wt[M]=(int32_t)w;M++;}
  free(g);N=nnode;for(int64_t e=0;e<M;e++)total+=wt[e];
  astart=malloc((N+1)*8);alen=malloc(N*4);int64_t*cnt=calloc(N+1,8);for(int64_t e=0;e<M;e++){cnt[src[e]]++;cnt[dst[e]]++;}
  int64_t acc=0;for(int i=0;i<N;i++){astart[i]=acc;acc+=cnt[i];}astart[N]=acc;adj=malloc(acc*8);int64_t*fill=malloc(N*8);memcpy(fill,astart,(size_t)N*8);
  for(int64_t e=0;e<M;e++){int32_t u=src[e],v=dst[e],w=wt[e];adj[fill[v]++]=((int64_t)u<<32)|(uint32_t)(w);adj[fill[u]++]=((int64_t)v<<32)|(uint32_t)(-w);}
  free(fill);free(cnt);
  for(int v=0;v<N;v++){int64_t s=astart[v];int n=(int)(astart[v+1]-s);qs64(adj+s,n);int o=0;
    for(int k=0;k<n;){int32_t u=(int32_t)(adj[s+k]>>32);int64_t sum=0;while(k<n&&(int32_t)(adj[s+k]>>32)==u){sum+=(int32_t)(adj[s+k]&0xffffffffLL);k++;}
      if(sum)adj[s+o++]=((int64_t)u<<32)|(uint32_t)(int32_t)sum;}alen[v]=o;if(o>maxdeg)maxdeg=o;}
  ord=malloc(N*4);pos=malloc(N*4);sbuf=malloc((size_t)(maxdeg+2)*8);wq=malloc(N*4);inq=calloc(N,1);tabu=calloc(N,1);locked=malloc(N*4);
  size_t ol;char*ob=slurp(argv[5],&ol);char*q=ob;while(*q&&*q!='\n')q++;q++;
  while(*q){if(*q<'0'||*q>'9'){q++;continue;}uint64_t id=rdu64(&q);q++;uint64_t o=rdu64(&q);while(*q&&*q!='\n')q++;if(*q)q++;ord[o]=lookup(id);}
  free(ob);for(int i=0;i<N;i++)pos[ord[i]]=i;
  int64_t cur=exact_score();fprintf(stderr,"start %lld (%.4f%%)\n",(long long)cur,100.0*cur/total);
  for(int i=0;i<N;i++)push(ord[i]);cur+=drain(0);
  fprintf(stderr,"local opt %lld (%.4f%%) %.1fs\n",(long long)cur,100.0*cur/total,now()-t0);
  double T=argc>6?atof(argv[6]):0.0;int Kmax=argc>7?atoi(argv[7]):8;if(Kmax<1)Kmax=1;int TABU=argc>8?atoi(argv[8]):1;int KMODE=argc>9?atoi(argv[9]):0;LOCKONCE=argc>10?atoi(argv[10]):0;int STRICT=argc>11?atoi(argv[11]):0;long smart_wins=0,smart_tries=0;int32_t kicked[256];
  double HUBP=argc>12?atof(argv[12]):0.0;int HUBMODE=argc>13?atoi(argv[13]):1;int HK=argc>14?atoi(argv[14]):16;if(HK>200)HK=200;
  double HUBFRAC=argc>15?atof(argv[15]):0.04;int HUBTGT=argc>16?atoi(argv[16]):2;
  double TOPOP=argc>17?atof(argv[17]):0.0;int TOPOL=argc>18?atoi(argv[18]):2000;int TOPOJ=argc>19?atoi(argv[19]):-1;int TOPOACC=argc>20?atoi(argv[20]):0;
  tseg=malloc(N*4);tnew=malloc(N*4);tdeg=malloc(N*4);theap=malloc((size_t)N*8);
  double GP=argc>21?atof(argv[21]):0.0;int GK=argc>22?atoi(argv[22]):16;if(GK>200)GK=200;if(GK<1)GK=1;int GMODE=argc>23?atoi(argv[23]):1;double GHUB=argc>24?atof(argv[24]):0.5;
  double SPR=argc>25?atof(argv[25]):0.0;int SK=argc>26?atoi(argv[26]):12;if(SK>SKMAX)SK=SKMAX;if(SK<4)SK=4;int SMODE=argc>27?atoi(argv[27]):1;double SHUB=argc>28?atof(argv[28]):0.3;
  sinw=malloc(sizeof(int32_t)*((size_t)SKMAX<<SKMAX));sf=malloc(sizeof(int64_t)<<SKMAX);sbv=malloc((size_t)1<<SKMAX);sid=malloc(N);memset(sid,-1,N);long stry=0,swin=0;int64_t sgain=0;double stime=0;
  fprintf(stderr,"slot dp: SP=%.2f SK=%d SMODE=%d SHUB=%.2f\n",SPR,SK,SMODE,SHUB);
  double IMP=argc>29?atof(argv[29]):0.0;int IML=argc>30?atoi(argv[30]):2000;if(IML>N/2)IML=N/2;int IMMODE=argc>31?atoi(argv[31]):1;
  long itry=0,iwin=0;int64_t igain=0;double itime=0;
  if(IMP>0){int CKm=1;while(CKm*CKm<IML)CKm++;size_t W=(size_t)N+1;imA=malloc(N*4);imB=malloc(N*4);imIdx=malloc(N*4);imG=malloc(W*4);imC=malloc(N*4);imNew=malloc(N*4);
    imIn=calloc(N,1);imD=calloc(W+1,8);imCk=malloc(W*4*(size_t)(CKm+2));imSeg=malloc(W*4*(size_t)(CKm+2));if(IMMODE==3)build_clusters(30,IML);}
  fprintf(stderr,"reinterleave: IMP=%.3f IML=%d IMMODE=%d\n",IMP,IML,IMMODE);
  double REGLO=argc>32?atof(argv[32]):0.0,REGHI=argc>33?atof(argv[33]):0.0;int RLO=0,RN=N;if(REGHI>REGLO){RLO=(int)(REGLO*N);RN=(int)(REGHI*N)-RLO;if(RN<1)RN=1;}
  fprintf(stderr,"region: %d..%d\n",RLO,RLO+RN);
  gmem=malloc(256*4);ing=calloc(N,1);gext=malloc(sizeof(int64_t)*(size_t)(maxdeg+1)*202);long gtry=0,gwin=0;int64_t ggain=0;double gtime=0;
  fprintf(stderr,"group: GP=%.2f GK=%d GMODE=%d GHUB=%.2f\n",GP,GK,GMODE,GHUB);
  long ttry=0,twin=0,tplat=0,tmoved=0;int64_t tgain=0,tsh=0;double ttime=0;
  fprintf(stderr,"topo: TOPOP=%.2f TOPOL=%d TOPOJ=%d TOPOACC=%d\n",TOPOP,TOPOL,TOPOJ,TOPOACC);
  int32_t*hubs=malloc(N*4);int nhub=0;{int64_t*dg=malloc(N*8);for(int i=0;i<N;i++)dg[i]=alen[i];qs64(dg,N);
    int64_t thr=dg[(int)((1.0-HUBFRAC)*(N-1))];free(dg);for(int i=0;i<N;i++)if(alen[i]>=thr)hubs[nhub++]=i;
    hubs_g=hubs;nhub_g=nhub;fprintf(stderr,"hubs: %d neurons with degree >= %lld  HUBP=%.2f HUBMODE=%d HK=%d HUBTGT=%d\n",nhub,(long long)thr,HUBP,HUBMODE,HK,HUBTGT);}
  long htry[2]={0,0},hwin[2]={0,0},ntry=0,nwin=0;int64_t hgain[2]={0,0},ngain=0;double htime[2]={0,0},ntime=0;
  int32_t*bestord=malloc(N*4),*tpos=malloc(N*4);memcpy(bestord,ord,(size_t)N*4);
  int64_t best=cur,dumped=-1;double lastd=0;long it=0,up=0,dn=0,sinceb=0,resets=0;
  while(now()-t0<budget){
    double ti=now();double r0=(double)(rnd()>>11)/9007199254740992.0;int topo=TOPOP>0&&r0<TOPOP;int grp=!topo&&GP>0&&r0<TOPOP+GP;int sdp=!topo&&!grp&&SPR>0&&r0<TOPOP+GP+SPR;int imv=!topo&&!grp&&!sdp&&IMP>0&&r0<TOPOP+GP+SPR+IMP;int hub=!topo&&!grp&&!sdp&&!imv&&HUBP>0&&r0<TOPOP+GP+SPR+IMP+HUBP;int htype=0;
    it++;nlog=0;int wi=(int)(rnd()%NW);int W=WIN[wi];int K=1+(int)(rnd()%(uint64_t)Kmax);if(K>64)K=64;int64_t d=0;int nk=0;int smart=(KMODE==1)||(KMODE==2&&(rnd()&1));if(smart)smart_tries++;
    if(hub){K=0;int32_t v=hubs[rnd()%(uint64_t)nhub];int32_t pv=pos[v],qq=-1;
      htype=(HUBTGT==2)?(int)(rnd()&1):HUBTGT;if(htype==1)qq=smart_target(v);
      if(htype==0||qq<0){htype=0;if(W==0)qq=(int32_t)(rnd()%(uint64_t)N);else{qq=pv+(int32_t)(rnd()%(uint64_t)(2*W+1))-W;if(qq<0)qq=0;if(qq>=N)qq=N-1;}}
      if(qq!=pv){d+=gain_to(v,qq);move_logged(v,qq);push(v);push_nbrs(v);if(TABU){tabu[v]=1;kicked[nk++]=v;}
        if(HUBMODE==2){int n=alen[v];int64_t s=astart[v];int m=0;
          for(int k=0;k<n;k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t w=(int32_t)(adj[s+k]&0xffffffffLL);
            int wrong=(w>0&&pos[u]>pos[v])||(w<0&&pos[u]<pos[v]);if(wrong&&!tabu[u])sbuf[m++]=((int64_t)(w>0?w:-w)<<32)|(uint32_t)u;}
          qs64(sbuf,m);
          for(int i=m-1;i>=0&&m-1-i<HK&&nk<256;i--){int32_t u=(int32_t)(sbuf[i]&0xffffffffLL);int32_t t=pos[v];if(pos[u]==t)continue;
            d+=gain_to(u,t);move_logged(u,t);push(u);push_nbrs(u);if(TABU){tabu[u]=1;kicked[nk++]=u;}}}}}
    if(topo){K=0;int L=(TOPOL<=0||TOPOL>N)?N:TOPOL;int a=(int)(rnd()%(uint64_t)(N-L+1));if(RN<N){L=RN<10000?(N<10000?N:10000):RN;a=RLO+RN/2-L/2;if(a<0)a=0;if(a+L>N)a=N-L;}
      int64_t ds=topo_shuffle(a,L,TOPOJ<0?L:TOPOJ);d+=ds;tsh+=ds;
      for(int i=0;i<L;i++){int32_t v=tseg[i];if(pos[v]!=a+i){push(v);push_nbrs(v);tmoved++;}}}
    if(imv){K=0;int64_t g=reinterleave(IML,IMMODE);if(g>0){d+=g;for(int i=0;i<N;i++)push(i);}}
    if(sdp){K=0;int32_t v=((double)(rnd()>>11)/9007199254740992.0<SHUB)?hubs[rnd()%(uint64_t)nhub]:(int32_t)(rnd()%(uint64_t)N);
      int kk=4+(int)(rnd()%(uint64_t)(SK-3));int ns=0;int64_t g=slot_dp(v,kk,SMODE,&ns);
      if(g>0){d+=g;for(int i=0;i<ns;i++){push(snode[i]);push_nbrs(snode[i]);}}}
    if(grp){K=0;int32_t v=((double)(rnd()>>11)/9007199254740992.0<GHUB)?hubs[rnd()%(uint64_t)nhub]:ord[RLO+(int)(rnd()%(uint64_t)RN)];
      if(RN<N){int tries=0;while((pos[v]<RLO||pos[v]>=RLO+RN)&&tries++<50)v=hubs[rnd()%(uint64_t)nhub];if(pos[v]<RLO||pos[v]>=RLO+RN)v=ord[RLO+(int)(rnd()%(uint64_t)RN)];}
      int kk=1+(int)(rnd()%(uint64_t)GK);int ng=0;int64_t dg=group_move(v,kk,GMODE,&ng);d+=dg;if(dg>0)gdirect++;
      for(int i=0;i<ng;i++){int32_t u=gmem[i];push(u);push_nbrs(u);if(TABU&&!tabu[u]&&nk<256){tabu[u]=1;kicked[nk++]=u;}}}
    for(int k=0;k<K;k++){int32_t v=ord[RLO+(int)(rnd()%(uint64_t)RN)];int32_t pv=pos[v],qq;
      if(smart){qq=smart_target(v);if(qq<0)continue;}
      else if(W==0)qq=(int32_t)(rnd()%(uint64_t)N);else{qq=pv+(int32_t)(rnd()%(uint64_t)(2*W+1))-W;if(qq<0)qq=0;if(qq>=N)qq=N-1;}
      if(qq==pv)continue;d+=gain_to(v,qq);move_logged(v,qq);push(v);push_nbrs(v);if(TABU&&!tabu[v]){tabu[v]=1;kicked[nk++]=v;}}
    phase1=1;nlocked=0;d+=drain(1);phase1=0;
    if(TABU){for(int i=0;i<nk;i++){tabu[kicked[i]]=0;push(kicked[i]);}for(int i=0;i<nlocked;i++){tabu[locked[i]]=0;push(locked[i]);}nlocked=0;d+=drain(1);}
    if(topo){if(d<0)fprintf(stderr,"!! topo iteration lost %lld\n",(long long)d);
      if(d>0||(TOPOACC&&d==0)){cur+=d;nlog=0;if(d>0)up++;else tplat++;}else{undo_all();topo_restore();}}
    else if(STRICT&&d==0){undo_all();}
    else if(d<0){if(T>0&&(double)(rnd()>>11)/9007199254740992.0<exp((double)d/T)){cur+=d;dn++;nlog=0;}else undo_all();}
    else{cur+=d;if(d>0){up++;if(smart)smart_wins++;}nlog=0;}
    {double dt=now()-ti;if(imv){itry++;itime+=dt;if(d>0){iwin++;igain+=d;}}else if(sdp){stry++;stime+=dt;if(d>0){swin++;sgain+=d;}}else if(grp){gtry++;gtime+=dt;if(d>0){gwin++;ggain+=d;}}else if(topo){ttry++;ttime+=dt;if(d>0){twin++;tgain+=d;}}else if(hub){htry[htype]++;htime[htype]+=dt;if(d>0){hwin[htype]++;hgain[htype]+=d;}}else{ntry++;ntime+=dt;if(d>0){nwin++;ngain+=d;}}}
    if(cur>best){best=cur;memcpy(bestord,ord,(size_t)N*4);sinceb=0;}else sinceb++;
    if(T>0&&sinceb>40000){memcpy(ord,bestord,(size_t)N*4);for(int i=0;i<N;i++)pos[ord[i]]=i;cur=best;sinceb=0;resets++;}
    if((it&1023)==0&&best>dumped&&now()-lastd>60){
      for(int i=0;i<N;i++)tpos[bestord[i]]=i;int64_t chk=0;for(int64_t e=0;e<M;e++)if(tpos[src[e]]<tpos[dst[e]])chk+=wt[e];
      if(chk!=best){fprintf(stderr,"!! mismatch %lld vs %lld\n",(long long)best,(long long)chk);best=chk;}
      memcpy(tpos,ord,(size_t)N*4);memcpy(ord,bestord,(size_t)N*4);dump(argv[2],best);memcpy(ord,tpos,(size_t)N*4);
      dumped=best;lastd=now();
      fprintf(stderr,"it %ld %.0f it/s up=%ld down=%ld resets=%ld cur=%lld best %lld (%.4f%%) %.0fs\n",it,it/(now()-t0),up,dn,resets,(long long)cur,(long long)best,100.0*best/total,now()-t0);}
  }
  for(int i=0;i<N;i++)tpos[bestord[i]]=i;int64_t chk=0;for(int64_t e=0;e<M;e++)if(tpos[src[e]]<tpos[dst[e]])chk+=wt[e];
  if(chk!=best)fprintf(stderr,"!! final mismatch %lld vs %lld\n",(long long)best,(long long)chk);
  memcpy(ord,bestord,(size_t)N*4);dump(argv[2],chk);
  fprintf(stderr,"tabu=%d T=%.1f Kmax=%d  up=%ld down=%ld resets=%ld\n",TABU,T,Kmax,up,dn,resets);
  fprintf(stderr,"kmode=%d lockonce=%d smart: tries %ld wins %ld\n",KMODE,LOCKONCE,smart_tries,smart_wins);
  fprintf(stderr,"normal kicks: tries %ld wins %ld gain %lld time %.0fs -> %.2f/s\n",ntry,nwin,(long long)ngain,ntime,ntime>0?ngain/ntime:0);
  for(int h=0;h<2;h++)fprintf(stderr,"hub kicks (%s): tries %ld wins %ld gain %lld time %.0fs -> %.2f/s\n",h?"best-alt":"window",htry[h],hwin[h],(long long)hgain[h],htime[h],htime[h]>0?hgain[h]/htime[h]:0);
  fprintf(stderr,"hub kicks total: gain %lld time %.0fs -> %.2f/s\n",(long long)(hgain[0]+hgain[1]),htime[0]+htime[1],htime[0]+htime[1]>0?(hgain[0]+hgain[1])/(htime[0]+htime[1]):0);
  fprintf(stderr,"topo kicks: tries %ld wins %ld ties kept %ld gain %lld (shuffle alone %lld) time %.0fs -> %.2f/s, moved %.0f per kick\n",ttry,twin,tplat,(long long)tgain,(long long)tsh,ttime,ttime>0?tgain/ttime:0,ttry?(double)tmoved/ttry:0);
  fprintf(stderr,"group moves: tries %ld wins %ld (block alone gained %ld) gain %lld time %.0fs -> %.2f/s\n",gtry,gwin,gdirect,(long long)ggain,gtime,gtime>0?ggain/gtime:0);
  fprintf(stderr,"slot dp: tries %ld wins %ld (dp alone gained %ld times, %lld; dp<cur %ld, dp==cur %ld) gain %lld time %.0fs -> %.2f/s\n",stry,swin,sdwin,(long long)sdgain,sdless,sdeq,(long long)sgain,stime,stime>0?sgain/stime:0);
  fprintf(stderr,"reinterleave: tries %ld wins %ld (dp alone gained %ld times, %lld) gain %lld time %.0fs -> %.2f/s\n",itry,iwin,imwin,(long long)imgain,(long long)igain,itime,itime>0?igain/itime:0);
  fprintf(stderr,"DONE %lld (%.4f%%)  iterations %ld\n",(long long)chk,100.0*chk/total,it);
  return 0;}
