"""The numbers that thesis Section 6.4 ("A second campaign") quotes from this campaign.

    python3 References/scicomp40-20261103/thesis_numbers.py

Reads ``scicomp40.sqlite`` next to this file (built by Y. Budanaz, see README.md) and its ``figures/pairs.csv``.
The two Qwen3.8-27B HIP arms combine the first run with the rerun of the kernels the HIP grading wedge damaged; the
database already holds only the rows that ``keep_reruns.py`` keeps. A kernel counts as solved when its last final
grade (``row_kind = 'submission'``) has a positive speedup over the faster of GCC and Numba. A fallback is a final
answer whose graded source is the staged C reference of its episode (``baseline_<kernel>_reference.c``).
"""

import csv
import math
import pathlib
import sqlite3

HERE = pathlib.Path(__file__).resolve().parent
TIE = 1.03


def geomean(xs: list[float]) -> float:
    return math.exp(sum(map(math.log, xs)) / len(xs)) if xs else math.nan


def compare(solved: dict, fallback: dict, a: str, b: str, drop: bool) -> str:
    both = sorted(k for k in set(solved[a]) & set(solved[b])
                  if not drop or not (fallback[a].get(k) or fallback[b].get(k)))
    r = [solved[a][k] / solved[b][k] for k in both]
    return (f"{len(both)} kernels; geomean {geomean(r):.2f}, "
            f"first faster on {sum(x > TIE for x in r)}, second faster on {sum(x < 1 / TIE for x in r)}")


def main() -> None:
    db = sqlite3.connect(f"file:{HERE / 'scicomp40.sqlite'}?mode=ro&immutable=1", uri=True)
    reference = {(s, k, e): h for s, k, e, h in db.execute(
        "SELECT setup, kernel, episode_id, sha256 FROM source_index "
        "WHERE kind = 'baseline' AND rel_path LIKE '%\\_reference.c' ESCAPE '\\'")}
    solved: dict[str, dict[str, float]] = {}
    fallback: dict[str, dict[str, bool]] = {}
    for setup, kernel, episode, speedup, blob in db.execute(
            "SELECT setup, kernel, episode_id, speedup, source_blob FROM observations "
            "WHERE row_kind = 'submission' ORDER BY ts_ms"):
        solved.setdefault(setup, {})[kernel] = speedup
        fallback.setdefault(setup, {})[kernel] = blob is not None and blob == reference.get((setup, kernel, episode))
    for setup in sorted(solved):
        solved[setup] = {k: v for k, v in solved[setup].items() if v and v > 0}
        v = list(solved[setup].values())
        print(f"{setup:34s} solved {len(v):2d}  geomean {geomean(v):5.2f}  "
              f"faster {sum(x > TIE for x in v):2d}  slower {sum(x < 1 / TIE for x in v):2d}  "
              f"fallbacks {sum(fallback[setup].get(k, False) for k in solved[setup])}")
    for lang in ("c", "hip"):
        a, b = f"scicomp40-qwen38-{lang}", f"scicomp40-oss120b-{lang}"
        print(f"without skills, {lang}, qwen38 / oss120b, solved by both: {compare(solved, fallback, a, b, False)}")
        print(f"  without fallbacks: {compare(solved, fallback, a, b, True)}")
    print("language skills against none (figures/pairs.csv, speedup leg):")
    for row in csv.DictReader(open(HERE / "figures" / "pairs.csv")):
        if row["leg"] == "speedup":
            print(f"  {row['setup_a']}: ratio {float(row['rho']):.2f} on {row['n_pairs']} kernels, "
                  f"p = {float(row['p_value']):.2f}, adjusted {float(row['p_adjusted']):.2f} ({row['verdict']})")


if __name__ == "__main__":
    main()
