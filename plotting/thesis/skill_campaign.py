"""The preconditioning-skill campaign for the thesis: one figure.

    python plotting/run.py plotting/thesis/skill_campaign.py

Reads ``References/solver14-report/speedup.csv`` (experiment solver14-precond, HPCAgent-Bench v0.2, snapshot
2026-10-19; copied from ``References/solver14-report-all.zip``) and writes ``skill_campaign.{pdf,png}`` under
the thesis repository's ``Paper/figures/`` (``$THESIS_FIGURES`` overrides).

One panel per model, one row per kernel: the benchmark score of the reported episode in each setting (no
skill, the skill as one page, as five pages). An answer the benchmark did not credit (wrong, missing,
rejected by a sanitizer) scores 1 and is drawn hollow; an episode still running at the snapshot has no mark.
Bottom row: the geometric mean over all fourteen kernels, an uncredited kernel counting as 1.
"""

import csv
import math
import os
import pathlib

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402

from hpcagent_bench.stats import palette, style, summary  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[2]
#: The report's local copy; ``References/old runs/`` is where it lives after the References folder was tidied.
DATA = next((d / "solver14-report" / "speedup.csv" for d in (ROOT / "References", ROOT / "References" / "old runs")
             if (d / "solver14-report" / "speedup.csv").is_file()), ROOT / "References" / "solver14-report" / "speedup.csv")
OUT = pathlib.Path(os.environ.get("THESIS_FIGURES", ROOT.parent / "BachelorthesisPaper" / "Paper" / "figures"))
TEXT_WIDTH_IN = 369.0 / 72.27

MODELS = (("oss120b", "GPT-OSS-120B"), ("qwen38", "Qwen3.8-27B"))
#: Legend labels use the thesis's names (Table 5.2): v2 is the tested skill, the benchmark's page is v1.
SETTINGS = (("none", "No skill"), ("preconditioner-v4-single", "v2-single (one page)"),
            ("preconditioner-v4-split", "v2-split (five pages)"))
COLOR = {"none": "black", "preconditioner-v4-single": palette.tab20_slot(0),
         "preconditioner-v4-split": palette.tab20_slot(2)}
MARKER = {"none": "o", "preconditioner-v4-single": "s", "preconditioner-v4-split": "D"}


def load() -> dict[tuple[str, str, str], tuple[float, bool]]:
    """(model, setting, kernel) -> (score, credited); running episodes are left out."""
    out = {}
    for r in csv.DictReader(open(DATA)):
        if r["outcome"] == "running":
            continue
        out[(r["model"], r["variant"], r["kernel"])] = (float(r["bench_speedup"]), r["outcome"] == "solved")
    return out


def main() -> None:
    data = load()
    kernels = sorted({k for (_, _, k) in data})
    # Order the kernels by the no-skill score of the first model, highest first.
    kernels.sort(key=lambda k: -data.get((MODELS[0][0], "none", k), (0.0, False))[0])
    style.apply()
    scale = style.PRINT_SCALE
    small = style.PRINT_LEGEND_PT
    offsets = {"none": -0.25, "preconditioner-v4-single": 0.0, "preconditioner-v4-split": 0.25}
    mean_y = len(kernels) + 0.6
    height_in = 3.3
    fig, axes = plt.subplots(1, 2, figsize=(TEXT_WIDTH_IN, height_in), sharey=True)
    for ax, (model, title) in zip(axes, MODELS):
        for i, kernel in enumerate(kernels):
            for setting, _ in SETTINGS:
                cell = data.get((model, setting, kernel))
                if cell is None:
                    continue
                score, credited = cell
                style.point_mark(ax, summary.log2_change(score), i + offsets[setting], COLOR[setting],
                                 MARKER[setting], filled=credited, size=9.0)
        for setting, _ in SETTINGS:
            scores = [data[(model, setting, k)][0] for k in kernels if (model, setting, k) in data]
            if len(scores) < len(kernels):
                continue  # an incomplete setting gets no mean
            g = math.exp(sum(math.log(s) for s in scores) / len(scores))
            style.point_mark(ax, summary.log2_change(g), mean_y + offsets[setting], COLOR[setting],
                             MARKER[setting], filled=True, size=16.0)
        ax.axvline(0.0, color=style.REFERENCE, linewidth=scale.hairline_width, zorder=2)
        ax.axhline(len(kernels) - 0.1, color=style.MUTED, linewidth=0.5, linestyle=(0, (3, 2)), zorder=1)
        for i in range(1, len(kernels)):
            ax.axhline(i - 0.5, color=style.RULE, linewidth=0.25, zorder=1)
        ax.set_xlim(-3.5, 6.5)
        ax.set_xticks(range(-2, 7, 2))
        ax.xaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(style.log2_ratio_tick))
        ax.tick_params(axis="x", labelsize=scale.tick_pt, length=2.5, width=0.6, pad=2)
        ax.grid(axis="x", which="major", color=style.RULE, linewidth=0.5, zorder=0)
        ax.set_title(title, fontsize=scale.label_pt, pad=3)
        ax.set_xlabel("Benchmark score (log scale)", fontsize=scale.label_pt, labelpad=2)
        style.despine(ax, keep=("bottom",))
    axes[0].set_ylim(mean_y + 0.6, -0.6)
    axes[0].set_yticks(list(range(len(kernels))) + [mean_y])
    axes[0].set_yticklabels(kernels + ["Geomean, 14 kernels"], fontsize=small)
    for tick in axes[0].get_yticklabels()[: len(kernels)]:
        tick.set_fontfamily("monospace")
    for ax in axes:
        ax.tick_params(axis="y", length=0, pad=2)
    handles = [Line2D([], [], linestyle="none", marker=MARKER[s], color=COLOR[s], markersize=3.5, label=label)
               for s, label in SETTINGS]
    handles.append(Line2D([], [], linestyle="none", marker="o", markerfacecolor="none", color=style.MUTED,
                          markersize=3.5, label="Not credited (score 1)"))
    fig.subplots_adjust(left=0.24, right=0.98, top=0.93, bottom=0.17, wspace=0.06)
    style.legend_below(fig, handles, ncol=4, y=0.005, fontsize=small, markerscale=1.0, columnspacing=1.0,
                       handlelength=1.0)
    style.save(fig, OUT / "skill_campaign", width_in=TEXT_WIDTH_IN)
    print(f"wrote {OUT / 'skill_campaign'}.pdf, .png")


if __name__ == "__main__":
    main()
