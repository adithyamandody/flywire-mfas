/* SpringRank initialiser: minimise sum_e w (x_v - x_u - 1)^2 over edges u->v.
   Normal equations: (D + alpha I) x - (A + A^T) x = d_in - d_out. Jacobi-preconditioned CG.
   usage: springrank graph.csv out.csv [alpha] [iters]                                  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#define HBITS 19
#define HSIZE (1u<<HBITS)
static uint64_t*hkey;static int32_t*hval;static int nnode=0;static uint64_t*id_of;
static inline uint64_t mix(uint64_t z){z+=0x9e3779b97f4a7c15ULL;z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;z=(z^(z>>27))*0x94d049bb133111ebULL;return z^(z>>31);}
static int32_t intern(uint64_t id){uint32_t h=mix(id)&(HSIZE-1);while(hval[h]>=0){if(hkey[h]==id)return hval[h];h=(h+1)&(HSIZE-1);}hkey[h]=id;hval[h]=nnode;id_of[nnode]=id;return nnode++;}
static char* slurp(const char*p,size_t*l){FILE*f=fopen(p,"rb");if(!f){perror(p);exit(1);}fseek(f,0,SEEK_END);long n=ftell(f);fseek(f,0,SEEK_SET);char*b=malloc(n+1);if(fread(b,1,n,f)!=(size_t)n)exit(1);b[n]=0;fclose(f);*l=n;return b;}
static inline uint64_t rdu64(char**p){uint64_t v=0;char*q=*p;while(*q>='0'&&*q<='9'){v=v*10+(*q-'0');q++;}*p=q;return v;}
static int32_t*src,*dst,*wt;static int64_t M=0,total=0;static int N;static double*diag;
static void matvec(const double*x,double*y){for(int i=0;i<N;i++)y[i]=diag[i]*x[i];
  for(int64_t e=0;e<M;e++){double w=wt[e];y[src[e]]-=w*x[dst[e]];y[dst[e]]-=w*x[src[e]];}}
typedef struct{double x;int32_t i;}XI;static int cmp(const void*a,const void*b){double p=((XI*)a)->x,q=((XI*)b)->x;return p<q?-1:(p>q?1:0);}
int main(int argc,char**argv){
  double alpha=argc>3?atof(argv[3]):0.01;int iters=argc>4?atoi(argv[4]):3000;
  hkey=malloc(HSIZE*8);hval=malloc(HSIZE*4);for(uint32_t i=0;i<HSIZE;i++)hval[i]=-1;id_of=malloc(400000*8);
  size_t gl;char*g=slurp(argv[1],&gl);char*p=g;while(*p&&*p!='\n')p++;p++;
  int64_t cap=8000000;src=malloc(cap*4);dst=malloc(cap*4);wt=malloc(cap*4);
  while(*p){if(*p<'0'||*p>'9'){p++;continue;}uint64_t a=rdu64(&p);p++;uint64_t b=rdu64(&p);p++;uint64_t w=rdu64(&p);
    while(*p&&*p!='\n')p++;if(*p)p++;src[M]=intern(a);dst[M]=intern(b);wt[M]=(int32_t)w;M++;}
  free(g);N=nnode;for(int64_t e=0;e<M;e++)total+=wt[e];
  diag=calloc(N,8);double*b=calloc(N,8);
  for(int64_t e=0;e<M;e++){diag[src[e]]+=wt[e];diag[dst[e]]+=wt[e];b[dst[e]]+=wt[e];b[src[e]]-=wt[e];}
  for(int i=0;i<N;i++)diag[i]+=alpha;
  double*x=calloc(N,8),*r=malloc(N*8),*z=malloc(N*8),*pp=malloc(N*8),*Ap=malloc(N*8);
  memcpy(r,b,N*8);for(int i=0;i<N;i++){z[i]=r[i]/diag[i];pp[i]=z[i];}
  double rz=0,bn=0;for(int i=0;i<N;i++){rz+=r[i]*z[i];bn+=b[i]*b[i];}bn=sqrt(bn);
  int k;for(k=0;k<iters;k++){
    matvec(pp,Ap);double pAp=0;for(int i=0;i<N;i++)pAp+=pp[i]*Ap[i];double a=rz/pAp;
    double rn=0;for(int i=0;i<N;i++){x[i]+=a*pp[i];r[i]-=a*Ap[i];rn+=r[i]*r[i];}
    if(sqrt(rn)<1e-9*bn)break;
    double rz2=0;for(int i=0;i<N;i++){z[i]=r[i]/diag[i];rz2+=r[i]*z[i];}
    double be=rz2/rz;rz=rz2;for(int i=0;i<N;i++)pp[i]=z[i]+be*pp[i];
    if(k%500==0)fprintf(stderr,"cg %d rel.res %.2e\n",k,sqrt(rn)/bn);
  }
  XI*xi=malloc(N*sizeof(XI));for(int i=0;i<N;i++){xi[i].x=x[i];xi[i].i=i;}qsort(xi,N,sizeof(XI),cmp);
  int32_t*rk=malloc(N*4);for(int i=0;i<N;i++)rk[xi[i].i]=i;
  int64_t f=0;for(int64_t e=0;e<M;e++)if(rk[src[e]]<rk[dst[e]])f+=wt[e];
  FILE*o=fopen(argv[2],"w");fprintf(o,"Node ID,Order\n");for(int i=0;i<N;i++)fprintf(o,"%llu,%d\n",(unsigned long long)id_of[xi[i].i],i);fclose(o);
  fprintf(stderr,"cg iters %d  SpringRank order: %lld (%.3f%%)\n",k,(long long)f,100.0*f/total);
  return 0;
}
