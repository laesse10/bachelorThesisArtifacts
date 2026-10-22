"""C against Fortran in the agents' code, for the thesis: one figure.

    python plotting/run.py plotting/thesis/agent_language.py

Reads ``llr40Matrix/v2/agent_language/median/stats.csv`` (benchmark default build, medians of 30 runs; see
``llr40Matrix/v2/agent_language/analysis.md``) and writes ``agent_language.{pdf,png}`` under the thesis
repository's ``Paper/figures/`` (``$THESIS_FIGURES`` overrides).

Two panels, one core and 72 threads. Per way of picking a submission (best, first, median, best of the
first k), the geometric mean of the speedup of C over Fortran with its 95% interval, pooled over the
three models (large mark) and for each model (small marks).
"""

import csv
import os
import pathlib

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402

from hpcagent_bench.stats import palette, style, summary  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[2]
DATA = ROOT / "llr40Matrix" / "v2" / "agent_language" / "median" / "stats.csv"
OUT = pathlib.Path(os.environ.get("THESIS_FIGURES", ROOT.parent / "BachelorthesisPaper" / "Paper" / "figures"))
TEXT_WIDTH_IN = 369.0 / 72.27

STATISTICS = (("best", "Best"), ("first", "First"), ("median", "Median"), ("bestfirstk", "Best of first $k$"))
MODELS = (("pooled", "All three models"), ("kimi27sglang", "Kimi-K2.7"), ("oss120b", "GPT-OSS-120B"),
          ("qwen38", "Qwen3.8-27B"))
COLOR = {"pooled": "black", "kimi27sglang": palette.tab20_slot(2), "oss120b": palette.tab20_slot(4),
         "qwen38": palette.tab20_slot(8)}
PANELS = (("1", "One core"), ("72", "72 threads"))


def load() -> dict[tuple[str, str, str], dict[str, float]]:
    out = {}
    for r in csv.DictReader(open(DATA)):
        if r["build"] == "default":
            out[(r["threads"], r["statistic"], r["model"])] = {
                k: float(r[k]) for k in ("geomean_f_over_c", "ci95_low", "ci95_high", "wilcoxon_p", "n_pairs")}
    return out


def main() -> None:
    data = load()
    style.apply()
    scale = style.PRINT_SCALE
    small = style.PRINT_LEGEND_PT
    offsets = {"pooled": -0.27, "kimi27sglang": -0.06, "oss120b": 0.12, "qwen38": 0.30}
    height_in = 2.1
    fig, axes = plt.subplots(1, 2, figsize=(TEXT_WIDTH_IN, height_in), sharey=True)
    for ax, (threads, title) in zip(axes, PANELS):
        for i, (stat, _) in enumerate(STATISTICS):
            for model, _ in MODELS:
                d = data[(threads, stat, model)]
                y = i + offsets[model]
                big = model == "pooled"
                ax.plot([summary.log2_change(d["ci95_low"]), summary.log2_change(d["ci95_high"])], [y, y],
                        color=COLOR[model], linewidth=scale.line_width if big else 0.6,
                        zorder=style.CONNECTOR_Z, solid_capstyle="butt")
                style.point_mark(ax, summary.log2_change(d["geomean_f_over_c"]), y, COLOR[model],
                                 "o" if big else "s", filled=True, size=18.0 if big else 8.0)
        ax.axvline(0.0, color=style.REFERENCE, linewidth=scale.hairline_width, zorder=2)
        for i in range(1, len(STATISTICS)):
            ax.axhline(i - 0.5, color=style.RULE, linewidth=0.3, zorder=1)
        ax.set_xlim(-1.0, 1.75)
        ax.set_xticks([-1, 0, 1])
        ax.xaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(style.log2_ratio_tick))
        style.minor_ticks(ax.xaxis, style.MinorKind.LOG2)
        ax.tick_params(axis="x", labelsize=scale.tick_pt, length=2.5, width=0.6, pad=2)
        ax.grid(axis="x", which="major", color=style.RULE, linewidth=0.5, zorder=0)
        ax.set_title(title, fontsize=scale.label_pt, pad=3)
        ax.set_xlabel("Speedup of C over Fortran", fontsize=scale.label_pt, labelpad=2)
        style.despine(ax, keep=("bottom",))
    axes[0].set_ylim(len(STATISTICS) - 0.5, -0.5)
    axes[0].set_yticks(range(len(STATISTICS)))
    axes[0].set_yticklabels([label for _, label in STATISTICS], fontsize=small)
    axes[0].tick_params(axis="y", length=0, pad=2)
    axes[1].tick_params(axis="y", length=0)
    handles = [Line2D([], [], linestyle="none", marker="o" if m == "pooled" else "s", color=COLOR[m],
                      markersize=4.0 if m == "pooled" else 3.0, label=label) for m, label in MODELS]
    fig.subplots_adjust(left=0.2, right=0.98, top=0.86, bottom=0.28, wspace=0.08)
    style.legend_below(fig, handles, ncol=4, y=0.005, fontsize=small, markerscale=1.0, columnspacing=1.2,
                       handlelength=1.2)
    style.save(fig, OUT / "agent_language", width_in=TEXT_WIDTH_IN)
    for threads, _ in PANELS:
        for stat, _ in STATISTICS:
            d = data[(threads, stat, "pooled")]
            print(f"threads {threads:>2} {stat:10s} {d['geomean_f_over_c']:.2f} "
                  f"[{d['ci95_low']:.2f}, {d['ci95_high']:.2f}] p={d['wilcoxon_p']:.3f} n={int(d['n_pairs'])}")
    print(f"wrote {OUT / 'agent_language'}.pdf, .png")


if __name__ == "__main__":
    main()
