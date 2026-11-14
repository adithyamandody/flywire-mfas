/* Hybrid feedforward sort: continuous relaxation + sifting local search + ILS.
   usage: solve graph.csv out.csv [seconds] [seed] [resume.csv]            */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+1e-9*t.tv_nsec;}

/* ---- sort ---- */
static void isort64(int64_t*a,int n){for(int i=1;i<n;i++){int64_t x=a[i];int j=i-1;while(j>=0&&a[j]>x){a[j+1]=a[j];j--;}a[j+1]=x;}}
static void qs64(int64_t*a,int n){while(n>24){int64_t x=a[0],y=a[n/2],z=a[n-1],p;if(x>y){int64_t t=x;x=y;y=t;}if(y>z)y=z;if(x>y)y=x;p=y;
 int i=0,j=n-1;while(i<=j){while(a[i]<p)i++;while(a[j]>p)j--;if(i<=j){int64_t t=a[i];a[i]=a[j];a[j]=t;i++;j--;}}
 if(j+1<n-i){qs64(a,j+1);a+=i;n-=i;}else{qs64(a+i,n-i);n=j+1;}}isort64(a,n);}

/* ---- id map ---- */
#define HBITS 19
#define HSIZE (1u<<HBITS)
static uint64_t*hkey;static int32_t*hval;static int nnode=0;static uint64_t*id_of;
static inline uint64_t mix(uint64_t z){z+=0x9e3779b97f4a7c15ULL;z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;z=(z^(z>>27))*0x94d049bb133111ebULL;return z^(z>>31);}
static int32_t intern(uint64_t id){uint32_t h=mix(id)&(HSIZE-1);while(hval[h]>=0){if(hkey[h]==id)return hval[h];h=(h+1)&(HSIZE-1);}hkey[h]=id;hval[h]=nnode;id_of[nnode]=id;return nnode++;}
static int32_t lookup(uint64_t id){uint32_t h=mix(id)&(HSIZE-1);while(hval[h]>=0){if(hkey[h]==id)return hval[h];h=(h+1)&(HSIZE-1);}return -1;}
static char* slurp(const char*p,size_t*l){FILE*f=fopen(p,"rb");if(!f){perror(p);exit(1);}fseek(f,0,SEEK_END);long n=ftell(f);fseek(f,0,SEEK_SET);char*b=malloc(n+1);if(fread(b,1,n,f)!=(size_t)n)exit(1);b[n]=0;fclose(f);*l=n;return b;}
static inline uint64_t rdu64(char**p){uint64_t v=0;char*q=*p;while(*q>='0'&&*q<='9'){v=v*10+(*q-'0');q++;}*p=q;return v;}

/* ---- globals ---- */
static int32_t*src,*dst,*wt;static int64_t M=0,total=0;static int N;
static int64_t*adj,*astart;static int32_t*alen;static int maxdeg=0;
static int32_t*ord,*pos,*tmpord,*bestord;
static float*x,*gr,*mm,*vv;
static int64_t*sbuf;
static uint64_t rs;
static inline uint64_t rnd(void){rs^=rs<<13;rs^=rs>>7;rs^=rs<<17;return rs;}
static inline double rnd01(void){return (double)(rnd()>>11)/9007199254740992.0;}
typedef struct{float x;int32_t i;}XI; static XI*xi;
static int cmpxi(const void*a,const void*b){float p=((XI*)a)->x,q=((XI*)b)->x;return p<q?-1:(p>q?1:0);}

static int64_t exact_score(void){int64_t f=0;for(int64_t e=0;e<M;e++)if(pos[src[e]]<pos[dst[e]])f+=wt[e];return f;}
static void rebuild_pos(void){for(int i=0;i<N;i++)pos[ord[i]]=i;}

/* ---- sifting to convergence ---- */
static int64_t sift(int64_t fwd,int maxsweep){
  static int32_t*perm=NULL; if(!perm){perm=malloc(N*4);for(int i=0;i<N;i++)perm[i]=i;}
  for(int sw=0;sw<maxsweep;sw++){
    for(int i=N-1;i>0;i--){int j=(int)(rnd()%(uint64_t)(i+1));int32_t t=perm[i];perm[i]=perm[j];perm[j]=t;}
    int64_t gained=0;
    for(int pi=0;pi<N;pi++){
      int32_t v=perm[pi];int n=alen[v];if(!n)continue;
      int64_t s=astart[v];int32_t pv=pos[v];
      for(int k=0;k<n;k++){int32_t u=(int32_t)(adj[s+k]>>32);int32_t d=(int32_t)(adj[s+k]&0xffffffffLL);
        sbuf[k]=((int64_t)pos[u]<<32)|(uint32_t)d;}
      qs64(sbuf,n);
      int m=0;while(m<n&&(int32_t)(sbuf[m]>>32)<pv)m++;
      int64_t best=0;int32_t target=pv,a2=0;
      for(int k=m;k<n;k++){a2+=(int32_t)(sbuf[k]&0xffffffffLL);if(a2>best){best=a2;target=(int32_t)(sbuf[k]>>32);}}
      a2=0;
      for(int k=m-1;k>=0;k--){a2-=(int32_t)(sbuf[k]&0xffffffffLL);if(a2>best){best=a2;target=(int32_t)(sbuf[k]>>32);}}
      if(best>0){
        if(target>pv){for(int i=pv;i<target;i++){ord[i]=ord[i+1];pos[ord[i]]=i;}}
        else{for(int i=pv;i>target;i--){ord[i]=ord[i-1];pos[ord[i]]=i;}}
        ord[target]=v;pos[v]=target;fwd+=best;gained+=best;
      }
    }
    if(!gained)break;
  }
  return fwd;
}

/* ---- relaxation; init_from_order=0 -> random x ---- */
static void relax(int steps,double tau0,double tau1,double lr0,double lr1,int init_from_order){
  if(init_from_order){ double sc=6.0/N; for(int i=0;i<N;i++) x[i]=(float)((pos[i]-N*0.5)*sc); }
  else { for(int i=0;i<N;i++) x[i]=(float)(rnd01()*2.0-1.0); }
  for(int i=0;i<N;i++){mm[i]=0;vv[i]=0;}
  double b1=0.9,b2=0.999;
  for(int s=1;s<=steps;s++){
    double fr=(double)s/steps, tau=tau0*pow(tau1/tau0,fr), lr=lr0*pow(lr1/lr0,fr);
    memset(gr,0,(size_t)N*4);
    float itau=(float)(1.0/tau);
    for(int64_t e=0;e<M;e++){
      float z=(x[src[e]]-x[dst[e]])*itau+0.5f;
      if(z>0.f&&z<1.f){float gw=(float)wt[e]*itau;gr[src[e]]+=gw;gr[dst[e]]-=gw;}
    }
    double bc1=1.0-pow(b1,s),bc2=1.0-pow(b2,s),mean=0.0;
    for(int i=0;i<N;i++){
      float gi=gr[i]/(float)total*1000.0f;
      if(gi>5.f)gi=5.f; if(gi<-5.f)gi=-5.f;
      mm[i]=(float)(b1*mm[i]+(1-b1)*gi);
      vv[i]=(float)(b2*vv[i]+(1-b2)*(double)gi*gi);
      x[i]-=(float)(lr*(mm[i]/bc1)/(sqrt(vv[i]/bc2)+1e-8));
      mean+=x[i];
    }
    mean/=N;double var=0.0;
    for(int i=0;i<N;i++){x[i]-=(float)mean;var+=(double)x[i]*x[i];}
    float sd=(float)sqrt(var/N);if(sd<1e-9f)sd=1e-9f;
    for(int i=0;i<N;i++)x[i]/=sd;
  }
  for(int i=0;i<N;i++){xi[i].x=x[i];xi[i].i=i;}
  qsort(xi,N,sizeof(XI),cmpxi);
  for(int i=0;i<N;i++)ord[i]=xi[i].i;
  rebuild_pos();
}

/* ---- perturbation: relocate K random nodes ---- */
static void perturb(int K){
  for(int t=0;t<K;t++){
    int32_t v=(int32_t)(rnd()%(uint64_t)N);
    int32_t pv=pos[v], q=(int32_t)(rnd()%(uint64_t)N);
    if(q==pv)continue;
    if(q>pv){for(int i=pv;i<q;i++){ord[i]=ord[i+1];pos[ord[i]]=i;}}
    else{for(int i=pv;i>q;i--){ord[i]=ord[i-1];pos[ord[i]]=i;}}
    ord[q]=v;pos[v]=q;
  }
}

static void dump(const char*path,int32_t*o,int64_t sc){
  char tmp[1024];snprintf(tmp,sizeof tmp,"%s.tmp",path);
  FILE*f=fopen(tmp,"w");fprintf(f,"Node ID,Order\n");
  for(int i=0;i<N;i++)fprintf(f,"%llu,%d\n",(unsigned long long)id_of[o[i]],i);
  fclose(f);rename(tmp,path);
  fprintf(stderr,"   [checkpoint %lld (%.4f%%)]\n",(long long)sc,100.0*sc/total);
}

int main(int argc,char**argv){
  if(argc<3){fprintf(stderr,"usage: %s graph.csv out.csv [secs] [seed] [resume.csv]\n",argv[0]);return 1;}
  double budget=argc>3?atof(argv[3]):3600.0;
  rs=argc>4?strtoull(argv[4],0,10):99;if(!rs)rs=99;
  double t0=now();
  hkey=malloc(HSIZE*8);hval=malloc(HSIZE*4);for(uint32_t i=0;i<HSIZE;i++)hval[i]=-1;id_of=malloc(400000*8);
  size_t gl;char*g=slurp(argv[1],&gl);char*p=g;while(*p&&*p!='\n')p++;p++;
  int64_t cap=8000000;src=malloc(cap*4);dst=malloc(cap*4);wt=malloc(cap*4);
  while(*p){if(*p<'0'||*p>'9'){p++;continue;}
    uint64_t a=rdu64(&p);p++;uint64_t b=rdu64(&p);p++;uint64_t w=rdu64(&p);
    while(*p&&*p!='\n')p++;if(*p)p++;
    src[M]=intern(a);dst[M]=intern(b);wt[M]=(int32_t)w;M++;}
  free(g);N=nnode;for(int64_t e=0;e<M;e++)total+=wt[e];

  astart=malloc((N+1)*8);alen=malloc(N*4);
  int64_t*cnt=calloc(N+1,8);
  for(int64_t e=0;e<M;e++){cnt[src[e]]++;cnt[dst[e]]++;}
  int64_t acc=0;for(int i=0;i<N;i++){astart[i]=acc;acc+=cnt[i];}astart[N]=acc;
  adj=malloc(acc*8);int64_t*fill=malloc(N*8);memcpy(fill,astart,(size_t)N*8);
  for(int64_t e=0;e<M;e++){int32_t u=src[e],v=dst[e],w=wt[e];
    adj[fill[v]++]=((int64_t)u<<32)|(uint32_t)(w);
    adj[fill[u]++]=((int64_t)v<<32)|(uint32_t)(-w);}
  free(fill);free(cnt);
  for(int v=0;v<N;v++){int64_t s=astart[v];int n=(int)(astart[v+1]-s);qs64(adj+s,n);int o=0;
    for(int k=0;k<n;){int32_t u=(int32_t)(adj[s+k]>>32);int64_t sum=0;
      while(k<n&&(int32_t)(adj[s+k]>>32)==u){sum+=(int32_t)(adj[s+k]&0xffffffffLL);k++;}
      adj[s+o++]=((int64_t)u<<32)|(uint32_t)(int32_t)sum;}
    alen[v]=o;if(o>maxdeg)maxdeg=o;}
  fprintf(stderr,"%lld edges %d nodes total=%lld maxdeg=%d (%.1fs)\n",(long long)M,N,(long long)total,maxdeg,now()-t0);

  ord=malloc(N*4);pos=malloc(N*4);tmpord=malloc(N*4);bestord=malloc(N*4);
  x=malloc(N*4);gr=malloc(N*4);mm=malloc(N*4);vv=malloc(N*4);
  xi=malloc(N*sizeof(XI));sbuf=malloc((size_t)(maxdeg+2)*8);

  int64_t cur;
  if(argc>5){
    size_t ol;char*ob=slurp(argv[5],&ol);char*q=ob;while(*q&&*q!='\n')q++;q++;
    while(*q){if(*q<'0'||*q>'9'){q++;continue;}uint64_t id=rdu64(&q);q++;uint64_t o=rdu64(&q);
      while(*q&&*q!='\n')q++;if(*q)q++;int32_t v=lookup(id);ord[o]=v;}
    free(ob);rebuild_pos();cur=exact_score();
    fprintf(stderr,"resumed from %s: %lld (%.4f%%)\n",argv[5],(long long)cur,100.0*cur/total);
  }else{
    relax(6000,2.0,0.0015,0.02,0.0008,0);
    cur=exact_score();
    fprintf(stderr,"after relax: %lld (%.4f%%)  %.1fs\n",(long long)cur,100.0*cur/total,now()-t0);
  }
  cur=sift(cur,400);
  int64_t best=cur;memcpy(bestord,ord,(size_t)N*4);
  fprintf(stderr,"after sift : %lld (%.4f%%)  %.1fs\n",(long long)cur,100.0*cur/total,now()-t0);
  dump(argv[2],bestord,best);

  int it=0;
  while(now()-t0<budget){
    it++;
    memcpy(tmpord,ord,(size_t)N*4);int64_t save=cur;
    int mode=it%4;
    if(mode==3){ relax(400,0.030,0.0020,0.004,0.0005,1); cur=exact_score(); }
    else       { perturb(60+(int)(rnd()%400)); cur=exact_score(); }
    cur=sift(cur,400);
    if(cur>best){ best=cur;memcpy(bestord,ord,(size_t)N*4);
      fprintf(stderr,"it %5d %s NEW BEST %lld (%.4f%%)  %.0fs\n",it,mode==3?"relax":"kick ",(long long)best,100.0*best/total,now()-t0);
      dump(argv[2],bestord,best);
    } else if(cur<save){ memcpy(ord,tmpord,(size_t)N*4);rebuild_pos();cur=save; }
    if(it%25==0) fprintf(stderr,"it %5d cur=%lld best=%lld (%.4f%%) %.0fs\n",it,(long long)cur,(long long)best,100.0*best/total,now()-t0);
  }
  fprintf(stderr,"DONE best=%lld (%.4f%%)\n",(long long)best,100.0*best/total);
  return 0;
}
