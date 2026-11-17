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

static void dump(const char*path,int64_t sc){char tmp[1024];snprintf(tmp,sizeof tmp,"%s.tmp",path);
  FILE*f=fopen(tmp,"w");fprintf(f,"Node ID,Order\n");for(int i=0;i<N;i++)fprintf(f,"%llu,%d\n",(unsigned long long)id_of[ord[i]],i);fclose(f);rename(tmp,path);
  fprintf(stderr,"   [checkpoint %lld (%.4f%%)]\n",(long long)sc,100.0*sc/total);}


/* ---------------------------------------------------------------- tmerge */
static int32_t*posA,*posB,*posS,*keyv,*saved,*tnew2;static uint8_t*done;static int64_t*aheap;static int ahn=0;static long nforced=0;
static void read_pos(const char*path,int32_t*P){size_t l;char*b=slurp(path,&l);char*q=b;while(*q&&*q!='\n')q++;q++;int n=0;
  while(*q){if(*q<'0'||*q>'9'){q++;continue;}uint64_t id=rdu64(&q);q++;uint64_t o=rdu64(&q);while(*q&&*q!='\n')q++;if(*q)q++;
    int32_t v=lookup(id);if(v<0){fprintf(stderr,"unknown node in %s\n",path);exit(1);}P[v]=(int32_t)o;n++;}
  free(b);if(n!=N){fprintf(stderr,"%s has %d rows, expected %d\n",path,n,N);exit(1);}}
static inline int rightX(const int32_t*P,int32_t u,int32_t v,int32_t w){return (w>0&&P[u]<P[v])||(w<0&&P[u]>P[v]);}
/* is the pair (v, partner u, net weight w seen from v) a constraint, pointing the right way?
   MODE 3: right in A unless B broke it (right in seed S, wrong in B), or B fixed it (wrong in S, right in B) */
static inline int keep3(int32_t u,int32_t v,int32_t w){int rS=rightX(posS,u,v,w),rB=rightX(posB,u,v,w);
  return (rightX(posA,u,v,w)&&!(rS&&!rB))||(!rS&&rB);}
static inline void apush(int64_t x){int i=ahn++;while(i){int p=(i-1)>>1;if(aheap[p]<=x)break;aheap[i]=aheap[p];i=p;}aheap[i]=x;}
static inline int64_t apop(void){int64_t top=aheap[0],x=aheap[--ahn];int i=0;
  for(;;){int c=2*i+1;if(c>=ahn)break;if(c+1<ahn&&aheap[c+1]<aheap[c])c++;if(aheap[c]>=x)break;aheap[i]=aheap[c];i=c;}aheap[i]=x;return top;}
/* MODE 3 sort: constraints may contain cycles; when nothing is ready, force the
   unplaced neuron with the smallest key (its unmet constraints are dropped) */
static void topo3(void){
  for(int v=0;v<N;v++){int c=0;int64_t s=astart[v];
    for(int k=0;k<alen[v];k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t w=(int32_t)(adj[s+k]&0xffffffffLL);
      if(w>0&&keep3(u,v,w))c++;}tdeg[v]=c;done[v]=0;}
  thn=0;ahn=0;for(int v=0;v<N;v++){int64_t x=((int64_t)keyv[v]<<32)|(uint32_t)v;apush(x);if(!tdeg[v])hpush(x);}
  int no=0;nforced=0;
  while(no<N){int32_t v;
    if(thn){v=(int32_t)(hpop()&0xffffffffLL);if(done[v])continue;}
    else{do v=(int32_t)(apop()&0xffffffffLL);while(done[v]);nforced++;}
    done[v]=1;tnew[no++]=v;int64_t s=astart[v];
    for(int k=0;k<alen[v];k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t w=(int32_t)(adj[s+k]&0xffffffffLL);
      if(w<0&&!done[u]&&keep3(u,v,w)&&--tdeg[u]==0)hpush(((int64_t)keyv[u]<<32)|(uint32_t)u);}}
  for(int i=0;i<N;i++){ord[i]=tnew[i];pos[tnew[i]]=i;}}
/* re-sort the whole order: keep pair (u before v) when A has it right
   [and, MODE 2, B has it right too]; next ready neuron = smallest key */
static void topo_all(int MODE){
  for(int v=0;v<N;v++){int c=0;int64_t s=astart[v];
    for(int k=0;k<alen[v];k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t w=(int32_t)(adj[s+k]&0xffffffffLL);
      if(w>0&&posA[u]<posA[v]&&(MODE==1||posB[u]<posB[v]))c++;}tdeg[v]=c;}
  thn=0;for(int v=0;v<N;v++)if(!tdeg[v])hpush(((int64_t)keyv[v]<<32)|(uint32_t)v);
  int no=0;
  while(thn){int32_t v=(int32_t)(hpop()&0xffffffffLL);tnew[no++]=v;int64_t s=astart[v];
    for(int k=0;k<alen[v];k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t w=(int32_t)(adj[s+k]&0xffffffffLL);
      if(w<0&&posA[u]>posA[v]&&(MODE==1||posB[u]>posB[v])&&--tdeg[u]==0)hpush(((int64_t)keyv[u]<<32)|(uint32_t)u);}}
  if(no!=N){fprintf(stderr,"!! topo_all sorted %d of %d\n",no,N);exit(2);}
  for(int i=0;i<N;i++){ord[i]=tnew[i];pos[tnew[i]]=i;}}

int main(int argc,char**argv){
  if(argc<9){fprintf(stderr,"usage: %s graph out MODE ALPHA PASSES seed.csv base.csv island.csv...\n",argv[0]);return 1;}
  int MODE=atoi(argv[3]);double ALPHA=atof(argv[4]);int PASSES=atoi(argv[5]);double t0=now();rs=12345;
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
  tnew=malloc(N*4);tdeg=malloc(N*4);theap=malloc((size_t)N*8);posA=malloc(N*4);posB=malloc(N*4);keyv=malloc(N*4);saved=malloc(N*4);posS=malloc(N*4);tnew2=malloc(N*4);done=malloc(N);aheap=malloc((size_t)N*8);
  read_pos(argv[6],posS);read_pos(argv[7],posB);{int32_t mn=posB[0];for(int v=0;v<N;v++)if(posB[v]<mn)mn=posB[v];
    for(int v=0;v<N;v++){int32_t o=posB[v]-mn;if(o<0||o>=N){fprintf(stderr,"base order out of range\n");exit(1);}ord[o]=v;}}
  for(int i=0;i<N;i++)pos[ord[i]]=i;
  int64_t cur=exact_score();for(int i=0;i<N;i++)push(ord[i]);cur+=drain(0);
  fprintf(stderr,"base %lld (%.4f%%)  %.1fs\n",(long long)cur,100.0*cur/total,now()-t0);int64_t base0=cur;
  int nisl=argc-8;
  for(int pass=0;pass<PASSES;pass++){int acc_any=0;
    for(int k=0;k<nisl;k++){double ti=now();
      read_pos(argv[8+k],posB);memcpy(posA,pos,(size_t)N*4);memcpy(saved,ord,(size_t)N*4);
      int64_t s1=0,s2=-1;int nalt=(MODE==3&&ALPHA!=0.5&&getenv("TM_BOTH"))?2:1;int32_t*bestc=tnew2;
      for(int alt=0;alt<nalt;alt++){double al=alt?1.0-ALPHA:ALPHA;
      if(alt){memcpy(ord,saved,(size_t)N*4);for(int i=0;i<N;i++)pos[ord[i]]=i;}
      for(int v=0;v<N;v++)keyv[v]=(int32_t)(al*posA[v]+(1.0-al)*posB[v]);
      if(MODE==3)topo3();else topo_all(MODE);int64_t c1=exact_score();
      if(MODE==1&&s1<cur)fprintf(stderr,"!! mode 1 lost weight before repair: %lld < %lld\n",(long long)s1,(long long)cur);
      for(int i=0;i<N;i++)push(ord[i]);int64_t c2=c1+drain(0);int64_t chk=exact_score();
      if(chk!=c2){fprintf(stderr,"!! repair bookkeeping %lld vs %lld\n",(long long)c2,(long long)chk);c2=chk;}
      if(c2>s2){s2=c2;s1=c1;memcpy(bestc,ord,(size_t)N*4);}}
      memcpy(ord,bestc,(size_t)N*4);for(int i=0;i<N;i++)pos[ord[i]]=i;
      int ok=s2>cur;
      fprintf(stderr,"pass %d island %2d: child %lld, repaired %lld (%+lld)  %s  forced %ld  %.1fs\n",pass+1,k+1,(long long)s1,(long long)s2,(long long)(s2-cur),ok?"KEPT":"dropped",nforced,now()-ti);
      if(ok){cur=s2;acc_any=1;}else{memcpy(ord,saved,(size_t)N*4);for(int i=0;i<N;i++)pos[ord[i]]=i;}}
    if(!acc_any)break;}
  int64_t chk=exact_score();if(chk!=cur)fprintf(stderr,"!! final mismatch %lld vs %lld\n",(long long)cur,(long long)chk);
  dump(argv[2],chk);
  fprintf(stderr,"TMERGED %lld  (base %lld, %+lld)  %.1fs\n",(long long)chk,(long long)base0,(long long)(chk-base0),now()-t0);
  return 0;}
