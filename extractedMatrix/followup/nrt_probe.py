#!/usr/bin/env python3
"""Which JIT code are the hottest [JIT] addresses in the Numba profiles? (not a timing)

perf cannot name Numba's JIT code, so its report shows bare addresses. This probe runs a Numba file of
warpx_esirkepov_deposition on the harness's preset-M inputs (Benchmark.get_data, as
opt_reports_extracted.py does), N calls after a compile call, and at the end prints the addresses of
Numba's runtime functions in THIS process (NRT_incref, NRT_decref, nrt_atomic_add, nrt_atomic_sub:
compiled by Numba itself from LLVM IR at start-up, numba/core/runtime/nrtdynmod.py). Run it under
`perf record -e cycles:u` and compare the report's hottest addresses with these.

usage: LLR40_BENCH=<bench> perf record -e cycles:u -c 100000 -o P -- python nrt_probe.py NUMBA_FILE N OUT_TXT
"""
import importlib.util, inspect, os, pathlib, sys

BENCH = pathlib.Path(os.environ["LLR40_BENCH"]).resolve()
sys.path[:0] = [str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")]
ARGV = [str(pathlib.Path(a).resolve()) if i in (1, 3) else a for i, a in enumerate(sys.argv)]   # before the chdir
os.chdir(BENCH)
K = "warpx_esirkepov_deposition"


def main():
    src, n, out = pathlib.Path(ARGV[1]), int(ARGV[2]), pathlib.Path(ARGV[3])
    from hpcagent_bench.frameworks.benchmark import Benchmark
    b = Benchmark(K)
    data = dict(b.get_data(preset="M", datatype="float64"))
    for k, v in b.info["parameters"]["M"].items():
        data.setdefault(k, v)
    sp = importlib.util.spec_from_file_location("nrt_probe_mod", src)
    mod = importlib.util.module_from_spec(sp); sp.loader.exec_module(mod)
    fn = vars(mod)[b.info["func_name"]]
    names = list(inspect.signature(fn).parameters)
    args = [data[p] for p in names]
    fn(*args)                                   # compile
    for _ in range(n):
        fn(*args)
    from numba.core.runtime import nrt
    lib = nrt.rtsys.library
    lines = []
    for f in ("NRT_incref", "NRT_decref", "nrt_atomic_add", "nrt_atomic_sub", "NRT_MemInfo_call_dtor"):
        try:
            lines.append(f"{f} 0x{lib.get_pointer_to_function(f):x}")
        except Exception as e:  # noqa: BLE001
            lines.append(f"{f} not found ({e!r})")
    out.write_text("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
