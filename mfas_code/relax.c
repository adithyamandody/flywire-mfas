// Continuous relaxation for feedforward ordering.
// Each node gets a real position x; soft backward-weight loss; Adam; tau annealing.
// usage: relax graph.csv out_order.csv [steps] [seed] [restarts]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+1e-9*t.tv_nsec;}

#define HBITS 19
#define HSIZE (1u<<HBITS)
static uint64_t *hkey; static int32_t *hval; static int nnode=0; static uint64_t *id_of;
static inline uint64_t mix(uint64_t z){z+=0x9e3779b97f4a7c15ULL;z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;z=(z^(z>>27))*0x94d049bb133111ebULL;return z^(z>>31);}
static int32_t intern(uint64_t id){uint32_t h=mix(id)&(HSIZE-1);while(hval[h]>=0){if(hkey[h]==id)return hval[h];h=(h+1)&(HSIZE-1);}hkey[h]=id;hval[h]=nnode;id_of[nnode]=id;return nnode++;}
static char* slurp(const char*p,size_t*l){FILE*f=fopen(p,"rb");if(!f){perror(p);exit(1);}fseek(f,0,SEEK_END);long n=ftell(f);fseek(f,0,SEEK_SET);char*b=malloc(n+1);if(fread(b,1,n,f)!=(size_t)n)exit(1);b[n]=0;fclose(f);*l=n;return b;}
static inline uint64_t rdu64(char**p){uint64_t v=0;char*q=*p;while(*q>='0'&&*q<='9'){v=v*10+(*q-'0');q++;}*p=q;return v;}

static int32_t *src,*dst,*wt; static int64_t M=0; static int N;
static uint64_t rs; static inline uint64_t rnd(void){rs^=rs<<13;rs^=rs>>7;rs^=rs<<17;return rs;}
static inline double rnd01(void){return (double)(rnd()>>11)/9007199254740992.0;}

typedef struct{float x;int32_t i;} XI;
static int cmpxi(const void*a,const void*b){float p=((XI*)a)->x,q=((XI*)b)->x;return p<q?-1:(p>q?1:0);}

int main(int argc,char**argv){
  if(argc<3){fprintf(stderr,"usage: %s graph.csv out.csv [steps] [seed] [restarts]\n",argv[0]);return 1;}
  int steps  = argc>3?atoi(argv[3]):4000;
  rs         = argc>4?strtoull(argv[4],0,10):7; if(!rs)rs=7;
  int restarts = argc>5?atoi(argv[5]):1;
  double t0=now();

  hkey=malloc(HSIZE*8);hval=malloc(HSIZE*4);for(uint32_t i=0;i<HSIZE;i++)hval[i]=-1;id_of=malloc(400000*8);
  size_t gl;char*g=slurp(argv[1],&gl);char*p=g;while(*p&&*p!='\n')p++;p++;
  int64_t cap=8000000;src=malloc(cap*4);dst=malloc(cap*4);wt=malloc(cap*4);
  while(*p){ if(*p<'0'||*p>'9'){p++;continue;}
    uint64_t a=rdu64(&p);p++;uint64_t b=rdu64(&p);p++;uint64_t w=rdu64(&p);
    while(*p&&*p!='\n')p++;if(*p)p++;
    src[M]=intern(a);dst[M]=intern(b);wt[M]=(int32_t)w;M++; }
  free(g); N=nnode;
  int64_t total=0; for(int64_t e=0;e<M;e++) total+=wt[e];
  fprintf(stderr,"%lld edges, %d nodes, total=%lld  (%.1fs)\n",(long long)M,N,(long long)total,now()-t0);

  float *x=malloc(N*4),*gr=malloc(N*4),*mm=malloc(N*4),*vv=malloc(N*4);
  XI *xi=malloc(N*sizeof(XI)); int32_t *posn=malloc(N*4);
  int32_t *bestord=malloc(N*4); int64_t bestscore=-1;

  for(int r=0;r<restarts;r++){
    for(int i=0;i<N;i++){x[i]=(float)(rnd01()*2.0-1.0);mm[i]=0;vv[i]=0;}
    double tau0=2.0,tau1=0.0015,lr0=0.02,lr1=0.0008,b1=0.9,b2=0.999;
    for(int s=1;s<=steps;s++){
      double fr=(double)s/steps;
      double tau=tau0*pow(tau1/tau0,fr), lr=lr0*pow(lr1/lr0,fr);
      memset(gr,0,N*4);
      float itau=(float)(1.0/tau);
      for(int64_t e=0;e<M;e++){
        float z=(x[src[e]]-x[dst[e]])*itau+0.5f;
        if(z>0.f&&z<1.f){ float gw=(float)wt[e]*itau; gr[src[e]]+=gw; gr[dst[e]]-=gw; }
      }
      double bc1=1.0-pow(b1,s), bc2=1.0-pow(b2,s);
      double mean=0.0;
      for(int i=0;i<N;i++){
        float gi=gr[i]/ (float)total * 1000.0f;            /* scale-stabilise */
        if(gi>5.f)gi=5.f; if(gi<-5.f)gi=-5.f;              /* clip */
        mm[i]=(float)(b1*mm[i]+(1-b1)*gi);
        vv[i]=(float)(b2*vv[i]+(1-b2)*(double)gi*gi);
        double mh=mm[i]/bc1, vh=vv[i]/bc2;
        x[i]-=(float)(lr*mh/(sqrt(vh)+1e-8));
        mean+=x[i];
      }
      mean/=N; double var=0.0;
      for(int i=0;i<N;i++){x[i]-=(float)mean;var+=(double)x[i]*x[i];}
      float sd=(float)sqrt(var/N); if(sd<1e-9f)sd=1e-9f;
      for(int i=0;i<N;i++)x[i]/=sd;

      if(s%500==0||s==steps){
        for(int i=0;i<N;i++){xi[i].x=x[i];xi[i].i=i;}
        qsort(xi,N,sizeof(XI),cmpxi);
        for(int i=0;i<N;i++)posn[xi[i].i]=i;
        int64_t f=0; for(int64_t e=0;e<M;e++) if(posn[src[e]]<posn[dst[e]]) f+=wt[e];
        fprintf(stderr,"  r%d step %5d tau=%.4f  fwd=%lld (%.3f%%)  %.1fs\n",r,s,tau,(long long)f,100.0*f/total,now()-t0);
        if(f>bestscore){bestscore=f;for(int i=0;i<N;i++)bestord[i]=xi[i].i;}
      }
    }
  }
  FILE*o=fopen(argv[2],"w");fprintf(o,"Node ID,Order\n");
  for(int i=0;i<N;i++)fprintf(o,"%llu,%d\n",(unsigned long long)id_of[bestord[i]],i);
  fclose(o);
  fprintf(stderr,"best=%lld (%.3f%%) -> %s\n",(long long)bestscore,100.0*bestscore/total,argv[2]);
  return 0;
}
