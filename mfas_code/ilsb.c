/* ils: fast iterated local search. After a kick, only the kicked neurons and
   (transitively) the neighbours of anything that moves are re-examined, so an
   iteration costs ~milliseconds instead of a full sweep. Moves are logged and
   undone exactly when an iteration loses weight.
   usage: ils graph.csv out.csv secs seed init.csv                         */
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
static int64_t*adj,*astart;static int32_t*alen;static int maxdeg=0;
static int32_t*ord,*pos;static int64_t*sbuf;
static int32_t*wq;static uint8_t*inq;static int qh=0,qt=0,qn=0;      /* ring worklist */
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
static int64_t drain(int logged){int64_t g=0;
  while(qn){int32_t v=wq[qh];qh=(qh+1)%N;qn--;inq[v]=0;int32_t t;int64_t b=best_ins(v,&t);
    if(b>0){if(logged)move_logged(v,t);else shift_to(v,t);g+=b;push_nbrs(v);}}
  return g;}
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
  ord=malloc(N*4);pos=malloc(N*4);sbuf=malloc((size_t)(maxdeg+2)*8);wq=malloc(N*4);inq=calloc(N,1);
  size_t ol;char*ob=slurp(argv[5],&ol);char*q=ob;while(*q&&*q!='\n')q++;q++;
  while(*q){if(*q<'0'||*q>'9'){q++;continue;}uint64_t id=rdu64(&q);q++;uint64_t o=rdu64(&q);while(*q&&*q!='\n')q++;if(*q)q++;ord[o]=lookup(id);}
  free(ob);for(int i=0;i<N;i++)pos[ord[i]]=i;
  int64_t cur=exact_score();fprintf(stderr,"start %lld (%.4f%%)\n",(long long)cur,100.0*cur/total);
  for(int i=0;i<N;i++)push(ord[i]);cur+=drain(0);
  fprintf(stderr,"local opt %lld (%.4f%%) %.1fs\n",(long long)cur,100.0*cur/total,now()-t0);
  int64_t dumped=-1;double lastd=0;long it=0,acc_eq=0,acc_up=0;int64_t wgain[NW]={0};long wwin[NW]={0},wtry[NW]={0};
  int mode=argc>6?atoi(argv[6]):2; long btry=0,bwin=0;int64_t bgain=0;
  while(now()-t0<budget){
    it++;nlog=0;int64_t d=0;int typeB=(mode==1)||(mode==2&&(rnd()&1));int wi=0;
    if(!typeB){
      wi=(int)(rnd()%NW);int W=WIN[wi];int K=1+(int)(rnd()%8);wtry[wi]++;
      for(int k=0;k<K;k++){int32_t v=(int32_t)(rnd()%(uint64_t)N);int32_t pv=pos[v],qq;
        if(W==0)qq=(int32_t)(rnd()%(uint64_t)N);else{qq=pv+(int32_t)(rnd()%(uint64_t)(2*W+1))-W;if(qq<0)qq=0;if(qq>=N)qq=N-1;}
        if(qq==pv)continue;d+=gain_to(v,qq);move_logged(v,qq);push(v);push_nbrs(v);}
    }else{
      btry++;int K=1+(int)(rnd()%3);
      for(int k=0;k<K;k++){int64_t e=-1;
        for(int t=0;t<256;t++){int64_t c=(int64_t)(rnd()%(uint64_t)M);
          if(pos[src[c]]>pos[dst[c]]&&(int)(rnd()%32)<wt[c]){e=c;break;}}
        if(e<0)continue;int32_t u=src[e],v=dst[e],node,qq;
        if(rnd()&1){node=u;qq=pos[v];}else{node=v;qq=pos[u];}
        d+=gain_to(node,qq);move_logged(node,qq);push(node);push_nbrs(node);}
    }
    d+=drain(1);
    if(d<0)undo_all();
    else{if(d>0){cur+=d;acc_up++;if(typeB){bwin++;bgain+=d;}else{wgain[wi]+=d;wwin[wi]++;}}else acc_eq++;nlog=0;}
    if((it&1023)==0&&cur>dumped&&now()-lastd>60){int64_t chk=exact_score();
      if(chk!=cur){fprintf(stderr,"!! mismatch %lld vs %lld\n",(long long)cur,(long long)chk);cur=chk;}
      dump(argv[2],cur);dumped=cur;lastd=now();
      fprintf(stderr,"it %ld  %.0f it/s  improving=%ld plateau=%ld  best %lld (%.4f%%)  %.0fs\n",it,it/(now()-t0),acc_up,acc_eq,(long long)cur,100.0*cur/total,now()-t0);}
  }
  int64_t chk=exact_score();if(chk!=cur)fprintf(stderr,"!! final mismatch %lld vs %lld\n",(long long)cur,(long long)chk);
  cur=chk;dump(argv[2],cur);
  for(int i=0;i<NW;i++)fprintf(stderr,"window %6d: tries %ld wins %ld gain %lld\n",WIN[i],wtry[i],wwin[i],(long long)wgain[i]);
  fprintf(stderr,"backward-edge kicks: tries %ld wins %ld gain %lld\n",btry,bwin,(long long)bgain);
  fprintf(stderr,"DONE %lld (%.4f%%)  iterations %ld\n",(long long)cur,100.0*cur/total,it);
  return 0;}
