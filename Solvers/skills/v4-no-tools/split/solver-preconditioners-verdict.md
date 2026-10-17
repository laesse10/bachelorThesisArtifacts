---
name: solver-preconditioners-verdict
description: "Decides whether a solver kernel's preconditioner may be changed or is frozen by what is graded, classifies the operator, and lists the moves that look like optimizations and are not."
when: "ALWAYS read this page first, before you parallelize or restructure a solver kernel: a Krylov, Newton, multigrid, Gauss-Seidel/SOR, triangular-solve, factorization, Lanczos or time-integration loop"
---

# Preconditioning a graded solver kernel: the verdict

The literature judges a preconditioner by iteration count. A graded kernel is judged elementwise
against one reference computation. **Those two standards disagree by six to ten orders of
magnitude**, so the first question is never "which preconditioner is best" but "may I change it at
all".

Compiling, running, converging and producing plausible numbers prove nothing here. A wrong
preconditioner converges.

Every table on this page is a run that was already done: the change tried, the outcome, and whether
it was good or bad. The NumPy code of those runs is on the page `solver-preconditioners-samples`. Read them as
patterns; there is nothing to run.

## Workflow

```
- Freedom verdict    -- may the preconditioner change?          (this page)
- Classify operator  -- symmetry, singularity, spectrum, spread  (this page)
- Choose the transform  -- frozen: solver-preconditioners-frozen
                        -- yours:  solver-preconditioners-ladder
- Verify agreement   -- budget ratio across the input range      (solver-preconditioners-verify)
- Account for cost   -- setup included, two problem sizes        (this page)
```

## Freedom verdict

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

**Verdict.** Three "no" answers mean the preconditioner is yours: classify the operator
(below), then the page `solver-preconditioners-ladder`.
Any "yes" means it is frozen: make the given preconditioner fast without changing what it
computes: the page `solver-preconditioners-frozen`.

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

## Classify the operator

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
  expose; see the page `solver-preconditioners-frozen`.

## Account for cost

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

## Other pages of this skill

- `solver-preconditioners-frozen`: the preconditioner is frozen: make it fast without changing what it computes
- `solver-preconditioners-ladder`: the preconditioner may change: which one to choose
- `solver-preconditioners-verify`: checking agreement and triaging a wrong answer
- `solver-preconditioners-samples`: NumPy code of the runs behind every table, with outcome and whether it was good or bad
