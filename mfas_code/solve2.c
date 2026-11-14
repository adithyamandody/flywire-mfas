/* solve2: multilevel block sifting + fine sifting + iterated local search.
   Coarse level: cut the current order into runs of s consecutive neurons
   (random offset), treat each run as one super-node, and sift the runs.
   This moves whole groups at once, which single-node insertion cannot.
   usage: solve2 graph.csv out.csv secs seed init.csv                      */
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
static uint64_t*hkey;static int32_t*hval;static int nnode=0;static uint64_t*id_of;
static inline uint64_t mix(uint64_t z){z+=0x9e3779b97f4a7c15ULL;z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;z=(z^(z>>27))*0x94d049bb133111ebULL;return z^(z>>31);}
static int32_t intern(uint64_t id){uint32_t h=mix(id)&(HSIZE-1);while(hval[h]>=0){if(hkey[h]==id)return hval[h];h=(h+1)&(HSIZE-1);}hkey[h]=id;hval[h]=nnode;id_of[nnode]=id;return nnode++;}
static int32_t lookup(uint64_t id){uint32_t h=mix(id)&(HSIZE-1);while(hval[h]>=0){if(hkey[h]==id)return hval[h];h=(h+1)&(HSIZE-1);}return -1;}
static char* slurp(const char*p,size_t*l){FILE*f=fopen(p,"rb");if(!f){perror(p);exit(1);}fseek(f,0,SEEK_END);long n=ftell(f);fseek(f,0,SEEK_SET);char*b=malloc(n+1);if(fread(b,1,n,f)!=(size_t)n)exit(1);b[n]=0;fclose(f);*l=n;return b;}
static inline uint64_t rdu64(char**p){uint64_t v=0;char*q=*p;while(*q>='0'&&*q<='9'){v=v*10+(*q-'0');q++;}*p=q;return v;}

static int32_t*src,*dst,*wt;static int64_t M=0,total=0;static int N;
static int64_t*adj,*astart;static int32_t*alen;
static int32_t*ord,*pos,*saveord,*bestord,*newfine,*perm;
static int64_t*sbuf; static size_t sbufcap=0;
/* coarse */
static int64_t*cadj,*cstart,*cfill;static int32_t*clen,*cord,*cpos;
static uint64_t rs;
static inline uint64_t rnd(void){rs^=rs<<13;rs^=rs>>7;rs^=rs<<17;return rs;}

static int64_t exact_score(void){int64_t f=0;for(int64_t e=0;e<M;e++)if(pos[src[e]]<pos[dst[e]])f+=wt[e];return f;}
static void rebuild_pos(void){for(int i=0;i<N;i++)pos[ord[i]]=i;}
static void need_sbuf(size_t n){if(n>sbufcap){sbufcap=n*2;sbuf=realloc(sbuf,sbufcap*8);}}

/* generic sifting on any (sub)graph; returns total gain */
static int64_t sift_g(int n,const int64_t*A,const int64_t*S,const int32_t*L,int32_t*O,int32_t*P,int maxsweep){
  for(int i=0;i<n;i++)perm[i]=i;
  int64_t tot=0;
  for(int sw=0;sw<maxsweep;sw++){
    for(int i=n-1;i>0;i--){int j=(int)(rnd()%(uint64_t)(i+1));int32_t t=perm[i];perm[i]=perm[j];perm[j]=t;}
    int64_t gained=0;
    for(int pi=0;pi<n;pi++){
      int32_t v=perm[pi];int len=L[v];if(!len)continue;
      int64_t s0=S[v];int32_t pv=P[v];
      need_sbuf((size_t)len+2);
      for(int k=0;k<len;k++){int32_t u=(int32_t)(A[s0+k]>>32);int32_t d=(int32_t)(A[s0+k]&0xffffffffLL);
        sbuf[k]=((int64_t)P[u]<<32)|(uint32_t)d;}
      qs64(sbuf,len);
      int lo=0,hi=len;while(lo<hi){int mid=(lo+hi)>>1;if((int32_t)(sbuf[mid]>>32)<pv)lo=mid+1;else hi=mid;}
      int m=lo;
      int64_t best=0,acc=0;int32_t target=pv;
      for(int k=m;k<len;k++){acc+=(int32_t)(sbuf[k]&0xffffffffLL);if(acc>best){best=acc;target=(int32_t)(sbuf[k]>>32);}}
      acc=0;
      for(int k=m-1;k>=0;k--){acc-=(int32_t)(sbuf[k]&0xffffffffLL);if(acc>best){best=acc;target=(int32_t)(sbuf[k]>>32);}}
      if(best>0){
        if(target>pv){for(int i=pv;i<target;i++){O[i]=O[i+1];P[O[i]]=i;}}
        else{for(int i=pv;i>target;i--){O[i]=O[i-1];P[O[i]]=i;}}
        O[target]=v;P[v]=target;gained+=best;
      }
    }
    tot+=gained;
    if(!gained)break;
  }
  return tot;
}

/* one multilevel pass at block size s with offset off; returns gain (order updated) */
static int64_t coarse_pass(int s,int off,int maxsweep){
  int B=(N+off+s-1)/s;
  memset(cfill,0,(size_t)(B+1)*8);
  for(int64_t e=0;e<M;e++){int bu=(pos[src[e]]+off)/s,bv=(pos[dst[e]]+off)/s;if(bu!=bv){cfill[bu]++;cfill[bv]++;}}
  int64_t acc=0;for(int b=0;b<B;b++){cstart[b]=acc;acc+=cfill[b];cfill[b]=cstart[b];}cstart[B]=acc;
  for(int64_t e=0;e<M;e++){int bu=(pos[src[e]]+off)/s,bv=(pos[dst[e]]+off)/s;if(bu==bv)continue;int32_t w=wt[e];
    cadj[cfill[bv]++]=((int64_t)bu<<32)|(uint32_t)(w);
    cadj[cfill[bu]++]=((int64_t)bv<<32)|(uint32_t)(-w);}
  for(int b=0;b<B;b++){int64_t s0=cstart[b];int n=(int)(cstart[b+1]-s0);qs64(cadj+s0,n);int o=0;
    for(int k=0;k<n;){int32_t u=(int32_t)(cadj[s0+k]>>32);int64_t sum=0;
      while(k<n&&(int32_t)(cadj[s0+k]>>32)==u){sum+=(int32_t)(cadj[s0+k]&0xffffffffLL);k++;}
      if(sum)cadj[s0+o++]=((int64_t)u<<32)|(uint32_t)(int32_t)sum;}
    clen[b]=o;}
  for(int b=0;b<B;b++){cord[b]=b;cpos[b]=b;}
  int64_t g=sift_g(B,cadj,cstart,clen,cord,cpos,maxsweep);
  if(g>0){
    int k=0;
    for(int i=0;i<B;i++){int b=cord[i];int lo=b*s-off,hi=(b+1)*s-off;if(lo<0)lo=0;if(hi>N)hi=N;
      for(int p=lo;p<hi;p++)newfine[k++]=ord[p];}
    memcpy(ord,newfine,(size_t)N*4);rebuild_pos();
  }
  return g;
}

static void perturb(int K){
  for(int t=0;t<K;t++){int32_t v=(int32_t)(rnd()%(uint64_t)N);int32_t pv=pos[v],q=(int32_t)(rnd()%(uint64_t)N);if(q==pv)continue;
    if(q>pv){for(int i=pv;i<q;i++){ord[i]=ord[i+1];pos[ord[i]]=i;}}else{for(int i=pv;i>q;i--){ord[i]=ord[i-1];pos[ord[i]]=i;}}
    ord[q]=v;pos[v]=q;}
}
static void dump(const char*path,int64_t sc){
  char tmp[1024];snprintf(tmp,sizeof tmp,"%s.tmp",path);
  FILE*f=fopen(tmp,"w");fprintf(f,"Node ID,Order\n");
  for(int i=0;i<N;i++)fprintf(f,"%llu,%d\n",(unsigned long long)id_of[bestord[i]],i);
  fclose(f);rename(tmp,path);
  fprintf(stderr,"   [checkpoint %lld (%.4f%%)]\n",(long long)sc,100.0*sc/total);
}

static const int SIZES[]={2,3,4,5,6,8,10,12,16,20,24,32,48,64,96,128,192,256,512};
#define NS ((int)(sizeof SIZES/sizeof SIZES[0]))

int main(int argc,char**argv){
  if(argc<6){fprintf(stderr,"usage: %s graph.csv out.csv secs seed init.csv\n",argv[0]);return 1;}
  double budget=atof(argv[3]);rs=strtoull(argv[4],0,10)*2654435761ULL+1;
  double t0=now();
  hkey=malloc(HSIZE*8);hval=malloc(HSIZE*4);for(uint32_t i=0;i<HSIZE;i++)hval[i]=-1;id_of=malloc(400000*8);
  size_t gl;char*g=slurp(argv[1],&gl);char*p=g;while(*p&&*p!='\n')p++;p++;
  int64_t cap=8000000;src=malloc(cap*4);dst=malloc(cap*4);wt=malloc(cap*4);
  while(*p){if(*p<'0'||*p>'9'){p++;continue;}uint64_t a=rdu64(&p);p++;uint64_t b=rdu64(&p);p++;uint64_t w=rdu64(&p);
    while(*p&&*p!='\n')p++;if(*p)p++;src[M]=intern(a);dst[M]=intern(b);wt[M]=(int32_t)w;M++;}
  free(g);N=nnode;for(int64_t e=0;e<M;e++)total+=wt[e];
  astart=malloc((N+1)*8);alen=malloc(N*4);int64_t*cnt=calloc(N+1,8);
  for(int64_t e=0;e<M;e++){cnt[src[e]]++;cnt[dst[e]]++;}
  int64_t acc=0;for(int i=0;i<N;i++){astart[i]=acc;acc+=cnt[i];}astart[N]=acc;
  adj=malloc(acc*8);int64_t*fill=malloc(N*8);memcpy(fill,astart,(size_t)N*8);
  for(int64_t e=0;e<M;e++){int32_t u=src[e],v=dst[e],w=wt[e];adj[fill[v]++]=((int64_t)u<<32)|(uint32_t)(w);adj[fill[u]++]=((int64_t)v<<32)|(uint32_t)(-w);}
  free(fill);free(cnt);
  for(int v=0;v<N;v++){int64_t s0=astart[v];int n=(int)(astart[v+1]-s0);qs64(adj+s0,n);int o=0;
    for(int k=0;k<n;){int32_t u=(int32_t)(adj[s0+k]>>32);int64_t sum=0;while(k<n&&(int32_t)(adj[s0+k]>>32)==u){sum+=(int32_t)(adj[s0+k]&0xffffffffLL);k++;}
      adj[s0+o++]=((int64_t)u<<32)|(uint32_t)(int32_t)sum;}alen[v]=o;}
  ord=malloc(N*4);pos=malloc(N*4);saveord=malloc(N*4);bestord=malloc(N*4);newfine=malloc(N*4);perm=malloc((N+1)*4);
  cadj=malloc((size_t)2*M*8);cstart=malloc((size_t)(N+2)*8);cfill=malloc((size_t)(N+2)*8);
  clen=malloc((N+1)*4);cord=malloc((N+1)*4);cpos=malloc((N+1)*4);
  need_sbuf(1<<16);

  size_t ol;char*ob=slurp(argv[5],&ol);char*q=ob;while(*q&&*q!='\n')q++;q++;
  while(*q){if(*q<'0'||*q>'9'){q++;continue;}uint64_t id=rdu64(&q);q++;uint64_t o=rdu64(&q);while(*q&&*q!='\n')q++;if(*q)q++;ord[o]=lookup(id);}
  free(ob);rebuild_pos();
  int64_t cur=exact_score();
  fprintf(stderr,"start %lld (%.4f%%)  load %.1fs\n",(long long)cur,100.0*cur/total,now()-t0);
  cur+=sift_g(N,adj,astart,alen,ord,pos,400);
  int64_t best=cur;memcpy(bestord,ord,(size_t)N*4);
  fprintf(stderr,"fine-sifted %lld\n",(long long)cur);

  int64_t lvl_gain[NS];int lvl_hits[NS],lvl_tries[NS];memset(lvl_gain,0,sizeof lvl_gain);memset(lvl_hits,0,sizeof lvl_hits);memset(lvl_tries,0,sizeof lvl_tries);
  int64_t kick_gain=0;int kick_wins=0,kick_tries=0;
  double lastdump=now(); int64_t dumped=-1; int it=0, dry=0;
  while(now()-t0<budget){
    it++;
    if(dry<2*NS){
      int li=(int)(rnd()%NS);int s=SIZES[li];int off=(int)(rnd()%(uint64_t)s);
      lvl_tries[li]++;
      int64_t gc=coarse_pass(s,off,30);
      if(gc>0){int64_t gf=sift_g(N,adj,astart,alen,ord,pos,400);cur+=gc+gf;lvl_gain[li]+=gc+gf;lvl_hits[li]++;dry=0;}
      else dry++;
    }else{
      memcpy(saveord,ord,(size_t)N*4);int64_t save=cur;
      perturb(30+(int)(rnd()%300));cur=exact_score();
      cur+=sift_g(N,adj,astart,alen,ord,pos,400);
      for(int r=0;r<4;r++){int li=(int)(rnd()%NS);int s=SIZES[li];int64_t gc=coarse_pass(s,(int)(rnd()%(uint64_t)s),30);
        if(gc>0)cur+=gc+sift_g(N,adj,astart,alen,ord,pos,400);}
      kick_tries++;
      if(cur>save){kick_wins++;kick_gain+=cur-save;dry=NS;}       /* re-run some coarse passes after a win */
      else if(cur<save){memcpy(ord,saveord,(size_t)N*4);rebuild_pos();cur=save;}
    }
    if(cur>best){best=cur;memcpy(bestord,ord,(size_t)N*4);}
    if(best>dumped && now()-lastdump>30){
      int64_t chk=0;{for(int i=0;i<N;i++)newfine[bestord[i]]=i;for(int64_t e=0;e<M;e++)if(newfine[src[e]]<newfine[dst[e]])chk+=wt[e];}
      if(chk!=best){fprintf(stderr,"!! score mismatch tracked=%lld exact=%lld\n",(long long)best,(long long)chk);best=chk;}
      dump(argv[2],best);dumped=best;lastdump=now();
      fprintf(stderr,"it %d  best %lld  (+%lld from start)  %.0fs\n",it,(long long)best,(long long)(best-0),now()-t0);
    }
  }
  if(best>dumped)dump(argv[2],best);
  fprintf(stderr,"\n--- gains by block size ---\n");
  for(int i=0;i<NS;i++)if(lvl_tries[i])fprintf(stderr,"s=%4d  tries=%5d hits=%5d  gain=%lld\n",SIZES[i],lvl_tries[i],lvl_hits[i],(long long)lvl_gain[i]);
  fprintf(stderr,"kicks: tries=%d wins=%d gain=%lld\n",kick_tries,kick_wins,(long long)kick_gain);
  fprintf(stderr,"DONE best=%lld (%.4f%%)\n",(long long)best,100.0*best/total);
  return 0;
}
