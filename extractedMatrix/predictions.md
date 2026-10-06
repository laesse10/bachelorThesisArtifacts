# Extracted-kernel matrix: predictions, written before measuring

Written and pushed on 2026-10-06, **before the first build, validation or timed run** of this matrix.
They rest only on reading the manifests, the NumPy references, the hand-written sources and the
translator output that the harness emits at HPCAgent-Bench `26a4f0cf` (`autogen.ensure_native` +
`ensure(numba_np)`, run on the login node; this is the generation step every sweep runs anyway).
Nothing was compiled or executed.

## Setup the predictions assume

Same protocol as the LLR-40 v2 matrix (`llr40Matrix/protocol.md`): preset M, the harness path
`hpcagent_bench.cli run --validate`, 5 warm-up + 30 timed runs, min-of-k, one exclusive node per
kernel with one core, `OMP_NUM_THREADS=1`, `NUMBA_NUM_THREADS=1`, `OMP_PROC_BIND=close`.
Compilers as in v2: C gcc 14.2.0, C++ g++ 13.3.1, Fortran gfortran 13.3.1, Numba 0.67.

| kernel | folder | preset M | declared dtypes | graded configuration (first value of every config domain) |
|---|---|---|---|---|
| `comet_int4_gemm` | dense_linear_algebra | num_vector 32, num_field 256 | int4 codes (stored as int8), int32 out | seed 0 |
| `quatrex_rgf` | sparse_linear_algebra | BS 16, NB 8, NE 8 | complex128 | none declared |
| `spgemm_hash` | sparse_linear_algebra | M=K=N 16384, nnz_A 131072, nnz_B 196608 | int64 CSR | none declared |
| `warpx_boris_push` | n_body_methods/boris_push | np_particles 262144 | float64 | momentum_push_type 0 (Full), dt 1e-13 (pinned) |
| `warpx_esirkepov_deposition` | n_body_methods/esirkepov_deposition | np_particles 8192, ncells 24, depos_order 3 | float64, int32 masks/indices | geom 3, n_rz_azimuthal_modes 1, do_ionization 0, enable_reduced_shape 0 |
| `warpx_field_gather` | n_body_methods/field_gather | np_particles 8192, ncells 24, depos_order 3 | float64, int32 types | geom 3, galerkin_interpolation 1, n_rz_azimuthal_modes 1 (all pinned) |
| `triangle_count` | graph_traversal | NV 8192, NE 196608 | int64 | none declared |
| `nfa_frontier` | finite_state_machine | C 1281, NS 42273, NE 70455, T 1048576 | int64, uint8 | none declared |

Validation: integer outputs are compared exactly by the harness (`compare_arrays`: "Integer and bool
outputs compare EXACTLY"). Float64 outputs use the harness band, rtol 1e-9 / atol 1e-11, because no
manifest overrides it.

## Provenance of every column (checked: first line of the source the harness uses)

| column | source | provenance |
|---|---|---|
| `c` | `cpp_backend/<k>_fp64.c` | autogen (numpyto_c) for all 8 |
| `cpp` | `cpp_backend/<k>_fp64.cpp` | autogen (numpyto_c) for all 8 |
| `fortran` | `cpp_backend/<k>_fp64.f90` | autogen (numpyto_fortran) for all 8 |
| `numba` | `<k>_numba_np.py` | autogen for 6. **Hand-written for `quatrex_rgf` and `warpx_esirkepov_deposition`** ("Hand-written parallel numba reference", tracked in git, no marker), so those two cells are labelled **`numba_hand`** |
| `native` | upstream extraction via a thin adapter | hand. `comet_int4_gemm` uses `tests/ports/comet_int4_gemm/comet_int4_gemm_ref.cpp`. The kernel folder's `comet_int4_gemm_reference.cpp` was confirmed to be translator output: its header says "emitted by HPCAgent-Bench's NumpyToX C++ translator", and it carries the autogen marker. `warpx_boris_push` uses `warpx_boris_push_reference.cpp` |

## Per cell

| kernel | column | expected status | why | expected time per call at M |
|---|---|---|---|---|
| `comet_int4_gemm` | `c` | ok | Integer arithmetic only: the translator lowers the int32 matmuls to loops, and the result is exact | well below 10 ms: about 1M multiply-adds |
| | `cpp` | ok | same source as `c` | same |
| | `fortran` | ok | integer loops, exact | same |
| | `numba` | ok | The emitter rewrote `li0 @ rj0.T` on int32 into explicit loops (numba's `@` handles only float/complex), so it should type-check | same, plus call overhead |
| | `native` | ok | Thin adapter (`adapters/comet_int4_gemm_native.cpp`): `num_left = num_right = num_vector`, int casts. The port computes the same tallies, so the output should be bit-exact | same |
| `quatrex_rgf` | `c` | ok, with a real risk of `incorrect` | complex128 is translated (127 complex constructs in the C), and `np.linalg.inv` becomes the translator's own inverse. Against LAPACK the rounding differs. For well-conditioned 16x16 blocks that should stay below rtol 1e-9, but this is the cell most likely to fail validation | around 10 ms (64 inverses + about 30 block products per energy) |
| | `cpp` | as `c` | same lowering | same |
| | `fortran` | as `c` | complex(8) is native in Fortran | same |
| | `numba_hand` | ok | Same association as the reference: BLAS `@` and LAPACK `inv`, with a parallel loop over the energies (serial at 1 thread) | similar or faster (LAPACK inverse) |
| | `native` | unsupported | the upstream extraction is Python (`quatrex_rgf_reference.py`), not a C ABI function | -- |
| `spgemm_hash` | `c` | ok | integer hash accumulation and a serial bitonic sort, exact | about 10-50 ms |
| | `cpp` | ok | same | same |
| | `fortran` | ok | same | same |
| | `numba` | ok | autogen, integer loops, one parallel loop | slower than GCC (bounds and wraparound handling) |
| | `native` | unsupported | the upstream source is CUDA (`spgemm_hash_reference.cu`) | -- |
| `warpx_boris_push` | `c` | ok | elementwise float64 with FMA contraction (`-ffp-contract=fast`). The differences from the strict oracle should be about 1e-16 relative, well inside the band | about 1-5 ms (262144 particles, memory-bound streams), so below 10 ms |
| | `cpp` | ok | same | same |
| | `fortran` | ok | same. The translator's parentheses keep gfortran from contracting to FMA, which can make it slightly slower but not wrong (LLR-40 follow-up 2b) | same |
| | `numba` | ok | autogen | same order |
| | `native` | ok | Thin adapter (`adapters/warpx_boris_push_native.cpp`), with `dt` = the manifest's pinned 1e-13 (see the note below). The transcription is the source the NumPy port was derived from, so it should agree within the band | same |
| `warpx_esirkepov_deposition` | `c` | ok | float64 scatter-adds in particle order, the same order as the reference | about 5-20 ms |
| | `cpp` | ok | same | same |
| | `fortran` | ok | same | same |
| | `numba_hand` | ok | Privatized grids per particle chunk, summed afterwards: a different summation order, but within rtol 1e-9 for these magnitudes | slower than GCC (an extra pass over the private grids) |
| | `native` | unsupported | The original takes the grid extents `n1, n2, ncomp, m1, m2`, which the harness ABI does not pass. An adapter would have to compute them (`ncells + 2*depos_order + 6`, `2*n_rz_azimuthal_modes - 1`), which is arithmetic | -- |
| `warpx_field_gather` | `c` | ok | float64 gathers, no reduction-order sensitivity | about 1-5 ms, so below 10 ms |
| | `cpp` | ok | same | same |
| | `fortran` | ok | same | same |
| | `numba` | ok | autogen | same order |
| | `native` | unsupported | The original takes `n0, n1, n2, ncomp`, derived from `ncells`/`depos_order` (arithmetic), and also `geom`, `galerkin_interpolation`, `n_rz_azimuthal_modes`, which the ABI does not pass | -- |
| `triangle_count` | `c` | ok | integer binary searches, exact | about 10-50 ms |
| | `cpp` | ok | same | same |
| | `fortran` | ok | same | same |
| | `numba` | ok | autogen, integer loops | slower than GCC |
| | `native` | unsupported | the upstream source is CUDA (`triangle_count_reference.cu`) | -- |
| `nfa_frontier` | `c` | ok | integer frontier simulation, exact | about 1-5 s (1M symbols x a frontier of hundreds to thousands of states) |
| | `cpp` | ok | same | same |
| | `fortran` | ok | same | same |
| | `numba` | ok | autogen | slower than GCC |
| | `native` | unsupported | `nfa_frontier_reference.cpp` is an excerpt of VASim's `Automata` class methods, not a function with the harness ABI | -- |

Expected totals: 34 `ok` (one of them, `quatrex_rgf` in the three GCC columns, at risk of
`incorrect`), 6 `unsupported` (native: quatrex_rgf, spgemm_hash, triangle_count, nfa_frontier,
warpx_esirkepov_deposition, warpx_field_gather), 0 `build_error`, 0 `timeout`.

Several cells are expected below 10 ms per call at preset M (comet_int4_gemm, warpx_boris_push,
warpx_field_gather, possibly quatrex_rgf and esirkepov). As the task says, they are measured at M
anyway and reported as such.

**Note on the boris adapter.** The rule is that an adapter only reorders or casts arguments. The
original takes `dt`, and the harness ABI has none, because `dt` is a pinned manifest value
(`config: dt: {value: 1.0e-13}`) that every translated column compiles in. Passing that literal
binds a manifest argument and computes nothing, so I treat it as argument mapping. If that is
judged too loose, the `warpx_boris_push` `native` cell becomes `unsupported` (recorded in
DEVIATIONS.md).

## Patterns from LLR-40 v2: what I expect to carry over

- **Fortran parentheses (LLR-40 follow-up 2b).** The translator parenthesises every Fortran
  subexpression, which blocks FMA and FP reassociation. Expected to matter on the FP kernels with
  multiply-add chains (`warpx_boris_push`, `warpx_field_gather`, `warpx_esirkepov_deposition`,
  `quatrex_rgf`): Fortran a few percent slower than C there. No effect on the integer kernels
  (integer reassociation is not blocked).
- **GCC 14 (C) against GCC 13 (C++).** Expected identical code on most kernels. Differences, if any,
  on the branchy integer kernels (`triangle_count`, `nfa_frontier`, `spgemm_hash`), where GCC 14
  if-converts more branches into selects (the `s316` pattern), in either direction.
- **Branch against select.** Expected to show on the search and probe loops (`triangle_count`,
  `spgemm_hash`). A select on a loop-carried chain costs latency; a predictable branch does not.
- **Numba against GCC.** Expected slower than GCC on the irregular integer kernels (bounds and
  negative-index wraparound selects on every subscript, the `wf_triangular` pattern), and close on
  the simple streaming kernel `warpx_boris_push`. `numba_hand` on `quatrex_rgf` may beat the
  translated C thanks to LAPACK.
- **Vectorisation.** Expected only in the elementwise WarpX loops and in `comet_int4_gemm`'s
  integer inner loop. Not in the graph, automaton and hash kernels (data-dependent control flow
  and indirect addressing).
