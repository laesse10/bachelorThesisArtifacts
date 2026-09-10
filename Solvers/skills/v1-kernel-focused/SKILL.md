---
name: solver-preconditioners
version: 1
status: superseded by v2 (generalised)
description: "Solver kernels: which kernels let you change the preconditioner and which freeze it, what every candidate substitution costs measured against the grading band, the level/colour/two-stage/polynomial/AMG playbook with the state of the art behind each, and which installed library supplies which piece."
when: the kernel applies M^-1, smooths, factorizes incompletely, solves triangular, or wraps a Krylov iteration -- every kernel under the `solvers` subtrack
---

Preconditioning is the largest lever in a sparse solver and the one most likely to score zero
here. Almost every kernel in this corpus returns a FIXED ITERATE, not a converged answer, and a
fixed iterate pins the preconditioner as tightly as it pins the operator. This page tells you,
per kernel, whether the door is open; what the standard substitutions actually cost when it is
shut; and -- where it is open, or where you are writing a solver from scratch -- what the current
best practice is and which installed library implements it.

Everything marked **measured** below was run against the corpus generators and the graded
tolerance. Section 9 has the reproduction script.

---

## 1. The one question that decides everything

> **Does the kernel iterate to a tolerance, or does it run a fixed trip count?**

A solve-to-tolerance kernel returns the answer. Two different preconditioners reach the same
answer, so the preconditioner is yours to choose.

A fixed-trip-count kernel returns iterate number `k` of one particular recurrence. The
preconditioner is part of that recurrence. Change it and every subsequent iterate changes, by
orders of magnitude more than the graded band -- and the band here is not generous:

| precision | rtol | atol |
|---|---|---|
| fp64 | 1e-9 | 1e-11 |
| fp32 | 1e-3 | 1e-5 |

(`docs/numerical_validation.md`. A manifest cannot loosen these; `spec.py` rejects an attempt.)

### Verdict per kernel

| kernel | graded output | trip count | preconditioner |
|---|---|---|---|
| `sgs_pcg` | `x` | `niter` fixed (25/50) | **FROZEN** |
| `mg_vcycle` | `u` | `ncycles` fixed (4/10) | **FROZEN** (smoother, cycle, weights) |
| `sptrsv_level` | `x` | exact solve | **FROZEN** (it *is* the preconditioner apply) |
| `ilu0` | `A_data` | exact factorization | **FROZEN** (it *is* the setup) |
| `rb_sor` | `u` | `TSTEPS` fixed | **FROZEN** (colour order is the algorithm) |
| `amg_setup` | `level_n`, `level_nnz`, `nlevels` | to `MAX_COARSE` | **FROZEN** (graded on hierarchy shape) |
| `sparse_cholesky` | `y` | direct | **FROZEN** (ordering arrives as an input) |
| `lanczos_reorth` | `Q`, `alpha`, `beta` | `m` fixed | **FROZEN** (unpreconditioned by construction) |
| `householder_qr` | `A`, `Q`, `R`, `x` | direct | n/a |
| `mixed_precision_ir` | `x`, `steps_out` | tolerance, but `steps_out` is graded | **FROZEN** (see 5.6) |
| `bdf_newton_krylov` | `u`, `v`, `order_history`, `diagnostics` | adaptive, histories graded | **FROZEN** |
| `rk4_ensemble` | `y` | fixed | n/a |
| `rk45_ensemble` | `y`, `n_accept`, `n_reject` | adaptive, counts graded | n/a |
| `jfnk_bratu` | `u` | Newton to 1e-10, capped at 20 | **OPEN at small N ONLY -- read 5.5** |

Ten kernels freeze the preconditioner outright, three have none to change, and the one that is
open closes above N ~ 64. **There is no kernel in this set where swapping in a better
preconditioner is the way to score.** That is the single most useful fact on this page, and
section 2 is the evidence. What you optimize instead is the preconditioner you were given:
sections 4 and 5.7.

**A discrete output freezes a kernel even when the floating-point output would not.**
`steps_out`, `n_accept`, `n_reject`, `order_history` and `diagnostics` are counts. A count is
graded like any other array, so an off-by-one is a factor-of-two relative error and fails. Any
change that alters how many steps the solver takes is out, however much better the answer is.

---

## 2. Measured: what the standard substitutions cost

The `sgs_pcg` kernel, run exactly as the reference does -- `x0 = 0`, fixed `niter` -- with only
the preconditioner swapped. The number is the worst per-element violation of
`|a - e| <= atol + rtol*|e|`, as a multiple of the budget. **A variant passes only at <= 1.0.**

| substitution | 16³, niter=25 | 32³, niter=50 | verdict |
|---|---|---|---|
| level-scheduled SGS, same row ordering | 2.5e-5 | 4.3e-5 | **PASSES**, 4 orders of margin |
| Jacobi instead of SGS | 1.3e+9 | 1.3e+9 | fails by 9 orders |
| two-stage SGS, 3 inner Jacobi sweeps | 4.3e+8 | 4.4e+8 | fails by 8 orders |
| two-stage SGS, 10 inner Jacobi sweeps | 1.9e+6 | 2.3e+6 | fails by 6 orders |
| multicolour SGS, 8 colours | 8.7e+8 | 9.2e+8 | fails by 9 orders |

Note the third and fourth rows. Two-stage Gauss-Seidel with three inner sweeps reproduces the
reference's **iteration count** exactly (below) and is still eight orders of magnitude outside
the band. Matching a convergence rate is not matching an iterate. Do not use iteration count as
evidence that a substitution is safe.

The same swaps, judged the way the literature judges them -- CG iterations to relative residual
1e-8 -- for when you are choosing a preconditioner rather than reproducing one:

| preconditioner | iters @16³ | iters @32³ | speed-up vs none |
|---|---|---|---|
| none | 85 | 133 | 1.00x |
| Jacobi (diagonal) | 69 | 108 | 1.23x |
| **SGS (the reference)** | **29** | **50** | **2.7-2.9x** |
| two-stage SGS, 1 inner Jacobi | 69 | 108 | identical to Jacobi |
| two-stage SGS, 2 inner Jacobi | 39 | 59 | 2.2x |
| two-stage SGS, 3 inner Jacobi | 32 | 50 | **matches SGS** |
| two-stage SGS, 10 inner Jacobi | 29 | 50 | matches SGS |
| multicolour SGS, 8 colours | 35 | 56 | 2.4x (+12% iters) |
| block Jacobi, contiguous block 8 | 68 | 100 | 1.25-1.33x |
| block Jacobi, contiguous block 64 | 66 | 115 | worse than block 8 |
| Chebyshev in D^-1A, degree 2/4/8/16 | 68-69 | 107-108 | 1.23x -- **buys nothing** |
| IC(0) | 23 | 39 | 3.4-3.7x |

Four things to carry away.

- **One inner Jacobi sweep IS Jacobi.** From a zero start the first sweep gives `y = r/D` and the
  triangular part never enters. Two-stage GS below two sweeps is not an approximation of
  Gauss-Seidel, it is a rename of Jacobi.
- **Three inner sweeps is the knee**, matching Berger-Vergiat et al.'s reported 1-3. Ten buys
  nothing over three.
- **Contiguous block Jacobi is not block Jacobi.** Blocks cut out of a 3-D lexicographic ordering
  capture only the `z`-direction coupling; block 64 is worse than block 8 at 32³. Blocks must
  follow the graph, not the index range.
- **Polynomial preconditioning is inert on this operator**, and section 3 says why.

---

## 3. The operator you are actually preconditioning

`make_stencil_3d` -- used by `sgs_pcg`, `amg_setup` and `lanczos_reorth` -- builds
`A_ii = sum_j w_ij`, `A_ij = -w_ij`, weights log-uniform on [1, 100], Dirichlet by clipping.
Every row therefore sums to exactly zero.

**Measured: `||A·1||_inf / ||A||_inf = 4.2e-16` at 16³ and 4.1e-16 at 32³.** This is a weighted
graph Laplacian. It is positive SEMI-definite and SINGULAR, with the constant vector in its null
space, not the positive-definite M-matrix the generator's docstring claims. The right-hand side
is built as `A @ x_true`, so the system is consistent and CG converges to the minimum-norm
solution; nothing is broken. But the consequences for preconditioning are real:

| quantity | 16³ | 32³ |
|---|---|---|
| lambda_1 | ~1e-13 (numerically zero) | ~1e-13 |
| lambda_2 | 5.92 | 1.60 |
| lambda_max | 1161 | 1233 |
| effective condition lambda_max / lambda_2 | 196 | 773 |

- The effective condition number grows like `k²`, the Laplacian rate: 196 -> 773 for a doubled
  edge. Iteration counts grow like `k`, which is what the table in section 2 shows.
- **A polynomial preconditioner cannot help.** Chebyshev damps a bounded interval `[a, b]` that
  must exclude the origin; the eigenvalues that cost you the iterations sit at `lambda_max/773`
  and fall further as the grid refines. You do not know `lambda_2` and the interval that would
  capture it is the whole spectrum. Measured at degree 16: 1.24x, indistinguishable from plain
  Jacobi. Polynomial preconditioning is genuinely strong on well-conditioned nonsymmetric
  systems (Loe et al.) -- it is the wrong tool for a Laplacian.
- **The near-null space is the constant vector**, which is exactly what smoothed aggregation's
  piecewise-constant tentative prolongator assumes. This is why `amg_setup` works and why the
  hierarchy is graded on shape.
- **IC(0) survives but sits on the last pivot.** The factorization completed on this singular
  operator in the measurement above and gave the best iteration count of anything tried (3.4x),
  but the final Schur complement is a rounding error away from zero. If you build an IC(0) here
  for any purpose, assert the pivot, do not assume it.

The strength threshold for `amg_setup` is **`theta = 0.03`**, and that number is load-bearing.
The smoothed-aggregation measure is `|A_ij| > theta * sqrt(|A_ii| |A_jj|)`; on a 27-point
operator whose diagonal is a sum of ~26 weights a typical normalized coupling is ~1/26 = 0.038.
The Ruge-Stueben 0.25 everyone quotes normalizes by the row's largest off-diagonal instead, does
not transfer, and produces an empty strength graph and no coarsening at any level.

The other operators: `sparse_cholesky` and `mg_vcycle` use the constant-coefficient 7-point
Poisson operator (analytic spectrum, no coefficient spread, so diagonal preconditioning there is
a scalar rescale and buys exactly 1.00x); `sptrsv_level` and `ilu0` read four fixed SuiteSparse
matrices.

---

## 4. Level scheduling: the one exact parallelization

Level-set scheduling reorders only rows the analysis has PROVED independent. Every row computes
the identical value. The only difference from a sequential sweep is reassociation inside the row
reduction, which the band covers -- measured at 2.5e-5 of budget, four orders of margin.
Multicolouring, by contrast, changes which values a row reads, and that is different mathematics
(section 2, ninth row).

So: **level scheduling is always legal, colouring almost never is.** The question is whether
level scheduling is worth anything, and that is a property of the matrix ORDERING, to be measured
per matrix and never inferred from size or nonzero count.

**Measured, 27-point operator, natural row order** (this is `sgs_pcg`'s forward and backward
sweeps, and `ilu0`'s row loop):

| grid | N | levels | avg rows/level | max rows/level |
|---|---|---|---|---|
| 16³ | 4,096 | 106 | 39 | 64 |
| 32³ | 32,768 | 218 | 150 | 256 |
| 48³ | 110,592 | 330 | 335 | 576 |
| 64³ | 262,144 | 442 | 593 | 1,024 |

The pattern is exact, confirmed on k = 16, 24, 32, 40, 48, 56, 64: **levels = 7k - 6**, **max
rows/level = (k/2)²**, avg ≈ k²/7. Extrapolating to the XL preset, 192³: 1,338 levels, ~5,300
rows per level.

Read that as a hardware decision:

- **On CPU, take it.** 5,300 independent rows across a few dozen cores is ample, and 1,338
  barriers over 7M points is nothing. This is the single best legal transform for `sgs_pcg`.
- **On GPU, it will not fill the machine.** 5,300 rows against ~270,000 resident threads is 2%
  occupancy, and you pay 1,338 global synchronizations per sweep. Do not spend the day on a
  level-scheduled CUDA SGS -- put the effort into the SpMV, the dot products and the vector
  updates, which is where the time in a PCG iteration actually lives.

**Measured, the four `sptrsv_level` / `ilu0` matrices:**

| matrix | N | nnz(L) | levels | avg rows/level | max | levels under 32 rows |
|---|---|---|---|---|---|---|
| `Schmid/thermal1` (S) | 82,654 | 328,556 | 971 | 85 | 14,203 | 471 |
| `Um/offshore` (M) | 259,789 | 2,251,231 | 3,452 | 75 | 963 | 1,226 |
| `Schmid/thermal2` (L) | 1,228,045 | 4,904,179 | 1,239 | 991 | 207,967 | 114 |
| `Oberwolfach/boneS10` (XL) | 914,898 | 28,191,660 | 2,367 | 387 | 5,991 | 102 |

These are savagely imbalanced -- `thermal1` has a level holding 14,203 rows and 471 levels
holding fewer than 32. A per-level kernel launch spends its life on empty launches. This is the
canonical case for a **synchronization-free SpTRSV**: one thread or warp per row, spinning on a
per-row ready flag, no analysis phase and no global barriers. CapelliniSpTRSV reports 4.74x over
cuSPARSE's level-scheduled solve across 245 matrices; AG-SpTRSV reports 3.99x over cuSPARSE and
3.81x over the earlier sync-free algorithm on A100. Note that `sptrsv_level` hands you
`level_ptr` and `perm` as inputs -- you may ignore them and compute the same `x` another way, so
long as `x` matches.

---

## 5. Where the door is open, and what to put through it

### 5.1 The cost model

    T = T_setup + N_iter * T_apply

A stronger preconditioner charges twice, once to build and once per application, and only
`N_iter` is visible in an iteration count. Time to a fixed accuracy with setup counted is the
objective; iteration count is a diagnostic. The clearest failure this catches is a barely
coarsening AMG hierarchy: it converges in FEWER iterations than a correct one because its cycle
approaches a direct solve, while each cycle costs tens of fine-grid operator applications.
Operator complexity catches it, iteration count does not.

### 5.2 Choose by operator class

- **SPD.** CG. The preconditioner must stay symmetric and positive definite -- an unsymmetric `M`
  does not merely slow CG down, the recurrence loses its meaning and the iterates wander. This is
  why symmetric Gauss-Seidel is symmetric: forward, scale by D, backward.
- **Symmetric indefinite.** MINRES. CG does not apply.
- **Nonsymmetric.** Restarted GMRES or BiCGStab; the restart length is a real knob (longer
  restart, fewer outer iterations, more memory and more orthogonalization per step).
- **Singular but consistent** (section 3). CG is fine. Keep the null vector out of the
  preconditioner's own solve, and expect any threshold-based factorization to flirt with a zero
  pivot on the last row.
- **Block or saddle-point.** Block semantics are the whole problem; a purely algebraic view
  discards the only information that makes it tractable.

### 5.3 The preconditioner ladder, cheapest first

**Jacobi.** One divide. Worth 1.2x here and exactly 1.00x on a constant-coefficient operator,
where it is a scalar rescale. Free, so always the baseline.

**Block Jacobi.** Blocks must follow the graph or the physics, never the index range (section 2).
Its virtue on GPUs is that a batched, variable-size block inverse is a single kernel with no
dependences at all. Anzt et al.'s adaptive-precision block-Jacobi in Ginkgo picks fp16/fp32/fp64
per block from that block's own condition number, matching or beating uniform fp32 storage while
preserving the fp64 solver's convergence rate -- the preconditioner apply is bandwidth bound, so
narrower storage is directly faster.

**Symmetric Gauss-Seidel.** 2.7-2.9x here, and the reference for `sgs_pcg`. Sequential in row
order; open it with level scheduling (section 4) if the level structure supports it.

**Multicolour SGS.** Full parallelism inside a colour, at a measured +12% iterations for 8
colours on this operator. A distance-1 greedy colouring of the 27-point graph gives exactly 8
colours with N/8 points each -- the 2x2x2 blocking, and about as good as colouring gets. Costs a
colouring and a reordering in setup.

**Two-stage Gauss-Seidel (GS2).** Replace each exact triangular solve with a fixed number of
inner Jacobi-Richardson sweeps. Measured knee at 3 sweeps, matching SGS's iteration count exactly;
Berger-Vergiat et al. report 1-3 sweeps sufficient across their applications and often 1 in
production, with GS2 outperforming triangular-solve GS on Volta. Convergence of the inner
iteration requires the spectral radius of `D^-1 L` below 1; on hard problems damp the inner
sweep. This is the best-known way to get Gauss-Seidel-quality smoothing onto a GPU -- and on
`sgs_pcg` specifically it is illegal, because that kernel returns a fixed iterate.

**Iterative (Jacobi) triangular solve inside an ILU apply.** The same idea applied to an
incomplete factor rather than to A. Anzt, Chow and Dongarra report speed-ups over cuSPARSE's
level-scheduled triangular solve exceeding 10x when each problem uses its best sweep count, with
blocking improving robustness. Same convergence caveat.

**ISAI / sparse approximate inverse.** Approximate `L^-1` and `U^-1` by sparse matrices with a
prescribed pattern, so the apply becomes two SpMVs -- no dependences at all. Ginkgo ships batched
ISAI generation using only thread-local memory, and a mixed-precision FSPAI for SPD operators.
The right answer when the triangular solve is the bottleneck and the level structure is hostile.

**IC(0) / ILU(0).** 3.4-3.7x here, the strongest of the local methods. Zero fill means the factor
keeps the input pattern element for element -- that is what the "(0)" means and it is checked
structurally, not inferred from values. Setup is sequential in row order with the same level
structure as the solve. **ParILU / ParILUT** (Chow & Patel; Anzt, Chow & Dongarra) replace that
with a fine-grained fixed-point iteration, one thread per matrix element, converging toward the
sequential factor; ParILUT interleaves it with a threshold-based pattern update. A few sweeps
reach comparable quality. Note the word "toward": a finite number of ParILU sweeps does not
reproduce the sequential factor, so it cannot pass `ilu0`'s elementwise grade.

**Polynomial / Chebyshev.** No triangular solve, no storage, no global reductions -- pure SpMV
and axpy, which is why it is the smoother of choice in parallel AMG (Adams et al.). It needs a
bound on `lambda_max`; Lanczos is the reliable estimator, and it is important not to
UNDERestimate. It needs a lower bound too, and that is where it dies on a Laplacian (section 3).

**Multigrid.** The only method here whose iteration count does not grow with the problem size,
and the right answer for a Laplacian. Smoothed aggregation for jumping coefficients; classical
Ruge-Stueben for scalar elliptic problems on unstructured meshes. On GPUs: hypre defaults to
**PMIS** coarsening rather than the CPU default HMIS, and hypre's own documentation notes that
aggressive coarsening -- a reliable CPU win -- is NOT as effective on GPUs, because the long-range
interpolation it forces is expensive there. Ginkgo's AMG is unsmoothed aggregation (parallel graph
match) only and is competitive with AmgX per iteration, with its mixed-precision variant beating
its own fp64 variant.

**Fast transform preconditioners.** If the operator has a constant-coefficient elliptic part, a
DST/FFT solve of that part is an exact inverse in O(N log N) and an extremely strong
preconditioner. Section 5.5 measures 62x fewer Jacobian applications from exactly this. FFTW is
requestable.

**Deflation and recycling.** Project out the few smallest eigenvectors, or carry a subspace
across a sequence of related solves. The natural fit for a Newton or time-stepping loop where
consecutive systems are close.

### 5.4 Mixed precision is a preconditioner decision

Store or apply the preconditioner in lower precision while the Krylov iteration stays fp64. The
preconditioner is an approximation already, so its own error budget is loose; its apply is
bandwidth bound, so narrower storage converts directly into speed. This is the highest-yield
low-risk idea in the current literature, and section 5.6 is the one place in this corpus where
it is also the specified algorithm.

The bound to remember, from Carson & Higham's three-precision iterative refinement: with an fp16
factorization, plain LU-based refinement is only guaranteed to reduce the error for
`cond(A) << 2e3`, while GMRES-IR -- solving the correction equation by GMRES preconditioned with
those same LU factors -- extends the guarantee to `cond(A) << 3e7`. If a low-precision
factorization is not converging, that is the fix, not more refinement steps.

### 5.5 `jfnk_bratu` -- open, then shut, and you must know where

This is the only kernel whose preconditioner is genuinely yours, and it is a trap.

**Measured, N=32 (the S preset), lambda=6:** the reference runs unpreconditioned matrix-free
GMRES and needs 204 Arnoldi steps across 5 Newton steps. Right-preconditioning the inner GMRES
with a DST fast-Poisson solve of the Laplacian part needs **16** -- 12.8x fewer matrix-free
Jacobian applications -- and the answer lands at 1.3e-3 of the graded budget. It passes, with
nearly three orders of margin, because Newton runs to `||F|| <= 1e-10 ||F0||` and both routes
reach the same root.

**Then it stops.** `max_newton` is 20, and the unpreconditioned reference stops converging:

| N | Newton steps used | final ||F||/||F0|| | |
|---|---|---|---|
| 32 | 5 / 20 | 1.9e-12 | converged |
| 48 | 6 / 20 | 7.7e-12 | converged |
| 64 | 14 / 20 | 7.1e-11 | converged |
| 96 | 20 / 20 | 1.0e-05 | **capped -- output is a fixed iterate** |
| 128 | 20 / 20 | 7.7e-04 | **capped -- output is a fixed iterate** |

At N=128 the preconditioned solve converges in 5 Newton steps to a far better answer and misses
the reference by **1.2e+6 times the budget**. The M, L and XL presets are N = 256, 512, 1024, all
of them capped; the fuzzer draws N from [8, 1024] with a secret seed, so held-out inputs straddle
the crossover between 64 and 96.

**So: do not change `jfnk_bratu`'s preconditioner.** It passes only on the smallest inputs and
fails everything above them. Optimize the residual evaluation, the stencil, the dot products and
the Arnoldi vector updates instead -- which is where the time is anyway. Record this as the
general rule: **a solve-to-tolerance kernel with an iteration cap becomes a fixed-iterate kernel
the moment the cap binds, and you must check where that happens across the whole fuzz range, not
at the preset you are testing.**

### 5.6 `mixed_precision_ir` -- the precision split IS the kernel

The factorization is deliberately fp32 and the residual `b - Ax` deliberately fp64. Computing the
residual in fp32 raises nothing, costs nothing, and stalls the refinement about nine orders of
magnitude short of the accuracy it reports as converged. Only a forward-error check catches it:
backward error for a factorization is small regardless of conditioning and cannot tell an fp32
factorization from an fp64 one.

`steps_out` is graded, so the number of refinement steps must match. A different fp32 LU -- a
blocked LAPACK `sgetrf`, a cuSOLVER factorization, MAGMA's `magma_dsgesv_gpu` -- computes the same
pivot sequence but different rounding in the trailing updates, and can land on a different step
count near the tolerance boundary. Check `steps_out` across the fuzz range before you trust it.

### 5.7 Kernels where the answer is not a preconditioner at all

- **`lanczos_reorth`.** The kernel is unpreconditioned by construction. Its cost is the double
  full reorthogonalization: `2(j+1)` separate dot-and-axpy passes over N at step j. Batching each
  pass into a GEMV pair (`Q[:, :j+1].T @ w`, then `w -= Q[:, :j+1] @ dots`) turns modified
  Gram-Schmidt into classical Gram-Schmidt, which is different arithmetic -- but with the
  reorthogonalization already done twice, it holds. **Measured: 9.1e-5 of budget at 16³/m=50 and
  3.0e-4 at 32³/m=100**, three to four orders of margin, drifting roughly linearly in `m` (verify
  at the XL preset, m=152). This unlocks BLAS-2/3 and is the largest legal win in the kernel.
- **`householder_qr`.** LAPACK's `dgeqrf` computes the same factorization: the reference's
  `sign = +1 if A[k,k] >= 0` choice gives `R[k,k] = -sign(A[k,k])*||x||`, the same convention
  `dlarfg` uses. **Measured against `scipy.linalg.qr` (dgeqrf + dorgqr) at M=2000, N=64: R at
  1.3e-4 of budget, Q matching to 5.9e-15 absolute.** Two shape traps: `Q` is declared `(M, N)`,
  which is LAPACK's economy-size Q, so use `dorgqr` and not the full square factor; and `A` is
  graded, with the reference leaving ~0 below the diagonal (measured 2.9e-15) where LAPACK leaves
  the reflectors -- you must zero them.
- **`sparse_cholesky`.** The ordering, elimination tree, fill and supernodes arrive as inputs from
  an untimed symbolic phase. METIS and Scotch have nothing to contribute. The timed work is the
  supernodal numeric factorization: dense trailing-block updates, which is a BLAS-3 problem.
- **`rk4_ensemble` / `rk45_ensemble`.** Embarrassingly parallel. Under adaptive stepping members
  finish at different step counts with their own accept/reject histories, and that divergent
  control flow IS the kernel; `n_accept` and `n_reject` are graded, so forcing a uniform step
  count is out.

---

## 6. Libraries: which one, for what

`envs/libraries.yaml` is the request table and the prompt's `Libraries:` line is what this host
actually resolved. **You may link anything listed there** -- put the `-l` tokens in the response
`build` field, in link order, including `-fopenmp` and `-lpthread` if you use them. A library not
listed can be built into the shared folder with `--prefix=$SHARED` before you submit.

Two mechanics that bite. Header-only libraries (eigen, CUTLASS, CuTe, cub, hipcub, boost, xsimd)
are discoverable but NOT requestable -- `library_tokens` returns nothing when the link tokens are
empty. And a `-L`-only link can bind a DIFFERENT build of the same library at load time without
any error; the harness adds an rpath for exactly this reason, so let it resolve the name rather
than hand-rolling paths.

### Link these, for these kernels

| need | CPU | CUDA | HIP |
|---|---|---|---|
| dense trailing updates, tall-skinny GEMM | `blas`/`lapack` (OpenBLAS), `blis` | cuBLAS | rocBLAS, hipBLAS |
| QR (`dgeqrf`/`dorgqr`), LU (`sgetrf`) | `lapack` (LAPACKE) | cuSOLVER | rocSOLVER, hipSOLVER |
| mixed-precision LU + refinement driver | `lapack` (`dsgesv`) | `magma` (`magma_dsgesv_gpu`) | `magma` (ROCm build) |
| SpMV, SpTRSV, ILU0/IC0 | -- | cuSPARSE | rocSPARSE, hipSPARSE |
| supernodal sparse Cholesky | `suitesparse` (CHOLMOD) | -- | -- |
| sparse direct LU | `superlu`, `mumps`, `strumpack` | -- | -- |
| fast Poisson / DST / FFT preconditioner | `fftw` | cuFFT | rocFFT, hipFFT |
| Lanczos / Arnoldi eigensolves | `arpack`, `slepc` | -- | -- |
| threading | `tbb` (C++), OpenMP | -- | -- |

Note what is missing from the CUDA and HIP columns: there is no vendor sparse triangular solve
worth calling for `sptrsv_level`. cuSPARSE's own level-scheduled `csrsv` is the baseline that
CapelliniSpTRSV and AG-SpTRSV beat by 4-5x and 4x, and cuSPARSE's legacy preconditioner routines
(`csrilu02`, `csric02`) are on NVIDIA's short-term deprecation list, with `cuSolverSp` already
deprecated in favour of cuDSS. Write the sync-free solve.

### Mine these, do not call these

`petsc`, `hypre`, `sundials`, `trilinos`, `slepc` are on the request table and they are the right
place to READ the algorithm and to VALIDATE your numbers against an independent implementation.
They are the wrong thing to submit. These kernels are graded elementwise against one specific
route; a call into BoomerAMG returns a different, equally valid hierarchy and scores zero on
`amg_setup`'s `level_n`. Use them like this:

- **PETSc** -- `KSPCG`/`KSPGMRES` with `PCSOR`, `PCILU`, `PCGAMG`, `PCHYPRE`; `-ksp_view` prints
  the exact preconditioner configuration, `-log_view` the per-event cost. The reference
  implementation of the cost model in 5.1.
- **hypre** -- BoomerAMG's coarsening, interpolation and GPU defaults (section 5.3).
- **SUNDIALS CVODE** -- variable-order BDF with the same order/step controller
  `bdf_newton_krylov` implements; the place to check an order history against.
- **Ginkgo** (not installed; build into the shared folder if you want it) -- ParILU, ParILUT,
  ISAI, adaptive-precision block-Jacobi, unsmoothed-aggregation AMG. The reference implementation
  of most of section 5.3.
- **PyAMG** -- smoothed aggregation in Python, the fastest way to check an `amg_setup` hierarchy's
  shape against an independent one before you optimize it.
- **Kokkos Kernels / Ifpack2** -- the production two-stage and multicolour Gauss-Seidel of section
  5.3, and the place to see how the inner sweep count is exposed as a parameter.
- **HPCG, and NVIDIA's `nvidia-hpcg`** -- `sgs_pcg` is HPCG's CG plus HPCG's symmetric
  Gauss-Seidel smoother, so the reference upstream is the first place to look for the shape of a
  fast implementation. Read it for the SpMV, the fused vector updates and the halo structure. Do
  NOT copy its preconditioner: the GPU HPCG implementations reach their numbers by multicolouring
  the smoother, which is exactly the substitution section 2 measures at nine orders outside the
  band. Optimized-HPCG results are reported against HPCG's own convergence criterion, not against
  a reference iterate.

---

## 7. Do not try these

Each of these looks like an optimization and is not. The first five are measured in section 2.

1. Replacing symmetric Gauss-Seidel with Jacobi to remove the dependence.
2. Replacing it with a multicolour sweep. Different ordering, different fixed-point trajectory.
3. Replacing the exact triangular solves with inner Jacobi sweeps in a fixed-iterate kernel, at
   any sweep count -- ten sweeps still misses by six orders.
4. Fusing two coloured half-sweeps. The second reads what the first wrote.
5. Adding, removing or strengthening a preconditioner in any kernel marked FROZEN in section 1.
6. Changing a smoother, a cycle type, a damping weight or a sweep count in `mg_vcycle`. `OMEGA =
   6/7` is the smoothing-optimal damped-Jacobi weight for the 3-D 7-point operator, `NU1 = NU2 =
   3` is what carries the per-cycle residual drop past 5x, and all of them are inside a fixed
   `ncycles`.
7. Skipping coarse levels because they hold little work. The collapse of parallelism toward the
   coarsest grid is what the benchmark measures.
8. Parallelizing the greedy aggregation in `amg_setup` or a fill-reducing ordering. A parallel
   aggregation produces DIFFERENT aggregates, a different coarse operator, and different graded
   `level_n`.
9. Reusing the Ruge-Stueben 0.25 strength threshold on a smoothed-aggregation measure (section 3).
10. Substituting ParILU or ParILUT for `ilu0`. It converges toward the sequential factor and
    `A_data` is graded elementwise.
11. Refactoring an operator inside a loop that was written to reuse one factorization.
12. Computing a refinement residual in the working precision instead of the declared one.
13. Forcing a uniform step count across an adaptive ensemble, or otherwise changing any graded
    count: `steps_out`, `n_accept`, `n_reject`, `order_history`, `diagnostics`.
14. Raising a tolerance, lowering an iteration cap, or shrinking a problem until a gate passes.
15. Calling a black-box solver -- BoomerAMG, AmgX, UMFPACK, `KSPSolve` -- in place of the
    algorithm. The algorithm is the benchmark.

What IS always legal: reassociating a reduction, level scheduling on the reference's own
ordering, batching an orthogonalization into BLAS-2/3 where the measurement supports it, tiling,
fusion, layout changes, SIMD, threading a loop that carries no dependence, and linking a library
whose kernel computes the same mathematics.

---

## 8. Before you submit

1. **Name the trip count.** Fixed or tolerance-driven? If tolerance-driven, find the input size
   at which the iteration cap starts binding, across the whole fuzz range and not just at your
   preset. Section 5.5 is what happens when you skip this.
2. **List the graded outputs**, and mark any that is a count.
3. **Diff against the reference on the fuzz range, not the preset.** Held-out inputs are drawn
   with a secret seed at grading time.
4. **Report the violation as a multiple of the budget**, `max |a-e| / (atol + rtol*|e|)`, not as
   "close". A substitution that is eight orders out and one that is fine both print as small
   numbers in a residual norm.
5. **Assert the pivot** in any incomplete factorization. Breakdown produces NaNs downstream that a
   loose comparison can pass.
6. **Check the true residual `b - Ax`**, not the recursively updated one. They drift apart.
7. **Count setup**, always, in any timing that motivates a preconditioner change.

---

## 9. Reproducing the measurements

Every table above comes from the corpus's own generators. The pattern:

```python
import sys, numpy as np, scipy.sparse as sp
sys.path.insert(0, "<HPCAgent-Bench checkout>")
from hpcagent_bench.support.helpers.sparse.generators import make_stencil_3d, make_suitesparse_csr

RTOL, ATOL = 1e-9, 1e-11          # fp64, from docs/numerical_validation.md
def budget(got, ref):             # <= 1.0 passes
    return float(np.max(np.abs(got - ref) / (ATOL + RTOL * np.abs(ref))))
```

Run the reference recurrence and the candidate on the SAME inputs, at two preset sizes, and print
`budget(...)`. Level structure is `level[i] = 1 + max(level[j] : j < i, A[i,j] != 0)` over the
rows in order; the SuiteSparse matrices need `HPCAGENT_BENCH_CACHE_DIR` pointed at a seeded cache
or they hit the network from inside `initialize()`.

---

## Sources

Distilled, not quoted. Nothing here is reproduced from the sources and none is in the corpus.

- Saad, *Iterative Methods for Sparse Linear Systems*, 2nd ed. (SIAM, 2003) -- preconditioned
  Krylov iterations, incomplete factorizations, level scheduling.
- Benzi, "Preconditioning techniques for large linear systems: a survey", *JCP* 182(2), 2002 --
  the taxonomy behind section 5.
- Chow & Patel, "Fine-grained parallel incomplete LU factorization", *SISC* 37(2), 2015; Anzt,
  Chow & Dongarra, "ParILUT -- a new parallel threshold ILU factorization", *SISC* 40(4), 2018 --
  ParILU and ParILUT.
- Anzt, Chow & Dongarra, "Iterative sparse triangular solves for preconditioning", Euro-Par 2015;
  Anzt et al., *JPDC* 2018 -- Jacobi triangular solves, over 10x against cuSPARSE, and blocking.
- Berger-Vergiat, Kelley, Rajamanickam et al., "Two-stage Gauss-Seidel preconditioners and
  smoothers for Krylov solvers on a GPU cluster", arXiv:2104.01196 -- GS2, 1-3 inner sweeps,
  Kokkos Kernels and Ifpack2.
- Anzt et al., "Adaptive precision in block-Jacobi preconditioning", *CCPE* 2019, and ACM *TOMS*
  47(2), 2021 -- fp16/fp32/fp64 per block, in Ginkgo.
- Anzt, Huckle, Bräckle & Dongarra, "Incomplete sparse approximate inverses for parallel
  preconditioning", *Parallel Computing* 71, 2018 -- ISAI.
- Su, Zhang, Li et al., "CapelliniSpTRSV", ICPP 2020; Lu, Liu et al., "AG-SpTRSV", ACM *TACO*
  21(3), 2024 -- synchronization-free SpTRSV and its margin over cuSPARSE.
- Adams, Brezina, Hu & Tuminaro, "Parallel multigrid smoothing: polynomial versus Gauss-Seidel",
  *JCP* 188(2), 2003 -- why polynomial smoothers win in parallel AMG.
- Vanek, Mandel & Brezina, "Algebraic multigrid by smoothed aggregation", *Computing* 56, 1996;
  Briggs, Henson & McCormick, *A Multigrid Tutorial*, 2nd ed. (SIAM, 2000).
- hypre user manual, BoomerAMG section -- PMIS as the GPU coarsening default, and aggressive
  coarsening being less effective on GPUs.
- Carson & Higham, "Accelerating the solution of linear systems by iterative refinement in three
  precisions", *SISC* 40(2), 2018 -- the cond(A) bounds for LU-IR versus GMRES-IR.
- Higham, *Accuracy and Stability of Numerical Algorithms*, 2nd ed. -- forward versus backward
  error, and why only one of them separates a precision change.
- Loe, Morgan et al., "Toward efficient polynomial preconditioning for GMRES", and "Polynomial
  preconditioned GMRES for GPU computing", 2021-2022.
- Knoll & Keyes, "Jacobian-free Newton-Krylov methods", *JCP* 193(2), 2004 -- the matrix-free
  Jacobian and the scaling of its finite-difference step.
- Golub & Van Loan, *Matrix Computations*, 4th ed.; Parlett, *The Symmetric Eigenvalue Problem* --
  Householder QR, Lanczos, loss of orthogonality.
- Davis, *Direct Methods for Sparse Linear Systems* (SIAM, 2006) -- symbolic phases, elimination
  trees, fill-reducing orderings, supernodes.
