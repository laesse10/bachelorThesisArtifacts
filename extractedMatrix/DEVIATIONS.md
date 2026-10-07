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
9. **`comet_int4_gemm` `c`, `cpp` and `native` re-measured with OpenBLAS from a uenv (job 4996043,
   2026-10-07, nid005778).** Items 4 and 5 recorded them as `build_error`: bare-metal daint has no
   `cblas.h` and no `openblas.pc`. The CSCS uenv `prgenv-gnu/25.6:v2` ships OpenBLAS 0.3.29 (spack,
   `threads=openmp`, DYNAMIC_ARCH) with both. It was mounted WITHOUT a view (`sbatch --uenv=prgenv-gnu/25.6:v2`),
   and only its OpenBLAS package was put on `PKG_CONFIG_PATH`
   (`/user-environment/linux-neoverse_v2/openblas-0.3.29-eekzy6wkqav6gy67vccd7iobftsnozea/lib/pkgconfig`).
   The harness resolves BLAS through `pkg-config openblas` and adds `-I`, `-L`, `-lopenblas` and an rpath
   (`languages.library_tokens`). Nothing else changed: the compilers are still `/usr/bin/gcc-14`,
   `~/bin/g++` 13.3.1 and `~/bin/gfortran` 13.3.1 (logged by the unit), FFTW still does not resolve, same
   benchmark commit 26a4f0cf, same scripts (`extracted.sbatch`, `run_extracted.sh`, `sweep_extracted.py`
   unchanged), preset M, validated, 5 warm-up + 30 timed, one exclusive node, one core. A debug job
   (4995938) checked first that the uenv mount and `PKG_CONFIG_PATH` reach the srun steps and that
   OpenBLAS runs one thread there (`openblas_get_num_threads() = 1`, affinity 1 core, `OMP_NUM_THREADS=1`).
   Only the three failed cells were re-run: the job-4990441 `fortran` and `numba` rows were kept in
   `parts/comet_int4_gemm.csv`, so the sweep's resume skipped them. The superseded rows and logs are in
   `parts/superseded_4990441/` and `logs/cells/superseded_4990441/`. The emitted sources are
   byte-identical to the ones from job 4990441 (sha256 in `results.csv`). The opt-report index
   `parts/optrep.comet_int4_gemm.csv` holds both runs, as `opt_reports_extracted.py` appends;
   `analyze_extracted.py` keeps the last row per cell. All three cells validate (bit-exact) and run in
   0.068-0.069 ms. Their BLAS is a library the other columns do not use, so `columns.csv` also reports
   each mean without `comet_int4_gemm`.
10. **Correction to item 6: the `warpx_field_gather` `numba` failure is on the graded path.** Item 6 and
   the first summary said the typing error lies in a configuration branch that never runs. It does
   not: the failing statement is in `_tap3` (line 493 of the emitted file), which the `GEOM_3D` branch
   calls, and the graded configuration is `geom=3` = `GEOM_3D`. The translator lowers the 4-D
   broadcast `sf_x[:, None, None, :] * sf_y[None, :, None, :] * sf_z[None, None, :, :]` to
   `sf_x[i] * sf_y[:, None] * sf_z`, which drops axes, and Numba rejects the assignment into a 3-D
   slice ("cannot index array(float64, 2d, C) with 3 indices"). The status is unchanged.
11. **Precision probe (not a timing): `precision_probe.py`, jobs 4996849 and 4996871 (debug partition).**
   It runs each float cell once through the harness (`cli._run_cell`, preset M, validate on) and
   records, per output, how many elements differ from NumPy and by how much. Job 4996849 forgot
   `PYTHONPATH`, so the harness's emit step failed for the compiled columns; its `numba` rows are valid
   and kept (`logs/ext-precision.4996849.no-pythonpath.out`). Job 4996871 has the compiled
   columns (all `ok`). Its output was first read only in part, because the SSH certificate expired;
   the full `precision_probe.csv` and both job logs (`logs/ext-precision.4996871.out`,
   `logs/ext-precision.4996849.no-pythonpath.out`) were copied afterwards and agree with the part read
   earlier. In the CSV the hand-written Numba cells of `quatrex_rgf` and `warpx_esirkepov_deposition`
   are labelled `numba` (the harness framework name); `warpx_field_gather` `numba` has no data (build
   error). The harness compares every output twice; the two comparisons agree.
12. **Integer-only kernels run the fp32 symbol; the hand-written CoMet cell timed translated code
   (found in `followup/`, 2026-10-07).** The harness's native wrapper (`cpp_runtime.wrap_kernel.call`)
   calls `<k>_fp64` only if some argument is a float64 or complex128 array, else `<k>_fp32`, whatever
   `--precision` says. `comet_int4_gemm`, `spgemm_hash`, `triangle_count` and `nfa_frontier` have integer
   arguments only, so the c, cpp and fortran cells timed `<k>_fp32`, while `opt_reports/` and `summary.md`
   analyse the fp64 functions. The fp32 sources equal the fp64 ones for `triangle_count` and `nfa_frontier`;
   `spgemm_hash`'s differ in `_select_bin(float)` only, with the same nine integer divisions
   (`followup/opt_reports/fp32/`). `comet_int4_gemm`'s fp32 C/C++ call **`cblas_sgemm`** on float32 copies
   (not `cblas_dgemm`, as item 4 and `summary.md` say), and its fp32 Fortran loops read the transposed copy
   with a stride of 32 float32 values; they vectorise the same way, from single-lane loads. The timings
   stand; the descriptions are corrected in the thesis. The **`native` cell of `comet_int4_gemm` is wrong**:
   the adapter replaces only `comet_int4_gemm_fp64.cpp`, so the harness called the translator's
   `comet_int4_gemm_fp32` in the same library (the profile of `followup` unit `cometN`, cell `native`:
   `comet_int4_gemm_fp32` + OpenBLAS `sgemm_small_kernel`). Its recorded 0.068 ms ("ties with C") is the
   translated C++ code. With the adapter over both files (`native32`), the hand-written code runs
   (profile: `tc_int4_gemm_impl`) at 0.565x of C on the same node (0.149 against 0.085 ms; job 4998244).
   The `native` row in `results.csv` is left as recorded; use `followup/` for this cell. `warpx_boris_push`'s
   `native` cell is unaffected (float64 arguments). No LLR-40 kernel is affected: each has a float64 argument.

