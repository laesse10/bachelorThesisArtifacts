"""The extracted-kernel matrix for the thesis: one figure and the CSV behind it.

    python plotting/run.py plotting/thesis/extracted_matrix.py

Reads ``extractedMatrix/results.csv`` (preset M, job 4990441 and DEVIATIONS 9), and two cells measured
elsewhere:

* ``warpx_field_gather`` Numba: the v0.2.1 translator's code, ``extractedMatrix/numba021/parts/`` (job
  5105398, DEVIATIONS 13), divided by the matrix's C;
* ``comet_int4_gemm`` hand-written: the adapter over both precision symbols (``native32``), from
  ``extractedMatrix/followup/parts/cometN.csv`` (job 4998244), divided by the C of the same job, because the
  matrix's ``native`` cell timed translated code (DEVIATIONS 12).

Writes ``extracted_speedup.{pdf,png,csv}`` under the thesis repository's ``Paper/figures/``
(``$THESIS_FIGURES`` overrides). Same conventions as ``llr40_matrix.py``: speedup over translated C on a
log2 axis, the median of 30 runs per cell with a distribution-free 95% interval of the ratio, a missing
version as a hollow cross at 1x, and the geometric mean of each column with its 95% interval below.
"""

import csv
import math
import os
import pathlib
import statistics

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402

from hpcagent_bench import experiment_tags as et  # noqa: E402
from hpcagent_bench.stats import palette, style, summary  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[2]
DATA = ROOT / "extractedMatrix"
OUT = pathlib.Path(os.environ.get("THESIS_FIGURES", ROOT.parent / "BachelorthesisPaper" / "Paper" / "figures"))
TEXT_WIDTH_IN = 369.0 / 72.27

KERNELS = ("comet_int4_gemm", "quatrex_rgf", "spgemm_hash", "warpx_boris_push", "warpx_esirkepov_deposition",
           "warpx_field_gather", "triangle_count", "nfa_frontier")
COLUMNS = ("cpp", "fortran", "numba", "hand")
NAME = {
    "cpp": et.framework_name("cpp"),
    "fortran": et.framework_name("fortran"),
    "numba": f"{et.framework_name('numba')} (generated)",
    "numba_hand": f"{et.framework_name('numba')} (hand-written)",
    "hand": "Hand-written original",
}
COLOR = {
    "cpp": palette.tab20_slot(0),
    "fortran": palette.framework_color("fortran"),
    "numba": palette.framework_color("numba"),
    "hand": palette.framework_color("cc"),
}
MARKER = {
    "cpp": palette.language_marker("cpp"),
    "fortran": palette.language_marker("fortran"),
    "numba": palette.language_marker("python"),
    "hand": palette.language_marker("c"),
}


def times(row: dict[str, str]) -> list[float]:
    return [float(x) for x in row["time_ns_all"].split()] if row and row.get("time_ns_all") else []


def median_interval(t: list[float], confidence: float = 0.95) -> tuple[float, float]:
    """Order statistics j and n-j+1 with the largest j that still covers ``confidence``."""
    from scipy.stats import binom

    t, n, j = sorted(t), len(t), 1
    while j + 1 <= n // 2 and binom.cdf(n - j - 1, n, 0.5) - binom.cdf(j, n, 0.5) >= confidence:
        j += 1
    return t[j - 1], t[n - j]


def load() -> dict[str, dict[str, tuple[float, float, float, bool]]]:
    """kernel -> column -> (speedup, lo, hi, hand_numba); nan speedup = no valid version."""
    rows = {(r["kernel"], r["representation"]): r for r in csv.DictReader(open(DATA / "results.csv"))}
    fg = {r["representation"]: r for r in csv.DictReader(open(DATA / "numba021" / "parts" / "warpx_field_gather.csv"))}
    comet: dict[str, list[float]] = {}
    for r in csv.DictReader(open(DATA / "followup" / "parts" / "cometN.csv")):
        # The "record" round reruns each cell under perf counters; the timed rounds are 1-3.
        if r["status"] == "ok" and r["round"] != "record":
            comet.setdefault(r["label"], []).extend(times(r))

    def ratio(base: list[float], other: list[float]) -> tuple[float, float, float]:
        (b_lo, b_hi), (o_lo, o_hi) = median_interval(base), median_interval(other)
        return statistics.median(base) / statistics.median(other), b_lo / o_hi, b_hi / o_lo

    out = {}
    for k in KERNELS:
        c = times(rows[(k, "c")])
        cells = {}
        for col in ("cpp", "fortran"):
            cells[col] = (*ratio(c, times(rows[(k, col)])), False)
        if k == "warpx_field_gather":
            cells["numba"] = (*ratio(c, times(fg["numba"])), False)
        elif (k, "numba_hand") in rows:
            cells["numba"] = (*ratio(c, times(rows[(k, "numba_hand")])), True)
        else:
            cells["numba"] = (*ratio(c, times(rows[(k, "numba")])), False)
        if k == "comet_int4_gemm":
            cells["hand"] = (*ratio(comet["c"], comet["native32"]), False)
        elif rows.get((k, "native"), {}).get("status") == "ok":
            cells["hand"] = (*ratio(c, times(rows[(k, "native")])), False)
        else:
            cells["hand"] = (math.nan, math.nan, math.nan, False)
        out[k] = cells
    return out


def geomeans(cells) -> dict[str, summary.Interval]:
    out = {}
    for col in COLUMNS:
        # The Numba mean covers the generated versions only, as in the text.
        r = [cells[k][col][0] for k in KERNELS if math.isfinite(cells[k][col][0]) and not cells[k][col][3]]
        out[col] = summary.geomean_interval(r)
    return out


def figure(cells, stats) -> None:
    style.apply()
    scale = style.PRINT_SCALE
    small = style.PRINT_LEGEND_PT
    dodge = (-0.27, -0.09, 0.09, 0.27)
    rows = [(float(i), k) for i, k in enumerate(KERNELS)]
    top = len(KERNELS) + 0.6
    summary_rows = [(top + i, col) for i, col in enumerate(COLUMNS)]
    total = summary_rows[-1][0] + 0.5
    pitch_in = 0.19
    height_in = 0.05 + 0.62 + (total + 0.5) * pitch_in
    fig = plt.figure(figsize=(TEXT_WIDTH_IN, height_in))
    ax = fig.add_axes((0.3, 0.62 / height_in, 0.65, 1 - 0.67 / height_in))
    ax.set_ylim(total, -0.7)
    ax.set_xlim(-3.0, 1.5)

    missing = []
    for y, k in rows:
        for off, col in zip(dodge, COLUMNS):
            s, lo, hi, hand_numba = cells[k][col]
            if not math.isfinite(s):
                missing.append((y + off, col))
                continue
            ax.plot([summary.log2_change(lo), summary.log2_change(hi)], [y + off] * 2, color=COLOR[col],
                    linewidth=0.6, alpha=0.8, zorder=style.CONNECTOR_Z, solid_capstyle="butt")
            # A hand-written Numba override is drawn hollow: it is not the translator's code.
            style.point_mark(ax, summary.log2_change(s), y + off, COLOR[col], MARKER[col], filled=not hand_numba,
                             size=13.0)
    for y, col in missing:
        style.point_mark(ax, 0.0, y, COLOR[col], MARKER[col], filled=False, size=13.0, delivered=False)
    for y, col in summary_rows:
        g = stats[col]
        if g.n >= 2:
            ax.plot([summary.log2_change(g.low), summary.log2_change(g.high)], [y, y], color=COLOR[col],
                    linewidth=scale.line_width, zorder=style.CONNECTOR_Z, solid_capstyle="butt")
        style.point_mark(ax, summary.log2_change(g.point), y, COLOR[col], MARKER[col], filled=True, size=20.0)

    ax.axvline(0.0, color=style.REFERENCE, linewidth=scale.hairline_width, zorder=2)
    ax.axhline(top - 0.8, color=style.MUTED, linewidth=0.5, linestyle=(0, (3, 2)), zorder=1)
    for y, _ in rows[1:]:
        ax.axhline(y - 0.5, color=style.RULE, linewidth=0.3, zorder=1)
    # The kernel identifiers, as the thesis text names them.
    labels = [k for _, k in rows]
    labels += [f"Geomean, {NAME[c].split(' (')[0] if c != 'hand' else 'hand-written'} (n = {stats[c].n})"
               for _, c in summary_rows]
    ax.set_yticks([y for y, _ in rows] + [y for y, _ in summary_rows])
    ax.set_yticklabels(labels, fontsize=small)
    for tick in ax.get_yticklabels()[: len(rows)]:
        tick.set_fontfamily("monospace")
    ax.tick_params(axis="y", length=0, pad=2)
    ax.set_xticks(range(-3, 2))
    ax.xaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(style.log2_ratio_tick))
    ax.tick_params(axis="x", labelsize=scale.tick_pt, length=2.5, width=0.6, pad=2)
    style.minor_ticks(ax.xaxis, style.MinorKind.LOG2)
    ax.grid(axis="x", which="major", color=style.RULE, linewidth=0.5, zorder=0)
    ax.set_xlabel("Speedup over translated C (log scale)", fontsize=scale.label_pt, labelpad=3)
    style.despine(ax, keep=("bottom",))

    handles = [Line2D([], [], linestyle="none", marker=MARKER[c], color=COLOR[c], markersize=4.0, label=NAME[c])
               for c in COLUMNS]
    handles.insert(3, Line2D([], [], linestyle="none", marker=MARKER["numba"], markerfacecolor="none",
                             color=COLOR["numba"], markersize=4.0, label=NAME["numba_hand"]))
    handles.append(Line2D([], [], linestyle="none", marker="x", color=style.MUTED, markersize=3.5,
                          label="Cannot run in the benchmark"))
    style.fill_width(fig, pad_in=0.06)
    style.legend_below(fig, handles, ncol=3, y=0.005, fontsize=small, markerscale=1.0, columnspacing=1.2,
                       handlelength=1.6)
    style.save(fig, OUT / "extracted_speedup", width_in=TEXT_WIDTH_IN)


def main() -> None:
    cells = load()
    stats = geomeans(cells)
    OUT.mkdir(parents=True, exist_ok=True)
    with open(OUT / "extracted_speedup.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["kernel", "column", "speedup_over_c", "ci95_lo", "ci95_hi", "hand_written_numba"])
        for k in KERNELS:
            for col in COLUMNS:
                s, lo, hi, h = cells[k][col]
                w.writerow([k, col, f"{s:.4f}", f"{lo:.4f}", f"{hi:.4f}", h])
        for col in COLUMNS:
            g = stats[col]
            w.writerow([f"geomean (n={g.n})", col, f"{g.point:.4f}", f"{g.low:.4f}", f"{g.high:.4f}", ""])
    figure(cells, stats)
    for k in KERNELS:
        print(f"{k:28s}", "  ".join(f"{c} {cells[k][c][0]:.2f}" for c in COLUMNS))
    for col in COLUMNS:
        g = stats[col]
        print(f"geomean {col:8s} n={g.n} {g.point:.3f} [{g.low:.3f}, {g.high:.3f}]")
    print(f"wrote {OUT / 'extracted_speedup'}.pdf, .png, .csv")


if __name__ == "__main__":
    main()
