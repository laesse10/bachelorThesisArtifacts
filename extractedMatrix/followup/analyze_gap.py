#!/usr/bin/env python3
"""Tables for the follow-up (README.md): per unit and cell, the min-of-k over all timed rounds, the
min of every round (to see whether the order moved it), and the perf counters per timed rep.
Ratios are against the unit's own `c` cell, measured on the same node in the same rounds.

usage: python3 analyze_gap.py > summary.md
"""
import csv, pathlib, statistics

HERE = pathlib.Path(__file__).resolve().parent


def main():
    out = ["| unit | kernel | cell | min of k (ms) | speedup over C | per-round min (ms), order | "
           "instructions / rep | cycles / rep | IPC | branch misses / rep | status |",
           "|---|---|---|---:|---:|---|---:|---:|---:|---:|---|"]
    for part in sorted((HERE / "parts").glob("*.csv")):
        if part.name.startswith("optrep."):
            continue
        rows = [r for r in csv.DictReader(open(part)) if r["round"] != "record"]
        if not rows:
            continue
        unit = part.stem
        labels = list(dict.fromkeys(r["label"] for r in rows))
        best = {}
        for lab in labels:
            ns = [int(x) for r in rows if r["label"] == lab and r["status"] == "ok" for x in r["time_ns_all"].split()]
            best[lab] = min(ns) if ns else None
        for lab in labels:
            rs = [r for r in rows if r["label"] == lab]
            per = ", ".join(f"r{r['round']}p{r['position']} {int(r['time_ns_min']) / 1e6:.3f}" if r["time_ns_min"] else
                            f"r{r['round']} {r['status']}" for r in rs)
            ok = [r for r in rs if r["status"] == "ok" and r["perf_cycles"]]
            n = sum(int(r["perf_gated_reps"].split(";")[-1]) for r in ok) or 1
            ins = sum(int(r["perf_instructions"]) for r in ok) / n if ok else 0
            cyc = sum(int(r["perf_cycles"]) for r in ok) / n if ok else 0
            bm = sum(int(r["perf_branch_misses"] or 0) for r in ok) / n if ok else 0
            sp = (best["c"] / best[lab]) if best.get("c") and best[lab] else None
            out.append(f"| {unit} | {rs[0]['kernel']} | {lab} | "
                       f"{best[lab] / 1e6:.3f} | {sp:.3f} | {per} | {ins:.4g} | {cyc:.4g} | "
                       f"{ins / cyc if cyc else 0:.2f} | {bm:.3g} | {'/'.join(sorted({r['status'] for r in rs}))} |"
                       if best[lab] else f"| {unit} | {rs[0]['kernel']} | {lab} | | | {per} | | | | | "
                       f"{'/'.join(sorted({r['status'] for r in rs}))} |")
    print("\n".join(out))


if __name__ == "__main__":
    main()
