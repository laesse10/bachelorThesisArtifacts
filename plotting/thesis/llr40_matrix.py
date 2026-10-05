"""The LLR-40 matrix for the thesis: one figure, two LaTeX tables and the CSVs behind them.

    python plotting/run.py plotting/thesis/llr40_matrix.py

Reads ``llr40Matrix/v2/results.csv`` (preset M, no flag variant) and ``labels.csv``, and the first
measurement ``llr40Matrix/results.csv`` to flag kernels whose GCC cells did not reproduce, and writes, under
``Paper/figures/``:

* ``llr40_speedup.{pdf,png}``  per-kernel speedup of every column over the translated C column,
  grouped by transformation class, with a geomean summary per column;
* ``llr40_speedup.csv``        the value behind every mark of the figure;
* ``llr40_geomean.{csv,tex}``  the summary rows as a table;
* ``llr40_matrix.tex``         the full matrix (min-of-k per cell) for the appendix.

Figure conventions are HPCAgent-Bench's (``plotting/docs/plotting.md``): log2 speedup axis with
ratio ticks, framework colours from the registry, a missing answer as a hollow cross at 1x, one
legend below, print type sizes, drawn at the width it is placed at.
"""

import csv
import math
import pathlib

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402
from matplotlib.transforms import blended_transform_factory  # noqa: E402

from hpcagent_bench import experiment_tags as et  # noqa: E402
from hpcagent_bench.stats import palette, style, summary  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[2]
DATA = ROOT / "llr40Matrix" / "v2"
OUT = ROOT / "Paper" / "figures"

#: Thesis \textwidth (369pt, memoir a4 11pt), the width the figure is placed at.
TEXT_WIDTH_IN = 369.0 / 72.27

BASELINE = "c"
COLUMNS = ("c_reference", "cpp", "fortran", "numba", "agent")
NAME = {
    "c": f"Translated C ({et.framework_name('cc')})",
    "c_reference": f"Hand-written C ({et.framework_name('cc')})",
    "cpp": et.framework_name("cpp"),
    "fortran": et.framework_name("fortran"),
    "numba": et.framework_name("numba"),
    "agent": "Agent",
}
#: Summary-row labels; the legend carries the full names.
SHORT = {"c_reference": "Hand-written C"}
COLOR = {
    "c_reference": palette.framework_color("cc"),
    "cpp": palette.framework_color("cpp"),
    "fortran": palette.framework_color("fortran"),
    "numba": palette.framework_color("numba"),
    "agent": palette.tab20_slot(6),
}
MARKER = {
    "c_reference": palette.language_marker("c"),
    "cpp": palette.language_marker("cpp"),
    "fortran": palette.language_marker("fortran"),
    "numba": palette.language_marker("python"),
    "agent": "*",
}

#: Class order: the restructuring classes first, then by mechanism.
CLASSES = (
    ("loop_interchange", "Interchange"),
    ("loop_distribution", "Distribution"),
    ("loop_fusion", "Fusion"),
    ("control_flow", "Control Flow"),
    ("reduction", "Reduction"),
    ("recurrence", "Recurrence"),
    ("scalar_expansion", "Scalar Exp."),
    ("linear_dependence", "Linear Dep."),
    ("dependence_distance", "Dep. Dist."),
    ("node_splitting", "Node Split."),
    ("interprocedural_dataflow", "Interproc."),
    ("indirect_addressing", "Indirect"),
    ("packing", "Packing"),
    ("wavefront", "Wavefront"),
)

#: Left out of the figure: no GCC column validates at preset M (FMA contraction), so there is no
#: baseline to divide by. It stays in the appendix table.
NO_BASELINE = ("tsvc_2_s115",)
#: Recorded ok at preset M, but the outputs compared equal only because both are mostly inf/NaN.
VACUOUS = ("wf_triangular", "wf_diff_skew")

TIE = 0.03  # +-3%: the faster / tie / slower split of the summary table

#: A kernel is bandwidth-unstable when the SAME compiled GCC code gave minima more than this far apart
#: in the two measurement campaigns (v1 at e2bceb68, v2 at 26a4f0cf). Its GCC columns then differ by
#: noise, not by code. The flagged set is separated from the rest: 1.20x and up versus 1.09x and down.
UNSTABLE_RATIO = 1.15
GCC_COLUMNS = ("c", "c_reference", "cpp", "fortran")


def load() -> tuple[dict[str, dict[str, dict[str, str]]], dict[str, str]]:
    cells: dict[str, dict[str, dict[str, str]]] = {}
    with open(DATA / "results.csv", newline="") as fh:
        for row in csv.DictReader(fh):
            if row["preset"] == "M" and not row["flags_variant"]:
                cells.setdefault(row["kernel"], {})[row["representation"]] = row
    with open(DATA / "labels.csv", newline="") as fh:
        labels = {row["kernel"]: row["optimization_class"] for row in csv.DictReader(fh)}
    return cells, labels


def unstable_kernels() -> set[str]:
    """Kernels whose GCC cells disagree between the two campaigns by more than ``UNSTABLE_RATIO``."""

    def minima(path: pathlib.Path) -> dict[tuple[str, str], float]:
        with open(path, newline="") as fh:
            return {
                (row["kernel"], row["representation"]): float(row["time_ns_min"])
                for row in csv.DictReader(fh)
                if row["preset"] == "M" and not row.get("flags_variant") and row["status"] == "ok"
            }

    first, second = minima(DATA.parent / "results.csv"), minima(DATA / "results.csv")
    flagged = set()
    for (kernel, column), t2 in second.items():
        t1 = first.get((kernel, column))
        if column in GCC_COLUMNS and t1 and max(t1 / t2, t2 / t1) > UNSTABLE_RATIO:
            flagged.add(kernel)
    return flagged


def minimum(cell: dict[str, str]) -> float:
    return float(cell["time_ns_min"]) if cell["status"] == "ok" else math.nan


def speedup(cells: dict[str, dict[str, str]], column: str) -> float:
    """``t_C / t_column``: above 1 the column is faster than translated C."""
    base, other = minimum(cells[BASELINE]), minimum(cells[column])
    return base / other if math.isfinite(base) and math.isfinite(other) else math.nan


def ordered_kernels(cells, labels) -> list[tuple[str, list[str]]]:
    groups = []
    for key, _ in CLASSES:
        kernels = [k for k in cells if labels[k] == key and k not in NO_BASELINE]
        agent = {k: speedup(cells[k], "agent") for k in kernels}
        kernels.sort(key=lambda k: -agent[k] if math.isfinite(agent[k]) else -1.0)
        if kernels:
            groups.append((key, kernels))
    return groups


def geomeans(cells) -> dict[str, dict[str, float]]:
    out = {}
    for column in COLUMNS:
        ratios = [r for k in cells if k not in NO_BASELINE for r in [speedup(cells[k], column)] if math.isfinite(r)]
        interval = summary.geomean_interval(ratios)
        out[column] = {
            "n": len(ratios),
            "geomean": interval.point,
            "lo": interval.low,
            "hi": interval.high,
            "faster": sum(r > 1 + TIE for r in ratios),
            "tie": sum(abs(r - 1) <= TIE for r in ratios),
            "slower": sum(r < 1 - TIE for r in ratios),
        }
    return out


def figure(cells, labels, stats, unstable) -> None:
    style.apply()
    scale = style.PRINT_SCALE
    small = style.PRINT_LEGEND_PT  # 6pt: kernel names, class names, legend

    groups = ordered_kernels(cells, labels)
    pitch_in, gap, dodge = 0.112, 0.7, (-0.32, -0.16, 0.0, 0.16, 0.32)
    rows: list[tuple[float, str]] = []
    spans: list[tuple[str, float, float]] = []
    y = 0.0
    for key, kernels in groups:
        top = y
        for kernel in kernels:
            rows.append((y, kernel))
            y += 1.0
        spans.append((key, top, y - 1.0))
        y += gap
    summary_top = y + 0.3
    summary_rows = [(summary_top + i, column) for i, column in enumerate(COLUMNS)]
    total = summary_rows[-1][0] + 0.5

    left_in, right_in, top_in, bottom_in = 0.62, 0.78, 0.05, 0.72
    height_in = top_in + bottom_in + (total + 0.5) * pitch_in
    fig = plt.figure(figsize=(TEXT_WIDTH_IN, height_in))
    ax = fig.add_axes(
        (left_in / TEXT_WIDTH_IN, bottom_in / height_in, 1 - (left_in + right_in) / TEXT_WIDTH_IN,
         1 - (top_in + bottom_in) / height_in)
    )
    ax.set_ylim(total, -0.5)
    xlo, xhi = -3.0, 4.2
    ax.set_xlim(xlo, xhi)

    missing = []
    for yy, kernel in rows:
        for offset, column in zip(dodge, COLUMNS):
            ratio = speedup(cells[kernel], column)
            if math.isfinite(ratio):
                style.point_mark(ax, summary.log2_change(ratio), yy + offset, COLOR[column], MARKER[column],
                                 filled=True, size=11.0)
            else:
                missing.append((yy + offset, column))
    # A missing answer sits at 1x, where the other columns crowd; drawn last, it stays on top.
    for yy, column in missing:
        style.point_mark(ax, 0.0, yy, COLOR[column], MARKER[column], filled=False, size=11.0, delivered=False)
    for yy, column in summary_rows:
        s = stats[column]
        ax.plot([summary.log2_change(s["lo"]), summary.log2_change(s["hi"])], [yy, yy],
                color=COLOR[column], linewidth=scale.line_width, zorder=style.CONNECTOR_Z, solid_capstyle="butt")
        style.point_mark(ax, summary.log2_change(s["geomean"]), yy, COLOR[column], MARKER[column], filled=True,
                         size=18.0)

    ax.axvline(0.0, color=style.REFERENCE, linewidth=scale.hairline_width, zorder=2)
    for _, top, bottom in spans[:-1]:
        ax.axhline(bottom + 0.5 + gap / 2.0, color=style.RULE, linewidth=0.4, zorder=1)
    ax.axhline(summary_top - 0.65, color=style.MUTED, linewidth=0.5, linestyle=(0, (3, 2)), zorder=1)

    ticks = [yy for yy, _ in rows] + [yy for yy, _ in summary_rows]
    names = [
        et.kernel_compact_display_name(k) + ("$^\\dagger$" if k in VACUOUS else "") + ("$^\\S$" if k in unstable else "")
        for _, k in rows
    ]
    for yy, kernel in rows:
        if kernel in unstable:
            ax.axhspan(yy - 0.5, yy + 0.5, color=style.MINOR_RULE, linewidth=0, zorder=0)
    names += [SHORT.get(c, NAME[c]) for _, c in summary_rows]
    ax.set_yticks(ticks)
    ax.set_yticklabels(names, fontsize=small)
    ax.tick_params(axis="y", length=0, pad=2)

    side = blended_transform_factory(ax.transAxes, ax.transData)
    label = dict(CLASSES)
    for key, top, bottom in spans:
        ax.text(1.02, (top + bottom) / 2.0, label[key], transform=side, fontsize=small, color=style.MUTED,
                ha="left", va="center")
    ax.text(1.02, (summary_rows[0][0] + summary_rows[-1][0]) / 2.0, "Geomean,\n95% CI", transform=side,
            fontsize=small, color=style.MUTED, ha="left", va="center", linespacing=1.1)

    ax.set_xticks(range(-3, 5))
    ax.xaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(style.log2_ratio_tick))
    ax.tick_params(axis="x", labelsize=scale.tick_pt, length=2.5, width=0.6, pad=2)
    style.minor_ticks(ax.xaxis, style.MinorKind.LOG2)
    ax.grid(axis="x", which="major", color=style.RULE, linewidth=0.5, zorder=0)
    ax.set_xlabel("Speedup over translated C (log scale)", fontsize=scale.label_pt, labelpad=3)
    style.despine(ax, keep=("bottom",))

    handles = [
        Line2D([], [], linestyle="none", marker=MARKER[c], color=COLOR[c], markersize=4.0, label=NAME[c])
        for c in COLUMNS
    ]
    handles.append(Line2D([], [], color=style.REFERENCE, linewidth=scale.hairline_width, label=f"{NAME['c']}, 1x"))
    handles.append(
        Line2D([], [], linestyle="none", marker="x", color=style.MUTED, markersize=3.5,
               label=style.NOT_DELIVERED_LABEL)
    )
    # Size the margins from the measured labels (kernel and summary names left, class names right),
    # then place the one legend under the stretched axes.
    style.fill_width(fig, pad_in=0.06)
    style.legend_below(fig, handles, ncol=4, y=0.005, fontsize=small, markerscale=1.0, columnspacing=1.2,
                       handlelength=1.6)
    style.save(fig, OUT / "llr40_speedup", width_in=TEXT_WIDTH_IN)


def write_csvs(cells, labels, stats, unstable) -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    with open(OUT / "llr40_speedup.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["kernel", "class", "column", "status", "t_c_ms", "t_column_ms", "speedup_over_c", "log2_speedup",
                    "in_figure", "bandwidth_unstable"])
        for kernel in sorted(cells):
            for column in COLUMNS:
                cell = cells[kernel].get(column)
                if cell is None:
                    continue
                ratio = speedup(cells[kernel], column)
                w.writerow([
                    kernel, labels[kernel], column, cell["status"],
                    f"{minimum(cells[kernel][BASELINE]) / 1e6:.4f}", f"{minimum(cell) / 1e6:.4f}",
                    f"{ratio:.4f}", f"{summary.log2_change(ratio):.4f}", kernel not in NO_BASELINE,
                    kernel in unstable,
                ])
    with open(OUT / "llr40_geomean.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["column", "n", "geomean_speedup_over_c", "ci95_lo", "ci95_hi", "faster", "tie_3pct", "slower"])
        for column in COLUMNS:
            s = stats[column]
            w.writerow([column, s["n"], f"{s['geomean']:.4f}", f"{s['lo']:.4f}", f"{s['hi']:.4f}", s["faster"],
                        s["tie"], s["slower"]])


def tex_name(text: str) -> str:
    return text.replace("_", r"\_")


def write_geomean_table(stats) -> None:
    lines = [
        r"% Generated by plotting/thesis/llr40_matrix.py -- do not edit.",
        r"\begin{tabular}{@{}lrcrrr@{}}",
        r"  \toprule",
        r"  Column & $n$ & Speedup over C (95\% CI) & Faster & Tie & Slower \\",
        r"  \midrule",
    ]
    for column in COLUMNS:
        s = stats[column]
        lines.append(
            f"  {tex_name(NAME[column])} & {s['n']} & {s['geomean']:.2f}$\\times$ "
            f"[{s['lo']:.2f}, {s['hi']:.2f}] & {s['faster']} & {s['tie']} & {s['slower']} \\\\"
        )
    lines += [r"  \bottomrule", r"\end{tabular}", ""]
    (OUT / "llr40_geomean.tex").write_text("\n".join(lines))


def write_matrix_table(cells, labels, unstable) -> None:
    order = ("c",) + COLUMNS
    header = ["C", "Hand-written C", "C++", "Fortran", "Numba", "Agent"]
    lines = [
        r"% Generated by plotting/thesis/llr40_matrix.py -- do not edit.",
        r"\begin{tabular}{@{}l" + "r" * len(order) + r"@{}}",
        r"  \toprule",
        "  Kernel & " + " & ".join(header) + r" \\",
        r"  \midrule",
    ]
    label = dict(CLASSES)
    for key, _ in CLASSES:
        kernels = sorted(k for k in cells if labels[k] == key)
        if not kernels:
            continue
        lines.append(f"  \\multicolumn{{{len(order) + 1}}}{{@{{}}l}}{{\\emph{{{label[key]}}}}} \\\\")
        for kernel in kernels:
            times = {c: minimum(cells[kernel][c]) for c in order if c in cells[kernel]}
            finite = [t for t in times.values() if math.isfinite(t)]
            best = min(finite) if finite else math.nan
            out = []
            for column in order:
                cell = cells[kernel].get(column)
                if cell is None or cell["status"] == "unsupported":
                    out.append("---")
                elif cell["status"] != "ok":
                    out.append(f"{float(cell['time_ns_min']) / 1e6:.1f}$^\\ddagger$")
                else:
                    text = f"{times[column] / 1e6:.1f}"
                    out.append(f"\\textbf{{{text}}}" if times[column] <= best * (1 + 1e-9) else text)
            dagger = ("$^\\dagger$" if kernel in VACUOUS else "") + ("$^\\S$" if kernel in unstable else "")
            lines.append(f"  \\quad\\texttt{{{tex_name(kernel)}}}{dagger} & " + " & ".join(out) + r" \\")
    lines += [r"  \bottomrule", r"\end{tabular}", ""]
    (OUT / "llr40_matrix.tex").write_text("\n".join(lines))


def main() -> None:
    cells, labels = load()
    unstable = unstable_kernels()
    stats = geomeans(cells)
    write_csvs(cells, labels, stats, unstable)
    write_geomean_table(stats)
    write_matrix_table(cells, labels, unstable)
    figure(cells, labels, stats, unstable)
    print("bandwidth-unstable kernels:", ", ".join(sorted(unstable)))
    for column in COLUMNS:
        s = stats[column]
        print(f"{column:12s} n={s['n']:2d} speedup over C {s['geomean']:.3f} [{s['lo']:.3f}, {s['hi']:.3f}]"
              f"  faster {s['faster']:2d}  tie {s['tie']:2d}  slower {s['slower']:2d}")
    print(f"wrote {OUT}/llr40_speedup.pdf, .png, .csv; llr40_geomean.csv/.tex; llr40_matrix.tex")


if __name__ == "__main__":
    main()
