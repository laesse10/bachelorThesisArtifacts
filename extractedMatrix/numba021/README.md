# numba021: the v0.2.1 Numba translator on the extracted-kernel matrix

Job 5105398 (2026-10-20, partition `normal`, nid006532 and nid006534). See `DEVIATIONS.md` item 13 for
the full method. In short: the HPCAgent-Bench v0.2.1 Numba translator (release-v0.2.1 305559cc9, with
PR #113) emitted the modules from the 26a4f0cf references and bench-info. The unchanged 26a4f0cf harness
timed them, with the matrix's protocol (preset M, validated, 5 warm-up + 30 timed, min-of-k, one
exclusive node, one core, Numba 0.67).

## Results (`cells.csv`)

| kernel | column | status | min | median | RSD | speedup over matrix `c` | over `c` in this job |
|---|---|---|---:|---:|---:|---:|---:|
| `warpx_field_gather` | `numba` (v0.2.1) | ok | 20.787 ms | 20.907 ms | 0.30% | 0.545 | 0.555 |
| `warpx_field_gather` | `c` (drift check) | ok | 11.540 ms | 11.774 ms | 1.03% | 0.982 | 1.000 |
| `warpx_esirkepov_deposition` | `numba` (v0.2.1) | no output (runner: `incorrect`) | | | | | |

- `warpx_field_gather` `numba` was `build_error` in the matrix (DEVIATIONS 10). It now runs and validates.
  It takes 1.8x the time of translated C.
- The C cell has the same source bytes as the matrix's (`analyze_numba021.py` checks this). It ran 1.8%
  slower than in job 4990441. That is the size of the day-to-day difference between the two jobs.
- `warpx_esirkepov_deposition`: the module types now (the "Dimension mismatch" of the matrix is gone),
  but the first call raises in Numba's parfor size check at line 332 (`gx = ...`, an outer product of
  two slices with run-time bounds, in the GEOM_3D branch the graded configuration runs). With JIT off,
  the same module matches NumPy exactly at preset S. The matrix keeps `numba_hand` for this cell.
- The other five translated kernels emit byte-identical under v0.2.1. Their matrix rows stand.

## The `numba` column with `warpx_field_gather` (`columns.csv`)

| column | n | geomean speedup over `c` | 95% interval | faster / within 3% / slower |
|---|---:|---:|---|---|
| `numba` (matrix, 26a4f0cf translator) | 5 | 1.085 | 0.546 - 2.157 | 3 / 0 / 2 |
| `numba` + field_gather v0.2.1 | 6 | 0.968 | 0.533 - 1.759 | 3 / 0 / 3 |
| same, against the C of this job for field_gather | 6 | 0.971 | 0.536 - 1.757 | 3 / 0 / 3 |
| same, without `comet_int4_gemm` | 5 | 1.134 | 0.636 - 2.021 | 3 / 0 / 2 |
| `numba` + `numba_hand` + field_gather v0.2.1 | 8 | 0.982 | 0.617 - 1.564 | 4 / 0 / 4 |

None of these means differs from 1 at 95%.

## Files

- `sources/`: the seven v0.2.1-emitted modules (`<k>_numba.v021.py`) and the 26a4f0cf bench-info each
  was emitted from.
- `tools/emit_v021.sh` (emit, macOS), `tools/seed_cache.py` (seed a 26a4f0cf tree),
  `tools/check_emit.py` (Numba against NumPy at one preset, outside the harness).
- `run_numba021.sh`, `numba021.sbatch`: the job. `srun_lines.txt`: every srun line.
- `parts/`, `logs/`, `emitted_sources/`, `opt_reports/`: what the sweep and the opt-report step wrote.
- `analyze_numba021.py` produces `cells.csv` and `columns.csv` from `parts/` and the matrix's
  `results.csv` and `cells.csv`.

Rerun on daint, from `extractedMatrix/`, after creating `HPCAgent-Bench-ext3` (a 26a4f0cf worktree of
`HPCAgent-Bench-v2`) and running `tools/seed_cache.py` for both kernels:

    sbatch --job-name=ext-numba021 --nodes=2 --time=01:00:00 \
      --export=ALL,N021_UNITS=warpx_field_gather=numba+c:warpx_esirkepov_deposition=numba,EXT_STEP_MIN=50 \
      numba021/numba021.sbatch
