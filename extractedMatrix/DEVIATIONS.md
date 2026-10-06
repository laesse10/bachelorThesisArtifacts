# extractedMatrix: deviations and interpretations

Every way this work departs from the task text or from the LLR-40 v2 protocol, with the reason.

1. **Part 0 job count corrected.** The first push of `PART0_agent_language_status.md` (9d43e07)
   said "35 jobs" since 2026-10-01. The exact count from `sacct -u $USER -S 2026-10-01 -X` is 33,
   all of them v2-matrix, follow-up or perfRuns jobs. Corrected in the predictions commit. The
   classification is unchanged.
2. **`warpx_boris_push` native adapter passes the pinned `dt`.** The task allows adapters that only
   reorder or cast arguments. The original `warpx_boris_push_original` takes `dt`, which the harness
   ABI does not carry, because `dt` is a pinned manifest value (`config: dt: {value: 1.0e-13}`) that
   the translated columns compile in (`constexpr double dt = 1e-13;`). The adapter passes the literal
   `1.0e-13`. That binds a manifest argument and involves no arithmetic, copy or loop, so it is
   treated as argument mapping. If this reading is rejected, the cell is `unsupported`.
3. **`warpx_esirkepov_deposition` and `warpx_field_gather` native cells are `unsupported`.** Their
   originals need the grid extents (`n1, n2, ncomp, m1, m2` / `n0, n1, n2, ncomp`), which the harness
   ABI does not pass. An adapter would have to compute them from `ncells`, `depos_order` and
   `n_rz_azimuthal_modes`, which is arithmetic and not allowed.
4. **`comet_int4_gemm` `c` and `cpp` are `build_error`: the translated code needs CBLAS, which
   bare-metal daint lacks.** The translator lowers the int32 matmuls to `cblas_dgemm`
   (`#include <cblas.h>`). The harness finds BLAS through `pkg-config openblas`
   (`envs/libraries.yaml`), which resolves inside the harness's container image. On daint, outside
   the container, there is no `openblas.pc` and no `cblas.h`; only the runtime library
   `/usr/lib64/libopenblas.so.0` exists. The cells are recorded as they failed. No header path was
   added, because that would change the matrix's environment. The prediction said `ok`; it missed
   the BLAS lowering.
5. **`comet_int4_gemm` `native` is `build_error` for the same reason, not because of its adapter.**
   The harness's `cpp` build compiles EVERY precision variant in `cpp_backend/` into
   `libcomet_int4_gemm_cpp.so`. The adapter replaces only `comet_int4_gemm_fp64.cpp`, and the
   translator's `comet_int4_gemm_fp32.cpp` beside it includes `cblas.h`
   (`logs/cells/comet_int4_gemm.native.log`). The adapter itself compiles: see its opt report.
   Removing or replacing the fp32 file would change a harness input, so it was not done.
6. **`warpx_field_gather` `numba` reclassified from `incorrect` to `build_error`.** The run never
   produced output. Numba's typing failed at the first call (`NumbaTypeError: cannot index
   array(float64, 2d, C) with 3 indices`; it types every branch of the generated code, including
   config branches that never run). The harness reported that as `validated=False`, which v2's
   rule maps to `incorrect` because it only recognises C/Fortran compiler errors as build failures.
   `merge_results.py` applies the correction from the saved log, and `status_runner` keeps the
   original value. `sweep_extracted.py` now classifies it this way itself (fixed after the sweep).
7. **Harness results-DB shard per unit.** Every unit sets `HPCAGENT_BENCH_DB_SHARD` (100 + kernel
   index), the harness's own knob, so concurrent nodes never write one SQLite file. Without it,
   SLURM_PROCID is 0 in every one-task step, and LLR-40 follow-up 2b lost five cells to
   "database is locked". The measurement path is unchanged.
8. **Partition `normal`**, one kernel per exclusive node (job 4990441, 8 nodes), with v2's step
   geometry and geometry probe. That is not v2's chained debug jobs. Every srun line is in
   `srun_lines.txt`.
