"""The numbers that thesis Section 6.4 ("A second campaign") quotes from this campaign.

    python3 References/scicomp40-20261102/thesis_numbers.py

Reads ``scicomp40.sqlite`` next to this file (built by Y. Budanaz, see README.md) and its ``figures/pairs.csv``.
Usable setups only: the two Qwen3.8-27B HIP arms are marked ``usable = 0`` (HIP grading wedge). A kernel counts as
solved when it has a final grade (``row_kind = 'submission'``) with a positive speedup over the faster of GCC and
Numba. The database keeps the Python baselines but not the staged C reference, so answers that resubmit that
reference are not separated here.
"""

import csv
import math
import pathlib
import sqlite3

HERE = pathlib.Path(__file__).resolve().parent
TIE = 1.03


def geomean(xs: list[float]) -> float:
    return math.exp(sum(map(math.log, xs)) / len(xs)) if xs else math.nan


def main() -> None:
    db = sqlite3.connect(f"file:{HERE / 'scicomp40.sqlite'}?mode=ro", uri=True)
    usable = {s for s, u in db.execute("SELECT setup, usable FROM setups") if u}
    solved: dict[str, dict[str, float]] = {}
    for setup, kernel, speedup in db.execute(
            "SELECT setup, kernel, speedup FROM observations WHERE row_kind = 'submission' AND speedup > 0"):
        if setup in usable:
            solved.setdefault(setup, {})[kernel] = speedup
    for setup in sorted(solved):
        v = list(solved[setup].values())
        print(f"{setup:34s} solved {len(v):2d}  geomean {geomean(v):5.2f}  "
              f"faster {sum(x > TIE for x in v):2d}  slower {sum(x < 1 / TIE for x in v):2d}")
    q, o = solved["scicomp40-qwen38-c"], solved["scicomp40-oss120b-c"]
    both = sorted(set(q) & set(o))
    r = [q[k] / o[k] for k in both]
    print(f"without skills, C, solved by both: {len(both)} kernels; qwen38 / oss120b geomean {geomean(r):.2f}, "
          f"qwen38 faster on {sum(x > TIE for x in r)}, oss120b faster on {sum(x < 1 / TIE for x in r)}")
    print("language skills against none (figures/pairs.csv, speedup leg):")
    for row in csv.DictReader(open(HERE / "figures" / "pairs.csv")):
        if row["leg"] == "speedup":
            print(f"  {row['setup_a']}: ratio {float(row['rho']):.2f} on {row['n_pairs']} kernels, "
                  f"p = {float(row['p_value']):.2f}, adjusted {float(row['p_adjusted']):.2f} ({row['verdict']})")


if __name__ == "__main__":
    main()
