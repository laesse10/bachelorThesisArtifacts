#!/usr/bin/env python3
"""Characterise the 121ms <-> 191ms bimodality on the REAL kernel.

tsvc_2_s3110 is a sequential max-scan over the whole 2.42 GB `aa` array, i.e. a pure streaming
read: 121 ms = 20.0 GB/s, 191 ms = 12.7 GB/s. So the question is which memory property changes.
Per rep we log elapsed time, the CPU we are on, that CPU's current frequency, and the process's
AnonHugePages -- the last one directly tests transparent-huge-page fallback, because it can
change *during* a run if khugepaged collapses or splits the mapping.
"""
import ctypes, os, pathlib, subprocess, sys, time
import numpy as np

BENCH = pathlib.Path(__file__).resolve().parent / "bench"
sys.path[:0] = [str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")]
KD = BENCH / "hpcagent_bench/benchmarks/loop_level_reasoning/tsvc_2_s3110/cpp_backend"
REPS = int(os.environ.get("DIAG_REPS", "90"))

libc = ctypes.CDLL("libc.so.6")


def cur_cpu():
    try: return libc.sched_getcpu()
    except Exception: return -1


def freq_khz(cpu):
    for f in (f"/sys/devices/system/cpu/cpu{cpu}/cpufreq/cpuinfo_cur_freq",
              f"/sys/devices/system/cpu/cpu{cpu}/cpufreq/scaling_cur_freq"):
        try: return int(open(f).read().strip())
        except OSError: continue
    return -1


def anon_huge_mb():
    try:
        for line in open("/proc/self/smaps_rollup"):
            if line.startswith("AnonHugePages:"):
                return int(line.split()[1]) / 1024
    except OSError: pass
    return -1.0


def main():
    from hpcagent_bench import languages
    flags = languages.baseline_flags("cpp").split()
    so = "/tmp/diag_s3110.so"
    # ABSOLUTE path, deliberately: `perf stat` prepends /usr/lib/perf-core:/usr/bin to PATH, so a
    # bare "g++" under perf resolves to /usr/bin/g++ = GCC 7.5.0, not the 13.3.1 in ~/bin that the
    # harness (and every recorded row) uses. Same binary as ~/bin/g++ -> /usr/bin/g++-13.
    gxx = os.path.realpath("/users/lhulsbergen/bin/g++")
    print(f"# compiler: {gxx}", flush=True)
    subprocess.run([gxx, *flags, "-std=c++23", "-shared",
                    str(KD / "tsvc_2_s3110_fp64.cpp"), "-o", so, "-lm"], check=True)
    lib = ctypes.CDLL(so)
    fn = lib.tsvc_2_s3110_fp64
    fn.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int64]

    from hpcagent_bench.frameworks import Benchmark
    d = Benchmark("tsvc_2_s3110").get_data(preset="M", datatype="float64")
    aa, bb, n = d["aa"], d["bb"], int(d["LEN_2D"])
    nbytes = aa.nbytes
    print(f"# array {nbytes/1e9:.2f} GB  n={n}  reps={REPS}  anon_huge={anon_huge_mb():.0f} MB", flush=True)
    print("rep,ms,GB_s,cpu,freq_MHz,anon_huge_MB,wall", flush=True)
    for r in range(REPS):
        c0 = cur_cpu(); f0 = freq_khz(c0); h0 = anon_huge_mb()
        t = time.perf_counter()
        fn(aa.ctypes.data, bb.ctypes.data, n)
        dt = time.perf_counter() - t
        print(f"{r},{dt*1e3:.1f},{nbytes/dt/1e9:.2f},{c0},{f0/1000:.0f},{h0:.0f},{time.time():.3f}",
              flush=True)


if __name__ == "__main__":
    main()
