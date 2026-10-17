---
name: solver-preconditioners
description: "Decides whether a solver kernel's preconditioner may be changed or is frozen by what is graded, then either makes the given preconditioner fast without changing what it computes, or chooses a better one. Covers preconditioned CG and GMRES, Gauss-Seidel and SOR sweeps, sparse triangular solves, incomplete factorizations, multigrid, AMG setup, Newton-Krylov, iterative refinement, Lanczos and implicit or adaptive integrators."
when: "ALWAYS read this page before you parallelize or restructure a solver kernel: a Krylov, Newton, multigrid, Gauss-Seidel/SOR, triangular-solve, factorization, Lanczos or time-integration loop. It lists what is legal and what makes such kernels fast"
---

# Preconditioning a graded solver kernel

The literature judges a preconditioner by iteration count. A graded kernel is judged elementwise
against one reference computation. **Those two standards disagree by six to ten orders of
magnitude**, so the first question is never "which preconditioner is best" but "may I change it at
all".

Compiling, running, converging and producing plausible numbers prove nothing here. A wrong
preconditioner converges.

Every table on this page is a run that was already done: the change tried, the outcome, and whether
it was good or bad. The NumPy code of those runs is in the Samples section at the end. Read them as
patterns; there is nothing to run.

## Workflow

```
- Step 1: Freedom verdict    -- may the preconditioner change?
- Step 2: Classify operator  -- symmetry, singularity, spectrum, spread, ordering
- Step 3: Choose the transform
- Step 4: Verify agreement   -- budget ratio, across the input RANGE
- Step 5: Account for cost   -- setup included, two problem sizes
```

Steps 1, 4 and 5 are where correctness is lost: follow them exactly. Steps 2 and 3 are judgment.

---

## Step 1: Freedom verdict

Ask three questions. Any single "yes" freezes the preconditioner.

**1. Is the output a fixed iterate rather than a converged answer?** A tolerance-driven kernel
returns the solution, so any converging preconditioner returns the same thing. A fixed trip count
returns iterate `k` of one recurrence, and the preconditioner is part of that recurrence. Look for a
loop bound that is a parameter (`niter`, `ncycles`, `nsteps`, `nsweeps`) rather than a residual
test.

**2. Is any graded output a count, a flag or a history?** Step counts, accept/reject tallies, order
histories, convergence flags. A count is compared like any other array, so an off-by-one is a
factor-of-two relative error.

**3. Does a tolerance loop have a cap that binds?** A kernel written "iterate until `||F|| <= tol`,
at most `max_steps`" is tolerance-driven where it converges and fixed-iterate where it does not.

Worked example: Jacobian-free Newton-Krylov on `-Lap(u) + u^3 = f` on an `N x N` grid, stopping at
`||F|| <= 1e-12 ||F0||` with at most 20 Newton steps, one GMRES(30) cycle per step, inner tolerance
1e-4. The fast-Poisson column is the same solve with a fast-Poisson preconditioner on the inner
GMRES, compared with the reference's output:

| N | Newton steps used | final residual ratio | | fast-Poisson: Jacobian applications | fast-Poisson: budget |
|---|---|---|---|---|---|
| 16 | 7 / 20 | 1.6e-14 | converged | 30 instead of 170 | 1.1e-3 **good** |
| 32 | 10 / 20 | 1.4e-13 | converged | 31 instead of 300 | 1.5e-2 **good** |
| 64 | 17 / 20 | 4.2e-13 | converged | 30 instead of 510 | 0.18 **good** |
| 96 | 20 / 20 | 3.0e-08 | **capped: the output is a fixed iterate** | 31 instead of 600 | 1.6e+4 **bad** |
| 128 | 20 / 20 | 3.1e-05 | **capped** | 31 instead of 600 | 1.8e+7 **bad** |

At N=128 the preconditioned solve converges to a strictly *better* answer and still misses by
**1.8e+7 times the budget**. Same change, opposite verdict, decided only by which side of the cap
the input fell on. Testing one small size would have called it legal.

> **MUST** establish where the cap binds across the whole legal input range before touching the
> preconditioner. Graded inputs are drawn from the whole range, not from the size you tested.

**Verdict.** Three "no" answers mean the preconditioner is yours: Step 2, then the ladder in Step 3.
Any "yes" means it is frozen: make the given preconditioner fast without changing what it computes
(Step 3, frozen case).

### What the verdict usually returns

Solver kernels come in a few shapes, and the verdict follows from the shape:

| kernel pattern | what is typically graded | verdict |
|---|---|---|
| Krylov iteration with a fixed sweep count | the iterate | frozen: the preconditioner is in the recurrence |
| multigrid cycle with a fixed cycle count | the corrected solution | frozen: smoother, cycle type, damping weight |
| sparse triangular solve | the solution vector | frozen: the solve *is* the preconditioner apply |
| incomplete factorization | the factor, on its own pattern | frozen: the factorization *is* the setup |
| coloured or ordered relaxation sweep | the relaxed field | frozen: the ordering is the algorithm |
| hierarchy setup, aggregation, coarsening | level sizes, nonzero counts | frozen: graded on the hierarchy's shape |
| direct factorization with a supplied ordering | the solution | frozen: the ordering is an input |
| Krylov basis construction | the basis and its coefficients | unpreconditioned by construction |
| dense factorization | the factors | no preconditioner involved |
| iterative refinement reporting a step count | the solution and the count | frozen by the count |
| implicit integrator reporting order or step history | the state and the histories | frozen by the histories |
| adaptive integrator reporting accept/reject tallies | the state and the counts | frozen by the counts |
| explicit ensemble integrator | the states | no preconditioner involved |
| Newton-Krylov to a tolerance under an iteration cap | the converged state | open below the cap, frozen above it |

Almost everything comes back frozen. That follows from elementwise grading itself: a reference picks
one route to the answer, and a grader that compares iterates asks you to reproduce that route. **The
work that scores is making the given preconditioner fast, not choosing a better one.**

### What a frozen preconditioner costs if you change it anyway

Preconditioned CG on a 9-point (8-neighbour) grid operator with edge weights log-uniform on [1,
100], `x0 = 0`, fixed sweep count, only the preconditioner swapped. Figures are the worst
per-element violation of `|a - e| <= atol + rtol*|e|` as a multiple of the budget at fp64. **Passes
only at <= 1.0.**

| substitution | 64², 25 sweeps | 128², 50 sweeps | outcome |
|---|---|---|---|
| level-scheduled SGS, same row ordering | 1.5e-4 | 6.1e-4 | **good**: passes |
| Jacobi instead of SGS | 5.0e+10 | 4.9e+10 | bad |
| two-stage SGS, 3 inner Jacobi sweeps | 2.7e+9 | 3.0e+9 | bad |
| two-stage SGS, 10 inner Jacobi sweeps | 2.4e+6 | 2.2e+6 | bad |
| multicolour SGS, 4 colours | 6.7e+9 | 7.2e+9 | bad |

Row four matters most: ten inner Jacobi sweeps reproduce the reference's **iteration count exactly**
(74 and 126 iterations to 1e-8, like SGS) and are still six orders outside the band. Only the first
row passes.

> **MUST NOT** use an iteration count as evidence that a substitution is numerically safe. Matching
> a convergence rate is not matching an iterate.

---

## Step 2: Classify the operator

Cheap to establish, and it decides everything downstream.

- **Symmetry and definiteness.** SPD takes CG, and the preconditioner must stay symmetric and
  positive definite or the recurrence loses its meaning. Symmetric indefinite takes MINRES.
  Nonsymmetric takes restarted GMRES or BiCGStab. Block or saddle-point structure is the whole
  problem and must not be treated purely algebraically.
- **Singularity.** Check it, do not trust the documentation: `||A·1||_inf / ||A||_inf` costs one
  matvec. A graph Laplacian built as `A_ii = sum_j w_ij`, `A_ij = -w_ij` sums to zero in every row.
  Example: an 8-neighbour grid graph Laplacian gave 1.0e-16, singular to machine precision with the
  constant vector in its null space. Documentation that calls such an operator positive definite is
  wrong, and an incomplete factorization of it can break down.
- **The spectrum, at both ends.** `lambda_max` from a few Lanczos steps, the smallest *nonzero*
  eigenvalue by shift-invert. On that operator `lambda_max / lambda_2` was 1,347 at 32² and 5,347 at
  64², the `k²` growth of a Laplacian. This ratio, not the formal condition number, decides which
  families can work.
- **Coefficient spread.** On a constant-coefficient operator, diagonal preconditioning is a scalar
  rescale worth exactly 1.00x. Jumping coefficients are what make the choice matter.
- **Ordering and sparsity structure.** Decides how much parallelism the given preconditioner can
  expose; see the level structure below.

---

## Step 3: Choose the transform

### If the preconditioner is frozen

> **Level scheduling preserves the mathematics. Reordering does not.**

Level scheduling groups rows the analysis *proved* independent, so every row computes the identical
value; the only difference from the sequential sweep is reassociation inside a row's reduction
(measured at 1.5e-4 of the budget, almost four orders of margin). Multicolouring changes which
values a row reads. That is different mathematics (measured at 6.7e+9, ten orders outside). Both are
called "parallelizing Gauss-Seidel"; only one parallelizes *your* Gauss-Seidel. Keep the analysis
phase out of the timed region where the kernel allows it.

**Compute the level structure, never infer it.** It is a property of the *ordering*, not of the size
or the nonzero count:

```
level[i] = 1 + max(level[j] : j < i, A[i,j] != 0)      (0 if row i has no such j)
```

Then look at the number of levels (depth: each is a synchronization point) and the rows per level
(width: what can run concurrently).

Worked example, a 9-point (8-neighbour) operator on a `k²` grid in lexicographic order:

| grid | N | levels | avg rows/level | max rows/level |
|---|---|---|---|---|
| 32² | 1,024 | 94 | 11 | 16 |
| 64² | 4,096 | 190 | 22 | 32 |
| 128² | 16,384 | 382 | 43 | 64 |
| 256² | 65,536 | 766 | 86 | 128 |

Exactly `levels = 3k - 2` and `max rows/level = k/2`, average about `k/3`: the depth grows like the
width while the problem grows quadratically, so a lexicographic 2-D sweep offers little parallelism
per barrier. Other stencils and dimensions give other laws (a 3-D stencil's width grows
quadratically in `k`); compute yours. Whether the trade pays is a property of the target machine,
not of the matrix: compare the width with the parallelism the device needs and the depth with the
cost of a barrier there.

Unstructured operators from applications are usually worse: level sizes are wildly imbalanced, with
a few huge levels next to hundreds of levels holding a handful of rows, so a per-level barrier
spends most of its time on near-empty levels. That is the case for a **synchronization-free
triangular solve**: one thread or warp per row, waiting on a per-row ready flag, no global barriers.
A row runs as soon as its dependencies are done. It computes the same values as the sequential
solve.

A level structure is only useful in a band. Many levels of one row each is a serial chain; a handful
of levels over half a million rows needs no schedule at all.

**Transforms that are always legal**, whatever is frozen:

- **Reassociate a reduction.** A parallel reduction necessarily reassociates, and the grading band
  covers it.
- **Batch an orthogonalization into BLAS-2/3.** A Gram-Schmidt sweep written as `2(j+1)` separate
  dot-and-axpy passes becomes one GEMV pair. That turns modified into classical Gram-Schmidt, which
  is different arithmetic, but where the reorthogonalization is already done twice it holds. Tried
  on a Lanczos iteration over a 9-point operator, 64² grid: **good**, the basis within 4.7e-5 of the
  budget at m=50, but 1.1e-3 at m=100. The drift grows much faster than the step count, so check the
  largest m the task allows. Usually the biggest legal win in a Krylov or Lanczos kernel.
- **Call a library routine that computes the same mathematics**, if the build links it. Tried:
  LAPACK's `dgeqrf`/`dorgqr` against a textbook Householder QR of a random 1500 x 80 matrix:
  **good**, R within 5.9e-4 of the budget and Q to 4.4e-15, because LAPACK uses the textbook
  reflector sign convention. Watch the conventions: economy versus full Q, and what is left below
  the diagonal.
- **Tile, fuse, block, vectorize, change the layout, thread a loop that carries no dependence.**

**Transforms that make frozen solver kernels fast on CPUs.** Each keeps the reference's arithmetic
per element, so it passes where the reference's own method passes. They are listed by the loop
structure they apply to.

| loop structure | transform | why it pays |
|---|---|---|
| a fixed number of sweeps of a local stencil update (Jacobi, red-black or SOR relaxation) | **temporal blocking**: tile space *and* sweeps. Copy a tile plus a halo one row wider per remaining sweep into a private cache-sized buffer, run all sweeps of the time tile there, recompute halos redundantly, write back only core rows. One parallel loop per time tile replaces a barrier per sweep | the sweeps run out of L2 instead of streaming memory every sweep, and barriers drop by the tile depth |
| a sequential sweep or triangular solve on a structured grid | **asynchronous wavefront**: group rows into lines in wavefront order; each line waits on ready flags of the lines it reads; no global barrier. Keep the per-row arithmetic in the reference's order | dependences are honored one line at a time instead of one level at a time, so no thread idles at a global barrier |
| an outer Newton, Krylov or time loop around short vector passes | **one persistent thread team**: open the parallel region once around the whole solve, run the scalar control flow redundantly on every thread, use `omp for` for the work loops, and sum reductions through per-thread partial slots in a fixed slot order before a barrier; a sense-reversing spin barrier beats a sleeping barrier when passes are short | thread start-up and sleeping barriers, not arithmetic, dominate short passes; the fixed slot order keeps reductions identical run to run |
| a dense factorization with rank-1 updates (Householder QR, LU) | **blocked BLAS-3**: accumulate a panel and apply it to the trailing matrix with GEMM (and SYRK), with the same reflectors, pivots and signs as the reference; in mixed-precision refinement, block the declared low-precision factorization on the low-precision GEMM | rank-1 updates are memory bound; a GEMM trailing update runs near peak |
| many independent small systems (ensembles of ODEs, batched small solves) | **SIMD across instances**: one system per vector lane, per-lane masks for accept/reject, step clipping and loop exit, so every lane follows its own scalar trajectory | the instances are the parallelism; threads alone leave the vector units idle |
| repeated passes over a stored basis or a strided array | **layout for streaming**: keep a transposed or packed copy so every pass is contiguous; write the graded layout once at the end | strided passes waste most of each cache line |
| multigrid cycles | **cycle bookkeeping**: fuse residual and restriction, ping-pong smoother buffers instead of copying back, specialize the first sweep from a zero guess, run tiny coarse levels on one thread | coarse levels are latency bound; fusion and fewer copies cut memory traffic on the fine ones |
| a factorization where every row needs all its parents (ILU, IC) | **lean serial**: precompute each row's update positions once and skip dense scatter/gather and marker checks; parallelize only if the level structure is wide | with thin levels, synchronization costs more than it buys |

Also legal and cheap: threads equal to physical cores; integer prefix sums and counting passes
parallelize freely, because any partition gives the same integers.

**The one-sentence test.** State why every row in your parallel region computes the value it
computed sequentially. "The analysis proved them independent" is scheduling: legal. "I coloured
them" is reordering: the mathematics changed. If you cannot write the sentence, the transform is a
substitution and must pass Step 4 before it is trusted.

### If the preconditioner is yours

Start from the default for the operator class; the ladder below is the escape hatch.

| operator | default | escape hatch |
|---|---|---|
| elliptic, any size worth optimizing | multigrid; smoothed aggregation for jumping coefficients | unsmoothed aggregation if setup dominates |
| elliptic with a constant-coefficient part | fast-transform (DST/FFT) solve of that part | multigrid |
| general sparse SPD, GPU | block Jacobi, blocks following the graph | ISAI, then ILU/IC |
| general sparse SPD, CPU | IC(0) | ILU with fill, if setup amortizes |
| Gauss-Seidel smoothing needed on a GPU | two-stage GS, 3 inner Jacobi sweeps | multicolour GS |
| nonsymmetric, well conditioned | polynomial (Chebyshev or GMRES-polynomial) | ILU(0) |
| a sequence of related solves | reuse or recycle the subspace, deflate | rebuild each time |

The shape of each trade, CG to a relative residual of 1e-8 on the 9-point operator above (an
example, not numbers to transplant):

| preconditioner | iters @64² | iters @128² | vs none |
|---|---|---|---|
| none | 232 | 433 | 1.00x |
| Jacobi | 179 | 320 | 1.30-1.35x |
| symmetric Gauss-Seidel | 74 | 126 | 3.1-3.4x |
| two-stage SGS, 1 inner Jacobi | 179 | 320 | identical to Jacobi |
| two-stage SGS, 2 inner Jacobi | 98 | 173 | 2.4-2.5x |
| two-stage SGS, 3 inner Jacobi | 81 | 141 | 2.9-3.1x |
| two-stage SGS, 10 inner Jacobi | 74 | 126 | matches SGS |
| multicolour SGS, 4 colours | 83 | 154 | 2.8x, +12-22% iterations over SGS |
| block Jacobi, contiguous block 8 | 166 | 294 | 1.4-1.5x |
| block Jacobi, contiguous block 64 | 155 | 290 | 1.5x |
| ILU(0) on the matrix pattern | 48 | 84 | 4.8-5.2x |

The ladder, cheapest first:

- **Jacobi.** One divide; the free baseline. Exactly 1.00x on a constant-coefficient operator.
- **Block Jacobi.** Blocks must follow the graph or the physics, never the index range: blocks cut
  from a lexicographic ordering capture only the fastest-varying direction's coupling, so even block
  64 buys barely more than plain Jacobi (1.5x against 1.3x above). Its virtue is real: a batched
  block inverse carries no dependences, and its apply is bandwidth bound, so lower-precision block
  storage can help.
- **Gauss-Seidel and SOR.** The workhorse smoother, sequential in row order. Symmetric GS (forward
  sweep, scale by D, backward sweep) is the SPD-safe form a CG recurrence requires.
- **Multicolour GS.** Each colour reads only the other colours, so a half-sweep is data parallel.
  Costs a colouring in setup and more iterations (a greedy distance-1 colouring of the 8-neighbour
  graph gives 4 colours and 12-22% more iterations than SGS). Half-sweeps stay ordered; never fuse
  them.
- **Two-stage GS.** Replace each exact triangular solve with a few inner Jacobi sweeps so the apply
  becomes SpMVs. Above: 3 sweeps get most of the way, 10 match SGS. **One inner sweep IS Jacobi**:
  from a zero start it gives `y = r/D` and the triangular part never enters. Damp the inner sweep if
  the spectral radius of `D^-1 L` approaches 1.
- **ISAI / sparse approximate inverse.** Approximate `L^-1` and `U^-1` on a fixed pattern so the
  apply is two SpMVs with no dependences. The answer when the triangular solve dominates and the
  level structure is hostile.
- **IC(0) / ILU(0).** The strongest local method here, 4.8-5.2x. Fill is the knob: fewer iterations,
  more memory and setup. **Breakdown is silent**: a zero or negative pivot produces NaNs downstream,
  so assert pivots. Setup is sequential with the same level structure as the solve. Fine-grained
  parallel ILU (ParILU, ParILUT) converges *toward* the sequential factor, never to it elementwise.
- **Polynomial / Chebyshev.** Pure SpMV and axpy, the parallel multigrid smoother of choice, but it
  needs a lower spectral bound and is weak on elliptic operators: the eigenvalues that cost the
  iterations sit near zero and fall as the grid refines, where a polynomial cannot separate them.
  Strong on well-conditioned nonsymmetric systems.
- **Multigrid.** The only family whose iteration count does not grow with problem size. A
  strength-of-connection threshold **belongs to its strength measure and does not transfer**: the
  familiar 0.25 normalizes by the row's largest off-diagonal; the smoothed-aggregation measure
  `|A_ij| > theta * sqrt(|A_ii| |A_jj|)` does not: where the diagonal sums the weights of n
  neighbours, a typical coupling under that measure is about 1/n, so with many neighbours 0.25
  admits nothing and nothing coarsens. Choose theta below 1/n. Give aggregation the right near-null
  space. Parallelism collapsing toward the coarsest grid is inherent; do not skip levels. A
  hierarchy that barely coarsens converges in *fewer* iterations while each cycle costs tens of
  fine-grid applications: operator complexity catches it, iteration count does not.
- **Fast transform.** If the operator has a constant-coefficient elliptic part, a DST/FFT solve of
  that part is its exact inverse in O(N log N). On the Newton example in Step 1: 510 Jacobian
  applications down to 30 at N=64. Check for it before reaching for anything algebraic.
- **Deflation and recycling.** Project out the smallest eigenvectors, or carry a Krylov subspace
  across a Newton or time-stepping sequence of related systems.
- **Precision.** Storing or applying the preconditioner in lower precision while the Krylov
  iteration stays in working precision is the highest-yield low-risk idea in the literature. But
  where the kernel *names* a precision, that precision is the algorithm: in mixed-precision
  iterative refinement the factorization is deliberately low and the residual deliberately high, and
  computing the residual low stalls refinement at the low precision's accuracy instead of the high
  one's. Keep index arrays 64-bit.

Two defaults that are wrong more often than they look: polynomial preconditioning on an elliptic
operator, and block Jacobi with contiguous index blocks.

**Libraries.** Link a routine that computes the same mathematics, if the target build provides it.
Read solver frameworks (PETSc, hypre, Ginkgo, Trilinos, SUNDIALS) for the algorithm, but do not
submit their answer: an algebraic multigrid package returns a different, equally valid hierarchy
that matches nothing. A heavily tuned benchmark implementation is a good model for its operator
application and data layout, not for its preconditioner: such codes typically multicolour the
smoother because they are scored on their own convergence criterion, not on a reference iterate.

---

## Step 4: Verify agreement

**The budget ratio** of one graded output is

```
max over elements of  |got - ref| / (atol + rtol * |ref|)        <= 1.0 passes
```

with the task's own `rtol` and `atol`. Report error as this multiple, never as "close".

1. **The grader's correctness check is the verification.** If the harness offers a preview of the
   grade, run it on every candidate: it is the only check that runs your code the way it is graded.
   A local comparison against the reference can help bisect a failure; it does not replace the
   grader.
2. **Cover the range.** A preview runs a few fixed inputs; graded inputs are drawn from the whole
   declared range. Anything that depends on size (a binding cap, a level structure, a step count)
   must be argued for the whole range, not read off the preview sizes.
3. **If a result is out of band, classify it by the ratio before changing anything:**

| ratio | almost certainly | what to do |
|---|---|---|
| 1 to ~10 | reassociation at the tolerance boundary, marginal accumulation order | tighten the accumulation, not the tolerance |
| ~10 to 1e3 | a genuine bug: an index, a boundary, a missed term | debug it as a bug |
| 1e3 to 1e6 | a different but convergent algorithm, or a changed step count | you changed the mathematics |
| above 1e6 | a different preconditioner, ordering or recurrence | it was frozen |

The causes, only some of them yours: a frozen-preconditioner violation; reordering mistaken for
scheduling; a **cap-boundary flip** (small ratio at small sizes, enormous at large ones); a graded
count that drifted while the floats agree; a genuine bug (index, boundary, sign, missed term,
uninitialized scratch); accumulation order (O(1) to O(10), never more); the toolchain (fast-math
reassociation, FMA contraction, a different library bound at load time, a non-deterministic threaded
reduction). Name the cause before fixing it.

> **MUST NOT** loosen a tolerance, lower an iteration cap or shrink a problem to obtain a pass.
> A change that needs a looser band is a change that is wrong.

> **MUST NOT** reshape correct mathematics to hide a toolchain failure. Fix the toolchain.

**Do not lose a good version.** Three common ways to lose a good result: submitting a version the
grader never checked, which then fails on held-out inputs; ending after a diagnostic experiment has
overwritten the best file, so a slower version is graded; and stopping while untried transforms
remain. Check every candidate with the grader, keep the best checked version in the file the grader
reads (experiments go in other files), submit only a version the grader has already passed, and keep
going while a transform from Step 3 is still untried.

**Defects in the reference.** A near-zero pivot, a documented type the data contradicts, an operator
claimed definite that measures singular. Do not silently reproduce it and do not silently repair it:
preserve well-defined behavior, and say so if you deliberately deviate.

---

## Step 5: Account for cost

```
T = T_setup + N_iter * T_apply
```

A stronger preconditioner charges twice, to build and on every application, and only `N_iter` shows
in an iteration count. **Iteration count is a diagnostic; time to a fixed accuracy with setup
counted is the objective.**

- Take timings from the harness that grades the code, on the machine it grades on. A local timing on
  a different node, core count or toolchain does not transfer.
- Include setup in any number that motivates a preconditioner change, and check two problem sizes:
  grid independence, or its absence, cannot be seen at one.

---

## Failure catalogue

Moves that look like optimizations and are not, in a kernel graded on an iterate:

1. Turning a Gauss-Seidel sweep into a Jacobi sweep to remove the dependence.
2. Reordering a sweep by colouring, permutation or reversal. Natural and red-black ordering are
   different trajectories that agree only once converged.
3. Replacing exact triangular solves with inner Jacobi sweeps, at any sweep count. Ten sweeps still
   miss by six orders.
4. Fusing two coloured half-sweeps. The second reads what the first wrote.
5. Substituting a fine-grained parallel incomplete factorization when the factor itself is graded.
6. Changing a smoother, cycle type, damping weight or sweep count inside a fixed cycle count.
7. Skipping coarse levels because they hold little work.
8. Parallelizing a greedy aggregation, a fill-reducing ordering or supernode detection, then
   comparing the resulting operator: a different valid hierarchy, not this one.
9. Transplanting a strength threshold between strength measures.
10. Refactoring an operator inside a loop written to reuse one factorization.
11. Computing a refinement residual in the working precision instead of the declared one.
12. Changing anything that alters a graded count: steps, accept/reject tallies, order histories,
    convergence flags.
13. Forcing a uniform step count across an adaptive ensemble. The divergent control flow is the
    kernel.
14. Raising a tolerance, lowering a cap or shrinking a problem until a check passes.
15. Replacing the algorithm with a black-box solver call when the algorithm is what is measured.

## Hard rules

- Establish what is graded before choosing a preconditioner.
- A fixed trip count, a graded count, or a binding iteration cap freezes the preconditioner.
- Locate where a cap binds across the whole input range, not at one size.
- Check singularity with one matvec rather than trusting the documentation.
- Level scheduling preserves the mathematics; reordering does not. Know which one you did.
- Compute a level structure; never infer it from size or nonzero count.
- Matching a convergence rate is not matching an iterate.
- Setup counts, always. Check two problem sizes.
- Report error as a multiple of the grading budget.
- Never loosen a tolerance to obtain a pass.
- Blocks follow the graph, not the index range.
- A strength threshold belongs to its strength measure.
- Assert pivots; incomplete-factorization breakdown is silent.
- Where a precision is named, it is the algorithm, not a knob.
- Triage a disagreement to a named cause before fixing it.
- Keep the best grader-checked version in the graded file; never submit an unchecked one.

---

## Samples: runs already done

NumPy formulations of the runs behind the tables above, each with what was run, what came out, and
whether the change was good or bad. They are patterns to read and translate into the kernel's own
language, not scripts to call.

### Sample 1: the budget ratio

```python
RTOL, ATOL = 1e-9, 1e-11                       # the task's tolerances; these are fp64 examples
def budget(got, ref):                          # <= 1.0 passes
    return float(np.max(np.abs(got - ref) / (ATOL + RTOL * np.abs(ref))))
```

Every number on this page marked "of the budget" or "times the budget" is this function.

### Sample 2: swapping the preconditioner inside a fixed-sweep PCG

```python
def pcg_fixed(A, b, niter, M):                 # the reference recurrence, x0 = 0, FIXED niter
    x = np.zeros_like(b); r = b.copy()
    z = M(r); p = z.copy(); rz = r @ z
    for _ in range(niter):
        q = A @ p; alpha = rz / (p @ q)
        x += alpha * p; r -= alpha * q
        z = M(r); rz_new = r @ z
        p = z + (rz_new / rz) * p; rz = rz_new
    return x

# sgs_seq: sequential symmetric Gauss-Seidel, row by row (the reference);
# sgs_levels: the same two sweeps, level-scheduled (Sample 3);
# gs2(r, k): k inner Jacobi sweeps per triangle; mcsgs: greedy-coloured SGS
ref = pcg_fixed(A, b, niter, sgs_seq)
for name, M in [("level-scheduled SGS", sgs_levels), ("Jacobi", lambda r: r / d),
                ("two-stage SGS, 3 inner", lambda r: gs2(r, 3)),
                ("two-stage SGS, 10 inner", lambda r: gs2(r, 10)), ("multicolour SGS", mcsgs)]:
    print(name, budget(pcg_fixed(A, b, niter, M), ref))
```

Ran on a 9-point grid operator with edge weights log-uniform on [1, 100], 64² with 25 sweeps and
128² with 50. Outcome: level-scheduled SGS 1.5e-4 and 6.1e-4 (**good**, passes); Jacobi 5.0e+10,
two-stage with 3 inner sweeps 2.7e+9, with 10 inner sweeps 2.4e+6, multicolour 6.7e+9 (**bad**, all
fail), although two-stage with 10 inner sweeps needs exactly the reference's iteration count to
converge.

### Sample 3: level-scheduled triangular solve, the legal parallel form

```python
def levels(A):                                 # level[i] = 1 + max(level[j] : j < i, A[i,j] != 0)
    lev = np.zeros(A.shape[0], dtype=np.int64)
    for i in range(A.shape[0]):
        cols = A.indices[A.indptr[i]:A.indptr[i+1]]
        lower = cols[cols < i]
        lev[i] = lev[lower].max() + 1 if lower.size else 0
    return lev

def solve_lower_by_levels(L_strict, d, b, lev):
    x = np.zeros_like(b)
    for l in range(lev.max() + 1):             # levels run in order: each is a barrier
        rows = np.where(lev == l)[0]           # rows inside a level are independent: parallel
        x[rows] = (b[rows] - L_strict[rows] @ x) / d[rows]
    return x
# the backward sweep is the same with the upper triangle and levels computed from j > i
```

Ran as the forward and backward sweep of SGS in Sample 2. Outcome: identical values up to
reassociation inside each row, 1.5e-4 of the budget (**good**). The level counts it produced are the
`3k - 2` table in Step 3. Contrast multicolouring, which also gives parallel groups but changes
which values each row reads (**bad**, 6.7e+9).

### Sample 4: locating where an iteration cap binds

```python
def newton(N, f, cap=20, prec=None):           # stop at ||F|| <= 1e-12 ||F0||, at most `cap` steps
    u = np.zeros((N, N)); F = residual(u, N, f); f0 = norm(F); steps = 0
    for _ in range(cap):
        if norm(F) <= 1e-12 * f0: break
        du = gmres_one_cycle(u, F, restart=30, tol=1e-4, prec=prec)   # Jacobian-free J·v
        u += du; F = residual(u, N, f); steps += 1
    return u, steps, norm(F) / f0

for N in (16, 32, 48, 64, 96, 128):            # span the whole declared size range
    ref, steps, ratio = newton(N, f)
    fast, _, _ = newton(N, f, prec=fast_poisson(N))
    print(N, steps, ratio, budget(fast, ref))
```

Ran on `-Lap(u) + u^3 = f`, unit square, zero boundary. Outcome: the reference converges at N up to
64 (7 to 17 Newton steps) and is capped at N=96 and N=128 (20 of 20). The fast-Poisson
preconditioned solve needs about 30 Jacobian applications instead of 170 to 600 at every N, and
agrees with the reference within 0.18 of the budget up to N=64 (**good** there), but misses by
1.6e+4 at N=96 and 1.8e+7 at N=128 (**bad** there). Testing one small size would have called it
legal.

### Sample 5: the singularity check, one matvec

```python
ones = np.ones(A.shape[0])
print(np.abs(A @ ones).max() / abs(A).sum(axis=1).max())   # ||A·1||_inf / ||A||_inf
```

Ran on an 8-neighbour grid graph Laplacian (diagonal = sum of the edge weights). Outcome: 1.0e-16,
singular to machine precision with the constant vector in its null space (**bad** news for anything
that assumes definiteness: incomplete factorizations can break down, CG drifts in the null space).

### Sample 6: batching a reorthogonalization into a GEMV

```python
for _ in range(2):                             # the reference reorthogonalizes twice
    # reference, modified Gram-Schmidt: w changes inside the loop
    #   for p in range(j + 1): w -= (Q[:, p] @ w) * Q[:, p]
    # batched, classical Gram-Schmidt: one GEMV pair against the same w
    dots = Q[:, :j+1].T @ w
    w -= Q[:, :j+1] @ dots
```

Ran on a Lanczos iteration over the 9-point operator, 64² grid. Outcome: the basis within 4.7e-5 of
the budget at m=50 (**good**) but 1.1e-3 at m=100, still passing, with the drift growing far faster
than m: check the largest m the task allows. This was measured only with the pass done twice; where
the reference does a single pass, do not assume the same margin, because classical Gram-Schmidt in
one pass is known to lose orthogonality.
