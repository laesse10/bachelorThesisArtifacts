"""The extracted-kernel matrix for the thesis: the overview figure, one small figure per paragraph, and the
CSV behind them.

    python plotting/run.py plotting/thesis/extracted_matrix.py

Reads ``extractedMatrix/results.csv`` (preset M, job 4990441 and DEVIATIONS 9), and the Numba cell of
``warpx_field_gather`` from the v0.2.1 translator, ``extractedMatrix/numba021/parts/`` (job 5105398,
DEVIATIONS 13). For QuaTrEx and the Esirkepov deposition, the Numba column is the benchmark's hand-written
Numba version (``numba_hand``), drawn hollow. The hand-written originals are not shown.

Writes ``extracted_speedup.{pdf,png,csv}``, ``extracted_<panel>.{pdf,png}`` (:data:`PANELS`) and the appendix
table ``extracted_matrix.tex`` under the
thesis repository's ``Paper/figures/`` (``$THESIS_FIGURES`` overrides). Same conventions as
``llr40_matrix.py``: speedup over the faster of translated C and Numba (the benchmark's baseline) on a log2
axis, the median of 30 runs per cell with a distribution-free 95% interval of the ratio, a missing version as
a hollow cross at 1x, and the geometric mean of each column with its 95% interval below.
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

import sys  # noqa: E402

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import panels  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[2]
DATA = ROOT / "extractedMatrix"
OUT = pathlib.Path(os.environ.get("THESIS_FIGURES", ROOT.parent / "BachelorthesisPaper" / "Paper" / "figures"))
TEXT_WIDTH_IN = 369.0 / 72.27

KERNELS = ("comet_int4_gemm", "quatrex_rgf", "spgemm_hash", "warpx_boris_push", "warpx_esirkepov_deposition",
           "warpx_field_gather", "triangle_count", "nfa_frontier")
#: The baseline of every speedup: the faster valid one of these two columns, kernel by kernel.
BASELINE = ("c", "numba")
BASELINE_NAME = "Faster of C and Numba"
COLUMNS = ("c", "cpp", "fortran", "numba")
NAME = {
    "c": "C",
    "cpp": "C++",
    "fortran": "Fortran",
    "numba": "Numba",
    "numba_hand": "Numba (benchmark's hand-written version)",
}
COLOR = {
    "c": palette.framework_color("cc"),
    "cpp": palette.tab20_slot(0),
    "fortran": palette.framework_color("fortran"),
    "numba": palette.framework_color("numba"),
}
MARKER = {
    "c": palette.language_marker("c"),
    "cpp": palette.language_marker("cpp"),
    "fortran": palette.language_marker("fortran"),
    "numba": palette.language_marker("python"),
}
#: One small figure per paragraph of the evaluation.
PANELS = {
    "comet": (("", ("comet_int4_gemm",)),),
    "division": (("", ("spgemm_hash", "triangle_count")),),
    "quatrex": (("", ("quatrex_rgf",)),),
    "warpx": (("", ("warpx_boris_push", "warpx_esirkepov_deposition", "warpx_field_gather")),),
    "nfa": (("", ("nfa_frontier",)),),
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


def load() -> tuple[dict[str, dict[str, tuple[float, float, float, bool]]], dict[str, tuple[str, float]]]:
    """kernel -> column -> (speedup, lo, hi, hand_numba), and kernel -> (baseline column, its median in ms); nan
    speedup = no valid version."""
    rows = {(r["kernel"], r["representation"]): r for r in csv.DictReader(open(DATA / "results.csv"))}
    fg = {r["representation"]: r for r in csv.DictReader(open(DATA / "numba021" / "parts" / "warpx_field_gather.csv"))}

    def ratio(base: list[float], other: list[float]) -> tuple[float, float, float]:
        (b_lo, b_hi), (o_lo, o_hi) = median_interval(base), median_interval(other)
        return statistics.median(base) / statistics.median(other), b_lo / o_hi, b_hi / o_lo

    def ok(row: dict[str, str]) -> list[float]:
        return times(row) if row.get("status", "ok") == "ok" else []

    out, base_of = {}, {}
    for k in KERNELS:
        t = {col: ok(rows[(k, col)]) for col in ("c", "cpp", "fortran")}
        hand = {"numba": (k, "numba_hand") in rows}
        if k == "warpx_field_gather":
            t["numba"] = ok(fg["numba"])
        else:
            t["numba"] = ok(rows[(k, "numba_hand" if hand["numba"] else "numba")])
        base = min((c for c in BASELINE if t[c]), key=lambda c: statistics.median(t[c]))
        out[k] = {col: (*ratio(t[base], t[col]), hand.get(col, False)) if t[col]
                  else (math.nan, math.nan, math.nan, False) for col in COLUMNS}
        base_of[k] = (base, statistics.median(t[base]) / 1e6)
    return out, base_of


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
    dodge = (-0.27, -0.09, 0.09, 0.27)  # one per column
    rows = [(float(i), k) for i, k in enumerate(KERNELS)]
    top = len(KERNELS) + 0.6
    summary_rows = [(top + i, col) for i, col in enumerate(COLUMNS)]
    total = summary_rows[-1][0] + 0.5
    pitch_in = 0.19
    height_in = 0.05 + 0.62 + (total + 0.5) * pitch_in
    fig = plt.figure(figsize=(TEXT_WIDTH_IN, height_in))
    ax = fig.add_axes((0.3, 0.62 / height_in, 0.65, 1 - 0.67 / height_in))
    ax.set_ylim(total, -0.7)
    ax.set_xlim(-3.0, 1.0)

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
    labels += [f"Geomean, {NAME[c]} (n = {stats[c].n})" for _, c in summary_rows]
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
    ax.set_xlabel("Speedup over the faster of C and Numba (log scale)", fontsize=scale.label_pt, labelpad=3)
    style.despine(ax, keep=("bottom",))

    handles = [Line2D([], [], linestyle="none", marker=MARKER[c], color=COLOR[c], markersize=4.0, label=NAME[c])
               for c in COLUMNS]
    handles.insert(4, Line2D([], [], linestyle="none", marker=MARKER["numba"], markerfacecolor="none",
                             color=COLOR["numba"], markersize=4.0, label=NAME["numba_hand"]))
    handles.append(Line2D([], [], color=style.REFERENCE, linewidth=scale.hairline_width,
                          label=f"{BASELINE_NAME}, 1x"))
    style.fill_width(fig, pad_in=0.06)
    style.legend_below(fig, handles, ncol=3, y=0.005, fontsize=small, markerscale=1.0, columnspacing=1.2,
                       handlelength=1.6)
    style.save(fig, OUT / "extracted_speedup", width_in=TEXT_WIDTH_IN)


#: Footnote marks of the appendix table: calls under 10 ms, and the Numba cell measured in another run.
SHORT_CALL = ("comet_int4_gemm", "warpx_boris_push", "warpx_esirkepov_deposition")
SEPARATE_RUN = ("warpx_field_gather",)


def write_table(cells, base_of, stats) -> None:
    """The appendix table: the baseline's median and every column's speedup over it."""
    lines = [
        "% Generated by plotting/thesis/extracted_matrix.py -- do not edit.",
        r"\begin{tabular}{@{}llrrrrr@{}}",
        r"  \toprule",
        r"  & \multicolumn{2}{c}{Baseline} & \multicolumn{4}{c@{}}{Speedup over the baseline} \\",
        r"  \cmidrule(lr){2-3} \cmidrule(l){4-7}",
        r"  Kernel & & (ms) & C & C++ & Fortran & Numba \\",
        r"  \midrule",
    ]
    for k in KERNELS:
        base, ms = base_of[k]
        out = []
        for col in COLUMNS:
            s, _, _, hand = cells[k][col]
            text = "---" if not math.isfinite(s) else f"{s:.2f}"
            if col == "numba":
                text += "$^\\ast$" if hand else "$^\\S$" if k in SEPARATE_RUN else ""
            out.append(text)
        mark = "$^\\dagger$" if k in SHORT_CALL else ""
        name = k.replace("_", r"\_")
        lines.append(f"  \\texttt{{{name}}}{mark} & {'C' if base == 'c' else 'Numba'} & {ms:.{0 if ms >= 100 else 1 if ms >= 10 else 2 if ms >= 1 else 3}f} & "
                     + " & ".join(out) + r" \\")
    lines.append(r"  \midrule")
    lines.append("  Geometric mean & & & " + " & ".join(f"{stats[c].point:.2f}" for c in COLUMNS) + r" \\")
    lines.append("  \\quad 95\\% CI & & & "
                 + " & ".join(f"\\scriptsize[{stats[c].low:.2f},{stats[c].high:.2f}]" for c in COLUMNS) + r" \\")
    lines += [r"  \bottomrule", r"\end{tabular}", ""]
    (OUT / "extracted_matrix.tex").write_text("\n".join(lines))


def panel_figures(cells) -> None:
    def value(kernel: str, column: str) -> panels.Value:
        return cells[kernel][column]

    for stem, groups in PANELS.items():
        panels.draw(OUT / f"extracted_{stem}", groups, COLUMNS, value, label=lambda k: k, name=NAME, color=COLOR,
                    marker=MARKER, baseline_name=BASELINE_NAME, hollow_name={"numba": NAME["numba_hand"]},
                    xlabel="Speedup over the faster of C and Numba (log scale)")


def main() -> None:
    cells, base_of = load()
    stats = geomeans(cells)
    OUT.mkdir(parents=True, exist_ok=True)
    with open(OUT / "extracted_speedup.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["kernel", "column", "baseline", "speedup_over_baseline", "ci95_lo", "ci95_hi",
                    "hand_written_numba"])
        for k in KERNELS:
            for col in COLUMNS:
                s, lo, hi, h = cells[k][col]
                w.writerow([k, col, base_of[k][0], f"{s:.4f}", f"{lo:.4f}", f"{hi:.4f}", h])
        for col in COLUMNS:
            g = stats[col]
            w.writerow([f"geomean (n={g.n})", col, "", f"{g.point:.4f}", f"{g.low:.4f}", f"{g.high:.4f}", ""])
    figure(cells, stats)
    panel_figures(cells)
    write_table(cells, base_of, stats)
    for k in KERNELS:
        print(f"{k:28s} base {base_of[k][0]:6s}", "  ".join(f"{c} {cells[k][c][0]:.2f}" for c in COLUMNS))
    for col in COLUMNS:
        g = stats[col]
        print(f"geomean {col:8s} n={g.n} {g.point:.3f} [{g.low:.3f}, {g.high:.3f}]")
    print(f"wrote {OUT / 'extracted_speedup'}.pdf, .png, .csv")


if __name__ == "__main__":
    main()
