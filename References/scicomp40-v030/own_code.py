"""The scicomp40 C campaign of HPCAgent-Bench v0.3.0 without the answers that are the staged C reference.

    python3 References/scicomp40-v030/own_code.py

Reads ``scicomp40-20261026-report.zip`` next to this file, without unpacking it. A graded answer counts as a
reference fallback when the episode's last submitted source is byte-identical to the
``baseline_<kernel>_reference.c`` staged in the agent's workspace. Prints, per model, the graded answers, the
answers with the model's own code, and the paired comparison on kernels where both models have their own code.
Speedups are over the faster of the GCC and Numba baselines (``plots/kernel-speedups-kernels.csv``).
"""

import csv
import hashlib
import io
import math
import pathlib
import re
import zipfile

ZIP = pathlib.Path(__file__).with_name("scicomp40-20261026-report.zip")
ROOT = "scicomp40-20261026/"
SETUPS = ("scicomp40-qwen38-c", "scicomp40-oss120b-c")
TIE = 1.03


def geomean(xs: list[float]) -> float:
    return math.exp(sum(map(math.log, xs)) / len(xs)) if xs else math.nan


def main() -> None:
    z = zipfile.ZipFile(ZIP)
    names = z.namelist()
    graded = {(r["framework"], r["kernel"]): float(r["speedup"])
              for r in csv.DictReader(io.TextIOWrapper(z.open(ROOT + "plots/kernel-speedups-kernels.csv")))}
    digest = {n: hashlib.sha256(z.read(n)).hexdigest() for n in names if n.endswith(".c")}

    def fallback(setup: str, kernel: str) -> bool:
        prefix = f"{ROOT}extract/sources/{setup}/{kernel}/"
        subs = sorted(n for n in digest if n.startswith(prefix) and re.search(r"/candidate_\d+_submission\.c$", n))
        refs = [n for n in digest if n.startswith(prefix) and n.endswith(f"/baseline_{kernel}_reference.c")]
        return bool(subs and refs and digest[subs[-1]] == digest[refs[0]])

    own = {}
    for setup in SETUPS:
        mine = {k: s for (a, k), s in graded.items() if a == setup}
        own[setup] = {k: s for k, s in mine.items() if not fallback(setup, k)}
        v = list(own[setup].values())
        print(f"{setup}: graded {len(mine)} (geomean {geomean(list(mine.values())):.2f}), "
              f"reference fallbacks {len(mine) - len(v)}, own code {len(v)} (geomean {geomean(v):.2f}, "
              f"faster {sum(x > TIE for x in v)}, slower {sum(x < 1 / TIE for x in v)})")
    a, b = (own[s] for s in SETUPS)
    both = sorted(set(a) & set(b))
    r = [a[k] / b[k] for k in both]
    print(f"own code in both: {len(both)} kernels; qwen38 / oss120b geomean {geomean(r):.2f}, "
          f"qwen38 faster on {sum(x > TIE for x in r)}, oss120b faster on {sum(x < 1 / TIE for x in r)}")


if __name__ == "__main__":
    main()
