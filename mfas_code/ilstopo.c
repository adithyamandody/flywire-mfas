/* ilstopo: ilsh + TopoShuffle kicks. With probability TOPOP an iteration
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
  long ttry=0,twin=0,tplat=0,tmoved=0;int64_t tgain=0,tsh=0;double ttime=0;
  fprintf(stderr,"topo: TOPOP=%.2f TOPOL=%d TOPOJ=%d TOPOACC=%d\n",TOPOP,TOPOL,TOPOJ,TOPOACC);
  int32_t*hubs=malloc(N*4);int nhub=0;{int64_t*dg=malloc(N*8);for(int i=0;i<N;i++)dg[i]=alen[i];qs64(dg,N);
    int64_t thr=dg[(int)((1.0-HUBFRAC)*(N-1))];free(dg);for(int i=0;i<N;i++)if(alen[i]>=thr)hubs[nhub++]=i;
    fprintf(stderr,"hubs: %d neurons with degree >= %lld  HUBP=%.2f HUBMODE=%d HK=%d HUBTGT=%d\n",nhub,(long long)thr,HUBP,HUBMODE,HK,HUBTGT);}
  long htry[2]={0,0},hwin[2]={0,0},ntry=0,nwin=0;int64_t hgain[2]={0,0},ngain=0;double htime[2]={0,0},ntime=0;
  int32_t*bestord=malloc(N*4),*tpos=malloc(N*4);memcpy(bestord,ord,(size_t)N*4);
  int64_t best=cur,dumped=-1;double lastd=0;long it=0,up=0,dn=0,sinceb=0,resets=0;
  while(now()-t0<budget){
    double ti=now();double r0=(double)(rnd()>>11)/9007199254740992.0;int topo=TOPOP>0&&r0<TOPOP;int hub=!topo&&HUBP>0&&r0<TOPOP+HUBP;int htype=0;
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
    if(topo){K=0;int L=(TOPOL<=0||TOPOL>N)?N:TOPOL;int a=(int)(rnd()%(uint64_t)(N-L+1));
      int64_t ds=topo_shuffle(a,L,TOPOJ<0?L:TOPOJ);d+=ds;tsh+=ds;
      for(int i=0;i<L;i++){int32_t v=tseg[i];if(pos[v]!=a+i){push(v);push_nbrs(v);tmoved++;}}}
    for(int k=0;k<K;k++){int32_t v=(int32_t)(rnd()%(uint64_t)N);int32_t pv=pos[v],qq;
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
    {double dt=now()-ti;if(topo){ttry++;ttime+=dt;if(d>0){twin++;tgain+=d;}}else if(hub){htry[htype]++;htime[htype]+=dt;if(d>0){hwin[htype]++;hgain[htype]+=d;}}else{ntry++;ntime+=dt;if(d>0){nwin++;ngain+=d;}}}
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
  fprintf(stderr,"DONE %lld (%.4f%%)  iterations %ld\n",(long long)chk,100.0*chk/total,it);
  return 0;}
