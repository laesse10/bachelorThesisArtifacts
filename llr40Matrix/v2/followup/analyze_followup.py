#!/usr/bin/env python3
"""Tables and write-ups for the four follow-up experiments.

usage: analyze_followup.py {minmax,s2710,unstable,regmem}

Reads the raw per-unit rows in parts/ and the opt reports in opt_reports/, and compares them with
the v2 matrix as committed at 632952d (read with `git show`, because main no longer carries
v2/results.csv; see DEVIATIONS.md). v1 values come from llr40Matrix/results.csv. Writes
<experiment>.csv (every row keeps all its timings) and the numbers the .md write-ups quote.
"""
import csv, glob, io, json, math, pathlib, re, statistics, subprocess, sys

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parents[2]          # repository root
V2C = "632952d"                 # last commit with the complete v2 tree
GCC = ["c", "c_reference", "cpp", "fortran"]
ALL6 = ["c", "c_reference", "cpp", "fortran", "numba", "agent"]


def git_show(path):
    return subprocess.run(["git", "-C", str(ROOT), "show", f"{V2C}:{path}"], capture_output=True,
                          text=True, check=True).stdout


def matrix(text):
    return {(r["kernel"], r["representation"]): r for r in csv.DictReader(io.StringIO(text))
            if r["preset"] == "M" and not r.get("flags_variant")}


V2 = matrix(git_show("llr40Matrix/v2/results.csv"))
V1 = matrix((ROOT / "llr40Matrix/results.csv").read_text())


def ns(row):
    return [int(x) for x in (row.get("time_ns_all") or "").split()]


def rsd(x):
    return 100 * statistics.pstdev(x) / statistics.mean(x) if len(x) > 1 else float("nan")


def ms(v):
    return f"{v / 1e6:.2f}" if v else "--"


def parts(pattern):
    rows = []
    for f in sorted(glob.glob(str(HERE / "parts" / pattern))):
        name = pathlib.Path(f).name
        if name.endswith(".attempts.csv") or (name.startswith("optrep.") and "optrep." not in pattern):
            continue
        rows += list(csv.DictReader(open(f)))
    return rows


def report_lines(path):
    return pathlib.Path(path).read_text().splitlines()


VEC = re.compile(r"optimized: loop vectorized")
MISSED = re.compile(r"missed:\s+not vectorized: |missed:\s+not vectorized:|relevant stmt not supported")


def first_match(lines, pat, skip=2):
    """(1-based line number, text) of the first report line after the argv/exit header that matches."""
    for i, l in enumerate(lines[skip:], skip + 1):
        if pat.search(l):
            return i, l.strip()
    return None, ""


def asm_counts(path, mnems):
    txt = pathlib.Path(path).read_text() if path and pathlib.Path(path).is_file() else ""
    out = {}
    for m in mnems:
        n = len(re.findall(rf"\t{re.escape(m)}\t", txt))
        if n:
            out[m] = n
    return out


def fmt_counts(d):
    return " ".join(f"{k}={v}" for k, v in d.items()) or "none"


def write_csv(name, rows, fields):
    with open(HERE / name, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=fields, extrasaction="ignore")
        w.writeheader()
        w.writerows(rows)
    print(f"wrote {name}: {len(rows)} rows")


# ----------------------------------------------------------------------------- experiment 1
MINMAX_K = ["tsvc_2_s316", "tsvc_2_s318", "tsvc_2_s3110", "argmax_with_index"]
REDUCE_MNEM = ["fminnmv", "fmaxnmv", "fminv", "fmaxv", "fminnm", "fmaxnm", "fmin", "fmax", "fcsel", "fcmgt",
               "fcmge", "fcmlt", "fcmle", "fcmpe", "fcmp", "sel", "csel", "ld1d", "whilelo"]


def minmax():
    var = {(r["kernel"], r["representation"]): r for r in parts("minmax/tsvc_*.csv") + parts("minmax/argmax*.csv")
           if not r["kernel"].startswith("optrep")}
    opt = {(r["kernel"], r["representation"]): r for r in parts("minmax/optrep.*.csv")}
    v2idx = {(r["kernel"], r["representation"]): r
             for r in csv.DictReader(io.StringIO(git_show("llr40Matrix/v2/opt_reports_index.csv")))}
    rows = []
    for k in MINMAX_K:
        for rep in GCC:
            r, o, b = var.get((k, rep), {}), opt.get((k, rep), {}), V2[(k, rep)]
            t, tb = ns(r), ns(b)
            row = {"kernel": k, "representation": rep, "variant": r.get("variant", ""),
                   "flags_variant": r.get("flags_variant", ""), "status": r.get("status", "not_run"),
                   "compiler_version": r.get("compiler_version", ""), "v2_compiler_version": b["compiler_version"]}
            if o.get("optreport"):
                lines = report_lines(HERE / o["optreport"])
                vl, vt = first_match(lines, VEC)
                ml, mt = first_match(lines, MISSED)
                row.update(vectorized="yes" if vl else "no",
                           vec_report=f"{o['optreport']}:{vl}: {vt}" if vl else "",
                           first_missed=f"{o['optreport']}:{ml}: {mt}" if (ml and not vl) else "",
                           compile_argv=o["compile_argv"],
                           asm_instructions=fmt_counts(asm_counts(HERE / o["asm"], REDUCE_MNEM)) if o.get("asm") else "")
                row["missed_reasons_all"] = " || ".join(sorted({re.sub(r"^.*?missed:\s+", "", l.strip()) for l in lines[2:]
                                                              if MISSED.search(l)}))
            bo = v2idx.get((k, rep), {})
            if bo.get("optreport"):
                bl = git_show(f"llr40Matrix/v2/{bo['optreport']}").splitlines()
                vl, vt = first_match(bl, VEC)
                ml, mt = first_match(bl, MISSED)
                row["v2_vectorized"] = "yes" if vl else "no"
                row["v2_first_missed"] = f"v2/{bo['optreport']}:{ml}: {mt}" if ml else ""
                v2asm = git_show(f"llr40Matrix/v2/{bo['asm']}")
                row["v2_asm_instructions"] = fmt_counts({m: n for m in REDUCE_MNEM + ["b.gt", "b.mi", "b.le", "b.pl"]
                                                         if (n := len(re.findall(rf"\t{re.escape(m)}\t", v2asm)))})
            if o.get("asm"):
                row["asm_instructions"] = fmt_counts(asm_counts(HERE / o["asm"], REDUCE_MNEM + ["b.gt", "b.mi", "b.le", "b.pl"]))
            row.update(n_reps=len(t), time_ns_min=min(t) if t else "", time_ns_median=int(statistics.median(t)) if t else "",
                       rsd_pct=f"{rsd(t):.2f}" if t else "", v2_time_ns_min=min(tb), v2_rsd_pct=f"{rsd(tb):.2f}",
                       min_ratio_variant_over_v2=f"{min(t) / min(tb):.3f}" if t else "",
                       slurm_job=r.get("slurm_job", ""), node=r.get("node", ""), notes=r.get("notes", ""),
                       time_ns_all=r.get("time_ns_all", ""))
            rows.append(row)
    fields = ["kernel", "representation", "variant", "flags_variant", "status", "vectorized", "vec_report",
              "first_missed", "missed_reasons_all", "v2_vectorized", "v2_first_missed", "asm_instructions",
              "v2_asm_instructions", "compiler_version",
              "v2_compiler_version", "n_reps", "time_ns_min", "time_ns_median", "rsd_pct", "v2_time_ns_min",
              "v2_rsd_pct", "min_ratio_variant_over_v2", "slurm_job", "node", "notes", "compile_argv", "time_ns_all"]
    write_csv("minmax_finite_math.csv", rows, fields)
    for r in rows:
        print(f"{r['kernel']:18s} {r['representation']:12s} {r['status']:9s} vec={r.get('vectorized')} "
              f"min={ms(r['time_ns_min'])} v2={ms(r['v2_time_ns_min'])} ratio={r['min_ratio_variant_over_v2']} "
              f"rsd={r['rsd_pct']} | {r.get('vec_report') or r.get('first_missed')}")
        print(f"{'':31s} asm: {r.get('asm_instructions')}  | v2 asm: {r.get('v2_asm_instructions')}")
        print(f"{'':31s} missed: {r.get('missed_reasons_all')}  | v2 first: {r.get('v2_first_missed')}")


# ----------------------------------------------------------------------------- experiment 2
def s2710():
    k = "tsvc_2_s2710"
    var = {r["representation"]: r for r in parts("s2710/tsvc_2_s2710.csv")}
    opt = {r["representation"]: r for r in parts("s2710/optrep.*.csv")}
    rows = []
    base = {rep: min(ns(V2[(k, rep)])) for rep in ("c", "cpp", "c_reference", "numba", "agent")}
    for rep, label in (("c", "A: translated C"), ("cpp", "B: translated C++"), ("c_reference", "B: hand-written C reference")):
        r, o = var[rep], opt[rep]
        t = ns(r)
        lines = report_lines(HERE / o["optreport"])
        vl, vt = first_match(lines, VEC)
        ml, mt = first_match(lines, MISSED)
        rows.append({"kernel": k, "representation": rep, "variant": r["variant"], "variant_label": label,
                     "status": r["status"], "vectorized": "yes" if vl else "no",
                     "vec_report": f"{o['optreport']}:{vl}: {vt}" if vl else "",
                     "first_missed": f"{o['optreport']}:{ml}: {mt}" if (ml and not vl) else "",
                     "asm_instructions": fmt_counts(asm_counts(HERE / o["asm"], ["ld1d", "st1d", "whilelo", "fcmgt", "fmla", "fmad", "ldr", "str", "fcmpe"])),
                     "compiler_version": r["compiler_version"], "n_reps": len(t), "time_ns_min": min(t),
                     "time_ns_median": int(statistics.median(t)), "rsd_pct": f"{rsd(t):.2f}",
                     "v2_same_column_min_ns": base[rep],
                     "speedup_vs_v2_same_column": f"{base[rep] / min(t):.2f}",
                     "v2_c_min_ns": base["c"], "v2_cpp_min_ns": base["cpp"], "v2_c_reference_min_ns": base["c_reference"],
                     "v2_numba_min_ns": base["numba"], "ratio_vs_v2_numba": f"{min(t) / base['numba']:.3f}",
                     "source_sha256": r["source_sha256"], "slurm_job": r["slurm_job"], "node": r["node"],
                     "notes": r["notes"], "time_ns_all": r["time_ns_all"]})
    write_csv("s2710_hoist.csv", rows, list(rows[0].keys()))
    for r in rows:
        print({x: r[x] for x in ("representation", "status", "vectorized", "time_ns_min", "time_ns_median", "rsd_pct",
                                 "speedup_vs_v2_same_column", "ratio_vs_v2_numba", "asm_instructions")})
    print("v2 mins:", {x: ms(v) for x, v in base.items()})


# ----------------------------------------------------------------------------- experiment 3
UNSTABLE_K = ["tsvc_2_s1232", "tsvc_2_s231", "tsvc_2_s235", "tsvc_2_s2275", "tsvc_2_s275"]


def perf_runtime_ns(raw):
    """enabled time of the cycles counter (4th field of perf stat -x, output)."""
    for part in (raw or "").split(" | "):
        f = part.split(",")
        if len(f) > 3 and f[2].startswith("cycles"):
            try:
                return int(float(f[3]))
            except ValueError:
                return None
    return None


def unstable():
    rows_all = parts("unstable/*.round*.csv")
    rows_all = [r for r in rows_all if "attempts" not in r.get("kernel", "")]
    out, summary = [], {}
    for k in UNSTABLE_K:
        rs = [r for r in rows_all if r["kernel"] == k]
        if not rs:
            continue
        gcc_fast = min(x for r in rs if r["representation"] in GCC for x in ns(r))
        own_fast = {rep: min((x for r in rs if r["representation"] == rep for x in ns(r)), default=None) for rep in ALL6}
        by_round = {}
        for r in rs:
            by_round.setdefault(int(r["round"]), {})[r["representation"]] = r
        for rd, cells in sorted(by_round.items()):
            mins = {rep: min(ns(c)) for rep, c in cells.items() if ns(c)}
            rank_all = {rep: i for i, rep in enumerate(sorted(mins, key=mins.get), 1)}
            gm = {rep: v for rep, v in mins.items() if rep in GCC}
            rank_gcc = {rep: i for i, rep in enumerate(sorted(gm, key=gm.get), 1)}
            best_gcc = min(gm.values()) if gm else None
            for rep in ALL6:
                c = cells.get(rep)
                if not c:
                    continue
                t = ns(c)
                cyc = int(c["perf_cycles"]) if c.get("perf_cycles") else None
                ins = int(c["perf_instructions"]) if c.get("perf_instructions") else None
                en = perf_runtime_ns(c.get("perf_raw"))
                out.append({
                    "kernel": k, "round": rd, "position": c["position"], "representation": rep, "status": c["status"],
                    "n_reps": len(t), "time_ns_min": min(t) if t else "", "time_ns_median": int(statistics.median(t)) if t else "",
                    "rsd_pct": f"{rsd(t):.2f}" if t else "",
                    "frac_within15_kernel_gcc_fastest": f"{sum(x <= 1.15 * gcc_fast for x in t) / len(t):.3f}" if t else "",
                    "frac_within15_own_fastest": f"{sum(x <= 1.15 * own_fast[rep] for x in t) / len(t):.3f}" if t else "",
                    "round_rank_all6": rank_all.get(rep, ""), "round_rank_gcc": rank_gcc.get(rep, ""),
                    "ratio_to_round_best_gcc": f"{mins[rep] / best_gcc:.3f}" if (rep in mins and best_gcc) else "",
                    "perf_cycles": cyc or "", "perf_instructions": ins or "",
                    "ipc": f"{ins / cyc:.3f}" if (cyc and ins) else "",
                    "ghz_counter_enabled": f"{cyc / en:.3f}" if (cyc and en) else "",
                    "perf_gated_reps": c.get("perf_gated_reps", ""), "vmstat_delta": c.get("vmstat_delta", ""),
                    "slurm_job": c["slurm_job"], "node": c["node"], "timestamp": c["timestamp"],
                    "compiler_version": c["compiler_version"], "notes": c["notes"], "time_ns_all": c["time_ns_all"]})
        summary[k] = {"gcc_fast": gcc_fast, "own_fast": own_fast, "by_round": by_round}
    if out:
        write_csv("unstable_interleaved.csv", out, list(out[0].keys()))
    return out, summary


def unstable_report():
    out, summary = unstable()
    for k, s in summary.items():
        print(f"\n=== {k}  fastest GCC run {ms(s['gcc_fast'])} ms")
        rows = [r for r in out if r["kernel"] == k]
        rounds = sorted({r["round"] for r in rows})
        print(f"{'repr':12s} {'overall':>8s} " + " ".join(f"{'r' + str(x):>7s}" for x in rounds)
              + "  spread  w15gcc  w15own   v2min   v1min  ipc range")
        for rep in ALL6:
            rr = [r for r in rows if r["representation"] == rep]
            if not rr:
                continue
            mins = {r["round"]: int(r["time_ns_min"]) for r in rr if r["time_ns_min"] != ""}
            allt = [x for r in rr for x in ns(r)]
            ipcs = [float(r["ipc"]) for r in rr if r["ipc"]]
            print(f"{rep:12s} {ms(min(allt)):>8s} " + " ".join(f"{ms(mins.get(x)):>7s}" for x in rounds)
                  + f"  {max(mins.values()) / min(mins.values()):.3f}"
                  + f"  {sum(x <= 1.15 * s['gcc_fast'] for x in allt) / len(allt):.3f}"
                  + f"  {sum(x <= 1.15 * min(allt) for x in allt) / len(allt):.3f}"
                  + f"  {ms(min(ns(V2[(k, rep)])))}  {ms(min(ns(V1[(k, rep)])) if ns(V1[(k, rep)]) else 0)}"
                  + (f"  {min(ipcs):.2f}-{max(ipcs):.2f}" if ipcs else ""))
        for rd in rounds:
            rr = sorted([r for r in rows if r["round"] == rd], key=lambda r: int(r["time_ns_min"] or 1e18))
            print(f"  round {rd}: " + " < ".join(f"{r['representation']}({ms(int(r['time_ns_min']))})" for r in rr))


# ----------------------------------------------------------------------------- experiment 4
ELEMENTS = {"versioned_distance_update": 95_000_000 - 1,           # i = K .. LEN_1D-1 at K = 1
            "wf_triangular": 17409 * (17409 - 1) // 2}             # sum_{i=1}^{N-1} (N - i)
ASSUMED_GHZ = 3.26
# Hot loop of each cell, read by hand from the disassembly (line numbers in the .s.txt files).
R = "opt_reports/regmem"
ASM = {
    ("versioned_distance_update", "c", ""): (
        "yes", f"{R}/unchanged/versioned_distance_update/c/versioned_distance_update_fp64.c.s.txt:43-51",
        "K=1 takes the scalar loop at 0xa0-0xc0 (cmp x5,#0x8; b.eq): ldr d26,[x5,x3,lsl #3] reloads a[i-1], "
        "which the previous iteration stored (str d26); chain = store -> load -> fmadd"),
    ("versioned_distance_update", "c", "vdu_k1_scalar"): (
        "no", f"{R}/vdu_k1_scalar/versioned_distance_update/c/versioned_distance_update_fp64.c.s.txt:44-51",
        "K==1 loop at 0xa4-0xc0 loads b[i], c[i] only; fmadd d24,d24,d23,d21 carries prev in d24; same fmul+fmadd "
        "contraction as the unchanged loop"),
    ("versioned_distance_update", "agent", ""): (
        "no", f"{R}/unchanged/versioned_distance_update/agent/versioned_distance_update_fp64.f90.s.txt:53-60",
        "loop at 0xc8-0xe4 (_omp_fn.0): fmadd d1,d1,d3,d0 carries the value in d1"),
    ("wf_triangular", "numba", ""): (
        "yes", f"{R}/unchanged/wf_triangular/numba/wf_triangular_numba_np.py.s.txt:56-86",
        "inner loop .LBB0_7 (unrolled x2): a[i,j-1] reloaded (ldur d1,[x3,#-8] line 69; ldr d1,[x4,x2] line 82) "
        "from an address built by the negative-index csel (lines 59, 61), then fadd and str to the same element"),
    ("wf_triangular", "numba", "wf_west_scalar"): (
        "no", f"{R}/wf_west_scalar/wf_triangular/numba/wf_triangular_numba_np.py.s.txt:52-72",
        "inner loop .LBB0_7 loads a[i,j] and a[i-1,j] only; fadd d0,d0,d1 carries west in d0; the j-1 csel is gone"),
    ("wf_triangular", "c", ""): (
        "no", f"{R}/unchanged/wf_triangular/c/wf_triangular_fp64.c.s.txt:14-21",
        "GCC already carries west in d31 (fadd d31,d31,d29 at 0x38); loads a[i][j], a[i-1][j] only"),
}


def regmem():
    rs = parts("regmem/*.base.csv") + parts("regmem/*.k1scalar.csv") + parts("regmem/*.westscalar.csv")
    rows = []
    for r in rs:
        k, t = r["kernel"], ns(r)
        el = ELEMENTS[k]
        cyc = int(r["perf_cycles"]) if r.get("perf_cycles") else None
        ins = int(r["perf_instructions"]) if r.get("perf_instructions") else None
        en = perf_runtime_ns(r.get("perf_raw"))
        tsum = sum(t)
        ghz = cyc / en if (cyc and en) else None
        reps = len(t)
        rows.append({
            "kernel": k, "representation": r["representation"], "variant": r["variant"], "status": r["status"],
            "n_reps": reps, "time_ns_min": min(t) if t else "", "time_ns_median": int(statistics.median(t)) if t else "",
            "rsd_pct": f"{rsd(t):.2f}" if t else "", "v2_time_ns_min": min(ns(V2[(k, r['representation'])])),
            "elements_per_call": el, "perf_cycles": cyc or "", "perf_instructions": ins or "",
            "perf_gated_reps": r.get("perf_gated_reps", ""), "perf_counter_enabled_ns": en or "",
            "timed_ns_sum": tsum, "ghz_measured": f"{ghz:.4f}" if ghz else "",
            "ghz_cycles_over_timed_ns": f"{cyc / tsum:.4f}" if (cyc and tsum) else "",
            "cycles_per_element_mean": f"{cyc / (reps * el):.3f}" if cyc else "",
            "cycles_per_element_at_min": f"{min(t) * ghz / el:.3f}" if (ghz and t) else "",
            "cycles_per_element_assumed_3.26GHz": f"{min(t) * ASSUMED_GHZ / el:.3f}" if t else "",
            "instructions_per_element": f"{ins / (reps * el):.3f}" if ins else "",
            "ipc": f"{ins / cyc:.3f}" if (cyc and ins) else "",
            "reload_in_hot_loop": ASM[(k, r["representation"], r["variant"])][0],
            "asm_citation": ASM[(k, r["representation"], r["variant"])][1],
            "asm_evidence": ASM[(k, r["representation"], r["variant"])][2],
            "source_sha256": r["source_sha256"], "slurm_job": r["slurm_job"], "node": r["node"],
            "compiler_version": r["compiler_version"], "notes": r["notes"], "perf_raw": r.get("perf_raw", ""),
            "time_ns_all": r["time_ns_all"]})
    if rows:
        write_csv("register_vs_memory.csv", rows, list(rows[0].keys()))
    for r in rows:
        print({x: r[x] for x in ("kernel", "representation", "variant", "status", "time_ns_min", "rsd_pct",
                                 "ghz_measured", "cycles_per_element_mean", "cycles_per_element_at_min",
                                 "cycles_per_element_assumed_3.26GHz", "instructions_per_element", "ipc",
                                 "perf_gated_reps")})


if __name__ == "__main__":
    {"minmax": minmax, "s2710": s2710, "unstable": unstable_report, "regmem": regmem}[sys.argv[1]]()
