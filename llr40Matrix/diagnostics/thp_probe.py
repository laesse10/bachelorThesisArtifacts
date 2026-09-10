#!/usr/bin/env python3
"""Correlate per-rep timing with the huge-page backing of the working set.

If the 121ms <-> 191ms bimodality is transparent-huge-page fallback, then a run whose anonymous
memory is backed by 2MB pages is fast and one backed by 4KB pages is slow, and AnonHugePages for
THIS process tracks it. Mirrors the s3110 access shape: a 2D column-strided max-reduction over a
2.42 GB float64 array.
"""
import ctypes, os, subprocess, sys, time
import numpy as np

N = 17409                      # preset-M LEN_2D for tsvc_2_s3110 -> 2.42 GB

def anon_huge_kb():
    try:
        for line in open("/proc/self/smaps_rollup"):
            if line.startswith("AnonHugePages:"):
                return int(line.split()[1])
    except OSError:
        pass
    return -1

src = r'''
#include <stdint.h>
void probe(const double *restrict aa, int64_t n, double *restrict out){
  double m = -1e308;
  for (int64_t i=0;i<n;i++) for (int64_t j=0;j<n;j++){ double v=aa[j*n+i]; if(v>m) m=v; }
  *out = m;
}'''
open("/tmp/thp_probe.c","w").write(src)
subprocess.run(["gcc","-O3","-march=native","-fPIC","-shared","/tmp/thp_probe.c","-o","/tmp/libthp.so"],check=True)
lib = ctypes.CDLL("/tmp/libthp.so")
lib.probe.argtypes=[ctypes.c_void_p, ctypes.c_int64, ctypes.c_void_p]

print(f"round  anon_huge_MB  frac_of_2.42GB   median_ms   samples")
for rnd in range(4):
    aa = np.ascontiguousarray(np.random.default_rng(rnd).standard_normal((N,N)))
    out = np.zeros(1)
    hk = anon_huge_kb()
    ts=[]
    for r in range(8):
        t=time.perf_counter()
        lib.probe(aa.ctypes.data, N, out.ctypes.data)
        ts.append((time.perf_counter()-t)*1e3)
    ts_s=sorted(ts)
    print(f"{rnd:<6} {hk/1024:<13.0f} {hk*1024/aa.nbytes:<16.3f} {ts_s[len(ts_s)//2]:<11.1f} "
          + " ".join(f"{x:.0f}" for x in ts))
    del aa
