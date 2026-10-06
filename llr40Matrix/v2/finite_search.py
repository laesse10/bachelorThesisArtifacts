#!/usr/bin/env python3
"""Task 3.1: largest size at which the NumPy reference output is 100% finite -> finite_sizes.csv.

NumPy reference only. Inputs come from the harness's own path, ``initialize.auto_initialize``,
with the same seed (``seeds.input_dist``) and distribution (``uniform``) that ``cli run`` uses
for a fixed preset; only the size symbol is overridden. Binary search on the size symbol between
preset S (must be finite) and preset M (known not to be). The inputs at size n are drawn fresh
for every n, so finiteness need not be monotone in n: the neighbourhood of the answer is scanned
as well and any non-monotone point is reported, not hidden.
"""
import argparse, csv, importlib, os, pathlib, sys, time

import numpy as np

BENCH = pathlib.Path(os.environ.get("LLR40_BENCH", "/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2")).resolve()
sys.path[:0] = [str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")]
import yaml  # noqa: E402
from hpcagent_bench import config  # noqa: E402
from hpcagent_bench.initialize import auto_initialize  # noqa: E402
from hpcagent_bench.precision import precision_from_datatype  # noqa: E402
from hpcagent_bench.frameworks.benchmark import Benchmark  # noqa: E402

LLR = BENCH / "hpcagent_bench/benchmarks/loop_level_reasoning"
KERNELS = {"tsvc_2_s115": "s115", "wf_triangular": "wf_triangular", "wf_diff_skew": "wf_diff_skew"}
SYMBOL = "LEN_2D"


def finite_fraction(kernel, func, n):
    info = yaml.safe_load(open(LLR / kernel / f"{kernel}.yaml"))
    spec = Benchmark(kernel).spec
    seed = config.get_int("seeds.input_dist", 0)
    values = auto_initialize(spec, "M", precision_from_datatype("fp64"), distribution="uniform",
                             seed=seed, params_override={SYMBOL: n})
    data = dict(zip(spec.init.output_args, values))
    args = {k: v for k, v in data.items()}
    args[SYMBOL] = n
    import inspect
    params = list(inspect.signature(func).parameters)
    func(*[args[p] for p in params])
    outs = [np.asarray(data[o]) for o in info["output_args"]]
    tot = sum(o.size for o in outs)
    fin = sum(int(np.isfinite(o).sum()) for o in outs)
    return fin / tot, tot


def main(a):
    rows = []
    for kernel, fname in KERNELS.items():
        info = yaml.safe_load(open(LLR / kernel / f"{kernel}.yaml"))
        lo, hi = info["parameters"]["S"][SYMBOL], info["parameters"]["M"][SYMBOL]
        mod = importlib.import_module(f"hpcagent_bench.benchmarks.loop_level_reasoning.{kernel}.{kernel}_numpy")
        func = vars(mod)[fname]
        probes = {}

        def probe(n):
            if n not in probes:
                t = time.time(); probes[n] = finite_fraction(kernel, func, n)[0]
                print(f"  {kernel} {SYMBOL}={n}: finite={probes[n]:.6f} ({time.time()-t:.1f}s)", flush=True)
            return probes[n]

        assert probe(lo) == 1.0, f"{kernel}: preset S is not finite"
        # M is known degenerate (summary.md); confirm rather than assume, but cap the cost.
        hi_frac = probe(hi) if a.check_m else None
        L, H = lo, hi            # invariant: L finite, H not (or untested upper bound)
        while H - L > 1:
            mid = (L + H) // 2
            if probe(mid) == 1.0:
                L = mid
            else:
                H = mid
        # neighbourhood scan: finiteness need not be monotone in n (fresh draws per n)
        nonmono = [n for n in range(max(lo, L - a.scan), L) if probe(n) < 1.0]
        above = [n for n in range(L + 1, L + 1 + a.scan) if probe(n) == 1.0]
        rows.append({
            "kernel": kernel, "size_symbol": SYMBOL, "preset_S": lo, "preset_M": hi,
            "largest_finite": L, "first_nonfinite": H, "finite_frac_at_first_nonfinite": f"{probes[H]:.6f}",
            "finite_frac_at_M": "" if hi_frac is None else f"{hi_frac:.6f}",
            "nonfinite_below_answer_in_scan": " ".join(map(str, nonmono)),
            "finite_above_answer_in_scan": " ".join(map(str, above)),
            "scan_window": a.scan, "seed": config.get_int("seeds.input_dist", 0), "distribution": "uniform",
            "dtype": "float64", "bench_commit": os.popen(f"git -C {BENCH} rev-parse HEAD").read().strip(),
            "n_probes": len(probes),
        })
        print(f"{kernel}: largest finite {SYMBOL}={L}", flush=True)
    with open(a.out, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)
    print(f"wrote {a.out}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="finite_sizes.csv")
    ap.add_argument("--scan", type=int, default=16, help="neighbourhood checked on each side of the answer")
    ap.add_argument("--check-m", action="store_true", help="also evaluate the reference at preset M (slow)")
    main(ap.parse_args())
