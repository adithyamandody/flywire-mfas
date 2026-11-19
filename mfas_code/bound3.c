/* bound3: upper bound on the maximum forward weight, by greedy cycle packing.

   In any ordering every directed cycle has at least one backward edge. Give
   each cycle C a weight y_C >= 0 such that for every edge e the cycles through
   e use at most w_e of it. Then for any ordering

       backward weight  =  sum over backward e of w_e
                        >= sum over backward e of (y through e)
                        >= sum_C y_C                    (each C has a backward e)

   so forward <= total - sum_C y_C. bound2.py is the special case of 2-cycles
   only. Here we keep a residual capacity per edge and push cycles greedily:
   2-cycles first (2 edges spent per unit of bound), then 3-cycles, then longer
   ones found by bounded BFS in what is left.

   The 3-cycle sweep is repeated with a fresh random node order each pass:
   a greedy pass blocks cycles it could have taken in a different order, so
   later passes still find more. The search for longer cycles visits at most
   VISITCAP nodes per attempt, which keeps one hub from eating the budget.

   With CYCLEFILE set in the environment, every cycle pushed is written there
   as "weight id1 id2 ..." so an independent script can check that each line is
   a real directed cycle and that no connection is used beyond its weight.

   usage: bound3 graph.csv [seconds] [maxdepth] [passes] [visitcap]            */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+1e-9*t.tv_nsec;}
static void isort64(int64_t*a,int n){for(int i=1;i<n;i++){int64_t x=a[i];int j=i-1;while(j>=0&&a[j]>x){a[j+1]=a[j];j--;}a[j+1]=x;}}
static void qs64(int64_t*a,int n){while(n>24){int64_t x=a[0],y=a[n/2],z=a[n-1],p;if(x>y){int64_t t=x;x=y;y=t;}if(y>z)y=z;if(x>y)y=x;p=y;
 int i=0,j=n-1;while(i<=j){while(a[i]<p)i++;while(a[j]>p)j--;if(i<=j){int64_t t=a[i];a[i]=a[j];a[j]=t;i++;j--;}}
 if(j+1<n-i){qs64(a,j+1);a+=i;n-=i;}else{qs64(a+i,n-i);n=j+1;}}isort64(a,n);}

#define HBITS 19
#define HSIZE (1u<<HBITS)
static uint64_t*hkey;static int32_t*hval;static int nnode=0;static uint64_t*id_of;static FILE*cycf=NULL;
static inline uint64_t mix(uint64_t z){z+=0x9e3779b97f4a7c15ULL;z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;z=(z^(z>>27))*0x94d049bb133111ebULL;return z^(z>>31);}
static int32_t intern(uint64_t id){uint32_t h=mix(id)&(HSIZE-1);while(hval[h]>=0){if(hkey[h]==id)return hval[h];h=(h+1)&(HSIZE-1);}hkey[h]=id;hval[h]=nnode;id_of[nnode]=id;return nnode++;}
static char* slurp(const char*p,size_t*l){FILE*f=fopen(p,"rb");if(!f){perror(p);exit(1);}fseek(f,0,SEEK_END);long n=ftell(f);fseek(f,0,SEEK_SET);char*b=malloc(n+1);if(fread(b,1,n,f)!=(size_t)n)exit(1);b[n]=0;fclose(f);*l=n;return b;}
static inline uint64_t rdu64(char**p){uint64_t v=0;char*q=*p;while(*q>='0'&&*q<='9'){v=v*10+(*q-'0');q++;}*p=q;return v;}

static uint64_t rs=88172645463325252ULL;
static inline uint64_t rnd(void){rs^=rs<<13;rs^=rs>>7;rs^=rs<<17;return rs;}

static int N;static int64_t M=0,total=0;
static int64_t*ostart,*istart;
static int32_t *odst,*ores,*owt,*isrc,*ieid;

/* index of edge u->v in the out arrays, or -1 */
static inline int64_t find_edge(int32_t u,int32_t v){
  int64_t lo=ostart[u],hi=ostart[u+1];
  while(lo<hi){int64_t m=(lo+hi)>>1;if(odst[m]<v)lo=m+1;else hi=m;}
  return (lo<ostart[u+1]&&odst[lo]==v)?lo:-1;}

int main(int argc,char**argv){
  if(argc<2){fprintf(stderr,"usage: %s graph.csv [seconds] [maxdepth]\n",argv[0]);return 1;}
  double budget=argc>2?atof(argv[2]):600.0;int MAXD=argc>3?atoi(argv[3]):6;
  int PASSES=argc>4?atoi(argv[4]):1;int VISITCAP=argc>5?atoi(argv[5]):1000000;if(VISITCAP<16)VISITCAP=16;
  double t0=now();
  hkey=malloc(HSIZE*8);hval=malloc(HSIZE*4);for(uint32_t i=0;i<HSIZE;i++)hval[i]=-1;id_of=malloc(400000*8);
  {const char*cf=getenv("CYCLEFILE");if(cf){cycf=fopen(cf,"w");if(!cycf){perror(cf);return 1;}}}
  size_t gl;char*g=slurp(argv[1],&gl);char*p=g;while(*p&&*p!='\n')p++;p++;
  int64_t cap=8000000;int32_t*esrc=malloc(cap*4),*edst=malloc(cap*4),*ewt=malloc(cap*4);
  while(*p){if(*p<'0'||*p>'9'){p++;continue;}uint64_t a=rdu64(&p);p++;uint64_t b=rdu64(&p);p++;uint64_t w=rdu64(&p);
    while(*p&&*p!='\n')p++;if(*p)p++;
    if(M==cap){cap*=2;esrc=realloc(esrc,cap*4);edst=realloc(edst,cap*4);ewt=realloc(ewt,cap*4);}
    esrc[M]=intern(a);edst[M]=intern(b);ewt[M]=(int32_t)w;M++;}
  free(g);N=nnode;for(int64_t e=0;e<M;e++)total+=ewt[e];
  fprintf(stderr,"%d neurons, %lld edges, total weight %lld  (%.1fs)\n",N,(long long)M,(long long)total,now()-t0);

  /* out lists, sorted by destination, parallel weights merged for duplicates */
  ostart=calloc(N+2,8);for(int64_t e=0;e<M;e++)ostart[esrc[e]+1]++;
  for(int i=0;i<N;i++)ostart[i+1]+=ostart[i];
  int64_t*buf=malloc((size_t)M*8);int64_t*fill=malloc((size_t)(N+1)*8);memcpy(fill,ostart,(size_t)(N+1)*8);
  for(int64_t e=0;e<M;e++)buf[fill[esrc[e]]++]=((int64_t)edst[e]<<32)|(uint32_t)ewt[e];
  odst=malloc((size_t)M*4);ores=malloc((size_t)M*4);owt=malloc((size_t)M*4);
  int64_t m2=0;int64_t*ns=malloc((size_t)(N+2)*8);
  for(int u=0;u<N;u++){int64_t s=ostart[u],n=ostart[u+1]-s;qs64(buf+s,(int)n);ns[u]=m2;
    for(int64_t k=0;k<n;){int32_t v=(int32_t)(buf[s+k]>>32);int64_t sum=0;
      while(k<n&&(int32_t)(buf[s+k]>>32)==v){sum+=(int32_t)(buf[s+k]&0xffffffffLL);k++;}
      odst[m2]=v;owt[m2]=(int32_t)sum;ores[m2]=(int32_t)sum;m2++;}}
  ns[N]=m2;memcpy(ostart,ns,(size_t)(N+1)*8);free(ns);
  fprintf(stderr,"%lld distinct connections after merging duplicates  (%.1fs)\n",(long long)m2,now()-t0);
  M=m2;free(esrc);free(edst);free(ewt);

  /* in lists: (source, index into the out arrays), sorted by source */
  istart=calloc(N+2,8);for(int u=0;u<N;u++)for(int64_t k=ostart[u];k<ostart[u+1];k++)istart[odst[k]+1]++;
  for(int i=0;i<N;i++)istart[i+1]+=istart[i];
  memcpy(fill,istart,(size_t)(N+1)*8);
  for(int u=0;u<N;u++)for(int64_t k=ostart[u];k<ostart[u+1];k++)buf[fill[odst[k]]++]=((int64_t)u<<32)|(uint32_t)k;
  isrc=malloc((size_t)M*4);ieid=malloc((size_t)M*4);
  for(int v=0;v<N;v++){int64_t s=istart[v],n=istart[v+1]-s;qs64(buf+s,(int)n);
    for(int64_t k=0;k<n;k++){isrc[s+k]=(int32_t)(buf[s+k]>>32);ieid[s+k]=(int32_t)(buf[s+k]&0xffffffffLL);}}
  free(buf);free(fill);
  fprintf(stderr,"adjacency built  (%.1fs)\n",now()-t0);

  int64_t LB=0;long n2=0,n3=0,nlong=0;

  /* ---- 2-cycles ---- */
  for(int u=0;u<N;u++)for(int64_t k=ostart[u];k<ostart[u+1];k++){
    int32_t v=odst[k];if(v<=u)continue;
    int64_t k2=find_edge(v,u);if(k2<0)continue;
    int32_t push=ores[k]<ores[k2]?ores[k]:ores[k2];
    if(push>0){ores[k]-=push;ores[k2]-=push;LB+=push;n2++;
      if(cycf)fprintf(cycf,"%d %llu %llu\n",push,(unsigned long long)id_of[u],(unsigned long long)id_of[v]);}}
  fprintf(stderr,"2-cycles: %ld used, forced backward %lld -> bound %lld (%.3f%%)  (%.1fs)\n",
    n2,(long long)LB,(long long)(total-LB),100.0*(total-LB)/total,now()-t0);

  /* ---- 3-cycles: cheapest nodes first, so a time limit costs least ---- */
  int64_t*ordv=malloc((size_t)N*8);
  for(int v=0;v<N;v++){int64_t din=istart[v+1]-istart[v],dout=ostart[v+1]-ostart[v];
    int64_t c=din*dout;if(c>0x7fffffffLL)c=0x7fffffffLL;ordv[v]=(c<<32)|(uint32_t)v;}
  qs64(ordv,N);
  int64_t work=0;
  for(int pass=0;pass<PASSES;pass++){
   long n3was=n3;int64_t lbwas=LB;
   if(pass){for(int i=N-1;i>0;i--){int j=(int)(rnd()%(uint64_t)(i+1));int64_t t=ordv[i];ordv[i]=ordv[j];ordv[j]=t;}}
   for(int i=0;i<N;i++){
    int32_t v=(int32_t)(ordv[i]&0xffffffffLL);
    if(now()-t0>budget*0.6){fprintf(stderr,"3-cycles: time limit in pass %d after %d of %d nodes\n",pass+1,i,N);break;}
    for(int64_t a=istart[v];a<istart[v+1];a++){
      int64_t e1=ieid[a];if(ores[e1]<=0)continue;int32_t u=isrc[a];
      for(int64_t b=ostart[v];b<ostart[v+1];b++){
        int32_t w=odst[b];if(w==u)continue;int64_t e2=b;if(ores[e2]<=0)continue;
        work++;
        int64_t e3=find_edge(w,u);if(e3<0||ores[e3]<=0)continue;
        int32_t push=ores[e1];if(ores[e2]<push)push=ores[e2];if(ores[e3]<push)push=ores[e3];
        ores[e1]-=push;ores[e2]-=push;ores[e3]-=push;LB+=push;n3++;
        if(cycf)fprintf(cycf,"%d %llu %llu %llu\n",push,(unsigned long long)id_of[u],(unsigned long long)id_of[v],(unsigned long long)id_of[w]);
        if(ores[e1]<=0)break;}}}
   fprintf(stderr,"3-cycles pass %d: +%ld cycles, +%lld forced -> bound %lld (%.3f%%)  (%.1fs)\n",
     pass+1,n3-n3was,(long long)(LB-lbwas),(long long)(total-LB),100.0*(total-LB)/total,now()-t0);
   if(LB-lbwas<total/2000000)break;
   if(now()-t0>budget*0.6)break;}
  fprintf(stderr,"3-cycles: %ld used in total (%lld pair probes)  (%.1fs)\n",n3,(long long)work,now()-t0);

  /* ---- longer cycles: for an edge u->v with capacity left, BFS v back to u ---- */
  if(VISITCAP>N)VISITCAP=N;
  int32_t*par=malloc((size_t)N*4),*pare=malloc((size_t)N*4),*stampv=calloc(N,4),*q=malloc((size_t)N*4);
  int32_t*dist=malloc((size_t)N*4);int32_t stamp=0;int32_t*cyc=malloc((size_t)N*4);
  int64_t*live=malloc((size_t)M*8);int64_t nlive=0;
  for(int u=0;u<N;u++)for(int64_t k=ostart[u];k<ostart[u+1];k++)if(ores[k]>0)live[nlive++]=((int64_t)u<<32)|(uint32_t)k;
  fprintf(stderr,"%lld connections still have capacity; search phase, max depth %d, visit cap %d\n",(long long)nlive,MAXD,VISITCAP);
  long tried=0;
  for(int64_t t=0;t<nlive;t++){
    if((t&255)==0&&now()-t0>budget)break;
    int64_t j=t+(int64_t)(rnd()%(uint64_t)(nlive-t));int64_t tmp=live[t];live[t]=live[j];live[j]=tmp;
    int32_t u=(int32_t)(live[t]>>32);int64_t e0=(int32_t)(live[t]&0xffffffffLL);
    if(ores[e0]<=0)continue;int32_t v=odst[e0];
    tried++;
    /* BFS from v, over edges with capacity left, looking for u */
    stamp++;int qh=0,qt=0;q[qt++]=v;stampv[v]=stamp;par[v]=-1;dist[v]=0;int found=0;
    while(qh<qt&&!found&&qt<VISITCAP){int32_t x=q[qh++];if(dist[x]>=MAXD-1)continue;
      for(int64_t k=ostart[x];k<ostart[x+1];k++){
        if(ores[k]<=0)continue;int32_t y=odst[k];
        if(y==u){par[u]=x;pare[u]=(int32_t)k;found=1;break;}
        if(stampv[y]==stamp)continue;
        stampv[y]=stamp;par[y]=x;pare[y]=(int32_t)k;dist[y]=dist[x]+1;q[qt++]=y;if(qt>=VISITCAP)break;}}
    if(!found)continue;
    int32_t push=ores[e0];
    for(int32_t x=u;x!=v;x=par[x]){int32_t e=pare[x];if(ores[e]<push)push=ores[e];}
    if(push<=0)continue;
    ores[e0]-=push;
    for(int32_t x=u;x!=v;x=par[x])ores[pare[x]]-=push;
    LB+=push;nlong++;
    if(cycf){int nc=0;for(int32_t x=u;x!=v;x=par[x])cyc[nc++]=x;cyc[nc++]=v;
      fprintf(cycf,"%d",push);for(int i=nc-1;i>=0;i--)fprintf(cycf," %llu",(unsigned long long)id_of[cyc[i]]);fputc('\n',cycf);}}
  fprintf(stderr,"longer cycles: %ld used out of %ld edges tried  (%.1fs)\n",nlong,tried,now()-t0);

  /* ---- check the packing is legal: no edge over-used ---- */
  int64_t used=0,bad=0;
  for(int64_t k=0;k<M;k++){if(ores[k]<0||ores[k]>owt[k])bad++;used+=owt[k]-ores[k];}
  printf("\nneurons %d, connections %lld, total weight %lld\n",N,(long long)M,(long long)total);
  printf("cycles packed: %ld of length 2, %ld of length 3, %ld longer\n",n2,n3,nlong);
  printf("capacity used %lld of %lld; over-used edges %lld (must be 0)\n",(long long)used,(long long)total,(long long)bad);
  printf("weight forced backward (any ordering)  %lld\n",(long long)LB);
  printf("UPPER BOUND on forward weight          %lld  (%.3f%%)\n",(long long)(total-LB),100.0*(total-LB)/total);
  if(cycf){fclose(cycf);fprintf(stderr,"cycles written to %s\n",getenv("CYCLEFILE"));}
  return bad?2:0;}
