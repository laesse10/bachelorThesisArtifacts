---
name: solver-preconditioners-ladder
description: "Chooses a preconditioner when the kernel allows changing it: defaults per operator class, the full ladder with what each family buys and costs, measured iteration counts, and library rules."
when: "the verdict says the preconditioner may change and you are choosing or tuning one"
---

# Choosing a preconditioner

A graded solver kernel is judged elementwise against one reference computation, not by iteration count. A preconditioner is **frozen** when the output is a fixed iterate (a fixed sweep, cycle or step count), when any graded output is a count, flag or history, or when a tolerance loop's iteration cap binds anywhere in the input range. The page `solver-preconditioners-verdict` decides which case you are in.

## If the preconditioner is yours

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
  that part is its exact inverse in O(N log N). On the Newton example on the page `solver-preconditioners-verdict`: 510 Jacobian
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

## Other pages of this skill

- `solver-preconditioners-verdict`: the first page to read for any solver kernel: whether the preconditioner may change
- `solver-preconditioners-frozen`: the preconditioner is frozen: make it fast without changing what it computes
- `solver-preconditioners-verify`: checking agreement and triaging a wrong answer
- `solver-preconditioners-samples`: NumPy code of the runs behind every table, with outcome and whether it was good or bad
