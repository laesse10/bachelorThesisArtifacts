#!/usr/bin/env python3
"""Phase 5: aggregate results.csv -> figures/ + tables. No hand-entered numbers.

Slowdown is per-row: a cell's median / the fastest OK median in that kernel's row. Class
aggregates use the GEOMETRIC mean, because slowdowns are ratios.
"""
import argparse, csv, math, pathlib, statistics
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

REPO = pathlib.Path(__file__).resolve().parent
REPRS = ["c", "c_reference", "cpp", "fortran", "numba", "agent"]


def load(results, labels, preset="M"):
    """The matrix is the rows at ``preset``; rows at any other preset are EXTRA (the three
    kernels whose reference overflows at M are additionally measured at S) and are returned
    separately so they never enter the aggregates."""
    lab = {r["kernel"]: r for r in csv.DictReader(open(labels))}
    cells, extra = {}, []
    for r in csv.DictReader(open(results)):
        if r.get("preset", preset) == preset:
            cells[(r["kernel"], r["representation"])] = r
        else:
            extra.append(r)
    kernels = sorted({k for k, _ in cells}, key=lambda k: (lab[k]["optimization_class"], k))
    return cells, lab, kernels, extra


def med(cell):
    """min-of-k is the reduction every aggregate uses: the diagnosed 1.6x bimodality is external
    memory-bandwidth interference, so the MINIMUM is the undegraded machine and the median is
    contaminated whenever more than half a series falls in a slow block. This is also the
    harness's own default reducer (harness/timing.py:reduce_min_of_k)."""
    if cell and cell["status"] == "ok":
        v = cell.get("time_ns_min") or cell.get("time_ns_median")
        if v:
            return float(v)
    return None


def slowdowns(cells, kernels):
    """kernel -> {repr: slowdown vs the fastest OK cell in that row}."""
    out = {}
    for k in kernels:
        row = {r: med(cells.get((k, r))) for r in REPRS}
        ok = [v for v in row.values() if v]
        base = min(ok) if ok else None
        out[k] = {r: (v / base if (v and base) else None) for r, v in row.items()}
    return out


def heatmap(sd, lab, kernels, path):
    M = np.full((len(kernels), len(REPRS)), np.nan)
    for i, k in enumerate(kernels):
        for j, r in enumerate(REPRS):
            v = sd[k][r]
            if v: M[i, j] = math.log10(v)
    fig, ax = plt.subplots(figsize=(7.2, max(6, 0.26 * len(kernels))))
    vmax = np.nanmax(np.abs(M)) if np.isfinite(M).any() else 1
    im = ax.imshow(M, aspect="auto", cmap="magma_r", vmin=0, vmax=vmax)
    ax.set_xticks(range(len(REPRS))); ax.set_xticklabels(REPRS, rotation=45, ha="right")
    ax.set_yticks(range(len(kernels)))
    ax.set_yticklabels([f"{k}  [{lab[k]['optimization_class']}]" for k in kernels], fontsize=6)
    # class separators
    prev, bounds = None, []
    for i, k in enumerate(kernels):
        c = lab[k]["optimization_class"]
        if prev is not None and c != prev: bounds.append(i - 0.5)
        prev = c
    for b in bounds: ax.axhline(b, color="white", lw=1.1)
    for i in range(len(kernels)):
        for j in range(len(REPRS)):
            cell = sd[kernels[i]][REPRS[j]]
            ax.text(j, i, "-" if cell is None else (f"{cell:.1f}" if cell < 100 else f"{cell:.0f}"),
                    ha="center", va="center", fontsize=4.6,
                    color="white" if (not np.isnan(M[i, j]) and M[i, j] > vmax * 0.55) else "black")
    fig.colorbar(im, ax=ax, label="log10(slowdown vs fastest in row)", shrink=0.6)
    ax.set_title("LLR-40: slowdown vs fastest representation per kernel\n"
                 "(preset M, float64, 1 thread, GNU 13.3.1; '-' = no ok timing)", fontsize=9)
    fig.tight_layout(); fig.savefig(path, dpi=190); fig.savefig(str(path).replace(".png", ".pdf"))
    plt.close(fig)


def class_aggregate(sd, lab, kernels, out_csv, out_png):
    classes = sorted({lab[k]["optimization_class"] for k in kernels})
    G = np.full((len(classes), len(REPRS)), np.nan)
    rows = []
    for i, c in enumerate(classes):
        ks = [k for k in kernels if lab[k]["optimization_class"] == c]
        for j, r in enumerate(REPRS):
            vals = [sd[k][r] for k in ks if sd[k][r]]
            if vals:
                g = math.exp(statistics.fmean(math.log(v) for v in vals))
                G[i, j] = g
                rows.append({"optimization_class": c, "representation": r, "n_kernels_in_class": len(ks),
                             "n_ok_cells": len(vals), "geomean_slowdown": round(g, 4)})
            else:
                rows.append({"optimization_class": c, "representation": r, "n_kernels_in_class": len(ks),
                             "n_ok_cells": 0, "geomean_slowdown": ""})
    with open(out_csv, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)
    fig, ax = plt.subplots(figsize=(8, 4.6))
    x = np.arange(len(classes)); w = 0.8 / len(REPRS)
    for j, r in enumerate(REPRS):
        ax.bar(x + j * w, np.nan_to_num(G[:, j]), w, label=r)
    ax.set_xticks(x + 0.4 - w / 2); ax.set_xticklabels(classes, rotation=35, ha="right", fontsize=7)
    ax.set_ylabel("geometric-mean slowdown"); ax.set_yscale("log")
    ax.axhline(1.0, color="k", lw=0.7, ls=":")
    ax.set_title("Geometric-mean slowdown per (optimization class x representation)", fontsize=10)
    ax.legend(fontsize=7, ncol=3)
    fig.tight_layout(); fig.savefig(out_png, dpi=190); fig.savefig(str(out_png).replace(".png", ".pdf"))
    plt.close(fig)
    return classes, G


def coverage(cells, lab, kernels, out_csv, out_png):
    classes = sorted({lab[k]["optimization_class"] for k in kernels})
    statuses = ["ok", "incorrect", "build_error", "timeout", "unsupported"]
    rows, M = [], np.zeros((len(classes), len(REPRS)))
    for i, c in enumerate(classes):
        ks = [k for k in kernels if lab[k]["optimization_class"] == c]
        for j, r in enumerate(REPRS):
            cnt = {s: 0 for s in statuses}
            for k in ks:
                cell = cells.get((k, r))
                if cell: cnt[cell["status"]] = cnt.get(cell["status"], 0) + 1
            M[i, j] = cnt["ok"] / max(1, len(ks))
            rows.append({"optimization_class": c, "representation": r, "n_kernels": len(ks), **cnt})
    with open(out_csv, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)
    fig, ax = plt.subplots(figsize=(6.4, 4.4))
    im = ax.imshow(M, cmap="Greens", vmin=0, vmax=1, aspect="auto")
    ax.set_xticks(range(len(REPRS))); ax.set_xticklabels(REPRS, rotation=45, ha="right")
    ax.set_yticks(range(len(classes))); ax.set_yticklabels(classes, fontsize=7)
    for i in range(len(classes)):
        for j in range(len(REPRS)):
            ax.text(j, i, f"{M[i,j]*100:.0f}%", ha="center", va="center", fontsize=6,
                    color="white" if M[i, j] > 0.6 else "black")
    ax.set_title("Share of cells with status=ok", fontsize=10)
    fig.colorbar(im, ax=ax, shrink=0.7)
    fig.tight_layout(); fig.savefig(out_png, dpi=190); fig.savefig(str(out_png).replace(".png", ".pdf"))
    plt.close(fig)


def outliers(sd, lab, kernels, classes, G, out_csv):
    """Cells whose slowdown departs from their class's geomean by >= 3x either way."""
    ci = {c: i for i, c in enumerate(classes)}
    rows = []
    for k in kernels:
        c = lab[k]["optimization_class"]
        for j, r in enumerate(REPRS):
            v, g = sd[k][r], G[ci[c], j]
            if not v or not np.isfinite(g) or g <= 0: continue
            ratio = v / g
            if ratio >= 3 or ratio <= 1 / 3:
                rows.append({"kernel": k, "representation": r, "optimization_class": c,
                             "cell_slowdown": round(v, 3), "class_geomean": round(float(g), 3),
                             "deviation_x": round(ratio, 2),
                             "direction": "slower than class" if ratio > 1 else "faster than class"})
    rows.sort(key=lambda r: -abs(math.log(r["deviation_x"])))
    with open(out_csv, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0]) if rows else
                           ["kernel", "representation", "optimization_class", "cell_slowdown",
                            "class_geomean", "deviation_x", "direction"])
        w.writeheader(); w.writerows(rows)
    return rows


def noise_floor(cells, out_csv):
    rows = []
    for (k, r), c in sorted(cells.items()):
        if c["status"] != "ok" or not c["time_ns_all"]: continue
        s = [int(x) for x in c["time_ns_all"].split()]
        if len(s) < 2: continue
        m, sd_ = statistics.fmean(s), statistics.stdev(s)
        rows.append({"kernel": k, "representation": r, "preset": c.get("preset", ""), "n": len(s),
                     "bimodal_suspect": "yes" if (100 * sd_ / m) > 5.0 else "no",
                     "median_ns": int(statistics.median(s)), "mean_ns": int(m),
                     "sd_ns": int(sd_), "rsd_pct": round(100 * sd_ / m, 3),
                     "min_ns": min(s), "max_ns": max(s), "max_over_min": round(max(s) / min(s), 3)})
    if rows:
        with open(out_csv, "w", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)
    return rows


def main(a):
    fig = REPO / "figures"; fig.mkdir(exist_ok=True)
    cells, lab, kernels, extra = load(a.results, a.labels)
    sd = slowdowns(cells, kernels)
    heatmap(sd, lab, kernels, fig / "heatmap.png")
    classes, G = class_aggregate(sd, lab, kernels, REPO / "class_aggregate.csv", fig / "class_aggregate.png")
    coverage(cells, lab, kernels, REPO / "coverage.csv", fig / "coverage.png")
    out = outliers(sd, lab, kernels, classes, G, REPO / "outliers.csv")
    nf = noise_floor(cells, REPO / "noise_floor.csv")
    import collections
    st = collections.Counter(c["status"] for c in cells.values())
    print(f"cells: {len(cells)}   status: {dict(st)}")
    print(f"classes: {len(classes)}   outliers (>=3x from class geomean): {len(out)}")
    if nf:
        r = [x["rsd_pct"] for x in nf]
        flagged = [x for x in nf if x["bimodal_suspect"] == "yes"]
        print(f"run-to-run RSD over {len(nf)} ok cells: median {statistics.median(r):.2f}%  max {max(r):.2f}%")
        print(f"bimodal-suspect cells (RSD>5%): {len(flagged)} of {len(nf)}")
    if extra:
        print(f"extra non-M rows carried separately (not in aggregates): {len(extra)}")
    print(f"figures -> {fig}")


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--results", default="results.csv")
    p.add_argument("--labels", default="labels.csv")
    main(p.parse_args())
