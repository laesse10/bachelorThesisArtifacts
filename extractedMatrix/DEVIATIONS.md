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
