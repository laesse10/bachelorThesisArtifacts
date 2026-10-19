"""The DaCe column of LLR-40 for the thesis: one figure and two LaTeX tables.

    python plotting/run.py plotting/thesis/dace_matrix.py

Reads ``llr40Matrix/v2/dace/results.csv`` (the codegen experiment) and the one-change follow-up under
``llr40Matrix/v2/dace/followup/out/`` and writes, under the thesis repository's ``Paper/figures/``
(``$THESIS_FIGURES`` overrides the directory):

* ``dace_speedup.{pdf,png}``  per-kernel speedup of DaCe (canonicalize pipeline, new code generator) over
  translated C++, with the 95% interval of the ratio of medians, coloured by the cause the follow-up found;
* ``dace_codegen.tex``         the three code-generator configurations on the kernels where they differ;
* ``dace_causes.tex``          every difference of more than 3%, the change that tests it, and the time after
  the change (appendix).

Every time is the median of the timed calls: 30 per cell in the codegen experiment, 90 (three rounds of 30)
per cell in the follow-up.
"""

import collections
import csv
import json
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
DACE = ROOT / "llr40Matrix" / "v2" / "dace"
OUT = pathlib.Path(os.environ.get("THESIS_FIGURES", ROOT.parent / "BachelorthesisPaper" / "Paper" / "figures"))
TEXT_WIDTH_IN = 369.0 / 72.27
TIE = 0.03

#: The follow-up's verdict per kernel: the group, the cause, and the follow-up cell that tests it.
#: ``apply``: the change applied to translated C++; ``undo``: undone or replaced in DaCe's code.
CAUSES = {
    "tsvc_2_s2233": ("gain", "apply", "cppinterfused", "interchange and fusion of the two nests"),
    "tsvc_2_s231": ("gain", "apply", "cppinter", "interchange"),
    "tsvc_2_s235": ("gain", "apply", "cppinter", "interchange and distribution"),
    "tsvc_2_s2275": ("gain", "apply", "cppinter", "interchange and distribution"),
    "tsvc_2_s1232": ("gain", "apply", "cppinter", "interchange"),
    "tsvc_2_s233": ("gain", "apply", "cppinter", "interchange of one of the two statements"),
    "fuse_diamond": ("gain", "apply", "cppfused", "fusion"),
    "fuse_stencil_through_transient": ("gain", "apply", "cppfused", "fusion"),
    "tsvc_2_s152": ("gain", "apply", "cppfused", "fusion"),
    "tsvc_2_s453": ("gain", "apply", "cppclosed", "closed form of the induction variable"),
    "tsvc_2_s255": ("gain", "apply", "cppremat", "carried values recomputed from the input"),
    "tsvc_2_s119": ("gain", "apply", "cppomp", "DaCe's OpenMP loop structure"),
    "tsvc_2_s311": ("gain", "undo", "dinorder", "library sum, reordered into partial sums"),
    "wf_triangular": ("gain", "undo", "duntiled", "skewed $64\\times64$ tiling"),
    "versioned_distance_update": ("gain", "undo", "dnolib", "library recurrence kept in a register"),
    "tsvc_2_s275": ("gain", "undo", "dhoist", "interchange, but loop bounds recomputed in the band loop"),
    "compact_threshold_pack": ("loss", "undo", "dseq", "scan-based compaction, four passes"),
    "tsvc_2_s323": ("loss", "undo", "dseq", "recurrence as a scan with two extra passes"),
    "tsvc_2_s3112": ("loss", "undo", "dnolib", "library prefix sum"),
    "scatter_accum_dup": ("loss", "undo", "dnoatomic", "one atomic update per element"),
    "tsvc_2_s115": ("loss", "undo", "dnoomp", "OpenMP region with a worksharing loop"),
    "wf_diff_skew": ("loss", "undo", "dnoomp", "OpenMP region with a worksharing loop"),
    "fuse_move_ifs": ("loss", "undo", "dnoomp", "OpenMP region; without it, the fusion gains"),
}
GROUP_NAME = {
    "gain": "Faster: a restructuring",
    "loss": "Slower: a form written for many threads",
    "tie": "Within 3%",
}
GROUP_COLOR = {"gain": palette.tab20_slot(4), "loss": palette.tab20_slot(6), "tie": style.MUTED}
GROUP_MARKER = {"gain": "o", "loss": "s", "tie": "D"}


def median_interval(series: list[float], confidence: float = 0.95) -> tuple[float, float]:
    from scipy.stats import binom

    t, n, j = sorted(series), len(series), 1
    while j + 1 <= n // 2 and binom.cdf(n - j - 1, n, 0.5) - binom.cdf(j, n, 0.5) >= confidence:
        j += 1
    return t[j - 1], t[n - j]


def load_cells() -> dict[tuple[str, str], list[float]]:
    cells = {}
    with open(DACE / "results.csv", newline="") as fh:
        for row in csv.DictReader(fh):
            if row["status"] == "ok":
                cells[(row["kernel"], row["cell"])] = [float(x) for x in row["series_ms"].split()]
    return cells


def load_followup() -> dict[tuple[str, str], list[float]]:
    runs = collections.defaultdict(list)
    for rows in (DACE / "followup" / "out" / "rows", DACE / "followup" / "out" / "round3" / "rows"):
        for f in sorted(rows.glob("*.json")):
            r = json.loads(f.read_text())
            for impl in (r.get("impls") or {}).values():
                runs[(r["kernel"], r["label"])] += impl.get("python") or []
    return runs


def roster() -> list[str]:
    return [line.strip() for line in open(DACE.parent / "roster40.txt") if line.strip()]


def figure(cells) -> None:
    style.apply()
    scale = style.PRINT_SCALE
    small = style.PRINT_LEGEND_PT
    rows = []
    for k in roster():
        cpp, dace = cells[(k, "cpp")], cells[(k, "canon_new")]
        ratio = statistics.median(cpp) / statistics.median(dace)
        (c_lo, c_hi), (d_lo, d_hi) = median_interval(cpp), median_interval(dace)
        group = CAUSES[k][0] if k in CAUSES else "tie"
        rows.append((ratio, c_lo / d_hi, c_hi / d_lo, k, group))
    rows.sort(key=lambda r: -r[0])
    pitch_in = 0.112
    left_in, right_in, top_in, bottom_in = 0.62, 0.1, 0.05, 0.55
    height_in = top_in + bottom_in + (len(rows) + 0.5) * pitch_in
    fig = plt.figure(figsize=(TEXT_WIDTH_IN, height_in))
    ax = fig.add_axes((left_in / TEXT_WIDTH_IN, bottom_in / height_in, 1 - (left_in + right_in) / TEXT_WIDTH_IN,
                       1 - (top_in + bottom_in) / height_in))
    ax.set_ylim(len(rows) - 0.5, -0.5)
    ax.set_xlim(-1.5, 4.0)
    for y, (ratio, lo, hi, k, group) in enumerate(rows):
        ax.plot([summary.log2_change(lo), summary.log2_change(hi)], [y, y], color=GROUP_COLOR[group],
                linewidth=0.6, alpha=0.8, zorder=style.CONNECTOR_Z, solid_capstyle="butt")
        style.point_mark(ax, summary.log2_change(ratio), y, GROUP_COLOR[group], GROUP_MARKER[group], filled=True,
                         size=11.0)
    ax.axvline(0.0, color=style.REFERENCE, linewidth=scale.hairline_width, zorder=2)
    ax.set_yticks(range(len(rows)))
    ax.set_yticklabels([et.kernel_compact_display_name(r[3]) for r in rows], fontsize=small)
    ax.tick_params(axis="y", length=0, pad=2)
    ax.set_xticks(range(-1, 5))
    ax.xaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(style.log2_ratio_tick))
    ax.tick_params(axis="x", labelsize=scale.tick_pt, length=2.5, width=0.6, pad=2)
    style.minor_ticks(ax.xaxis, style.MinorKind.LOG2)
    ax.grid(axis="x", which="major", color=style.RULE, linewidth=0.5, zorder=0)
    ax.set_xlabel("Speedup of DaCe over translated C++ (log scale)", fontsize=scale.label_pt, labelpad=3)
    style.despine(ax, keep=("bottom",))
    handles = [Line2D([], [], linestyle="none", marker=GROUP_MARKER[g], color=GROUP_COLOR[g], markersize=4.0,
                      label=GROUP_NAME[g]) for g in ("gain", "loss", "tie")]
    style.fill_width(fig, pad_in=0.06)
    style.legend_below(fig, handles, ncol=3, y=0.005, fontsize=small, markerscale=1.0, columnspacing=1.2,
                       handlelength=1.6)
    style.save(fig, OUT / "dace_speedup", width_in=TEXT_WIDTH_IN)


def fmt(ms: float) -> str:
    return f"{ms:.0f}" if ms >= 100 else f"{ms:.1f}" if ms >= 10 else f"{ms:.2f}"


def tex(name: str) -> str:
    return name.replace("_", r"\_")


def short(kernel: str) -> str:
    name = kernel.removeprefix("tsvc_2_")
    return tex(name if len(name) <= 16 else name[:14]) + (r"\ldots" if len(name) > 16 else "")


def causes_table(followup) -> None:
    med = {key: statistics.median(v) for key, v in followup.items() if v}
    lines = [
        r"% Generated by plotting/thesis/dace_matrix.py -- do not edit.",
        r"\begin{tabular}{@{}lrr>{\raggedright\arraybackslash}p{5.4cm}r@{}}",
        r"  \toprule",
        r"  & \multicolumn{2}{c}{Time (ms)} & & Changed \\",
        r"  \cmidrule(lr){2-3}",
        r"  Kernel & C++ & DaCe & Cause, tested by one change & (ms) \\",
        r"  \midrule",
    ]
    groups = (("gain", "apply", "DaCe faster: the restructuring, applied to C++"),
              ("gain", "undo", "DaCe faster: the restructuring, undone in DaCe's code"),
              ("loss", "undo", "DaCe slower: the parallel form, replaced by a plain loop in DaCe's code"))
    for group, side, title in groups:
        lines.append(rf"  \multicolumn{{5}}{{@{{}}l}}{{\emph{{{title}}}}} \\")
        for k, (g, s, label, cause) in CAUSES.items():
            if (g, s) != (group, side):
                continue
            row = [rf"\quad\texttt{{{short(k)}}}", fmt(med[(k, "cpp")]), fmt(med[(k, "canon")]), cause,
                   fmt(med[(k, label)])]
            lines.append("  " + " & ".join(row) + r" \\")
    lines += [r"  \bottomrule", r"\end{tabular}", ""]
    (OUT / "dace_causes.tex").write_text("\n".join(lines))


def codegen_table(cells) -> None:
    kernels = ("tsvc_2_s3111", "tsvc_2_s319", "tsvc_2_s313", "tsvc_2_vdotr")
    lines = [
        r"% Generated by plotting/thesis/dace_matrix.py -- do not edit.",
        r"\begin{tabular}{@{}llrrrr@{}}",
        r"  \toprule",
        r"  Kernel & Pipeline & C++ & old & old, two settings on & new \\",
        r"  \midrule",
    ]
    for k in kernels:
        for pipe in ("canon", "none"):
            if (k, f"{pipe}_old") not in cells:
                continue
            m = {c: statistics.median(cells[(k, c)]) for c in ("cpp", f"{pipe}_old", f"{pipe}_legacy", f"{pipe}_new")}
            name = rf"\texttt{{{short(k)}}}"
            lines.append(
                f"  {name} & {'optimised' if pipe == 'canon' else 'none'} & {fmt(m['cpp'])} & "
                f"{fmt(m[pipe + '_old'])} & {fmt(m[pipe + '_legacy'])} & {fmt(m[pipe + '_new'])} \\\\"
            )
    geo = {}
    for c in ("canon_old", "canon_legacy", "canon_new"):
        logs = [math.log(statistics.median(cells[(k, "cpp")]) / statistics.median(cells[(k, c)])) for k in roster()]
        geo[c] = math.exp(sum(logs) / len(logs))
    lines += [
        r"  \midrule",
        f"  \\multicolumn{{3}}{{@{{}}l}}{{Geomean speedup over C++, LLR-40}} & {geo['canon_old']:.2f}$\\times$ & "
        f"{geo['canon_legacy']:.2f}$\\times$ & {geo['canon_new']:.2f}$\\times$ \\\\",
        r"  \bottomrule", r"\end{tabular}", "",
    ]
    (OUT / "dace_codegen.tex").write_text("\n".join(lines))
    print("geomeans over C++:", {c: round(v, 3) for c, v in geo.items()})


def main() -> None:
    cells, followup = load_cells(), load_followup()
    figure(cells)
    causes_table(followup)
    codegen_table(cells)
    logs = [math.log(statistics.median(cells[(k, "cpp")]) / statistics.median(cells[(k, "canon_new")]))
            for k in roster()]
    interval = summary.geomean_interval([math.exp(x) for x in logs])
    differ = sum(abs(math.exp(x) - 1) > TIE for x in logs)
    print(f"DaCe over C++: {interval.point:.3f} [{interval.low:.3f}, {interval.high:.3f}], differ >3% on {differ}")
    print(f"wrote {OUT}/dace_speedup.pdf, dace_codegen.tex, dace_causes.tex")


if __name__ == "__main__":
    main()
