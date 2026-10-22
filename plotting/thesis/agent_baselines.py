"""Agents against compilers on LLR-40 and the scientific-computing set, for the thesis: one figure, two CSVs.

    python plotting/run.py plotting/thesis/agent_baselines.py

Reads ``References/hb-baselines-v021/`` (the no-skill arm of the HPCAgent-Bench v0.2.1 campaign, two models in
C and HIP on llr40 and scicomp40, and the compiler sweep beside it; snapshot 2026-10-22, see its README.md).
Writes ``agent_baselines.{pdf,png,csv}`` and ``agent_baselines_summary.csv`` under the thesis repository's
``Paper/figures/`` (``$THESIS_FIGURES`` overrides).

Every speedup is over the faster of the benchmark's GCC and Numba references, on an AMD MI300A node.

* Agent rows: the graded speedup of each kernel's final answer (``plots/<set>-<lang>-baselines-kernels.csv``),
  which the judge times against the faster of its C and Numba references at the episode's own inputs. A kernel
  without a verified, timed answer has no mark.
* Compiler rows: the sweep's median time of the column (``data/canon.db``), divided by the faster of the sweep's
  own Numba and automatically parallelized C (``cc_autopar``) times for the kernel (run ``<set>-base``). The
  package's own plots divide the compiler columns by ``cc_autopar`` alone; Numba is the faster reference on 12 of
  the 40 llr40 kernels and 27 of the 38 scicomp40 kernels, so they overstate the compilers there. A kernel the
  compiler does not handle has no mark. PPCG is left out: its numbers predate a synchronization fix (README).

Per row: one small mark per kernel, the geometric mean over the marked kernels with a 95% log-t interval (as
``summary.geomean_interval`` everywhere else in the thesis), and the count of marked kernels.
``agent_baselines_summary.csv`` also gives the geometric mean over all kernels of the set with an unmarked kernel
counted as 1, and the paired comparisons quoted in the text (Wilcoxon signed-rank on the log ratios).
"""

import csv
import math
import os
import pathlib
import sqlite3

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402

from hpcagent_bench.stats import palette, style, summary  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[2]
DATA = ROOT / "References" / "hb-baselines-v021"
OUT = pathlib.Path(os.environ.get("THESIS_FIGURES", ROOT.parent / "BachelorthesisPaper" / "Paper" / "figures"))
TEXT_WIDTH_IN = 369.0 / 72.27

SETS = (("llr40", "llr40v021", "LLR-40 (40 kernels)", 40),
        ("scicomp40", "scicomp40", "Scientific computing (38 kernels)", 38))
#: (key, label, device group, kind); agent keys are filled in per set.
ROWS = (
    ("qwen38-c", "Qwen3.8-27B, C", "CPU", "agent"),
    ("oss120b-c", "GPT-OSS-120B, C", "CPU", "agent"),
    ("dace_cpu_canon", "DaCe (CPU)", "CPU", "compiler"),
    ("pluto", "Pluto", "CPU", "compiler"),
    ("polly", "Polly", "CPU", "compiler"),
    ("numba_fastmath", "Numba, fastmath", "CPU", "compiler"),
    ("qwen38-hip", "Qwen3.8-27B, HIP", "GPU", "agent"),
    ("oss120b-hip", "GPT-OSS-120B, HIP", "GPU", "agent"),
    ("dace_gpu_canon", "DaCe (GPU)", "GPU", "compiler"),
)
COLOR = {
    "qwen38-c": palette.tab20_slot(0), "qwen38-hip": palette.tab20_slot(0),
    "oss120b-c": palette.tab20_slot(2), "oss120b-hip": palette.tab20_slot(2),
    "dace_cpu_canon": palette.tab20_slot(4), "dace_gpu_canon": palette.tab20_slot(4),
    "pluto": palette.tab20_slot(8), "polly": palette.tab20_slot(10), "numba_fastmath": palette.framework_color("numba"),
}
MARKER = {"agent": "o", "compiler": "s"}


def agent_speedups(setname: str, exp: str) -> dict[str, dict[str, float]]:
    out: dict[str, dict[str, float]] = {}
    for lang in ("c", "hip"):
        for r in csv.DictReader(open(DATA / "plots" / f"{setname}-{lang}-baselines-kernels.csv")):
            if r["framework"].startswith(exp + "-") and r["denominator_ms"]:
                key = r["framework"][len(exp) + 1:]
                out.setdefault(key, {})[r["kernel"]] = float(r["speedup"])
    return out


def compiler_speedups(setname: str) -> dict[str, dict[str, float]]:
    """Compiler time from the package's per-kernel rows, reference = min(numba, cc_autopar) of the sweep."""
    db = sqlite3.connect(DATA / "data" / "canon.db")
    ref: dict[str, float] = {}
    for col, kernel, ms in db.execute(
            'select "column", kernel, median_ms from canon where run = ? and validated = 1 and status = \'ok\' '
            'and median_ms is not null and "column" in (\'numba\', \'cc_autopar\')', (f"{setname}-base",)):
        ref[kernel] = min(ref.get(kernel, math.inf), ms)
    out: dict[str, dict[str, float]] = {}
    for r in csv.DictReader(open(DATA / "plots" / f"{setname}-c-baselines-kernels.csv")):
        if r["framework"] in COLOR and r["denominator_ms"] and r["kernel"] in ref:
            out.setdefault(r["framework"], {})[r["kernel"]] = ref[r["kernel"]] / float(r["denominator_ms"])
    return out


def wilcoxon_p(ratios: list[float]) -> float:
    from scipy.stats import wilcoxon

    logs = [math.log(x) for x in ratios]
    return float(wilcoxon(logs).pvalue) if len(logs) >= 6 and any(logs) else math.nan


def main() -> None:
    data = {}
    for setname, exp, _, _ in SETS:
        cells = agent_speedups(setname, exp)
        cells.update(compiler_speedups(setname))
        data[setname] = cells

    OUT.mkdir(parents=True, exist_ok=True)
    with open(OUT / "agent_baselines.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["set", "row", "kernel", "speedup_over_best_of_c_numba"])
        for setname, *_ in SETS:
            for key, *_ in ROWS:
                for kernel, s in sorted(data[setname].get(key, {}).items()):
                    w.writerow([setname, key, kernel, f"{s:.4f}"])

    summary_rows = []
    for setname, _, _, total in SETS:
        for key, label, device, kind in ROWS:
            v = list(data[setname].get(key, {}).values())
            g = summary.geomean_interval(v)
            g_all = summary.geomean_interval(v + [1.0] * (total - len(v)))
            summary_rows.append({"set": setname, "row": key, "label": label, "device": device, "kind": kind,
                                 "n": len(v), "of": total, "geomean": g.point, "ci95_low": g.low,
                                 "ci95_high": g.high, "geomean_all_unmarked_as_1": g_all.point,
                                 "all_ci95_low": g_all.low, "all_ci95_high": g_all.high})
        cells = data[setname]
        for a, b in (("qwen38-c", "oss120b-c"), ("qwen38-hip", "oss120b-hip"), ("qwen38-hip", "qwen38-c"),
                     ("oss120b-hip", "oss120b-c"), ("qwen38-c", "dace_cpu_canon"), ("oss120b-c", "dace_cpu_canon"),
                     ("qwen38-hip", "dace_gpu_canon"), ("oss120b-hip", "dace_gpu_canon")):
            ks = sorted(set(cells.get(a, {})) & set(cells.get(b, {})))
            r = [cells[a][k] / cells[b][k] for k in ks]
            g = summary.geomean_interval(r)
            summary_rows.append({"set": setname, "row": f"{a} / {b}", "label": "paired", "device": "", "kind": "paired",
                                 "n": len(ks), "of": total, "geomean": g.point, "ci95_low": g.low,
                                 "ci95_high": g.high, "wins": sum(x > 1.03 for x in r),
                                 "losses": sum(x < 1 / 1.03 for x in r), "wilcoxon_p": wilcoxon_p(r)})
    fields = ["set", "row", "label", "device", "kind", "n", "of", "geomean", "ci95_low", "ci95_high",
              "geomean_all_unmarked_as_1", "all_ci95_low", "all_ci95_high", "wins", "losses", "wilcoxon_p"]
    with open(OUT / "agent_baselines_summary.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=fields)
        w.writeheader()
        for row in summary_rows:
            w.writerow({k: (f"{v:.4f}" if isinstance(v, float) else v) for k, v in row.items()})

    figure(data)
    for row in summary_rows:
        extra = (f" W/L {row['wins']}/{row['losses']} p={row['wilcoxon_p']:.4f}" if row["kind"] == "paired"
                 else f"  all {row['geomean_all_unmarked_as_1']:.2f}")
        print(f"{row['set']:10s} {row['row']:32s} n={row['n']:2d}/{row['of']} "
              f"{row['geomean']:8.2f} [{row['ci95_low']:.2f}, {row['ci95_high']:.2f}]{extra}")
    print(f"wrote {OUT / 'agent_baselines'}.pdf, .png, .csv and agent_baselines_summary.csv")


def figure(data) -> None:
    style.apply()
    scale = style.PRINT_SCALE
    small = style.PRINT_LEGEND_PT
    gap = 0.6
    ys, y = [], 0.0
    for i, (_, _, device, _) in enumerate(ROWS):
        if i and device != ROWS[i - 1][2]:
            y += gap
        ys.append(y)
        y += 1.0
    height_in = 0.62 + 0.25 + (ys[-1] + 1.0) * 0.25
    fig, axes = plt.subplots(1, 2, figsize=(TEXT_WIDTH_IN, height_in), sharey=True)
    for ax, (setname, _, title, total) in zip(axes, SETS):
        cells = data[setname]
        for yy, (key, _, _, kind) in zip(ys, ROWS):
            values = sorted(cells.get(key, {}).values())
            for j, s in enumerate(values):
                jitter = ((j * 0.6180339887) % 1.0 - 0.5) * 0.36
                ax.scatter([summary.log2_change(s)], [yy + jitter], s=4.0, color=COLOR[key], alpha=0.45,
                           linewidths=0, zorder=3, marker=MARKER[kind])
            g = summary.geomean_interval(values)
            if values:
                if math.isfinite(g.low):
                    ax.plot([summary.log2_change(g.low), summary.log2_change(g.high)], [yy, yy], color=COLOR[key],
                            linewidth=scale.line_width, zorder=4, solid_capstyle="butt")
                style.point_mark(ax, summary.log2_change(g.point), yy, COLOR[key], MARKER[kind], filled=True,
                                 size=20.0)
            ax.text(1.0, yy, f" {len(values)}/{total}", transform=matplotlib.transforms.blended_transform_factory(
                ax.transAxes, ax.transData), fontsize=small, color=style.MUTED, ha="left", va="center")
        ax.axvline(0.0, color=style.REFERENCE, linewidth=scale.hairline_width, zorder=2)
        for i in range(1, len(ROWS)):
            if ROWS[i][2] != ROWS[i - 1][2]:
                ax.axhline((ys[i - 1] + ys[i]) / 2.0, color=style.MUTED, linewidth=0.5, linestyle=(0, (3, 2)))
        ax.set_xlim(-7.0, 13.0)
        ax.set_xticks(range(-4, 13, 4))
        style.minor_ticks(ax.xaxis, style.MinorKind.LOG2)
        ax.xaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(style.log2_ratio_tick))
        ax.tick_params(axis="x", labelsize=scale.tick_pt, length=2.5, width=0.6, pad=2)
        ax.grid(axis="x", which="major", color=style.RULE, linewidth=0.5, zorder=0)
        ax.set_title(title, fontsize=scale.label_pt, pad=3)
        style.despine(ax, keep=("bottom",))
    axes[0].set_ylim(ys[-1] + 0.7, -0.7)
    axes[0].set_yticks(ys)
    axes[0].set_yticklabels([label for _, label, _, _ in ROWS], fontsize=small)
    for ax in axes:
        ax.tick_params(axis="y", length=0, pad=2)
    handles = [Line2D([], [], linestyle="none", marker="o", color="black", markersize=3.5, label="Agent"),
               Line2D([], [], linestyle="none", marker="s", color="black", markersize=3.5, label="Compiler"),
               Line2D([], [], linestyle="none", marker="o", color="black", alpha=0.45, markersize=2.0,
                      label="One kernel"),
               Line2D([], [], color="black", linewidth=scale.line_width, label="Geomean, 95% CI")]
    fig.subplots_adjust(left=0.2, right=0.92, top=0.92, bottom=0.62 / height_in + 0.03, wspace=0.24)
    fig.text(0.56, 0.30 / height_in, "Speedup over the faster of GCC and Numba (log scale)",
             ha="center", va="bottom", fontsize=scale.label_pt)
    style.legend_below(fig, handles, ncol=4, y=0.005, fontsize=small, markerscale=1.0, columnspacing=1.2,
                       handlelength=1.4)
    style.save(fig, OUT / "agent_baselines", width_in=TEXT_WIDTH_IN)


if __name__ == "__main__":
    main()
