#!/usr/bin/env python3
"""Task 4: gfortran -ftree-parallelize-loops A/B -> ftree_parallelize_ab.csv.

Arm `with`    = the sweep's fortran build (flags.DO_CONCURRENT_GFORTRAN = -ftree-parallelize-loops={n})
Arm `without` = the same commit with that one constant emptied (variants/no_parallelize_loops.patch)
Both arms: preset M, 5 warmup + 30 timed, one exclusive node per kernel, the two arms run one after
the other on the SAME node (order alternates by array index). ratio = min_without / min_with:
> 1 means the flag HELPS at one thread, < 1 means it costs.

`gomp_calls_*` counts GOMP_parallel call sites in each arm's built .so (worktree build dir), i.e.
whether parloops actually outlined a loop; with n>1 at build time and OMP_NUM_THREADS=1 at run
time such a loop runs on one thread through the libgomp dispatch.
"""
import csv, pathlib, re, statistics, subprocess

REPO = pathlib.Path(__file__).resolve().parent
WT = {"with": "/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2-abwith",
      "without": "/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2-nopar"}
LLR = "hpcagent_bench/benchmarks/loop_level_reasoning"


def gomp_calls(arm, k):
    so = pathlib.Path(WT[arm]) / LLR / k / "cpp_backend/build" / f"lib{k}_fortran.so"
    if not so.is_file():
        return ""
    d = subprocess.run(["objdump", "-d", str(so)], capture_output=True, text=True).stdout
    return len(re.findall(r"bl\s+\S+ <GOMP_parallel@plt>", d))


def main():
    jobs = [l.split("\t") for l in (REPO / "variants/jobs.tsv").read_text().splitlines() if l.strip()]
    rows = []
    for idx, (kind, k) in enumerate(jobs, 1):
        if kind != "AB":
            continue
        arm = {}
        for a, tag in (("with", "abwith"), ("without", "abwithout")):
            p = REPO / "variant_parts" / f"{tag}.{k}.csv"
            r = next(iter(csv.DictReader(open(p))), None) if p.is_file() else None
            arm[a] = r
        def g(a, f):
            return (arm[a] or {}).get(f, "")
        def rsd(a):
            s = [int(x) for x in (g(a, "time_ns_all") or "").split()]
            return round(100 * statistics.stdev(s) / statistics.fmean(s), 2) if len(s) > 1 else ""
        mw, mo = g("with", "time_ns_min"), g("without", "time_ns_min")
        ratio = round(int(mo) / int(mw), 4) if (mw and mo) else ""
        flag_w = re.search(r"-ftree-parallelize-loops=\d+", g("with", "flags") or "")
        rows.append({
            "kernel": k, "array_index": idx, "order": "with,without" if idx % 2 else "without,with",
            "status_with": g("with", "status"), "status_without": g("without", "status"),
            "min_ns_with": mw, "min_ns_without": mo, "ratio_min_without_over_with": ratio,
            "median_ns_with": g("with", "time_ns_median"), "median_ns_without": g("without", "time_ns_median"),
            "rsd_pct_with": rsd("with"), "rsd_pct_without": rsd("without"),
            "flag_in_with_arm": flag_w.group(0) if flag_w else "",
            "gomp_calls_with": gomp_calls("with", k), "gomp_calls_without": gomp_calls("without", k),
            "n_reps_with": g("with", "n_reps"), "n_reps_without": g("without", "n_reps"),
        })
    with open(REPO / "ftree_parallelize_ab.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)
    rs = [r["ratio_min_without_over_with"] for r in rows if r["ratio_min_without_over_with"] != ""]
    if rs:
        import math
        gm = math.exp(statistics.fmean(math.log(x) for x in rs))
        print(f"{len(rs)} kernels; ratio min_without/min_with: geomean {gm:.4f}, "
              f"min {min(rs):.4f}, max {max(rs):.4f}")
    for r in rows:
        print(r["kernel"], r["ratio_min_without_over_with"], r["gomp_calls_with"], r["gomp_calls_without"])


if __name__ == "__main__":
    main()
