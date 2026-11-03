# scicomp40 standalone data (2026-11-03)

`scicomp40.sqlite` needs no repository. Built by `build_standalone.py` from the 8 run DBs; run folders were not opened.

Tables:
- `setups`: setup, job, rows. All 8 arms usable. Each qwen38 HIP arm spans two jobs: the first run (710590 No Skills, 711539 Skills) for the kernels the HIP grading wedge left intact, and the rerun (714849, 714850) for the kernels listed in `reruns/` (`keep_reruns.py` drops the first run's rows for those). 714849 ended in a node failure; its one missing kernel (lulesh) reruns as job 716033. 714850 was still running at extraction, so its token columns are empty.
- `observations`: 4783 rows, one per call, submission, episode or attempt (`row_kind`). Columns are in docs/observations.md. Final grades are `row_kind='submission'`.
- `source_index`: 5513 rows, each source with setup, kernel, kind (`baseline`, `candidate`), attempt `seq` and `sha256`.
- `sources`: 1308 distinct texts keyed by `sha256`.

Final speedup per kernel:

    SELECT setup, kernel, speedup, tokens FROM observations WHERE row_kind = 'submission';

Code of one kernel:

    SELECT i.kind, i.seq, s.text FROM source_index i JOIN sources s USING (sha256)
    WHERE i.setup = 'scicomp40-qwen38-c' AND i.kernel = 'cloudsc';

Figures in `figures/`, from `plots.sh`, all 8 arms. The efficacy figure uses kernels solved in both arms; the per-kernel figure's geomean uses each setup's own solved kernels, so the two numbers differ. `extract/` holds the same data as CSV and files.

## Results at a glance

| Setup | Solved, No Skills → Skills | Geomean speedup, No Skills | Skills | Paired ratio (skills / control) | Kernels paired |
|---|---|---|---|---|---|
| qwen38 C | 39 → 40 | 4.29x | 3.14x | 0.72x | 39 |
| qwen38 HIP | 37 → 37 | 43.5x | 56.8x | 1.19x | 36 |
| oss120b C | 36 → 31 | 0.98x | 1.16x | 1.01x | 31 |
| oss120b HIP | 22 → 20 | 12.8x | 16.4x | 0.94x | 17 |

Speedup is over the best of GCC and numba. None of the four paired effects survives correction within this family. Each arm is one run, so the intervals cover kernel sampling only.

## Reading the database

Python, with nothing but pandas:

    import sqlite3, pandas as pd
    db = sqlite3.connect("scicomp40.sqlite")
    finals = pd.read_sql("SELECT setup, kernel, speedup, tokens FROM observations WHERE row_kind = 'submission'", db)

`plots.sh` and `build_standalone.py` are included for the record. They need the HPCAgent-Bench repository; the database and figures do not.
