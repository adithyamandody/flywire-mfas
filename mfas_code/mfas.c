// Feedforward sort via sifting local search (insertion neighbourhood).
// usage: mfas graph.csv init_order.csv out_order.csv [max_seconds] [seed]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec + 1e-9*t.tv_nsec; }

/* ---------- int64 sort ---------- */
static void isort64(int64_t *a,int n){
  for(int i=1;i<n;i++){int64_t x=a[i];int j=i-1;while(j>=0&&a[j]>x){a[j+1]=a[j];j--;}a[j+1]=x;}
}
static void qs64(int64_t *a,int n){
  while(n>24){
    int64_t x=a[0],y=a[n/2],z=a[n-1],p;
    if(x>y){int64_t t=x;x=y;y=t;} if(y>z)y=z; if(x>y)y=x; p=y;
    int i=0,j=n-1;
    while(i<=j){ while(a[i]<p)i++; while(a[j]>p)j--; if(i<=j){int64_t t=a[i];a[i]=a[j];a[j]=t;i++;j--;} }
    if(j+1 < n-i){ qs64(a,j+1); a+=i; n-=i; } else { qs64(a+i,n-i); n=j+1; }
  }
  isort64(a,n);
}

/* ---------- hash map: uint64 id -> int32 index ---------- */
#define HBITS 19
#define HSIZE (1u<<HBITS)
static uint64_t *hkey; static int32_t *hval; static int nnode=0;
static uint64_t *id_of;
static inline uint64_t mix(uint64_t z){ z+=0x9e3779b97f4a7c15ULL; z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL; z=(z^(z>>27))*0x94d049bb133111ebULL; return z^(z>>31); }
static int32_t intern(uint64_t id){
  uint32_t h = mix(id) & (HSIZE-1);
  while(hval[h]>=0){ if(hkey[h]==id) return hval[h]; h=(h+1)&(HSIZE-1); }
  hkey[h]=id; hval[h]=nnode; id_of[nnode]=id; return nnode++;
}
static int32_t lookup(uint64_t id){
  uint32_t h = mix(id) & (HSIZE-1);
  while(hval[h]>=0){ if(hkey[h]==id) return hval[h]; h=(h+1)&(HSIZE-1); }
  return -1;
}

/* ---------- read whole file ---------- */
static char* slurp(const char*path,size_t*len){
  FILE*f=fopen(path,"rb"); if(!f){perror(path);exit(1);}
  fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET);
  char*b=malloc(n+1); if(fread(b,1,n,f)!=(size_t)n){fprintf(stderr,"short read\n");exit(1);}
  b[n]=0; fclose(f); *len=n; return b;
}
static inline uint64_t rdu64(char**p){ uint64_t v=0; char*q=*p; while(*q>='0'&&*q<='9'){v=v*10+(*q-'0');q++;} *p=q; return v; }

/* ---------- globals ---------- */
static int32_t *src,*dst,*wt; static int64_t M=0;
static int64_t *adj; static int64_t *astart; static int32_t *alen;
static int32_t *ord,*pos;

static uint64_t rng_s;
static inline uint64_t rnd(void){ rng_s^=rng_s<<13; rng_s^=rng_s>>7; rng_s^=rng_s<<17; return rng_s; }

int main(int argc,char**argv){
  if(argc<4){fprintf(stderr,"usage: %s graph.csv init.csv out.csv [secs] [seed]\n",argv[0]);return 1;}
  double budget = argc>4? atof(argv[4]) : 120.0;
  rng_s = argc>5? (uint64_t)strtoull(argv[5],0,10) : 12345ULL; if(!rng_s) rng_s=1;
  double t0=now();

  hkey=malloc(HSIZE*8); hval=malloc(HSIZE*4); for(uint32_t i=0;i<HSIZE;i++)hval[i]=-1;
  id_of=malloc(400000*8);

  /* ---- parse graph ---- */
  size_t glen; char*g=slurp(argv[1],&glen);
  char*p=g; while(*p&&*p!='\n')p++; p++;             /* skip header */
  int64_t cap=8000000; src=malloc(cap*4);dst=malloc(cap*4);wt=malloc(cap*4);
  while(*p){
    if(*p<'0'||*p>'9'){p++;continue;}
    uint64_t a=rdu64(&p); p++; uint64_t b=rdu64(&p); p++; uint64_t w=rdu64(&p);
    while(*p&&*p!='\n')p++; if(*p)p++;
    src[M]=intern(a); dst[M]=intern(b); wt[M]=(int32_t)w; M++;
  }
  free(g);
  int N=nnode;
  fprintf(stderr,"parsed %lld edges, %d nodes  (%.1fs)\n",(long long)M,N,now()-t0);

  /* ---- build signed adjacency: for v, entries (u, w(u->v)-w(v->u)) ---- */
  astart=malloc((N+1)*8); alen=malloc(N*4);
  int64_t *cnt=calloc(N+1,8);
  for(int64_t e=0;e<M;e++){ cnt[src[e]]++; cnt[dst[e]]++; }
  int64_t acc=0; for(int i=0;i<N;i++){ astart[i]=acc; acc+=cnt[i]; } astart[N]=acc;
  adj=malloc(acc*8);
  int64_t *fill=malloc(N*8); memcpy(fill,astart,N*8);
  for(int64_t e=0;e<M;e++){
    int32_t u=src[e],v=dst[e],w=wt[e];
    adj[fill[v]++] = ((int64_t)u<<32) | (uint32_t)( w);   /* u -> v : +w for v */
    adj[fill[u]++] = ((int64_t)v<<32) | (uint32_t)(-w);   /* u -> v : -w for u */
  }
  free(fill); free(cnt);
  /* sort+merge per node */
  int maxdeg=0;
  for(int v=0;v<N;v++){
    int64_t s=astart[v]; int n=(int)(astart[v+1]-s);
    qs64(adj+s,n);
    int o=0;
    for(int k=0;k<n;){
      int32_t u=(int32_t)(adj[s+k]>>32); int64_t sum=0;
      while(k<n && (int32_t)(adj[s+k]>>32)==u){ sum += (int32_t)(adj[s+k]&0xffffffffLL); k++; }
      if(sum!=0 || 1) adj[s+o++] = ((int64_t)u<<32) | (uint32_t)(int32_t)sum;
    }
    alen[v]=o; if(o>maxdeg)maxdeg=o;
  }
  fprintf(stderr,"adjacency built, maxdeg=%d  (%.1fs)\n",maxdeg,now()-t0);

  /* ---- initial order ---- */
  ord=malloc(N*4); pos=malloc(N*4);
  size_t olen; char*ob=slurp(argv[2],&olen);
  char*q=ob; while(*q&&*q!='\n')q++; q++;
  int filled=0;
  while(*q){
    if(*q<'0'||*q>'9'){q++;continue;}
    uint64_t id=rdu64(&q); q++; uint64_t o=rdu64(&q);
    while(*q&&*q!='\n')q++; if(*q)q++;
    int32_t v=lookup(id); if(v<0){fprintf(stderr,"order has unknown node\n");return 1;}
    ord[o]=v; pos[v]=(int32_t)o; filled++;
  }
  free(ob);
  if(filled!=N){fprintf(stderr,"order covers %d of %d nodes\n",filled,N);return 1;}

  /* ---- initial score ---- */
  int64_t fwd=0,total=0;
  for(int64_t e=0;e<M;e++){ total+=wt[e]; if(pos[src[e]]<pos[dst[e]]) fwd+=wt[e]; }
  fprintf(stderr,"start  fwd=%lld  (%.2f%%)\n",(long long)fwd,100.0*fwd/total);

  /* ---- sifting sweeps ---- */
  int64_t *buf=malloc((size_t)(maxdeg+2)*8);
  int32_t *perm=malloc(N*4); for(int i=0;i<N;i++)perm[i]=i;
  int sweep=0;
  while(now()-t0 < budget){
    for(int i=N-1;i>0;i--){ int j=(int)(rnd()%(uint64_t)(i+1)); int32_t t=perm[i];perm[i]=perm[j];perm[j]=t; }
    int64_t gained=0; int moves=0;
    for(int pi=0;pi<N;pi++){
      int32_t v=perm[pi];
      int n=alen[v]; if(!n) continue;
      int64_t s=astart[v]; int32_t pv=pos[v];
      for(int k=0;k<n;k++){
        int32_t u=(int32_t)(adj[s+k]>>32); int32_t d=(int32_t)(adj[s+k]&0xffffffffLL);
        buf[k] = ((int64_t)pos[u]<<32) | (uint32_t)d;
      }
      qs64(buf,n);
      int m=0; while(m<n && (int32_t)(buf[m]>>32) < pv) m++;
      int64_t best=0; int32_t target=pv, a2=0;
      for(int k=m;k<n;k++){ a2 += (int32_t)(buf[k]&0xffffffffLL);
        if(a2>best){ best=a2; target=(int32_t)(buf[k]>>32); } }
      a2=0;
      for(int k=m-1;k>=0;k--){ a2 -= (int32_t)(buf[k]&0xffffffffLL);
        if(a2>best){ best=a2; target=(int32_t)(buf[k]>>32); } }
      if(best>0){
        if(target>pv){ for(int i=pv;i<target;i++){ ord[i]=ord[i+1]; pos[ord[i]]=i; } }
        else         { for(int i=pv;i>target;i--){ ord[i]=ord[i-1]; pos[ord[i]]=i; } }
        ord[target]=v; pos[v]=target; fwd+=best; gained+=best; moves++;
      }
    }
    sweep++;
    fprintf(stderr,"sweep %3d  moves=%7d  gain=%10lld  fwd=%lld (%.3f%%)  %.1fs\n",
            sweep,moves,(long long)gained,(long long)fwd,100.0*fwd/total,now()-t0);
    if(gained==0) break;
  }

  /* ---- verify from scratch ---- */
  int64_t chk=0; for(int64_t e=0;e<M;e++) if(pos[src[e]]<pos[dst[e]]) chk+=wt[e];
  fprintf(stderr,"tracked=%lld  recomputed=%lld  %s\n",(long long)fwd,(long long)chk, fwd==chk?"OK":"MISMATCH");

  FILE*out=fopen(argv[3],"w"); fprintf(out,"Node ID,Order\n");
  for(int i=0;i<N;i++) fprintf(out,"%llu,%d\n",(unsigned long long)id_of[ord[i]],i);
  fclose(out);
  fprintf(stderr,"wrote %s  final=%lld (%.3f%%)\n",argv[3],(long long)chk,100.0*chk/total);
  return 0;
}
