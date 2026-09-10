# Libraries: link the kernel, mine the framework

Two different uses, and conflating them is how submissions score zero.

## Contents

- Link these
- What is deliberately absent from the GPU columns
- Mine these
- Linking mechanics that bite

## Link these

A library routine that computes the same mathematics is an implementation, not a substitution, and
is legal wherever the conventions match.

| need | CPU | CUDA | HIP |
|---|---|---|---|
| dense trailing updates, tall-skinny GEMM | OpenBLAS, BLIS | cuBLAS | rocBLAS, hipBLAS |
| QR, LU, Cholesky | LAPACK / LAPACKE | cuSOLVER | rocSOLVER, hipSOLVER |
| mixed-precision LU with refinement | LAPACK `dsgesv` | MAGMA `magma_dsgesv_gpu` | MAGMA |
| SpMV, sparse triangular solve, ILU0/IC0 | -- | cuSPARSE | rocSPARSE, hipSPARSE |
| supernodal sparse Cholesky | SuiteSparse CHOLMOD | -- | -- |
| sparse direct LU | SuperLU, MUMPS, STRUMPACK | cuDSS | -- |
| fast Poisson, DST, FFT preconditioner | FFTW | cuFFT | rocFFT, hipFFT |
| eigen-estimates for Chebyshev bounds | ARPACK, SLEPc | -- | -- |
| fill-reducing ordering | METIS, Scotch | -- | -- |
| threading | OpenMP, oneTBB | -- | -- |

Verify the routine's conventions before trusting it. Sign conventions, economy versus full factors,
and what is left in the input array differ between a textbook algorithm and a library one even when
the mathematics is identical.

## What is deliberately absent from the GPU columns

A vendor sparse triangular solve is level-scheduled, so it inherits whatever the level structure
gives it. When that structure is deep and imbalanced, the synchronization-free algorithms are the
ones designed for it and the vendor routine is the baseline they are measured against. Write the
sync-free solve, and measure it against the vendor call on your own hardware rather than assuming
either way.

<details>
<summary>Legacy vendor preconditioner routines</summary>

cuSPARSE's `csrilu02` and `csric02` belong to its legacy API, which NVIDIA does not improve and
has flagged for removal. `cuSolverSp` is deprecated in favour of cuDSS. Prefer the generic API and
cuDSS for new work.

</details>

## Mine these

PETSc, hypre, Trilinos, SUNDIALS, Ginkgo, Kokkos Kernels, PyAMG and AmgX are where the algorithms
live and where you validate your numbers against an independent implementation. They are the wrong
thing to *submit* when the kernel is graded elementwise against one specific route: a call into an
algebraic multigrid package returns a different, equally valid hierarchy and matches nothing.

- **PETSc** -- `KSPCG` and `KSPGMRES` with `PCSOR`, `PCILU`, `PCGAMG`, `PCHYPRE`. `-ksp_view` prints
  the exact preconditioner configuration and `-log_view` the per-event cost. The reference
  implementation of the `T_setup + N_iter * T_apply` cost model.
- **hypre** -- BoomerAMG's coarsening, interpolation and GPU defaults.
- **Ginkgo** -- ParILU, ParILUT, ISAI, adaptive-precision block-Jacobi, unsmoothed-aggregation AMG.
  The reference implementation of most of the ladder.
- **Kokkos Kernels and Ifpack2** -- production two-stage and multicolour Gauss-Seidel, and how the
  inner sweep count is exposed as a parameter.
- **SUNDIALS** -- variable-order BDF with a real order and step controller.
- **PyAMG** -- smoothed aggregation in Python, the fastest way to sanity-check a hierarchy's shape.
**A note on optimized benchmark implementations.** Where a well-known benchmark has a
heavily-tuned vendor implementation, it is worth reading for its operator application, its fused
vector updates and its data layout. Do not copy its preconditioner. Such implementations reach their
numbers by reordering the smoother, typically by multicolouring, and they are permitted to because
they are scored against their own convergence criterion rather than against a reference iterate.
That is the substitution measured at nine orders outside the band when the grading is elementwise.
The general rule: a tuned implementation is only a safe model where its correctness contract matches
yours.

## Linking mechanics that bite

- **Header-only libraries are often discoverable but not requestable**, because there are no link
  tokens for a resolver to return. Eigen, CUTLASS, CuTe, cub, hipcub, Boost and xsimd fall here.
- **A search-path-only link can silently bind a different build** of the same library at load time,
  with no error. That is worse than a load failure, because the number you report is then a timing
  of an implementation nobody chose. Let the harness resolve the library by name so it can attach
  its own rpath, and check with `ldd` which copy was bound.
- **List every link dependency in order**, dependents before dependencies, including the OpenMP and
  pthread flags if you use them.
