#!/usr/bin/env python3
"""Part B analysis: C against Fortran agent submissions on GH200.

Inputs: parts/<kernel>.csv (sweep_agents.py), ../agent_picks.json. Outputs: results.csv (all rows),
validity.csv, paired_<stat>.csv per statistic and setting, stats.csv (the per-statistic summaries),
spearman.csv; analysis.md quotes them.

Definitions
- valid: the candidate's default/T1 run validated against the oracle (status ok). gcc14 validity is
  its own gcc14/T1 status.
- time of a candidate = its min-of-k (30 timed runs) in that (build, threads) setting.
- per (model, kernel, language), over that model's submissions in that language:
    best      the fastest valid submission
    FIRST     the submission with the lowest seq of the earliest job; missing if it is not valid
    median    the median time over the valid submissions
    bestfirstk  k = min(#C submissions, #Fortran submissions) of that (model, kernel); each language's
              first k submissions in campaign order (job, then seq), and the fastest valid one among
              them; missing if none of the k is valid
- pairing: (model, kernel) where both languages have the statistic. Ratio r = t_Fortran / t_C (> 1:
  C faster). Geometric mean of r with a 95% interval from the t distribution on log r; Wilcoxon
  signed-rank p on log r (two-sided); counts C better (r > 1.1), Fortran better (r < 1/1.1),
  within 10%.
"""
import csv, glob, json, math, pathlib, statistics, sys
from collections import defaultdict

HERE = pathlib.Path(__file__).resolve().parent
picks = json.load(open(HERE.parent / "agent_picks.json"))
rows = []
for f in sorted(glob.glob(str(HERE / "parts" / "*.csv"))):
    rows += list(csv.DictReader(open(f)))
if not rows:
    sys.exit("no rows")
with open(HERE / "results.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=list(rows[0].keys())); w.writeheader(); w.writerows(rows)

cand = [r for r in rows if r["role"] == "candidate"]
ref = {(r["kernel"], r["threads"]): int(r["time_ns_min"]) for r in rows
       if r["role"] == "translated_c" and r["status"] == "ok" and r["time_ns_min"]}
by = {}   # (sha256, build, threads) -> row
for r in cand:
    by[(r["kernel"], r["sha256"], r["build"], r["threads"])] = r


def tmin(r):
    return int(r["time_ns_min"]) if r and r["status"] == "ok" and r["time_ns_min"] else None


# ---------------------------------------------------------------- build / validity shares
def share_table(build):
    out = []
    keyf = lambda r: (r["language"], r["model"])
    groups = defaultdict(list)
    for r in cand:
        if r["build"] == build and r["threads"] == "1":
            groups[keyf(r)].append(r)
            groups[(r["language"], "all")].append(r)
    for (lang, model), rs in sorted(groups.items()):
        n = len(rs)
        built = sum(r["status"] != "build_error" for r in rs)
        valid = sum(r["status"] == "ok" for r in rs)
        fc = defaultdict(int)
        for r in rs:
            if r["status"] != "ok":
                fc[r["failure_class"] or r["status"]] += 1
        out.append({"build": build, "language": lang, "model": model, "submissions": n, "built": built,
                    "built_share": f"{built / n:.3f}", "valid": valid, "valid_share": f"{valid / n:.3f}",
                    "build_error_x86_intrinsics": fc["x86_intrinsics"], "build_error_other": fc["other"],
                    "incorrect": fc["incorrect"], "timeout": fc["timeout"],
                    "uses_x86_intrinsics": sum(r["uses_x86_intrinsics"] == "yes" for r in rs),
                    "sets_thread_count": sum(r["sets_thread_count"] == "yes" for r in rs)})
    return out


validity = share_table("default") + share_table("gcc14")
with open(HERE / "validity.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=list(validity[0].keys())); w.writeheader(); w.writerows(validity)


# ---------------------------------------------------------------- statistics
def subs(kernel, model, lang):
    """this model's submissions of this kernel in this language, in campaign order"""
    return sorted([c for c in picks[kernel] if c["arm"].split("-")[1] == model and c["language"] == lang],
                  key=lambda c: (int(c["job"]), int(c["seq"])))


def stat_value(kernel, model, lang, stat, build, threads, k_first):
    ss = subs(kernel, model, lang)
    if not ss:
        return None, 0
    def t(c):
        # timed only if valid in its own build: default T72 needs default/T1 valid; gcc14 its gcc14/T1
        thr = threads.split("/")[0]
        if build == "default" and thr == "72":
            v = by.get((kernel, c["sha256"], "default", "1"))
            if not v or v["status"] != "ok":
                return None
            if threads.endswith("/identical"):
                r72 = by.get((kernel, c["sha256"], "default", "72"))
                if not r72 or not same_binary(v, r72):
                    return None
        return tmin(by.get((kernel, c["sha256"], build, thr)))
    times = [t(c) for c in ss]
    valid = [x for x in times if x is not None]
    if stat == "best":
        return (min(valid) if valid else None), len(valid)
    if stat == "first":
        return times[0], 1
    if stat == "median":
        return (statistics.median(valid) if valid else None), len(valid)
    if stat == "bestfirstk":
        sub = [x for x in times[:k_first] if x is not None]
        return (min(sub) if sub else None), len(sub)
    raise ValueError(stat)


def tq975(df):
    from scipy.stats import t
    return float(t.ppf(0.975, df))


def summarize(pairs):
    if not pairs:
        return {"n_pairs": 0}
    lr = [math.log(p["ratio_f_over_c"]) for p in pairs]
    n = len(lr)
    gm = math.exp(statistics.mean(lr))
    if n >= 2 and statistics.stdev(lr) > 0:
        h = tq975(n - 1) * statistics.stdev(lr) / math.sqrt(n)
        lo, hi = math.exp(statistics.mean(lr) - h), math.exp(statistics.mean(lr) + h)
    else:
        lo = hi = None
    try:
        from scipy.stats import wilcoxon
        p = float(wilcoxon(lr).pvalue) if n >= 2 and any(x != 0 for x in lr) else None
    except ValueError:
        p = None
    return {"n_pairs": n, "geomean_f_over_c": f"{gm:.3f}", "ci95_low": f"{lo:.3f}" if lo else "",
            "ci95_high": f"{hi:.3f}" if hi else "", "wilcoxon_p": f"{p:.4f}" if p is not None else "",
            "c_better": sum(x > math.log(1.1) for x in lr), "fortran_better": sum(x < -math.log(1.1) for x in lr),
            "within_10pct": sum(abs(x) <= math.log(1.1) for x in lr)}


def same_binary(r1, r72):
    """T72 library byte-identical to default/T1's (pilot rows predate the column: compare hashes)."""
    flag = r72.get("t72_binary_equals_t1", "")
    if flag:
        return flag == "yes"
    return bool(r72["lib_sha256"]) and r72["lib_sha256"] == r1["lib_sha256"]


settings = [("default", "1"), ("default", "72"), ("default", "72/identical"), ("gcc14", "1")]
models = sorted({c["arm"].split("-")[1] for v in picks.values() for c in v})
stats_rows = []
for build, threads in settings:
    for stat in ("best", "first", "median", "bestfirstk"):
        pairs = []
        for kernel, v in picks.items():
            for model in models:
                nc, nf = len(subs(kernel, model, "c")), len(subs(kernel, model, "fortran"))
                if nc == 0 or nf == 0:
                    continue
                kf = min(nc, nf)
                tc, nvc = stat_value(kernel, model, "c", stat, build, threads, kf)
                tf, nvf = stat_value(kernel, model, "fortran", stat, build, threads, kf)
                if tc is None or tf is None:
                    continue
                pairs.append({"kernel": kernel, "model": model, "n_c": nc, "n_fortran": nf, "k": kf,
                              "t_c_ns": int(tc), "t_fortran_ns": int(tf), "ratio_f_over_c": tf / tc})
        with open(HERE / f"paired_{stat}_{build}_t{threads.replace('/', '_')}.csv", "w", newline="") as fh:
            fields = ["kernel", "model", "n_c", "n_fortran", "k", "t_c_ns", "t_fortran_ns", "ratio_f_over_c"]
            w = csv.DictWriter(fh, fieldnames=fields); w.writeheader(); w.writerows(pairs)
        for model in ["pooled"] + models:
            sel = [p for p in pairs if model == "pooled" or p["model"] == model]
            stats_rows.append({"build": build, "threads": threads, "statistic": stat, "model": model, **summarize(sel)})
fields = ["build", "threads", "statistic", "model", "n_pairs", "geomean_f_over_c", "ci95_low", "ci95_high",
          "wilcoxon_p", "c_better", "fortran_better", "within_10pct"]
with open(HERE / "stats.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=fields, extrasaction="ignore"); w.writeheader(); w.writerows(stats_rows)

# ---------------------------------------------------------------- Spearman per arm
from scipy.stats import spearmanr
sp = []
arms = sorted({c["arm"] for v in picks.values() for c in v})
for arm in arms:
    for threads in ("1", "72"):
        xs, ys = [], []
        for r in cand:
            if r["arm"] != arm or r["build"] != "default" or r["threads"] != threads or r["status"] != "ok":
                continue
            if threads == "72" and by.get((r["kernel"], r["sha256"], "default", "1"), {}).get("status") != "ok":
                continue
            rt = ref.get((r["kernel"], threads))
            if not rt or not r["time_ns_min"]:
                continue
            xs.append(float(r["campaign_speedup"])); ys.append(rt / int(r["time_ns_min"]))
        rho, p = spearmanr(xs, ys) if len(xs) >= 3 else (float("nan"), float("nan"))
        sp.append({"arm": arm, "threads": threads, "n": len(xs), "spearman_rho": f"{rho:.3f}", "p": f"{p:.4g}"})
with open(HERE / "spearman.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=["arm", "threads", "n", "spearman_rho", "p"]); w.writeheader(); w.writerows(sp)

if __name__ == "__main__":
    for v in validity:
        print({k: v[k] for k in ("build", "language", "model", "submissions", "built", "valid",
                                 "build_error_x86_intrinsics", "build_error_other", "incorrect", "timeout")})
    for s in stats_rows:
        if s["model"] == "pooled":
            print(s)
    for s in sp:
        print(s)
