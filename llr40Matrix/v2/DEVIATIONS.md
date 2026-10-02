# v2: deviations, corrections and incidents

Each entry lists what happened, why, and where the evidence is. The protocol (`../protocol.md`)
is unchanged. Wherever v2 departs from the task text, from the protocol's letter, or from v1,
it is listed here.

## Departures from the task text

1. **Partition and job length.** The matrix was measured as chained 30-minute jobs on the `debug`
   partition, at the user's request during the run, not as `sweep_array.sbatch`'s 3 h array on
   `normal`. Each unit is `srun --nodes=1 --ntasks=1 -w <node> --exclusive --cpu-bind=cores
   --hint=nomultithread`, one kernel per node, never two. Cells are checkpointed one by one, and
   every row records its job in `slurm_job`. Every srun line is in `srun_lines.txt`. Consequence:
   the protocol's 1800 s `timeout` status cannot occur within a 30-minute job. No cell came close
   (the longest kernel unit took under 5 minutes).
2. **Python 3.12.13 instead of 3.11.13.** `26a4f0cf` does not import on 3.11 (`seal.py` uses
   `os.CLONE_NEWUSER`). numpy 2.4.6, numba 0.67.0 and llvmlite 0.49.0 are unchanged. GPU-only
   wheels (torch, cuda, jax, triton, xgboost) were not installed (`requirements-frozen.txt`).
3. **Agent fallbacks: 13 kernels, not 12.** v1 also did not take the top pick for
   `tsvc_2_s3111` (rank 2 of 19), which the task text's list omits.
4. **Task 4 has 10 kernels, not 11.** `tsvc_2_s3110` is itself one of the 10 reduction kernels.
5. **Preset F needs a manifest edit.** The harness accepts only S/M/L/XL (`spec.Preset`), so the
   F runs use a separate worktree whose manifest has the S size replaced by the finite size
   (`variants/presetF.patch`), run as `--preset S` and recorded as `preset=F`. Inputs are drawn
   exactly as `finite_search.py` drew them.
6. **Flag variants are one-line patches in separate worktrees**: `-ffp-contract=off`
   (`variants/fpcontract_off.patch`) and the A/B "without" arm, which empties
   `DO_CONCURRENT_GFORTRAN` (`variants/no_parallelize_loops.patch`).

## Corrections to the v1 record

7. **C was compiled by GCC 14.2.0, in v1 and in v2.** The harness enforces a per-driver floor
   (`languages.COMPILER_MIN_MAJOR`: gcc >= 14, because the C block pins `-std=c23`, which 13.3.1
   rejects), so the C build skips `gcc` 13.3.1 and uses `/usr/bin/gcc-14`. The `.comment` section
   of the v1 libraries says 14.2.0, while v1's `results.csv`, `env_record.txt` and protocol §2 say
   13.3.1. C++ and Fortran are 13.3.1. Consequence: the `c`/`c_reference` vs `cpp` columns differ
   in compiler version as well as language (see `opt_findings.md`, `tsvc_2_s316`).
8. **v1 recorded compile failures as `incorrect`.** The driver reports `validated=False` when the
   library never built. v2 records these as `build_error`, which only affects agent attempts:
   every `incorrect` in the matrix is a genuine mismatch.
9. **`-ftree-parallelize-loops` was 1 in v1, as recorded.** v1's 1-CPU step cgroup made the
   harness resolve n=1 (parloops off). The `GOMP_parallel` calls in v1's Fortran libraries are
   OpenMP from agent sources (`*._omp_fn.N`), not parloops (`*._loopfn.N`). v2 keeps v1's geometry,
   and each chain job checks it on every node before measuring (`chain_claims/*/probe.summary`).

## Harness changes between the two commits that matter here

10. **The NumPy oracle is njit-compiled at `26a4f0cf`** (`frameworks/test.py:njit_reference`, no
    `parallel`, no `fastmath`). For `tsvc_2_s319` at preset M, v1's interpreted oracle took
    22.8 s per call. That is why v2 runs about 10x faster in wall time. The timed samples agree
    (74.48 vs 73.72 ms median, same node) and the status counts are identical.
11. **`cli run --output` is resolved against the driver's own cwd** (the bench checkout); v1 was
    not. sweep.py now passes absolute paths.

## Changes to sweep.py (v2/sweep.py vs ../sweep.py)

12. Every attempted agent candidate is logged (`agent_attempts.csv`, full driver output in
    `agent_attempt_logs/`). Compile failures become `build_error`. The compiler version is that of
    the driver the harness resolved. Flags are resolved under the build child's `numactl` binding.
    Substituted or restored sources are touched (mtime). Emitted sources are snapshotted, path
    arguments are made absolute, cells are checkpointed and resumed, and a leftover non-autogen
    source is removed and never timed.

## Discarded and supplementary runs

13. `discarded/4969228/`: relative `--scratch` (item 11), so every cell was recorded `build_error`.
    Invalid.
14. `discarded/4969249/`: cancelled because I wrongly believed v1 had parloops active. Its 25
    rows were v1-faithful; they are superseded by a complete run in the same geometry, not
    merged. Its README carries the correction.
15. `supplementary_wholenode/` (job 4969394): a complete run with whole-node steps (n=72,
    parloops on). It is not the matrix, and it is kept as the only measurement with the autopar
    flag active. Probably three cells are invalid there (item 16).
16. `discarded/contaminated_pristine_4969572/`: in job 4969572, `tsvc_2_s115` (.f90),
    `tsvc_2_s311` (.f90) and `wf_triangular` (.c) timed an agent source left behind by a cancelled
    job. All 6 cells of these 3 kernels were re-measured in chain job 4970062 (`slurm_job`), with
    the same geometry and code. The re-measured values are back in line with v1 (e.g. `s311`
    fortran 135.4 ms vs v1 136.7; `wf_triangular` c 99.5 ms vs v1 98.3). Afterwards, all 160 timed
    snapshots in `emitted_sources/` were checked byte-identical to an independent regeneration at
    `26a4f0cf`.

## Not done or not changed

17. `labels.csv` is unchanged. The derived-label disputes (`derived_labels_check.md`) are
    reported, not applied, because v1 and v2 share the file and the analysis keys on it.
18. Opt reports come from a compile-only rebuild with the harness's own compile argv
    (`languages.build_kernel_lib_commands`) plus the three `-fopt-info` flags. They were built on
    a compute node in the same step geometry, not captured from the timed builds. numba assembly
    is `inspect_asm()` for the signature built from preset-S inputs, which matches preset M:
    float64 C-contiguous arrays and int64 sizes.
19. **Opt-report incidents.** (a) The first OPTREP run also reported the four `-ffp-contract=off`
    rows of `s115` (a filter bug, fixed): duplicate index rows were dropped, and their files were
    byte-identical rewrites. (b) The second OPTREP run (job 4970131) hit the numba cache written by
    the first ("No module named '<dynamic>'"), so all 40 numba rows were regenerated with a fresh
    `NUMBA_CACHE_DIR` (`opt_reports_numba.sbatch`, `opt_reports.py --only numba`). `cache=True`
    does not affect code generation.
20. **Chain resubmit rejected.** The debug QoS allows 2 submitted jobs per user, shared with this
    user's other sessions. One resubmit was refused (`QOSMaxSubmitJobPerUserLimit`) and submitted
    by hand. The chain now retries and leaves `chain_claims/RESUBMIT_FAILED`.
21. **Job IDs of the final data:** main matrix, preset F, `-ffp-contract=off` and A/B in 4969572
    (except the 3 re-measured kernels: 4970062). Opt reports in 4970131; numba opt reports in 4970181
    (numba-only). Every srun line is in `srun_lines.txt`.

