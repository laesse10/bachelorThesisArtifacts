---
name: solver-preconditioners
version: 2
status: LIVE -- this is the installed skill
description: "Preconditioning strategy for any solver kernel you are asked to make fast: how to work out whether the preconditioner is yours to change or frozen by what is graded, how to classify the operator before choosing one, the full ladder from Jacobi to multigrid with the state of the art and the failure mode behind each, how to parallelize the preconditioner you were given without changing its mathematics, and which library supplies which piece."
when: the kernel applies M^-1, smooths, relaxes, factorizes incompletely, solves triangular, builds a multigrid hierarchy, or wraps any of those in a Krylov or Newton iteration
---

Preconditioning is the largest lever in a sparse solver and the easiest place to lose the whole
score. The reason is a mismatch of standards: the literature judges a preconditioner by iteration
count or time to a tolerance, while a graded kernel is judged elementwise against one specific
reference computation. Those two standards disagree, and they disagree by eight or nine orders of
magnitude, not by a little.

This page is the procedure. Sections 1 to 3 decide what you are allowed to do. Sections 4 to 7 are
the playbook for when you are free, and for when you are not. Section 10 gives the verdict by
kernel pattern, so you can predict it before reading a line of code.

Numbers marked **measured** were produced by the scripts in `measurements/`. They are quoted as
evidence for a general rule, not as a lookup table.

## 1. First decide whether the preconditioner is yours

Ask three questions, in this order. Any single "yes" freezes the preconditioner.

**1. Is the output a fixed iterate rather than a converged answer?** A solve-to-tolerance kernel
returns the solution; two preconditioners that both converge return the same thing. A kernel with
a fixed trip count returns iterate number `k` of one particular recurrence, and the preconditioner
is part of that recurrence. Look for a loop bound that is a parameter (`niter`, `ncycles`,
`TSTEPS`, `nsweeps`) rather than a residual test.

**2. Is any graded output a count, a flag, or a history?** Step counts, accept/reject tallies,
order histories, iteration numbers, convergence flags. A count is compared like any other array,
so an off-by-one is a factor-of-two relative error. Any change to how many steps the solver takes
fails, however much better the answer is.

**3. Does a tolerance loop have a cap that binds?** This is the one that catches people. A kernel
written as "iterate until `||F|| <= tol`, at most `max_steps`" is a tolerance kernel where it
converges and a fixed-iterate kernel where it does not. The boundary sits at some problem size, it
moves with the parameters, and held-out inputs are drawn on both sides of it. **Measured** on a
Jacobian-free Newton-Krylov Bratu solve with `max_newton = 20`:

| N | Newton steps used | final ratio of residual norms |  |
|---|---|---|---|
| 32 | 5 / 20 | 1.9e-12 | converged |
| 48 | 6 / 20 | 7.7e-12 | converged |
| 64 | 14 / 20 | 7.1e-11 | converged |
| 96 | 20 / 20 | 1.0e-05 | **capped: output is a fixed iterate** |
| 128 | 20 / 20 | 7.7e-04 | **capped** |

A fast-Poisson preconditioner on the inner GMRES of that same solve cuts matrix-free Jacobian
applications from 204 to 16, a 12.8x reduction, and lands at 1.3e-3 of the grading budget at
N=32. It passes. At N=128 it converges in 5 Newton steps to a strictly better answer and misses
the reference by **1.2e+6 times the budget**. Same code, same change, opposite verdict, and the
only difference is which side of the cap the input fell on.

> **Rule.** Establish where the cap binds across the whole legal input range before you touch the
> preconditioner. Not at the one size you happen to be testing.

If all three answers are "no", the preconditioner is yours. Go to section 3. If any is "yes", the
preconditioner is frozen and your job is section 5: make the given preconditioner fast without
changing what it computes.

### What "frozen" costs, measured

Preconditioned CG on a 27-point variable-coefficient operator, run exactly as its reference does
with `x0 = 0` and a fixed sweep count, with only the preconditioner swapped. The figure is the
worst per-element violation of `|a - e| <= atol + rtol*|e|` as a multiple of the budget, at
fp64 tolerances `rtol = 1e-9`, `atol = 1e-11`. **A variant passes only at <= 1.0.**

| substitution | 16³, 25 sweeps | 32³, 50 sweeps |
|---|---|---|
| level-scheduled SGS, same row ordering | 2.5e-5 | 4.3e-5 |
| Jacobi instead of SGS | 1.3e+9 | 1.3e+9 |
| two-stage SGS, 3 inner Jacobi sweeps | 4.3e+8 | 4.4e+8 |
| two-stage SGS, 10 inner Jacobi sweeps | 1.9e+6 | 2.3e+6 |
| multicolour SGS, 8 colours | 8.7e+8 | 9.2e+8 |

Read row three against section 4: three inner Jacobi sweeps reproduce the reference's iteration
count **exactly** and are still eight orders outside the band.

> **Rule.** Matching a convergence rate is not matching an iterate. Never use an iteration count
> as evidence that a substitution is numerically safe.

Only the first row passes, and section 5 explains why it is the one transform that always does.

---

## 2. Know the grading band before you argue about accuracy

Elementwise comparison against a reference is the usual contract. A representative band:

| precision | rtol | atol |
|---|---|---|
| fp64 | 1e-9 | 1e-11 |
| fp32 | 1e-3 | 1e-5 |

Two habits follow. Report your error as `max |a-e| / (atol + rtol*|e|)`, a multiple of the budget,
never as "close" or as a residual norm: a change that is eight orders out and one that is fine
both look small in a residual. And check against the whole legal input range, since held-out
inputs are usually drawn with a seed you cannot see.

A second comparison path is often layered on: a normwise LAPACK-style test ratio,
`max|a-e| / (eps * f(n) * ||e||_inf)`, which stays meaningful where cancellation has destroyed an
individual element's digits. It admits answers the elementwise rule rejects, never the reverse.

---

## 3. Classify the operator before choosing anything

Every later decision follows from this, and all of it is cheap to establish.

**Symmetry and definiteness.**
- *SPD* -- conjugate gradients. The preconditioner must stay symmetric and positive definite. An
  unsymmetric `M` does not merely slow CG down; the recurrence loses its meaning and the iterates
  wander. This is why symmetric Gauss-Seidel is symmetric: forward sweep, scale by D, backward
  sweep.
- *Symmetric indefinite* -- MINRES. CG does not apply.
- *Nonsymmetric* -- restarted GMRES or BiCGStab. The restart length is a real knob: longer restart,
  fewer outer iterations, more memory and more orthogonalization per step.
- *Block or saddle-point* -- the block semantics are the whole problem. A purely algebraic view
  discards the only information that makes such an operator tractable.

**Singularity.** Test it. `||A·1||_inf / ||A||_inf` costs one matvec, and a graph Laplacian built
as `A_ii = sum_j w_ij`, `A_ij = -w_ij` sums to zero in every row. **Measured** on such an operator
at 16³ and 32³: 4.2e-16 and 4.1e-16, so it is singular to machine precision with the constant
vector in its null space, whatever the docstring says. If the right-hand side is in the range the
system is consistent and CG converges to the minimum-norm solution, so nothing is broken, but the
consequences for preconditioning are large. See the next paragraph.

**The spectrum, at both ends.** `lambda_max` by a few Lanczos steps, the smallest *nonzero*
eigenvalue by shift-invert. The ratio, not the formal condition number, is what drives iteration
counts. **Measured** on the same operator:

| | 16³ | 32³ |
|---|---|---|
| lambda_1 | ~1e-13, numerically zero | ~1e-13 |
| lambda_2 | 5.92 | 1.60 |
| lambda_max | 1161 | 1233 |
| lambda_max / lambda_2 | 196 | 773 |

Four times worse for a doubled edge: the `k²` growth of a Laplacian, which means iteration counts
grow like `k`. This single table decides which families in section 4 can possibly work. A method
that damps a bounded interval away from the origin cannot fix an eigenvalue at
`lambda_max / 773` that keeps falling as you refine.

**Coefficient spread.** On a constant-coefficient operator, diagonal preconditioning is a scalar
rescale and buys exactly 1.00x, and smoothed aggregation builds the same hierarchy geometric
multigrid would. Jumping coefficients are what make preconditioner choice measurable at all, and
what make algebraic methods worth their setup.

**Ordering and sparsity structure.** Section 5 -- this is what decides how much parallelism the
preconditioner you were given can expose, and it must be measured, never inferred from the size
or the nonzero count.

---

## 4. The ladder

Cheapest first. For each: what it buys, what it costs, where it fails. The iteration counts are
**measured** on the 27-point variable-coefficient operator above, CG to relative residual 1e-8,
and are there to show the *shape* of the trade rather than to be transplanted.

| preconditioner | iters @16³ | iters @32³ | vs none |
|---|---|---|---|
| none | 85 | 133 | 1.00x |
| Jacobi | 69 | 108 | 1.23x |
| symmetric Gauss-Seidel | 29 | 50 | 2.7-2.9x |
| two-stage SGS, 1 inner Jacobi | 69 | 108 | identical to Jacobi |
| two-stage SGS, 2 inner Jacobi | 39 | 59 | 2.2x |
| two-stage SGS, 3 inner Jacobi | 32 | 50 | matches SGS |
| two-stage SGS, 10 inner Jacobi | 29 | 50 | matches SGS |
| multicolour SGS, 8 colours | 35 | 56 | 2.4x, +12% iters |
| block Jacobi, contiguous block 8 | 68 | 100 | 1.25-1.33x |
| block Jacobi, contiguous block 64 | 66 | 115 | worse than block 8 |
| Chebyshev in D^-1A, degree 2/4/8/16 | 68-69 | 107-108 | 1.23x, buys nothing |
| IC(0) | 23 | 39 | 3.4-3.7x |

**Jacobi.** One divide. Always the baseline because it is free. Exactly 1.00x on a
constant-coefficient operator.

**Block Jacobi.** Blocks must follow the graph or the physics, never the index range. **Measured:**
blocks cut out of a 3-D lexicographic ordering capture only the fastest-varying direction's
coupling, and block 64 is *worse* than block 8. Its virtue on GPUs is real, though: a batched
variable-size block inverse is one kernel with no dependences at all. Anzt et al.'s
adaptive-precision block-Jacobi in Ginkgo chooses fp16, fp32 or fp64 per block from that block's
own condition number, matching or beating uniform fp32 storage while preserving the fp64 solver's
convergence rate. The apply is bandwidth bound, so narrower storage is directly faster.

**Gauss-Seidel and SOR.** The workhorse smoother. Sequential in row order; section 5 is how to open
that loop. Symmetric Gauss-Seidel is the SPD-safe form.

**Multicolour Gauss-Seidel.** Every point of one colour reads only the other colours, so a colour's
half-sweep is fully data parallel; that is the entire reason colouring exists. The half-sweeps stay
ordered with respect to each other and must not be fused. Costs a colouring and a reordering in
setup, and typically a few percent more iterations. **Measured:** a distance-1 greedy colouring of
a 27-point graph gives exactly 8 colours holding N/8 points each, the 2x2x2 blocking, for +12%
iterations. That is about as good as colouring gets.

**Two-stage Gauss-Seidel (GS2).** Replace each exact triangular solve with a fixed number of inner
Jacobi-Richardson sweeps, so the apply becomes SpMVs. **Measured knee at 3 sweeps**, matching
exact SGS's iteration count; Berger-Vergiat et al. report 1 to 3 sufficient across their
applications and often 1 in production. The appeal is structural: the apply becomes SpMVs, which
carry no dependence, in exchange for a convergence rate the sweep count controls. Two caveats.
Convergence of the inner iteration needs the spectral radius of `D^-1 L` below 1, so damp the inner
sweep on hard problems. And **one inner sweep IS Jacobi**: from a zero start the first sweep gives
`y = r/D` and the triangular part never enters, which the measured table confirms exactly.

**Iterative triangular solve inside an incomplete-factorization apply.** The same idea applied to
the factors rather than to A. Anzt, Chow and Dongarra established the approach and found blocking
improves its robustness. How many sweeps a factor needs is problem-dependent, and whether the
cheaper apply repays the extra outer iterations depends on your target. Both are measurements.

**ISAI and sparse approximate inverse.** Approximate `L^-1` and `U^-1` by sparse matrices on a
prescribed pattern, so the apply becomes two SpMVs with no dependences at all. Ginkgo ships batched
ISAI generation using only thread-local memory, plus a mixed-precision factorized SPAI for SPD
operators. The right answer when the triangular solve dominates and the level structure is hostile.

**Incomplete factorization, IC(0) / ILU(0) and threshold variants.** **Measured** at 3.4-3.7x, the
strongest local method here. Zero fill means the factor keeps the input pattern element for
element. Fill is the knob: fewer iterations, more memory, more setup, and unbounded fill turns a
sparse method into a dense one. Two hazards. Breakdown is quiet -- a zero or negative pivot
produces NaNs downstream that a loose comparison can pass, so assert the pivot rather than assume
it, especially on a singular or nearly singular operator. And setup is sequential in row order with
the same level structure as the solve. **ParILU and ParILUT** (Chow & Patel; Anzt, Chow &
Dongarra) replace that with a fine-grained fixed-point iteration, one thread per matrix element,
converging *toward* the sequential factor; ParILUT interleaves it with a threshold-based pattern
update. A few sweeps reach comparable quality. Note "toward": a finite sweep count does not
reproduce the sequential factor elementwise.

**Polynomial and Chebyshev.** No triangular solve, no stored factor, no global reductions -- pure
SpMV and axpy, which is why it is the smoother of choice in parallel and GPU multigrid (Adams et
al.). It needs a bound on `lambda_max`, for which Lanczos is the reliable estimator, and it is
important not to *under*estimate. It also needs a lower bound, and that is where it dies:
**measured** at degree 16 on the Laplacian above, 1.24x, indistinguishable from plain Jacobi,
because the eigenvalues that cost the iterations sit near zero and fall as the grid refines.
Polynomial preconditioning is genuinely strong on well-conditioned nonsymmetric systems (Loe,
Morgan et al.) and is the wrong tool for an elliptic operator with a near-null space.

**Multigrid.** The only family whose iteration count does not grow with problem size, and therefore
the right answer for any elliptic operator large enough to care about. Smoothed aggregation for
jumping coefficients, classical Ruge-Stueben for scalar elliptic problems on unstructured meshes,
unsmoothed aggregation where setup cost dominates. The knobs and their prices:

- *Coarsening aggressiveness* -- cheaper hierarchy to build and apply, potentially worse
  convergence. On GPUs, hypre's own documentation notes aggressive coarsening is **not** as
  effective as on CPUs, because the long-range interpolation it forces is expensive there. hypre's
  GPU coarsening default is PMIS where its CPU default is HMIS.
- *Strength-of-connection threshold* -- decides which couplings survive into the coarse problem,
  and **a threshold is tied to its strength measure, not portable between measures**. The familiar
  Ruge-Stueben 0.25 normalizes by the row's largest off-diagonal. The smoothed-aggregation measure
  `|A_ij| > theta * sqrt(|A_ii| |A_jj|)` does not, and on an operator whose diagonal is a sum of
  ~26 weights a typical normalized coupling is about 0.038 -- so 0.25 admits almost nothing and
  produces an empty strength graph and no coarsening at any level. The right value there is ~0.03.
- *Smoother* -- see the Chebyshev and colouring entries above. Parallelism collapses toward the
  coarsest grid and that collapse is inherent, not a defect to engineer away by skipping levels.
- *Near-null space* -- the constant vector for a Laplacian, the rigid-body modes for elasticity.
  Smoothed aggregation's piecewise-constant tentative prolongator assumes the constant; give it the
  right modes or it will not coarsen correctly.

**Fast transform preconditioners.** If the operator has a constant-coefficient elliptic part, a
DST or FFT solve of that part is its exact inverse in O(N log N) and an extremely strong
preconditioner. **Measured** on the Bratu Jacobian in section 1: 204 Arnoldi steps down to 16.
Underused, and worth checking for before reaching for anything algebraic.

**Deflation and recycling.** Project out the few smallest eigenvectors, or carry a Krylov subspace
across a sequence of related solves. The natural fit for a Newton or time-stepping loop where
consecutive systems are close.

**Domain decomposition.** Additive Schwarz with overlap, or a two-level method with a coarse space.
Overlap buys coupling and costs local work and communication; without a coarse space, iteration
counts grow with the subdomain count.

---

## 5. Making a frozen preconditioner fast

This is the common case, and it has one governing distinction.

> **Level scheduling preserves the mathematics exactly. Reordering does not.**

Level-set scheduling groups rows the analysis has *proved* independent, so every row computes the
identical value and the only difference from a sequential sweep is reassociation inside the row
reduction. **Measured at 2.5e-5 of the grading budget**, four orders of margin. Multicolouring
changes which values a row reads, which is different mathematics -- **measured at 8.7e+8**, nine
orders outside. Both are "parallelizing Gauss-Seidel" in the literature. Only one is a
parallelization of *your* Gauss-Seidel.

The analysis phase belongs outside the timed region: one schedule amortizes over many solves, which
is how these methods are used.

### Measure the level structure; never infer it

It is a property of the *ordering*, not of the size or the nonzero count.

**Structured, lexicographic ordering.** A 27-point operator on a `k³` grid, **measured** and
confirmed on k = 16, 24, 32, 40, 48, 56, 64:

| grid | N | levels | avg rows/level | max rows/level |
|---|---|---|---|---|
| 16³ | 4,096 | 106 | 39 | 64 |
| 32³ | 32,768 | 218 | 150 | 256 |
| 48³ | 110,592 | 330 | 335 | 576 |
| 64³ | 262,144 | 442 | 593 | 1,024 |

Exactly `levels = 7k - 6`, `max rows/level = (k/2)²`, average about `k²/7`. At 192³ that is 1,338
levels holding ~5,300 rows each. Those two numbers are what the schedule offers:

- **width**, the rows available to run concurrently, about `k²/7`, growing only quadratically while
  the problem grows cubically;
- **depth**, the ordered levels, each a synchronization point, growing linearly in `k`.

Whether that is a good trade is a property of your target, not of the matrix. Compare the width
against the concurrency your device wants to keep busy and the depth against the cost of a barrier
there, then **measure it on the hardware you are targeting**. The width and depth transfer; a
speed-up does not. If the schedule is too thin, the remaining time sits in the operator application,
the dot products and the vector updates, which carry no dependence at all.

**Unstructured matrices.** **Measured** on four SPD operators drawn from applications:

| matrix | N | nnz(L) | levels | avg | max | levels under 32 rows |
|---|---|---|---|---|---|---|
| thermal conduction, small | 82,654 | 328,556 | 971 | 85 | 14,203 | 471 |
| electromagnetics | 259,789 | 2,251,231 | 3,452 | 75 | 963 | 1,226 |
| thermal conduction, large | 1,228,045 | 4,904,179 | 1,239 | 991 | 207,967 | 114 |
| structural mechanics | 914,898 | 28,191,660 | 2,367 | 387 | 5,991 | 102 |

Savagely imbalanced: one level of 14,203 rows alongside 471 levels holding fewer than 32. A
per-level kernel launch spends its life on empty launches. That is the canonical case for a
**synchronization-free SpTRSV** -- one thread or warp per row, spinning on a per-row ready flag,
no analysis phase and no global barriers. It tolerates the imbalance instead of paying for it: a
row runs as soon as its dependencies land, so a level holding three rows costs three rows of work
rather than a launch. CapelliniSpTRSV (Su et al.) and AG-SpTRSV (Lu, Liu et al.) are the
implementations to read. Both report large margins over a level-scheduled solve, but those margins
are theirs, on their hardware and their matrices: take the algorithm and measure the margin
yourself.

A level structure is only useful in a band. Many levels of one row each is a serial chain wearing
a schedule; seven levels over half a million rows is embarrassingly parallel and needs no schedule.

### The other transforms that are always available

- **Reassociate reductions.** A parallel reduction necessarily reassociates and the band covers it.
- **Batch an orthogonalization into BLAS-2/3.** A Gram-Schmidt sweep written as `2(j+1)` separate
  dot-and-axpy passes becomes one GEMV pair. This turns modified Gram-Schmidt into classical
  Gram-Schmidt, which is different arithmetic -- but where the reorthogonalization is already done
  twice it holds: **measured at 9.1e-5 of budget for m=50 and 3.0e-4 for m=100**, drifting roughly
  linearly in the step count, so verify at your largest size. Usually the single biggest legal win
  in a Krylov or Lanczos kernel.
- **Call a library kernel that computes the same mathematics.** LAPACK's `dgeqrf` uses the same
  reflector sign convention as the textbook algorithm, so **measured** against a scalar reference
  at M=2000, N=64: R at 1.3e-4 of budget, Q matching to 5.9e-15. Watch the conventions: economy
  versus full Q, and whether the reference leaves zeros below the diagonal where LAPACK leaves the
  reflectors.
- **Tile, fuse, block, vectorize, thread a loop that carries no dependence, change layout.** All
  ordinary and all legal.

---

## 6. Precision is part of the algorithm

Store or apply the preconditioner in lower precision while the Krylov iteration stays in working
precision. The preconditioner is an approximation already, so its own error budget is loose, and
its apply is bandwidth bound, so narrower storage converts directly into speed. This is the
highest-yield low-risk idea in the current literature.

Where a kernel *names* a precision, that precision is the algorithm and is not a tuning knob.
Mixed-precision iterative refinement is the clear case: the factorization is deliberately the low
precision because it is the expensive cubic step, and the residual is deliberately the high one.
Computing that residual in the low precision raises nothing, costs nothing, and stalls the
refinement about nine orders of magnitude short of the accuracy it reports as converged.

Two rules that follow.

- **Backward error cannot detect a precision change; forward error can.** Backward error for a
  factorization is small regardless of conditioning. Forward error tracks the condition number and
  separates an fp32 factorization from an fp64 one cleanly.
- **Know the conditioning bound.** From Carson & Higham's three-precision iterative refinement:
  with an fp16 factorization, plain LU-based refinement is only guaranteed to reduce the error for
  `cond(A) << 2e3`, while GMRES-IR -- solving the correction equation by GMRES preconditioned with
  those same LU factors -- extends the guarantee to `cond(A) << 3e7`. If a low-precision
  factorization is not converging, that is the fix, not more refinement steps.

Index arrays stay 64-bit. A narrower index silently wraps on the large sizes rather than failing.

---

## 7. Judge a preconditioner honestly

    T = T_setup + N_iter * T_apply

A stronger preconditioner charges twice, once to build and again on every application, and only
`N_iter` is visible in an iteration count.

> **Iteration count is a diagnostic. Time to a fixed accuracy with setup counted is the objective.**

The sharpest illustration is a multigrid hierarchy that barely coarsens. It converges in *fewer*
iterations than a correct one, because its cycle approaches a direct solve on a barely reduced
operator, while each of those iterations costs tens of fine-grid operator applications. Iteration
count is the metric that failure mode passes; operator complexity is the metric that catches it.

Stopping and convergence, which is where wrong conclusions are drawn:

- Measure the relative residual against the right-hand side norm, from the stated initial guess.
  Starting from zero, the initial residual is the right-hand side.
- The recursively updated residual and the true `b - Ax` drift apart over many iterations. A
  convergence claim checked only against the recursive one can be wrong.
- Quadratic convergence in a Newton iteration is a property to verify, not assume. A residual
  history falling linearly means the Jacobian is wrong, most often a badly scaled finite-difference
  step, and such a solver still converges and still passes a naive test.
- Report a preconditioner comparison at two problem sizes. Grid independence, or its absence, is
  the whole point and no single size shows it.

---

## 8. Libraries: link the kernel, mine the framework

Two different uses, and conflating them is how submissions score zero.

**Link these.** A library routine that computes the same mathematics is an implementation, not a
substitution, and is legal wherever the conventions match.

| need | CPU | CUDA | HIP |
|---|---|---|---|
| dense trailing updates, tall-skinny GEMM | OpenBLAS, BLIS | cuBLAS | rocBLAS, hipBLAS |
| QR, LU, Cholesky | LAPACK / LAPACKE | cuSOLVER | rocSOLVER, hipSOLVER |
| mixed-precision LU with refinement | LAPACK `dsgesv` | MAGMA `magma_dsgesv_gpu` | MAGMA |
| SpMV, SpTRSV, ILU0/IC0 | -- | cuSPARSE | rocSPARSE, hipSPARSE |
| supernodal sparse Cholesky | SuiteSparse CHOLMOD | -- | -- |
| sparse direct LU | SuperLU, MUMPS, STRUMPACK | -- | cuDSS |
| fast Poisson, DST, FFT preconditioner | FFTW | cuFFT | rocFFT, hipFFT |
| eigen-estimates for Chebyshev bounds | ARPACK, SLEPc | -- | -- |
| fill-reducing ordering | METIS, Scotch | -- | -- |
| threading | OpenMP, oneTBB | -- | -- |

Note what is absent from the GPU columns: there is no vendor sparse triangular solve worth calling
when the level structure is bad. cuSPARSE's level-scheduled `csrsv` is the baseline the sync-free
algorithms beat by 4-5x, and its legacy preconditioner routines `csrilu02` and `csric02` are on
NVIDIA's deprecation list, with `cuSolverSp` already deprecated in favour of cuDSS.

**Mine these.** PETSc, hypre, Trilinos, SUNDIALS, Ginkgo, Kokkos Kernels, PyAMG and AMGX are
where the algorithms live and where you validate your numbers against an independent
implementation. They are the wrong thing to *submit* when the kernel is graded elementwise against
one specific route: a call into BoomerAMG returns a different, equally valid hierarchy and matches
nothing.

- **PETSc** -- `KSPCG`/`KSPGMRES` with `PCSOR`, `PCILU`, `PCGAMG`, `PCHYPRE`. `-ksp_view` prints
  the exact preconditioner configuration and `-log_view` the per-event cost. The reference
  implementation of section 7's cost model.
- **hypre** -- BoomerAMG's coarsening, interpolation and GPU defaults.
- **Ginkgo** -- ParILU, ParILUT, ISAI, adaptive-precision block-Jacobi, unsmoothed-aggregation AMG.
  The reference implementation of most of section 4.
- **Kokkos Kernels / Ifpack2** -- production two-stage and multicolour Gauss-Seidel, and how the
  inner sweep count is exposed as a parameter.
- **SUNDIALS** -- variable-order BDF with a real order and step controller.
- **PyAMG** -- smoothed aggregation in Python, the fastest way to sanity-check a hierarchy's shape.
- **A tuned benchmark implementation**, where one exists, is worth reading for its operator
  application, its fused vector updates and its data layout. Do not copy its preconditioner. Such
  implementations reach their numbers by reordering the smoother, typically by multicolouring, and
  they are permitted to because they are scored against their own convergence criterion rather than
  against a reference iterate. That is the substitution section 1 measures at nine orders outside
  the band. A tuned implementation is only a safe model where its correctness contract matches
  yours.

Two mechanical traps when a harness resolves library requests for you. Header-only libraries are
often discoverable but not requestable, because there are no link tokens to resolve. And a
search-path-only link can silently bind a *different* build of the same library at load time with
no error at all, which is worse than a load failure because the number you report is a timing of an
implementation nobody chose. Let the harness resolve the name and add its own rpath.

---

## 9. Failure catalogue

Things that look like optimizations and are not. Where a rule is measured, section 1 or 5 has the
number.

1. Turning a Gauss-Seidel sweep into a Jacobi sweep to remove the dependence.
2. Reordering a sweep -- colouring, permutation, reversal -- in a kernel graded on an iterate.
   Natural ordering and red-black ordering are different fixed-point trajectories that agree only
   once converged, never sweep by sweep.
3. Replacing exact triangular solves with inner Jacobi sweeps in a fixed-iterate kernel, at any
   sweep count. Ten sweeps still misses by six orders.
4. Fusing two coloured half-sweeps. The second reads what the first wrote.
5. Substituting a fine-grained parallel incomplete factorization for a sequential one when the
   factor itself is graded. It converges toward the sequential factor, not to it.
6. Changing a smoother, cycle type, damping weight or sweep count inside a fixed cycle count.
7. Skipping coarse levels because they hold little work. The collapse of parallelism toward the
   coarsest grid is inherent.
8. Parallelizing a greedy aggregation, a fill-reducing ordering, or supernode detection and then
   comparing the resulting operator. Different aggregates, different coarse operator, still a valid
   hierarchy but not this one.
9. Transplanting a strength threshold between strength measures.
10. Refactoring an operator inside a loop written to reuse one factorization.
11. Computing a refinement residual in the working precision instead of the declared one.
12. Changing anything that alters a graded count: step counts, accept/reject tallies, order
    histories, convergence flags.
13. Forcing a uniform step count across an adaptive ensemble. The divergent control flow is the
    kernel.
14. Raising a tolerance, lowering an iteration cap, or shrinking a problem until a gate passes.
15. Replacing the algorithm with a black-box solver call. When the algorithm is what is being
    measured, a library solve replaces the measurement with a different one.

---

## 10. What the verdict returns in practice

Solver kernels recur in a small number of shapes, and the verdict follows from the shape. Applying
section 1's three questions across a full solver benchmark gives this distribution.

| kernel pattern | what is typically graded | verdict |
|---|---|---|
| Krylov iteration with a fixed sweep count | the iterate | frozen: the preconditioner is in the recurrence |
| multigrid cycle with a fixed cycle count | the corrected solution | frozen: smoother, cycle, damping weight |
| sparse triangular solve | the solution vector | frozen: the solve *is* the apply |
| incomplete factorization | the factor, on its own pattern | frozen: the factorization *is* the setup |
| coloured or ordered relaxation sweep | the relaxed field | frozen: the ordering is the algorithm |
| hierarchy setup, aggregation or coarsening | level sizes, nonzero counts, complexity | frozen: graded on shape |
| direct factorization with a supplied ordering | the solution | frozen: the ordering is an input |
| Krylov basis construction | the basis and its coefficients | unpreconditioned by construction |
| dense factorization | the factors | no preconditioner involved |
| iterative refinement reporting a step count | the solution and the count | frozen by the count |
| implicit integrator reporting order or step history | the state and the histories | frozen by the histories |
| adaptive integrator reporting accept/reject tallies | the state and the counts | frozen by the counts |
| explicit ensemble integrator | the states | no preconditioner involved |
| Newton-Krylov to a tolerance under an iteration cap | the converged state | open below the cap, frozen above |

Most freeze the preconditioner outright, a few have none to change, and the only open case closes
again as soon as its iteration cap binds.

That is not an accident of one benchmark. It follows from what elementwise grading against a
reference means: a reference picks one route to the answer, and a grader comparing iterates rather
than converged solutions is asking you to reproduce that route. Any benchmark built this way gives
the same distribution. So the work that scores is section 5, not section 4, and section 1's three
questions come before assuming otherwise.

---

## 11. Before you submit

1. Answer section 1's three questions in writing, including where the cap binds across the whole
   legal input range.
2. List the graded outputs and mark every one that is a count, a flag or a history.
3. Diff against the reference across the input range, not at one size.
4. Report the violation as a multiple of the budget, `max |a-e| / (atol + rtol*|e|)`.
5. Assert the pivot in any incomplete factorization.
6. Check the true residual `b - Ax`, not the recursively updated one.
7. Count setup in any timing that motivates a preconditioner change, and report two problem sizes.

### Reproducing a measurement

```python
import numpy as np
RTOL, ATOL = 1e-9, 1e-11          # fp64
def budget(got, ref):             # <= 1.0 passes
    return float(np.max(np.abs(got - ref) / (ATOL + RTOL * np.abs(ref))))
```

Run the reference recurrence and the candidate on identical inputs at two sizes and print
`budget(...)`. Level structure is `level[i] = 1 + max(level[j] : j < i, A[i,j] != 0)` taken over
the rows in order. `measurements/` has all seven scripts behind the tables above, with a README
mapping each to the section it supports.

---

## Sources

Distilled, not quoted.

- Saad, *Iterative Methods for Sparse Linear Systems*, 2nd ed. (SIAM, 2003) -- preconditioned
  Krylov iterations, incomplete factorizations, level scheduling.
- Benzi, "Preconditioning techniques for large linear systems: a survey", *JCP* 182(2), 2002 --
  the taxonomy behind section 4.
- Chow & Patel, "Fine-grained parallel incomplete LU factorization", *SISC* 37(2), 2015; Anzt,
  Chow & Dongarra, "ParILUT -- a new parallel threshold ILU factorization", *SISC* 40(4), 2018.
- Anzt, Chow & Dongarra, "Iterative sparse triangular solves for preconditioning", Euro-Par 2015;
  Anzt et al., *JPDC* 2018 -- Jacobi triangular solves and blocking.
- Berger-Vergiat, Kelley, Rajamanickam et al., "Two-stage Gauss-Seidel preconditioners and
  smoothers for Krylov solvers on a GPU cluster", arXiv:2104.01196 -- GS2 and its inner sweep count.
- Anzt et al., "Adaptive precision in block-Jacobi preconditioning", *CCPE* 2019, and ACM *TOMS*
  47(2), 2021.
- Anzt, Huckle, Bräckle & Dongarra, "Incomplete sparse approximate inverses for parallel
  preconditioning", *Parallel Computing* 71, 2018 -- ISAI.
- Su, Zhang, Li et al., "CapelliniSpTRSV", ICPP 2020; Lu, Liu et al., "AG-SpTRSV", ACM *TACO*
  21(3), 2024 -- synchronization-free SpTRSV.
- Adams, Brezina, Hu & Tuminaro, "Parallel multigrid smoothing: polynomial versus Gauss-Seidel",
  *JCP* 188(2), 2003.
- Vanek, Mandel & Brezina, "Algebraic multigrid by smoothed aggregation", *Computing* 56, 1996;
  Briggs, Henson & McCormick, *A Multigrid Tutorial*, 2nd ed. (SIAM, 2000); hypre's BoomerAMG
  manual for the GPU coarsening and interpolation defaults.
- Carson & Higham, "Accelerating the solution of linear systems by iterative refinement in three
  precisions", *SISC* 40(2), 2018.
- Higham, *Accuracy and Stability of Numerical Algorithms*, 2nd ed. -- forward versus backward
  error.
- Loe, Morgan et al., "Toward efficient polynomial preconditioning for GMRES" and "Polynomial
  preconditioned GMRES for GPU computing", 2021-2022.
- Knoll & Keyes, "Jacobian-free Newton-Krylov methods", *JCP* 193(2), 2004.
- Golub & Van Loan, *Matrix Computations*, 4th ed.; Parlett, *The Symmetric Eigenvalue Problem*.
- Davis, *Direct Methods for Sparse Linear Systems* (SIAM, 2006).
