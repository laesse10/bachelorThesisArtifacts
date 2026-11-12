#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifndef RTOL
#define RTOL 1.0e-6
#endif
#ifndef ATOL
#define ATOL 1.0e-9
#endif
#define MAX_STEPS 200000
#define H0 1.0e-4
#ifndef ILP_K
#define ILP_K 2
#endif

/* One DOPRI5 (RK45) step. Updates Y1,Y2,Y3 (on accept), T (on accept), H, ACC, REJ, STEPS. */
#define DO_STEP(Y1,Y2,Y3,T,H,ACC,REJ,STEPS) do { \
    if ((T) + (H) > t_end) (H) = t_end - (T); \
    double _p = 1.0e4*(Y2)*(Y3), _q = 3.0e7*(Y2)*(Y2); \
    double _k1x = -0.04*(Y1) + _p, _k1y = 0.04*(Y1) - _p - _q, _k1z = _q; \
    double _s2x = (Y1) + (H)*(1.0/5.0)*(_k1x), _s2y = (Y2) + (H)*(1.0/5.0)*(_k1y), _s2z = (Y3) + (H)*(1.0/5.0)*(_k1z); \
    _p = 1.0e4*_s2y*_s2z; _q = 3.0e7*_s2y*_s2y; \
    double _k2x = -0.04*_s2x + _p, _k2y = 0.04*_s2x - _p - _q, _k2z = _q; \
    double _s3x = (Y1) + (H)*(3.0/40.0*_k1x + 9.0/40.0*_k2x), \
           _s3y = (Y2) + (H)*(3.0/40.0*_k1y + 9.0/40.0*_k2y), \
           _s3z = (Y3) + (H)*(3.0/40.0*_k1z + 9.0/40.0*_k2z); \
    _p = 1.0e4*_s3y*_s3z; _q = 3.0e7*_s3y*_s3y; \
    double _k3x = -0.04*_s3x + _p, _k3y = 0.04*_s3x - _p - _q, _k3z = _q; \
    double _s4x = (Y1) + (H)*(44.0/45.0*_k1x - 56.0/15.0*_k2x + 32.0/9.0*_k3x), \
           _s4y = (Y2) + (H)*(44.0/45.0*_k1y - 56.0/15.0*_k2y + 32.0/9.0*_k3y), \
           _s4z = (Y3) + (H)*(44.0/45.0*_k1z - 56.0/15.0*_k2z + 32.0/9.0*_k3z); \
    _p = 1.0e4*_s4y*_s4z; _q = 3.0e7*_s4y*_s4y; \
    double _k4x = -0.04*_s4x + _p, _k4y = 0.04*_s4x - _p - _q, _k4z = _q; \
    double _s5x = (Y1) + (H)*(19372.0/6561.0*_k1x - 25360.0/2187.0*_k2x + 64448.0/6561.0*_k3x - 212.0/729.0*_k4x), \
           _s5y = (Y2) + (H)*(19372.0/6561.0*_k1y - 25360.0/2187.0*_k2y + 64448.0/6561.0*_k3y - 212.0/729.0*_k4y), \
           _s5z = (Y3) + (H)*(19372.0/6561.0*_k1z - 25360.0/2187.0*_k2z + 64448.0/6561.0*_k3z - 212.0/729.0*_k4z); \
    _p = 1.0e4*_s5y*_s5z; _q = 3.0e7*_s5y*_s5y; \
    double _k5x = -0.04*_s5x + _p, _k5y = 0.04*_s5x - _p - _q, _k5z = _q; \
    double _s6x = (Y1) + (H)*(9017.0/3168.0*_k1x - 355.0/33.0*_k2x + 46732.0/5247.0*_k3x + 49.0/176.0*_k4x - 5103.0/18656.0*_k5x), \
           _s6y = (Y2) + (H)*(9017.0/3168.0*_k1y - 355.0/33.0*_k2y + 46732.0/5247.0*_k3y + 49.0/176.0*_k4y - 5103.0/18656.0*_k5y), \
           _s6z = (Y3) + (H)*(9017.0/3168.0*_k1z - 355.0/33.0*_k2z + 46732.0/5247.0*_k3z + 49.0/176.0*_k4z - 5103.0/18656.0*_k5z); \
    _p = 1.0e4*_s6y*_s6z; _q = 3.0e7*_s6y*_s6y; \
    double _k6x = -0.04*_s6x + _p, _k6y = 0.04*_s6x - _p - _q, _k6z = _q; \
    double _s1 = (Y1) + (H)*(35.0/384.0*_k1x + 500.0/1113.0*_k3x + 125.0/192.0*_k4x - 2187.0/6784.0*_k5x + 11.0/84.0*_k6x), \
           _s2 = (Y2) + (H)*(35.0/384.0*_k1y + 500.0/1113.0*_k3y + 125.0/192.0*_k4y - 2187.0/6784.0*_k5y + 11.0/84.0*_k6y), \
           _s3 = (Y3) + (H)*(35.0/384.0*_k1z + 500.0/1113.0*_k3z + 125.0/192.0*_k4z - 2187.0/6784.0*_k5z + 11.0/84.0*_k6z); \
    _p = 1.0e4*_s2*_s3; _q = 3.0e7*_s2*_s2; \
    double _k7x = -0.04*_s1 + _p, _k7y = 0.04*_s1 - _p - _q, _k7z = _q; \
    double _y41 = (Y1) + (H)*(5179.0/57600.0*_k1x + 7571.0/16695.0*_k3x + 393.0/640.0*_k4x - 92097.0/339200.0*_k5x + 187.0/2100.0*_k6x + 1.0/40.0*_k7x), \
           _y42 = (Y2) + (H)*(5179.0/57600.0*_k1y + 7571.0/16695.0*_k3y + 393.0/640.0*_k4y - 92097.0/339200.0*_k5y + 187.0/2100.0*_k6y + 1.0/40.0*_k7y), \
           _y43 = (Y3) + (H)*(5179.0/57600.0*_k1z + 7571.0/16695.0*_k3z + 393.0/640.0*_k4z - 92097.0/339200.0*_k5z + 187.0/2100.0*_k6z + 1.0/40.0*_k7z); \
    double _e1 = _s1 - _y41, _e2 = _s2 - _y42, _e3 = _s3 - _y43; \
    double _enorm = sqrt(_e1*_e1 + _e2*_e2 + _e3*_e3); \
    double _ynorm = sqrt(_s1*_s1 + _s2*_s2 + _s3*_s3); \
    double _err = _enorm / (ATOL + RTOL*_ynorm); \
    (STEPS) = (STEPS) + 1; \
    if (_err <= 1.0) { (Y1)=_s1; (Y2)=_s2; (Y3)=_s3; (T)=(T)+(H); (ACC)=(ACC)+1; } \
    else { (REJ)=(REJ)+1; } \
    double _factor; \
    if (_err > 0.0) { _factor = 0.9*(pow(_err,-0.2)); if(_factor>5.0)_factor=5.0; if(_factor<0.2)_factor=0.2; } \
    else { _factor = 5.0; } \
    (H) = (H)*_factor; \
} while(0)

static inline void write_out(int64_t n, double y1, double y2, double y3,
                             int64_t acc, int64_t rej,
                             double *restrict y, int64_t *restrict na, int64_t *restrict nr) {
    y[3*n+0]=y1; y[3*n+1]=y2; y[3*n+2]=y3; na[n]=acc; nr[n]=rej;
}

static inline void step1(int64_t n, const double *restrict y0, double t_end,
                         double *restrict y, int64_t *restrict na, int64_t *restrict nr) {
    double y1=y0[3*n+0], y2=y0[3*n+1], y3=y0[3*n+2];
    double t=0.0, h=H0; int64_t acc=0, rej=0, steps=0;
    while (t < t_end && steps < MAX_STEPS) DO_STEP(y1,y2,y3,t,h,acc,rej,steps);
    write_out(n, y1,y2,y3, acc,rej, y,na,nr);
}

static inline void step2(int64_t n, const double *restrict y0, double t_end,
                         double *restrict y, int64_t *restrict na, int64_t *restrict nr) {
    int64_t b=n+1;
    double ya1=y0[3*n+0], ya2=y0[3*n+1], ya3=y0[3*n+2];
    double yb1=y0[3*b+0], yb2=y0[3*b+1], yb3=y0[3*b+2];
    double ta=0.0, ha=H0, tb=0.0, hb=H0;
    int64_t aa=0,ar=0,ba=0,br=0,sa=0,sb=0;
    int da=0, db=0;
    while(!da || !db){
        if(!da && ta < t_end && sa < MAX_STEPS){
            DO_STEP(ya1,ya2,ya3,ta,ha,aa,ar,sa);
            if(ta >= t_end || sa >= MAX_STEPS){ da=1; write_out(n, ya1,ya2,ya3, aa,ar, y,na,nr); }
        }
        if(!db && tb < t_end && sb < MAX_STEPS){
            DO_STEP(yb1,yb2,yb3,tb,hb,ba,br,sb);
            if(tb >= t_end || sb >= MAX_STEPS){ db=1; write_out(b, yb1,yb2,yb3, ba,br, y,na,nr); }
        }
    }
}

static inline void step4(int64_t n, const double *restrict y0, double t_end,
                         double *restrict y, int64_t *restrict na, int64_t *restrict nr) {
    double y0a[12];
    for(int i=0;i<4;i++){ y0a[3*i+0]=y0[3*(n+i)+0]; y0a[3*i+1]=y0[3*(n+i)+1]; y0a[3*i+2]=y0[3*(n+i)+2]; }
    double Y[4][3]={{y0a[0],y0a[1],y0a[2]},{y0a[3],y0a[4],y0a[5]},{y0a[6],y0a[7],y0a[8]},{y0a[9],y0a[10],y0a[11]}};
    double T[4]={0,0,0,0}, H[4]={H0,H0,H0,H0};
    int64_t ACC[4]={0,0,0,0}, REJ[4]={0,0,0,0}, STEPS[4]={0,0,0,0};
    int done[4]={0,0,0,0};
    int any=4;
    while(any){
        for(int i=0;i<4;i++){
            if(!done[i] && T[i] < t_end && STEPS[i] < MAX_STEPS){
                DO_STEP(Y[i][0],Y[i][1],Y[i][2],T[i],H[i],ACC[i],REJ[i],STEPS[i]);
                if(T[i] >= t_end || STEPS[i] >= MAX_STEPS){ done[i]=1; any--; write_out(n+i, Y[i][0],Y[i][1],Y[i][2], ACC[i],REJ[i], y,na,nr); }
            }
        }
    }
}

void rk45_ensemble_fp64(
    int64_t *restrict n_accept,
    int64_t *restrict n_reject,
    double *restrict y,
    const double *restrict y0,
    const int64_t NSYS,
    const double t_end,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    (void)workspace; (void)workspace_size;
    const int64_t NGROUPS = (NSYS + ILP_K - 1) / ILP_K;
    #pragma omp parallel for schedule(dynamic, 128)
    for (int64_t g = 0; g < NGROUPS; g++) {
        int64_t base = g * ILP_K;
        int64_t rem  = NSYS - base;
#if ILP_K == 4
        if (rem >= 4) { step4(base, y0, t_end, y, n_accept, n_reject); }
        else {
            if (rem >= 2) { step2(base, y0, t_end, y, n_accept, n_reject); base += 2; }
            if (rem >= 1) { step1(base, y0, t_end, y, n_accept, n_reject); }
        }
#elif ILP_K == 2
        if (rem >= 2) { step2(base, y0, t_end, y, n_accept, n_reject); }
        else          { step1(base, y0, t_end, y, n_accept, n_reject); }
#else
        step1(base, y0, t_end, y, n_accept, n_reject);
#endif
    }
}
