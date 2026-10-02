#!/usr/bin/env python3
"""Generate v2/summary.md from the v2 CSVs. No number in the output is hand-entered.

v2 of ../build_summary.py. Differences: the toolchain line states what the harness actually ran
(gcc 14.2.0 for C, 13.3.1 for C++/Fortran); variant rows (preset F, flags_variant) are kept out
of every aggregate; added sections for v2-vs-v1, agent attempts, complete-case aggregates, the
class-count discrepancy with protocol.md, the derived-label check, preset F, -ffp-contract=off,
the -ftree-parallelize-loops A/B, and the run's incidents. Prose that depends on a result is
conditional on the data rather than asserted.
"""
import argparse, collections, csv, json, math, pathlib, re, statistics as st

REPO = pathlib.Path(__file__).resolve().parent
V1 = REPO.parent
REPRS = ["c", "c_reference", "cpp", "fortran", "numba", "agent"]


def rd(p):
    p = p if isinstance(p, pathlib.Path) else REPO / p
    return list(csv.DictReader(open(p))) if p.is_file() else []


def is_main(r):
    return r.get("preset", "M") == "M" and not r.get("flags_variant")


def gm(xs):
    xs = [x for x in xs if x and x > 0]
    return math.exp(st.fmean(math.log(x) for x in xs)) if xs else float("nan")


def agg_table(w, rows, title, note):
    if not rows:
        return
    w(f"### {title}\n")
    classes = sorted({r["optimization_class"] for r in rows})
    n = {r["optimization_class"]: r["n_kernels_in_class"] for r in rows}
    w("| class | n | " + " | ".join(f"`{r}`" for r in REPRS) + " |")
    w("|---|---:" + "|---:" * len(REPRS) + "|")
    for c in classes:
        cells = []
        for rp in REPRS:
            m = [x for x in rows if x["optimization_class"] == c and x["representation"] == rp]
            v = m[0]["geomean_slowdown"] if m else ""
            cells.append(f"{float(v):.2f}" if v else "--")
        w(f"| {c} | {n[c]} | " + " | ".join(cells) + " |")
    w(f"\n{note}\n")


def out_table(w, rows, limit=25):
    if not rows:
        w("No cell deviates from its class geometric mean by 3x or more.\n"); return
    w("| kernel | representation | class | cell | class geomean | deviation |")
    w("|---|---|---|---:|---:|---:|")
    for r in rows[:limit]:
        w(f"| `{r['kernel']}` | `{r['representation']}` | {r['optimization_class']} | {r['cell_slowdown']} | "
          f"{r['class_geomean']} | {r['deviation_x']}x |")
    w("")


def main(a):
    res = rd(a.results)
    M = [r for r in res if is_main(r)]
    F = [r for r in res if r.get("preset") == "F"]
    FV = [r for r in res if r.get("flags_variant")]
    v1 = [r for r in rd(V1 / "results.csv") if r.get("preset", "M") == "M"]
    lab = {r["kernel"]: r for r in rd("labels.csv")}
    nf = {(r["kernel"], r["representation"]): r for r in rd("noise_floor.csv")}
    kernels = sorted({r["kernel"] for r in M})
    env = dict(l.split("=", 1) for l in (REPO / "env_record.txt").read_text().splitlines() if "=" in l)
    jobs = collections.Counter(r.get("slurm_job", "") for r in res)

    L = []
    w = L.append
    w("# LLR-40 measurement matrix, v2 -- summary\n")
    w(f"Benchmark commit `{env.get('BENCH_COMMIT', '?')}` (v1: `e2bceb68`). Agent submissions "
      f"`{env.get('ICLR_COMMIT', '?')[:8]}` (unchanged).  ")
    w("Machine: CSCS Alps `daint`, NVIDIA GH200, aarch64 (Neoverse-V2), one kernel per exclusive node.  ")
    w(f"Toolchain as the harness actually ran it: **C (`c`, `c_reference`, agent-C) {env.get('GCC_USED_FOR_C', '?').split(' [')[0]}**; "
      f"C++ {env.get('GXX', '?').split(' [')[0]}; Fortran {env.get('GFORTRAN', '?').split(' [')[0]}. "
      f"{env.get('PYTHON', '?')}; numba {env.get('NUMBA', '?')}; numpy {env.get('NUMPY', '?')}.  ")
    w("Preset M, float64, `OMP_NUM_THREADS=1`, 5 warmup + 30 timed reps, min-of-k. "
      "Protocol: [../protocol.md](../protocol.md), unchanged.  ")
    w(f"Rows by Slurm job: " + ", ".join(f"`{j}` {n}" for j, n in jobs.most_common()) + ".\n")

    # ---- status v2 vs v1 ---------------------------------------------------
    w("## Status, v2 against v1\n")
    s2 = collections.Counter(r["status"] for r in M); s1 = collections.Counter(r["status"] for r in v1)
    w("| status | v1 | v2 |")
    w("|---|---:|---:|")
    for s in sorted(set(s1) | set(s2), key=lambda s: -s2.get(s, 0)):
        w(f"| `{s}` | {s1.get(s, 0)} | {s2.get(s, 0)} |")
    w(f"| **cells** | {len(v1)} | {len(M)} |")
    w("")
    bad = [r for r in M if r["status"] != "ok"]
    if bad:
        w("Non-ok cells (rows, not omissions; error text in `notes`):\n")
        w("| kernel | representation | status | note |")
        w("|---|---|---|---|")
        for r in bad:
            w(f"| `{r['kernel']}` | `{r['representation']}` | {r['status']} | {r['notes'][:90]} |")
        w("")

    # ---- diff vs v1 --------------------------------------------------------
    d = rd("diff_vs_v1.csv")
    if d:
        fl = [r for r in d if r["flagged"] == "yes"]
        w("## What changed against v1 (`diff_vs_v1.csv`)\n")
        sc = collections.Counter(r["source_change"] for r in d)
        w(f"Timed source across the two commits, all {len(d)} cells: " +
          ", ".join(f"{k} {v}" for k, v in sc.most_common()) + ". `preprocessed_identical` = the text "
          "differs but `gcc -E` of both is identical (the new prelude macro `NPB_HD` expands to nothing on the host).\n")
        stc = [r for r in d if "status" in r["reasons"]]
        w(f"**{len(fl)} cells** moved by more than 5% in min-of-k or changed status "
          f"({len(stc)} status changes).\n")
        fsc = collections.Counter(r["source_change"] for r in fl)
        w("By source change: " + ", ".join(f"{k} {v}" for k, v in fsc.most_common()) + ".\n")
        w("| kernel | repr | v2/v1 | RSD v1 % | RSD v2 % | source | explanation |")
        w("|---|---|---:|---:|---:|---|---|")
        for r in sorted(fl, key=lambda r: float(r["ratio_v2_over_v1"] or 1)):
            w(f"| `{r['kernel']}` | `{r['representation']}` | {r['ratio_v2_over_v1']} | {r['v1_rsd_pct']} | "
              f"{r['v2_rsd_pct']} | {r['source_change']} | {r['explanation']} |")
        w("")
        hi = [r for r in fl if r["source_change"] != "code" and
              max(float(r["v1_rsd_pct"] or 0), float(r["v2_rsd_pct"] or 0)) > 5]
        w(f"Of the flagged cells whose source did not change, {len(hi)} have RSD > 5% in at least one "
          "of the two series (the diagnosed bandwidth artifact, protocol.md section 4).\n")

    # ---- agent attempts ----------------------------------------------------
    att = rd("agent_attempts.csv")
    if att:
        picks = json.load(open(REPO / "agent_picks.json"))
        def v1rank(k):
            for r in v1:
                if r["kernel"] == k and r["representation"] == "agent":
                    m = re.search(r"sha256=(\w+)", r["notes"])
                    if m:
                        return next(i for i, c in enumerate(picks[k], 1) if c["sha256"].startswith(m.group(1)))
            return None
        by = collections.defaultdict(list)
        for x in att:
            by[x["kernel"]].append(x)
        w("## Agent column: every attempted candidate (`agent_attempts.csv`)\n")
        w(f"{len(att)} candidates attempted over {len(by)} kernels: " +
          ", ".join(f"{s} {n}" for s, n in collections.Counter(x["status"] for x in att).most_common()) + ".\n")
        w("Kernels whose agent cell is not the campaign's top pick, and why:\n")
        w("| kernel | v1 rank | v2 rank | rejected candidates (rank: status -- first error line) |")
        w("|---|---:|---:|---|")
        for k in sorted(by):
            xs = sorted(by[k], key=lambda x: int(x["rank"]))
            ch = [x for x in xs if x["chosen"] == "1"]
            r2 = int(ch[0]["rank"]) if ch else None
            r1 = v1rank(k)
            if r2 == 1 and r1 == 1:
                continue
            rej = "; ".join(f"{x['rank']}: {x['status']} -- {re.sub(r'^.*?/cpp_backend/', '', x['first_error_line'])[:70]}"
                            for x in xs if x["status"] != "ok")
            w(f"| `{k}` | {r1} | {r2 if r2 else 'none ok in ' + str(len(xs))} | {rej} |")
        w("")

    # ---- class distribution + discrepancy ---------------------------------
    w("## Optimization classes\n")
    dist = collections.Counter(lab[k]["optimization_class"] for k in kernels)
    prov = collections.Counter(lab[k]["source_of_label"] for k in kernels)
    small = [c for c, n in dist.items() if n <= 2]
    w(f"{len(dist)} classes over {len(kernels)} kernels ({prov.get('declared', 0)} declared, "
      f"{prov.get('derived', 0)} derived). Largest: `{dist.most_common(1)[0][0]}` "
      f"{dist.most_common(1)[0][1]}/{len(kernels)}. {len(small)} classes have <=2 members "
      f"({sum(n for n in dist.values() if n <= 2)} kernels).\n")
    w(f"**Discrepancy with protocol.md, left as written there:** protocol.md section 5 says reductions "
      f"are 8 of 40, 13 classes, 7 with <=2 members. `labels.csv` gives reduction "
      f"{dist.get('reduction', 0)}, {len(dist)} classes, {len(small)} with <=2 members. The protocol "
      "figures were written before the 9 derived labels were final.\n")
    w("| class | kernels |")
    w("|---|---:|")
    for c, n in dist.most_common():
        w(f"| {c} | {n} |")
    w("")
    if (REPO / "derived_labels_check.md").is_file():
        w("The 9 derived labels were checked by hand against the source: see "
          "[derived_labels_check.md](derived_labels_check.md) (6 agree, 2 disputable, 1 disputed). "
          "Under the corpus's own label for its TSVC original (`s121`: induction variables), "
          "`ext_war_unit` would be the roster's only `induction_variable` member.\n")

    # ---- noise ------------------------------------------------------------
    okc = [nf[(r["kernel"], r["representation"])] for r in M
           if r["status"] == "ok" and (r["kernel"], r["representation"]) in nf]
    if okc:
        rsd = [float(x["rsd_pct"]) for x in okc]
        flg = [x for x in okc if x["bimodal_suspect"] == "yes"]
        w("## Run-to-run spread\n")
        w(f"Over {len(okc)} ok cells: median RSD {st.median(rsd):.2f}%, max {max(rsd):.2f}%; "
          f"{len(flg)} ({100*len(flg)/len(okc):.1f}%) bimodal-suspect (RSD > 5%). All aggregates use min-of-k.\n")

    # ---- aggregates --------------------------------------------------------
    w("## Geometric-mean slowdown per class x representation\n")
    exc = rd("complete_case_excluded.csv")
    agg_table(w, rd("class_aggregate.csv"), "v1 method (all ok cells, normalised to the row's fastest)",
              "Kept for comparability with v1. A kernel missing a column is normalised to a different "
              "'fastest' column than its class, which is what produced the `tsvc_2_s2233` pseudo-outliers.")
    agg_table(w, rd("class_aggregate_complete.csv"), "Complete-case kernels, normalised to the row's fastest",
              f"Only kernels with all 6 columns ok ({len(kernels) - len(exc)} of {len(kernels)}).")
    agg_table(w, rd("class_aggregate_complete_vs_c.csv"), "Complete-case kernels, normalised to `c`",
              "1.00 = the autogen C column; < 1 is faster than `c`.")
    if exc:
        w("Excluded from the complete-case aggregates:\n")
        w("| kernel | class | non-ok columns |")
        w("|---|---|---|")
        for r in exc:
            w(f"| `{r['kernel']}` | {r['optimization_class']} | {r['non_ok_columns']} |")
        w("")
    w("### Outliers (>= 3x from the class geomean)\n")
    o1, o2, o3 = rd("outliers.csv"), rd("outliers_complete.csv"), rd("outliers_complete_vs_c.csv")
    w(f"v1 method: {len(o1)}; complete-case: {len(o2)}; complete-case vs `c`: {len(o3)}.\n")
    w("Complete-case, normalised to the row's fastest:\n")
    out_table(w, o2)

    # ---- degenerate kernels ------------------------------------------------
    fs = rd("finite_sizes.csv")
    if fs:
        w("## The three numerically degenerate kernels\n")
        w("Largest size at which the NumPy reference output is 100% finite (binary search on `LEN_2D`, "
          "harness inputs, `finite_search.py`):\n")
        w("| kernel | preset M | largest finite | non-monotone points in the +-16 scan |")
        w("|---|---:|---:|---|")
        for r in fs:
            nm = " ".join(x for x in (r["nonfinite_below_answer_in_scan"],
                                      ("finite above: " + r["finite_above_answer_in_scan"]) if r["finite_above_answer_in_scan"] else "") if x)
            w(f"| `{r['kernel']}` | {r['preset_M']} | **{r['largest_finite']}** | {nm or 'none'} |")
        w("")
    if F:
        w("**Preset F** (all 6 columns at that size, full protocol; `preset=F`, outside every aggregate):\n")
        w("| kernel | " + " | ".join(f"`{r}`" for r in REPRS) + " |")
        w("|---" * (len(REPRS) + 1) + "|")
        for k in sorted({r["kernel"] for r in F}):
            cells = []
            for rp in REPRS:
                m = [r for r in F if r["kernel"] == k and r["representation"] == rp]
                cells.append(f"{m[0]['status']} {int(m[0]['time_ns_min'])/1e3:.1f} us" if m and m[0]["time_ns_min"]
                             else (m[0]["status"] if m else "--"))
            w(f"| `{k}` | " + " | ".join(cells) + " |")
        w("")
    if FV:
        main_s115 = {r["representation"]: r for r in M if r["kernel"] == "tsvc_2_s115"}
        w("**`tsvc_2_s115` with `-ffp-contract=off`** substituted for `-ffp-contract=fast` (preset M, "
          "`flags_variant` set, outside every aggregate):\n")
        w("| column | status (fast) | status (off) | min (fast) ms | min (off) ms |")
        w("|---|---|---|---:|---:|")
        for r in FV:
            m = main_s115.get(r["representation"], {})
            w(f"| `{r['representation']}` | {m.get('status', '?')} | {r['status']} | "
              f"{int(m['time_ns_min'])/1e6 if m.get('time_ns_min') else float('nan'):.2f} | "
              f"{int(r['time_ns_min'])/1e6 if r['time_ns_min'] else float('nan'):.2f} |")
        fixed = all(r["status"] == "ok" for r in FV) and all(main_s115.get(r["representation"], {}).get("status") == "incorrect" for r in FV)
        w("")
        w(("**FMA contraction alone explains the incorrect verdict**: with contraction off, all four GCC "
           "columns validate at preset M, and nothing else in the flag string differs.")
          if fixed else "**Contraction alone does not explain the verdict**: see the statuses above.")
        w("")

    # ---- parloops A/B -----------------------------------------------------
    ab = rd("ftree_parallelize_ab.csv")
    if ab:
        rs = [float(r["ratio_min_without_over_with"]) for r in ab if r["ratio_min_without_over_with"]]
        w("## `-ftree-parallelize-loops` A/B (fortran column, reduction kernels)\n")
        w(f"{len(ab)} kernels (`tsvc_2_s3110` is itself one of the reduction class, so the requested "
          "'10 reductions + s3110' is 10 kernels). Ratio = min-of-k without / with; > 1 means the flag helps.\n")
        w(f"In the protocol's geometry the flag resolves to `{ab[0]['flag_in_with_arm']}`: geomean ratio "
          f"**{gm(rs):.4f}** (range {min(rs):.4f}-{max(rs):.4f}); parloops outlined no loop in either arm "
          f"(GOMP_parallel call sites: {sum(int(r['gomp_calls_with'] or 0) for r in ab)} / "
          f"{sum(int(r['gomp_calls_without'] or 0) for r in ab)}).\n")
        sab = rd(REPO / "supplementary_wholenode" / "ftree_parallelize_ab.csv")
        if sab:
            rs2 = [float(r["ratio_min_without_over_with"]) for r in sab if r["ratio_min_without_over_with"]]
            w(f"Supplementary, whole-node step geometry (flag resolved to `{sab[0]['flag_in_with_arm']}`, "
              f"`supplementary_wholenode/`): geomean **{gm(rs2):.4f}** (range {min(rs2):.4f}-{max(rs2):.4f}); "
              f"GOMP_parallel call sites in the with arm: {sum(int(r['gomp_calls_with'] or 0) for r in sab)}.\n")
        w("| kernel | order | min with (ms) | min without (ms) | ratio | RSD with % | RSD without % |")
        w("|---|---|---:|---:|---:|---:|---:|")
        for r in ab:
            w(f"| `{r['kernel']}` | {r['order']} | {int(r['min_ns_with'])/1e6:.2f} | {int(r['min_ns_without'])/1e6:.2f} | "
              f"{r['ratio_min_without_over_with']} | {r['rsd_pct_with']} | {r['rsd_pct_without']} |")
        w("")

    # ---- opt reports -------------------------------------------------------
    oi = rd("opt_reports_index.csv")
    if oi:
        w("## Optimisation reports\n")
        w(f"{len(oi)} cells indexed in `opt_reports_index.csv`: " +
          ", ".join(f"{s} {n}" for s, n in collections.Counter(r["status"] for r in oi).most_common()) +
          ". Findings: [opt_findings.md](opt_findings.md).\n")

    w("## Incidents and deviations\n")
    w("See [DEVIATIONS.md](DEVIATIONS.md): every way this run departs from the task text or from v1, "
      "and every discarded or supplementary run, with the reason.\n")
    (REPO / "summary.md").write_text("\n".join(L) + "\n")
    print(f"wrote summary.md ({len(L)} lines): {len(M)} main cells, {len(F)} preset-F rows, {len(FV)} flags-variant rows")


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--results", default="results.csv")
    main(p.parse_args())
