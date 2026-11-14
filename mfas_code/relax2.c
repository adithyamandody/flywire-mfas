/* relax2: continuous relaxation with selectable loss, long schedules, periodic re-ranking.
   usage: relax2 graph out steps seed loss tau0 tau1 lr0 lr1 rerank [init.csv]
   loss: 0 = clamped linear (zero gradient outside band)
         1 = arctan  (gradient tau/(tau^2+d^2): heavy tails, never exactly zero)
         2 = logistic                                                         */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+1e-9*t.tv_nsec;}
#define HBITS 19
#define HSIZE (1u<<HBITS)
static uint64_t*hkey;static int32_t*hval;static int nnode=0;static uint64_t*id_of;
static inline uint64_t mix(uint64_t z){z+=0x9e3779b97f4a7c15ULL;z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;z=(z^(z>>27))*0x94d049bb133111ebULL;return z^(z>>31);}
static int32_t intern(uint64_t id){uint32_t h=mix(id)&(HSIZE-1);while(hval[h]>=0){if(hkey[h]==id)return hval[h];h=(h+1)&(HSIZE-1);}hkey[h]=id;hval[h]=nnode;id_of[nnode]=id;return nnode++;}
static int32_t lookup(uint64_t id){uint32_t h=mix(id)&(HSIZE-1);while(hval[h]>=0){if(hkey[h]==id)return hval[h];h=(h+1)&(HSIZE-1);}return -1;}
static char* slurp(const char*p,size_t*l){FILE*f=fopen(p,"rb");if(!f){perror(p);exit(1);}fseek(f,0,SEEK_END);long n=ftell(f);fseek(f,0,SEEK_SET);char*b=malloc(n+1);if(fread(b,1,n,f)!=(size_t)n)exit(1);b[n]=0;fclose(f);*l=n;return b;}
static inline uint64_t rdu64(char**p){uint64_t v=0;char*q=*p;while(*q>='0'&&*q<='9'){v=v*10+(*q-'0');q++;}*p=q;return v;}
static int32_t*src,*dst;static float*wf;static int32_t*wt;static int64_t M=0,total=0;static int N;
static uint64_t rs;static inline uint64_t rnd(void){rs^=rs<<13;rs^=rs>>7;rs^=rs<<17;return rs;}
static inline double rnd01(void){return (double)(rnd()>>11)/9007199254740992.0;}
typedef struct{float x;int32_t i;}XI;static XI*xi;
static int cmpxi(const void*a,const void*b){float p=((XI*)a)->x,q=((XI*)b)->x;return p<q?-1:(p>q?1:0);}
static float*x;static int32_t*rk;
static int64_t rank_and_score(void){
  for(int i=0;i<N;i++){xi[i].x=x[i];xi[i].i=i;}qsort(xi,N,sizeof(XI),cmpxi);
  for(int i=0;i<N;i++)rk[xi[i].i]=i;
  int64_t f=0;for(int64_t e=0;e<M;e++)if(rk[src[e]]<rk[dst[e]])f+=wt[e];return f;}
int main(int argc,char**argv){
  if(argc<11){fprintf(stderr,"usage\n");return 1;}
  int steps=atoi(argv[3]);rs=strtoull(argv[4],0,10)*0x9E3779B97F4A7C15ULL+1;int loss=atoi(argv[5]);
  double tau0=atof(argv[6]),tau1=atof(argv[7]),lr0=atof(argv[8]),lr1=atof(argv[9]);int rerank=atoi(argv[10]);
  double t0=now();
  hkey=malloc(HSIZE*8);hval=malloc(HSIZE*4);for(uint32_t i=0;i<HSIZE;i++)hval[i]=-1;id_of=malloc(400000*8);
  size_t gl;char*g=slurp(argv[1],&gl);char*p=g;while(*p&&*p!='\n')p++;p++;
  int64_t cap=8000000;src=malloc(cap*4);dst=malloc(cap*4);wt=malloc(cap*4);wf=malloc(cap*4);
  while(*p){if(*p<'0'||*p>'9'){p++;continue;}uint64_t a=rdu64(&p);p++;uint64_t b=rdu64(&p);p++;uint64_t w=rdu64(&p);
    while(*p&&*p!='\n')p++;if(*p)p++;src[M]=intern(a);dst[M]=intern(b);wt[M]=(int32_t)w;wf[M]=(float)w;M++;}
  free(g);N=nnode;for(int64_t e=0;e<M;e++)total+=wt[e];
  x=malloc(N*4);float*gr=malloc(N*4),*mm=calloc(N,4),*vv=calloc(N,4);xi=malloc(N*sizeof(XI));rk=malloc(N*4);
  if(argc>11){
    size_t ol;char*ob=slurp(argv[11],&ol);char*q=ob;while(*q&&*q!='\n')q++;q++;
    while(*q){if(*q<'0'||*q>'9'){q++;continue;}uint64_t id=rdu64(&q);q++;uint64_t o=rdu64(&q);while(*q&&*q!='\n')q++;if(*q)q++;
      x[lookup(id)]=(float)(((double)o-(N-1)*0.5)*sqrt(12.0)/N);}
    free(ob);
  }else for(int i=0;i<N;i++)x[i]=(float)(rnd01()*2.0-1.0);
  const double b1=0.9,b2=0.999;
  for(int s=1;s<=steps;s++){
    double fr=(double)s/steps,tau=tau0*pow(tau1/tau0,fr),lr=lr0*pow(lr1/lr0,fr);
    memset(gr,0,(size_t)N*4);
    const float it=(float)(1.0/tau),tf=(float)tau,t2=(float)(tau*tau);
    if(loss==0){for(int64_t e=0;e<M;e++){float z=(x[src[e]]-x[dst[e]])*it+0.5f;if(z>0.f&&z<1.f){float gw=wf[e]*it;gr[src[e]]+=gw;gr[dst[e]]-=gw;}}}
    else if(loss==1){for(int64_t e=0;e<M;e++){float d=x[src[e]]-x[dst[e]];float gw=wf[e]*tf/(t2+d*d);gr[src[e]]+=gw;gr[dst[e]]-=gw;}}
    else{for(int64_t e=0;e<M;e++){float z=(x[src[e]]-x[dst[e]])*it;if(z>30.f||z<-30.f)continue;float sg=1.f/(1.f+expf(-z));float gw=wf[e]*it*sg*(1.f-sg);gr[src[e]]+=gw;gr[dst[e]]-=gw;}}
    double bc1=1.0-pow(b1,s),bc2=1.0-pow(b2,s),mean=0.0;
    for(int i=0;i<N;i++){
      float gi=gr[i]/(float)total*1000.0f;if(gi>5.f)gi=5.f;if(gi<-5.f)gi=-5.f;
      mm[i]=(float)(b1*mm[i]+(1-b1)*gi);vv[i]=(float)(b2*vv[i]+(1-b2)*(double)gi*gi);
      x[i]-=(float)(lr*(mm[i]/bc1)/(sqrt(vv[i]/bc2)+1e-8));mean+=x[i];}
    mean/=N;double var=0.0;for(int i=0;i<N;i++){x[i]-=(float)mean;var+=(double)x[i]*x[i];}
    float sd=(float)sqrt(var/N);if(sd<1e-9f)sd=1e-9f;for(int i=0;i<N;i++)x[i]/=sd;
    if(rerank>0&&s%rerank==0&&s<steps){rank_and_score();for(int i=0;i<N;i++)x[i]=(float)(((double)rk[i]-(N-1)*0.5)*sqrt(12.0)/N);}
    if(s%(steps/8)==0){int64_t f=rank_and_score();fprintf(stderr,"step %6d tau=%.5f raw=%lld (%.3f%%) %.0fs\n",s,tau,(long long)f,100.0*f/total,now()-t0);}
  }
  int64_t f=rank_and_score();
  FILE*o=fopen(argv[2],"w");fprintf(o,"Node ID,Order\n");for(int i=0;i<N;i++)fprintf(o,"%llu,%d\n",(unsigned long long)id_of[xi[i].i],i);fclose(o);
  fprintf(stderr,"RAW %lld (%.3f%%) %.0fs\n",(long long)f,100.0*f/total,now()-t0);
  return 0;
}
