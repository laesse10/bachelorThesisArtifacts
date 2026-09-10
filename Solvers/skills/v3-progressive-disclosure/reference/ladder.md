# The preconditioner ladder

Every family, cheapest first: what it buys, what it costs, where it fails. Read this when Step 1
established that the preconditioner is yours to choose.

## Contents

- Measured baseline on one operator
- Jacobi
- Block Jacobi
- Gauss-Seidel and SOR
- Multicolour Gauss-Seidel
- Two-stage Gauss-Seidel (GS2)
- Iterative triangular solve inside an incomplete-factorization apply
- ISAI and sparse approximate inverse
- Incomplete factorization: IC(0), ILU(0), threshold variants, ParILU/ParILUT
- Polynomial and Chebyshev
- Multigrid
- Fast transform preconditioners
- Deflation and recycling
- Domain decomposition
- Precision as part of the preconditioner
- Sources

## Measured baseline on one operator

CG to relative residual 1e-8 on a 27-point variable-coefficient operator with edge weights
log-uniform on [1, 100]. Shown to give the *shape* of each trade, not to be transplanted.

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

Reproduce with `measurements/02_preconditioner_iteration_counts.py`.

## Jacobi

One divide. Always the baseline because it is free. Exactly 1.00x on a constant-coefficient
operator, where it is a scalar rescale.

## Block Jacobi

Blocks must follow the graph or the physics, never the index range. Measured above: blocks cut out
of a 3-D lexicographic ordering capture only the fastest-varying direction's coupling, and block 64
is *worse* than block 8.

Its structural virtue is real: a batched variable-size block inverse carries no dependences at all.
Anzt et al.'s adaptive-precision block-Jacobi in Ginkgo chooses fp16, fp32 or fp64 per block from
that block's own condition number, and the result to carry away is the algorithmic one: the fp64
solver's convergence rate is preserved at lower storage precision. The apply is bandwidth bound, so
narrower storage should help, by an amount only your target can tell you.

## Gauss-Seidel and SOR

The workhorse smoother, 2.7-2.9x above. Sequential in row order. Symmetric Gauss-Seidel, forward
sweep then scale by D then backward sweep, is the SPD-safe form and is what a CG recurrence
requires.

## Multicolour Gauss-Seidel

Every point of one colour reads only the other colours, so a colour's half-sweep is fully data
parallel. That is the entire reason colouring exists. The half-sweeps stay ordered with respect to
each other and must not be fused.

Costs a colouring and a reordering in setup, plus a few percent more iterations. Measured: a
distance-1 greedy colouring of a 27-point graph gives exactly 8 colours holding N/8 points each,
the 2x2x2 blocking, for +12% iterations. That is about as good as colouring gets.

## Two-stage Gauss-Seidel (GS2)

Replace each exact triangular solve with a fixed number of inner Jacobi-Richardson sweeps, so the
apply becomes SpMVs. Measured knee at 3 sweeps, matching exact SGS's iteration count.
Berger-Vergiat et al. report 1 to 3 sufficient across their applications and often 1 in production.
The appeal is structural: the apply becomes SpMVs, which carry no dependence, in exchange for a
convergence rate that the sweep count controls.

Two caveats. Convergence of the inner iteration needs the spectral radius of `D^-1 L` below 1, so
damp the inner sweep on hard problems. And **one inner sweep IS Jacobi**: from a zero start the
first sweep gives `y = r/D` and the triangular part never enters, which the measured table confirms
exactly.

## Iterative triangular solve inside an incomplete-factorization apply

The same idea applied to the factors rather than to A: replace the exact solve with a few Jacobi
sweeps. Anzt, Chow and Dongarra established the approach and found blocking improves its robustness.
How many sweeps a factor needs is problem-dependent, and whether the cheaper apply repays the extra
outer iterations depends on your target. Both are measurements, not constants.

## ISAI and sparse approximate inverse

Approximate `L^-1` and `U^-1` by sparse matrices on a prescribed pattern, so the apply becomes two
SpMVs with no dependences at all. Ginkgo ships batched ISAI generation using only thread-local
memory, plus a mixed-precision factorized SPAI for SPD operators. The right answer when the
triangular solve dominates and the level structure is hostile.

## Incomplete factorization

IC(0) and ILU(0) measured at 3.4-3.7x, the strongest local method here. Zero fill means the factor
keeps the input pattern element for element.

Fill is the knob: fewer iterations, more memory, more setup. Unbounded fill turns a sparse method
into a dense one.

Two hazards.

- **Breakdown is quiet.** A zero or negative pivot produces NaNs downstream that a loose comparison
  can pass. Assert the pivot rather than assume it, especially on a singular or nearly singular
  operator.
- **Setup is sequential** in row order, with the same level structure as the solve.

**ParILU and ParILUT** (Chow & Patel; Anzt, Chow & Dongarra) replace that setup with a fine-grained
fixed-point iteration, one thread per matrix element, converging *toward* the sequential factor;
ParILUT interleaves it with a threshold-based pattern update. A few sweeps reach comparable quality.
Note "toward": a finite sweep count does not reproduce the sequential factor elementwise, so it
cannot pass a grade on the factor itself.

## Polynomial and Chebyshev

No triangular solve, no stored factor, no global reductions. Pure SpMV and axpy, which is why it is
the smoother of choice in parallel and GPU multigrid (Adams et al.).

It needs a bound on `lambda_max`, for which Lanczos is the reliable estimator, and it is important
not to *under*estimate. It also needs a lower bound, and that is where it dies on an elliptic
operator: measured at degree 16 on a Laplacian, 1.24x, indistinguishable from plain Jacobi, because
the eigenvalues that cost the iterations sit near zero and fall as the grid refines.

Polynomial preconditioning is genuinely strong on well-conditioned nonsymmetric systems (Loe, Morgan
et al.). It is the wrong tool for an operator with a near-null space.

## Multigrid

The only family whose iteration count does not grow with problem size, and therefore the right
answer for any elliptic operator large enough to care about. Smoothed aggregation for jumping
coefficients, classical Ruge-Stueben for scalar elliptic problems on unstructured meshes, unsmoothed
aggregation where setup cost dominates.

The knobs and their prices:

- **Coarsening aggressiveness.** Cheaper hierarchy to build and apply, potentially worse
  convergence. On GPUs, hypre's own documentation notes aggressive coarsening is *not* as effective
  as on CPUs, because the long-range interpolation it forces is expensive there. hypre's GPU
  coarsening default is PMIS where its CPU default is HMIS.
- **Strength-of-connection threshold.** Decides which couplings survive into the coarse problem, and
  **a threshold belongs to its strength measure and does not transfer between measures**. The
  familiar Ruge-Stueben 0.25 normalizes by the row's largest off-diagonal. The smoothed-aggregation
  measure `|A_ij| > theta * sqrt(|A_ii| |A_jj|)` does not, and on an operator whose diagonal is a
  sum of ~26 weights a typical normalized coupling is about 0.038. So 0.25 admits almost nothing and
  produces an empty strength graph with no coarsening at any level. The right value there is ~0.03.
- **Smoother.** See the Chebyshev and colouring entries above. Parallelism collapses toward the
  coarsest grid and that collapse is inherent, not a defect to engineer away by skipping levels.
- **Near-null space.** The constant vector for a Laplacian, the rigid-body modes for elasticity.
  Smoothed aggregation's piecewise-constant tentative prolongator assumes the constant. Give it the
  right modes or it will not coarsen correctly.

Watch for the hierarchy that barely coarsens: it converges in *fewer* iterations because its cycle
approaches a direct solve, while each cycle costs tens of fine-grid operator applications. Operator
complexity catches this; iteration count does not.

## Fast transform preconditioners

If the operator has a constant-coefficient elliptic part, a DST or FFT solve of that part is its
exact inverse in O(N log N) and an extremely strong preconditioner. Measured on a Bratu Jacobian:
204 Arnoldi steps down to 16, a 12.8x reduction. Underused, and worth checking for before reaching
for anything algebraic.

## Deflation and recycling

Project out the few smallest eigenvectors, or carry a Krylov subspace across a sequence of related
solves. The natural fit for a Newton or time-stepping loop where consecutive systems are close.

## Domain decomposition

Additive Schwarz with overlap, or a two-level method with a coarse space. Overlap buys coupling and
costs local work and communication. Without a coarse space, iteration counts grow with the subdomain
count.

## Precision as part of the preconditioner

Store or apply the preconditioner in lower precision while the Krylov iteration stays in working
precision. The preconditioner is an approximation already, so its own error budget is loose, and its
apply is bandwidth bound, so narrower storage converts directly into speed. This is the
highest-yield low-risk idea in the current literature.

Where a kernel *names* a precision, that precision is the algorithm and is not a tuning knob.
Mixed-precision iterative refinement is the clear case: the factorization is deliberately the low
precision because it is the expensive cubic step, and the residual is deliberately the high one.
Computing that residual in the low precision raises nothing, costs nothing, and stalls the
refinement about nine orders of magnitude short of the accuracy it reports as converged.

- **Backward error cannot detect a precision change; forward error can.** Backward error for a
  factorization is small regardless of conditioning. Forward error tracks the condition number and
  separates an fp32 factorization from an fp64 one cleanly.
- **Know the conditioning bound.** From Carson & Higham's three-precision iterative refinement: with
  an fp16 factorization, plain LU-based refinement is only guaranteed to reduce the error for
  `cond(A) << 2e3`, while GMRES-IR, solving the correction equation by GMRES preconditioned with
  those same LU factors, extends the guarantee to `cond(A) << 3e7`. If a low-precision factorization
  is not converging, that is the fix, not more refinement steps.
- Index arrays stay 64-bit. A narrower index silently wraps on large sizes rather than failing.

## Sources

- Saad, *Iterative Methods for Sparse Linear Systems*, 2nd ed. (SIAM, 2003).
- Benzi, "Preconditioning techniques for large linear systems: a survey", *JCP* 182(2), 2002.
- Chow & Patel, "Fine-grained parallel incomplete LU factorization", *SISC* 37(2), 2015; Anzt, Chow
  & Dongarra, "ParILUT", *SISC* 40(4), 2018.
- Anzt, Chow & Dongarra, "Iterative sparse triangular solves for preconditioning", Euro-Par 2015;
  Anzt et al., *JPDC* 2018.
- Berger-Vergiat, Kelley, Rajamanickam et al., "Two-stage Gauss-Seidel preconditioners and smoothers
  for Krylov solvers on a GPU cluster", arXiv:2104.01196.
- Anzt et al., "Adaptive precision in block-Jacobi preconditioning", *CCPE* 2019, and ACM *TOMS*
  47(2), 2021.
- Anzt, Huckle, Bräckle & Dongarra, "Incomplete sparse approximate inverses for parallel
  preconditioning", *Parallel Computing* 71, 2018.
- Adams, Brezina, Hu & Tuminaro, "Parallel multigrid smoothing: polynomial versus Gauss-Seidel",
  *JCP* 188(2), 2003.
- Vanek, Mandel & Brezina, "Algebraic multigrid by smoothed aggregation", *Computing* 56, 1996;
  Briggs, Henson & McCormick, *A Multigrid Tutorial*, 2nd ed. (SIAM, 2000); hypre's BoomerAMG manual.
- Carson & Higham, "Accelerating the solution of linear systems by iterative refinement in three
  precisions", *SISC* 40(2), 2018; Higham, *Accuracy and Stability of Numerical Algorithms*, 2nd ed.
- Loe, Morgan et al., "Toward efficient polynomial preconditioning for GMRES", 2021-2022.
- Knoll & Keyes, "Jacobian-free Newton-Krylov methods", *JCP* 193(2), 2004.
