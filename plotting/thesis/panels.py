"""Small per-paragraph figures for the thesis: a few kernels, a few columns, one speedup axis.

Shared by ``llr40_matrix.py`` and ``extracted_matrix.py``; each passes its own values, so this module only
draws. Same conventions as the overview figures: log2 speedup axis with ratio ticks, the 95% interval of
the ratio as a line, a missing or invalid version as a hollow cross at 1x, one legend below, print type
sizes, drawn at the text width it is placed at.
"""

import math
import pathlib
from collections.abc import Callable, Sequence

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402
from matplotlib.transforms import blended_transform_factory  # noqa: E402

from hpcagent_bench.stats import style, summary  # noqa: E402

#: Thesis \textwidth (369pt, memoir a4 11pt).
TEXT_WIDTH_IN = 369.0 / 72.27

#: (speedup, ci_lo, ci_hi, hollow); a nan speedup draws the missing cross.
Value = tuple[float, float, float, bool]


def x_range(values: Sequence[float]) -> tuple[float, float]:
    """Whole log2 octaves around the values, at least 0.5x to 2x."""
    logs = [summary.log2_change(v) for v in values if math.isfinite(v) and v > 0] or [0.0]
    return min(-1, math.floor(min(logs) - 0.1)), max(1, math.ceil(max(logs) + 0.1))


def draw(
    stem: pathlib.Path,
    groups: Sequence[tuple[str, Sequence[str]]],
    columns: Sequence[str],
    value: Callable[[str, str], Value],
    label: Callable[[str], str],
    name: dict[str, str],
    color: dict[str, str],
    marker: dict[str, str],
    baseline_name: str,
    xlabel: str,
    summary_rows: Sequence[tuple[str, str, tuple[float, float, float]]] = (),
    hollow_name: dict[str, str] | None = None,
    width_in: float = TEXT_WIDTH_IN,
) -> None:
    """One figure: ``groups`` of kernel rows (a group label is printed right of its rows), each row with one
    mark per column, then optional ``summary_rows`` of (label, column, (point, low, high)). A hollow mark is a
    column's stand-in version; ``hollow_name`` gives its legend label."""
    style.apply()
    scale = style.PRINT_SCALE
    small = style.PRINT_LEGEND_PT
    n = len(columns)
    spread = 0.62 if n > 2 else 0.4
    dodge = [0.0] if n == 1 else [-spread / 2 + spread * i / (n - 1) for i in range(n)]
    gap = 0.45
    rows: list[tuple[float, str]] = []
    spans: list[tuple[str, float, float]] = []
    y = 0.0
    for title, kernels in groups:
        top = y
        for kernel in kernels:
            rows.append((y, kernel))
            y += 1.0
        spans.append((title, top, y - 1.0))
        y += gap
    y -= gap
    srows = [(y + 0.6 + i, lab, col, iv) for i, (lab, col, iv) in enumerate(summary_rows)]
    total = (srows[-1][0] if srows else y - 1.0) + 0.5

    values = [value(k, c)[i] for _, k in rows for c in columns for i in (0, 1, 2)]
    values += [v for *_, iv in srows for v in iv]
    xlo, xhi = x_range(values)

    pitch_in = 0.21 if n <= 3 else 0.26
    top_in, bottom_in = 0.04, 0.62
    height_in = top_in + bottom_in + (total + 0.5) * pitch_in
    fig = plt.figure(figsize=(width_in, height_in))
    ax = fig.add_axes((0.3, bottom_in / height_in, 0.6, 1 - (top_in + bottom_in) / height_in))
    ax.set_ylim(total, -0.5)
    ax.set_xlim(xlo, xhi)

    missing, hollows, filled = [], [], set()
    for yy, kernel in rows:
        for offset, column in zip(dodge, columns):
            s, lo, hi, hollow = value(kernel, column)
            if not math.isfinite(s):
                missing.append((yy + offset, column))
                continue
            if math.isfinite(lo) and math.isfinite(hi):
                ax.plot([summary.log2_change(lo), summary.log2_change(hi)], [yy + offset] * 2, color=color[column],
                        linewidth=0.7, alpha=0.8, zorder=style.CONNECTOR_Z, solid_capstyle="butt")
            style.point_mark(ax, summary.log2_change(s), yy + offset, color[column], marker[column],
                             filled=not hollow, size=16.0)
            if hollow and column not in hollows:
                hollows.append(column)
            filled |= set() if hollow else {column}
    for yy, column in missing:
        style.point_mark(ax, 0.0, yy, color[column], marker[column], filled=False, size=16.0, delivered=False)
    for yy, _, column, iv in srows:
        point, low, high = iv
        ax.plot([summary.log2_change(low), summary.log2_change(high)], [yy, yy], color=color[column],
                linewidth=scale.line_width, zorder=style.CONNECTOR_Z, solid_capstyle="butt")
        style.point_mark(ax, summary.log2_change(point), yy, color[column], marker[column], filled=True,
                         size=22.0)

    ax.axvline(0.0, color=style.REFERENCE, linewidth=scale.hairline_width, zorder=2)
    for _, top, bottom in spans[:-1]:
        ax.axhline(bottom + 0.5 + gap / 2.0, color=style.RULE, linewidth=0.4, zorder=1)
    if srows and rows:
        ax.axhline(srows[0][0] - 0.8, color=style.MUTED, linewidth=0.5, linestyle=(0, (3, 2)), zorder=1)
    ax.set_yticks([yy for yy, _ in rows] + [yy for yy, *_ in srows])
    ax.set_yticklabels([label(k) for _, k in rows] + [lab for _, lab, *_ in srows], fontsize=small)
    ax.tick_params(axis="y", length=0, pad=2)
    side = blended_transform_factory(ax.transAxes, ax.transData)
    for title, top, bottom in spans:
        if title:
            ax.text(1.02, (top + bottom) / 2.0, title, transform=side, fontsize=small, color=style.MUTED,
                    ha="left", va="center")

    ax.set_xticks(range(xlo, xhi + 1))
    ax.xaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(style.log2_ratio_tick))
    ax.tick_params(axis="x", labelsize=scale.tick_pt, length=2.5, width=0.6, pad=2)
    style.minor_ticks(ax.xaxis, style.MinorKind.LOG2)
    ax.grid(axis="x", which="major", color=style.RULE, linewidth=0.5, zorder=0)
    ax.set_xlabel(xlabel, fontsize=scale.label_pt, labelpad=3)
    style.despine(ax, keep=("bottom",))

    shown = filled | {c for _, c in missing} | {c for *_, c, _ in srows}
    handles = [Line2D([], [], linestyle="none", marker=marker[c], color=color[c], markersize=4.0, label=name[c])
               for c in columns if c in shown]
    for c in hollows:
        handles.append(Line2D([], [], linestyle="none", marker=marker[c], markerfacecolor="none", color=color[c],
                              markersize=4.0, label=(hollow_name or {}).get(c, name[c])))
    handles.append(Line2D([], [], color=style.REFERENCE, linewidth=scale.hairline_width,
                          label=f"{baseline_name}, 1x"))
    if missing:
        handles.append(Line2D([], [], linestyle="none", marker="x", color=style.MUTED, markersize=3.5,
                              label="No valid result"))
    style.fill_width(fig, pad_in=0.06)
    style.legend_below(fig, handles, ncol=len(handles), y=0.005, fontsize=small, markerscale=1.0,
                       columnspacing=1.2, handlelength=1.6)
    style.save(fig, stem, width_in=width_in)
