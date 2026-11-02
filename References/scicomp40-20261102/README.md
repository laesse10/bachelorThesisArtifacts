# scicomp40 standalone data (2026-11-02)

`scicomp40.sqlite` needs no repository. Built by `build_standalone.py` from copies of the 8 run DBs; run folders were not opened.

Tables:
- `setups`: setup, job, rows, `usable`. The two qwen38 HIP arms have `usable=0`: the HIP grading wedge damaged them.
- `observations`: 3888 rows, one per call, submission, episode or attempt (`row_kind`). Columns are in docs/observations.md. Final grades are `row_kind='submission'`.
- `source_index`: 3040 rows, each source with setup, kernel, kind (`baseline`, `candidate`), attempt `seq` and `sha256`.
- `sources`: 1077 distinct texts keyed by `sha256`.

Final speedup per kernel:

    SELECT setup, kernel, speedup, tokens FROM observations WHERE row_kind = 'submission';

Code of one kernel:

    SELECT i.kind, i.seq, s.text FROM source_index i JOIN sources s USING (sha256)
    WHERE i.setup = 'scicomp40-qwen38-c' AND i.kernel = 'cloudsc';

Figures in `figures/`, from `plots.sh`, usable setups only. The efficacy figure uses kernels solved in both arms; the per-kernel figure's geomean uses each setup's own solved kernels, so the two numbers differ. `extract/` holds the same data as CSV and files.

## Results at a glance

| Setup | Solved, No Skills → Skills | Geomean speedup, No Skills | Skills | Paired ratio (skills / control) | Kernels paired |
|---|---|---|---|---|---|
| qwen38 C | 39 → 40 | 4.29x | 3.14x | 0.72x | 39 |
| oss120b C | 36 → 31 | 0.98x | 1.16x | 1.01x | 31 |
| oss120b HIP | 22 → 20 | 12.8x | 16.4x | 0.94x | 17 |

Speedup is over the best of GCC and numba. None of the three paired effects survives correction within this family. Each arm is one run, so the intervals cover kernel sampling only.

## Reading the database

Python, with nothing but pandas:

    import sqlite3, pandas as pd
    db = sqlite3.connect("scicomp40.sqlite")
    finals = pd.read_sql("SELECT setup, kernel, speedup, tokens FROM observations WHERE row_kind = 'submission'", db)

`plots.sh` and `build_standalone.py` are included for the record. They need the HPCAgent-Bench repository; the database and figures do not.
