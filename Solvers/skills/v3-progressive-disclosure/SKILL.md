---
name: solver-preconditioners
version: 3
status: candidate -- not installed
description: "Decides whether a solver kernel's preconditioner may be changed or is frozen by what is graded, then either selects one from the full ladder (Jacobi, block Jacobi, Gauss-Seidel, multicolour, two-stage, ISAI, ILU/IC, ParILU, polynomial, multigrid, fast-transform, deflation) or parallelizes the given one without altering its mathematics. Use when optimizing, porting, or GPU-accelerating a solver kernel: preconditioned CG or GMRES, Gauss-Seidel or SOR smoothers, sparse triangular solves, incomplete factorizations, multigrid cycles, AMG setup, Newton-Krylov or JFNK, iterative refinement, Lanczos or Arnoldi. Also use when an optimized solver disagrees with its reference, or when choosing between PETSc, hypre, Ginkgo, cuSPARSE, rocSPARSE and a hand-written kernel."
when: the kernel applies M^-1, smooths, relaxes, factorizes incompletely, solves triangular, builds a multigrid hierarchy, or wraps any of those in a Krylov or Newton iteration
---

# Preconditioning a graded solver kernel

The literature judges a preconditioner by iteration count. A graded kernel is judged elementwise
against one reference computation. **Those standards disagree by eight or nine orders of
magnitude**, so the first job is never "which preconditioner is best" but "may I change it at all".

Compiling, running and producing plausible numbers proves nothing here. A wrong preconditioner
converges.

## Workflow

Copy this checklist and check items off as you go.

```
Preconditioner work:
- [ ] Step 1: Freedom verdict     -- may the preconditioner change?
- [ ] Step 2: Classify operator   -- symmetry, singularity, spectrum, spread, ordering
- [ ] Step 3: Choose the transform
- [ ] Step 4: Verify agreement    -- budget ratio across the input RANGE
- [ ] Step 5: Account for cost    -- setup included, two problem sizes
- [ ] Step 6: Report status
```

Steps 1, 4 and 5 are **low freedom**: follow them exactly, they are where correctness is lost.
Steps 2 and 3 are **high freedom**: judgment and the operator decide.

---

## Step 1: Freedom verdict

Ask three questions. Any single "yes" freezes the preconditioner.

**1. Is the output a fixed iterate rather than a converged answer?** A tolerance-driven kernel
returns the solution, so any converging preconditioner returns the same thing. A fixed trip count
returns iterate `k` of one recurrence, and the preconditioner is part of that recurrence. Look for
a loop bound that is a parameter (`niter`, `ncycles`, `TSTEPS`, `nsweeps`) rather than a residual
test.

**2. Is any graded output a count, a flag, or a history?** Step counts, accept/reject tallies,
order histories, convergence flags. A count is compared like any other array, so an off-by-one is
a factor-of-two relative error.

**3. Does a tolerance loop have a cap that binds?** A kernel written "iterate until
`||F|| <= tol`, at most `max_steps`" is tolerance-driven where it converges and fixed-iterate where
it does not. **Measured**, a Jacobian-free Newton-Krylov Bratu solve with `max_newton = 20`:

| N | Newton steps used | final ratio of residual norms | |
|---|---|---|---|
| 32 | 5 / 20 | 1.9e-12 | converged |
| 64 | 14 / 20 | 7.1e-11 | converged |
| 96 | 20 / 20 | 1.0e-05 | **capped: output is a fixed iterate** |
| 128 | 20 / 20 | 7.7e-04 | **capped** |

A fast-Poisson preconditioner on that solve's inner GMRES cuts Jacobian applications from 204 to
16 and passes at N=32 at 1.3e-3 of budget. At N=128 it converges to a strictly better answer and
misses by **1.2e+6 times the budget**. Same change, opposite verdict, decided only by which side of
the cap the input fell on.

> **MUST** locate where the cap binds across the whole legal input range before touching the
> preconditioner. Not at the one size you happen to be testing.

**Verdict:** all three "no" means the preconditioner is yours, go to Step 2 then
[reference/ladder.md](reference/ladder.md). Any "yes" means it is frozen and your job is
[reference/parallelizing.md](reference/parallelizing.md): make the given preconditioner fast
without changing what it computes.

### What "frozen" costs

Preconditioned CG on a 27-point variable-coefficient operator, `x0 = 0`, fixed sweep count, only
the preconditioner swapped. Figures are the worst per-element violation of
`|a - e| <= atol + rtol*|e|` as a multiple of the budget at fp64. **Passes only at <= 1.0.**

| substitution | 16³, 25 sweeps | 32³, 50 sweeps |
|---|---|---|
| level-scheduled SGS, same row ordering | 2.5e-5 | 4.3e-5 |
| Jacobi instead of SGS | 1.3e+9 | 1.3e+9 |
| two-stage SGS, 3 inner Jacobi sweeps | 4.3e+8 | 4.4e+8 |
| two-stage SGS, 10 inner Jacobi sweeps | 1.9e+6 | 2.3e+6 |
| multicolour SGS, 8 colours | 8.7e+8 | 9.2e+8 |

Row three matters most: three inner Jacobi sweeps reproduce the reference's **iteration count
exactly** and are still eight orders outside the band.

> **MUST NOT** use an iteration count as evidence that a substitution is numerically safe.
> Matching a convergence rate is not matching an iterate.

Only the first row passes. [reference/parallelizing.md](reference/parallelizing.md) explains why it
is the one transform that always does.

---

## Step 2: Classify the operator

Cheap to establish, and it decides everything downstream.

- **Symmetry and definiteness.** SPD takes CG and the preconditioner must stay symmetric and
  positive definite, or the recurrence loses its meaning. Symmetric indefinite takes MINRES.
  Nonsymmetric takes restarted GMRES or BiCGStab. Block or saddle-point structure is the whole
  problem and must not be viewed purely algebraically.
- **Singularity.** Test it, do not trust the documentation. `||A·1||_inf / ||A||_inf` costs one
  matvec. A graph Laplacian built as `A_ii = sum_j w_ij`, `A_ij = -w_ij` sums to zero in every row.
  **Measured** on such an operator: 4.2e-16, singular to machine precision with the constant vector
  in its null space, despite a docstring claiming positive definite.
- **The spectrum, at both ends.** `lambda_max` by a few Lanczos steps, the smallest *nonzero*
  eigenvalue by shift-invert. **Measured** on that operator, `lambda_max / lambda_2` runs 196 at
  16³ and 773 at 32³, the `k²` growth of a Laplacian. This ratio, not the formal condition number,
  decides which families can work at all.
- **Coefficient spread.** On a constant-coefficient operator, diagonal preconditioning is a scalar
  rescale worth exactly 1.00x. Jumping coefficients are what make the choice measurable.
- **Ordering and sparsity structure.** Decides how much parallelism the given preconditioner can
  expose. Measure it: see [reference/parallelizing.md](reference/parallelizing.md).

---

## Step 3: Choose the transform

### If the preconditioner is frozen

Read [reference/parallelizing.md](reference/parallelizing.md). One rule governs everything there:

> **Level scheduling preserves the mathematics. Reordering does not.**

Level scheduling groups rows the analysis *proved* independent, so every row computes the identical
value. Multicolouring changes which values a row reads. Both are called "parallelizing
Gauss-Seidel"; only one is a parallelization of *your* Gauss-Seidel.

### If the preconditioner is yours

Start from the default for your operator class, and treat the ladder as the escape hatch.

| operator | default | escape hatch |
|---|---|---|
| elliptic, any size worth optimizing | multigrid, smoothed aggregation for jumping coefficients | if setup dominates, unsmoothed aggregation |
| elliptic with a constant-coefficient part | fast transform (DST/FFT) solve of that part | multigrid |
| general sparse SPD, GPU | block Jacobi, blocks following the graph | ISAI, then ILU/IC |
| general sparse SPD, CPU | IC(0) | ILU with fill, if setup amortizes |
| Gauss-Seidel smoothing needed on GPU | two-stage GS, 3 inner Jacobi sweeps | multicolour GS |
| nonsymmetric, well conditioned | polynomial (Chebyshev or GMRES-polynomial) | ILU(0) |
| a sequence of related solves | reuse or recycle the subspace, deflate | rebuild each time |

Full ladder with what each buys, costs and fails at, plus the measured iteration counts:
**[reference/ladder.md](reference/ladder.md)**.

Two defaults that are wrong more often than they look:

- **Polynomial preconditioning on an elliptic operator.** It damps a bounded interval away from
  the origin, and the eigenvalues that cost the iterations sit near zero and fall as the grid
  refines. **Measured** at degree 16: 1.24x, indistinguishable from plain Jacobi.
- **Block Jacobi with contiguous index blocks.** Blocks must follow the graph. **Measured**, blocks
  cut from a 3-D lexicographic ordering give 1.25x at block 8 and get *worse* at block 64.

### Library or hand-written

Link a library routine that computes the same mathematics; mine a solver framework for the
algorithm but do not submit its answer. Tables and the reasoning:
**[reference/libraries.md](reference/libraries.md)**.

---

## Step 4: Verify agreement

**Low freedom. Run exactly this loop until it passes.**

```python
import numpy as np
RTOL, ATOL = 1e-9, 1e-11                      # fp64; use 1e-3 / 1e-5 for fp32

def budget(got, ref):                         # <= 1.0 passes
    return float(np.max(np.abs(got - ref) / (ATOL + RTOL * np.abs(ref))))
```

1. Run the reference and the candidate on **identical** deterministic inputs.
2. Print `budget(...)` for every graded output, at no fewer than two sizes spanning the legal range.
3. If any ratio exceeds 1.0, triage it with [reference/triage.md](reference/triage.md) before
   changing anything. The ratio band itself identifies the cause.
4. Fix the named cause and return to step 1.
5. Proceed only when every ratio passes at every size tested.

> **MUST NOT** loosen a tolerance to obtain a pass. Tolerances come from the datatype and the
> operation structure. A change that needs a looser band is a change that is wrong.

> **MUST NOT** reshape correct mathematics to hide a toolchain or backend failure. Fix the
> toolchain, or report the limitation as independent of the kernel.

Runnable validators for the structural checks, each executable rather than reference reading:

```bash
python measurements/01_level_structure_stencil.py          # levels, rows per level
python measurements/03_substitution_vs_graded_band.py      # budget ratio per substitution
python measurements/07_jfnk_convergence_crossover.py       # where an iteration cap starts binding
```

See [measurements/README.md](measurements/README.md) for all seven and what each establishes.

---

## Step 5: Account for cost

    T = T_setup + N_iter * T_apply

A stronger preconditioner charges twice, to build and on every application, and only `N_iter` shows
in an iteration count.

> **Iteration count is a diagnostic. Time to a fixed accuracy with setup counted is the objective.**

- **MUST** include setup in any number that motivates a preconditioner change.
- **MUST** report two problem sizes. Grid independence, or its absence, cannot be seen at one.
- Watch for the hierarchy that barely coarsens: it converges in *fewer* iterations while each
  costs tens of fine-grid operator applications. Operator complexity catches it, iteration count
  does not.

---

## Step 6: Report status

Report exactly one.

**VERIFIED.** Freedom verdict established from what is graded; the transform's semantic argument
stated in one sentence; agreement measured across the input range as budget ratios; cost accounted
with setup at two sizes.

**PARTIALLY VERIFIED.** Implemented and agreeing where tested, but a step is unestablished. Name
it. Usually: one size only, a cap boundary not located, or cost quoted without setup.

**BLOCKED.** An essential step cannot be completed. State the step, the evidence, what was
attempted, and the smallest unresolved question.

Report structure: verdict and operator classification, the transform and its semantic argument,
budget ratios with the sizes named, cost against baseline, anything triaged, open questions,
status.

> **MUST NOT** report agreement that was not measured, or infer it from a residual norm, an
> iteration count, or a successful run.

Flag rather than guess, naming the smallest decision that would resolve each: whether the kernel is
frozen and where its cap binds; whether a transform preserves or approximates the reference; whether
a disagreement is your bug or a property of the reference; whether a structure holds at sizes you
could not run.

---

## Reference files

- **[reference/ladder.md](reference/ladder.md)** -- every preconditioner family, what it buys and
  costs, the state of the art behind each, measured iteration counts.
- **[reference/parallelizing.md](reference/parallelizing.md)** -- the frozen case: level scheduling
  versus reordering, measured level structures, the transforms that are always legal.
- **[reference/triage.md](reference/triage.md)** -- classifying a disagreement by its budget ratio,
  seven causes, and which are yours.
- **[reference/libraries.md](reference/libraries.md)** -- which library supplies which piece, and
  which to mine rather than call.
- **[reference/calibration.md](reference/calibration.md)** -- the verdict by kernel pattern, why
  almost everything comes back frozen, and the catalogue of things that look like optimizations and
  are not.
- **[measurements/README.md](measurements/README.md)** -- runnable validators behind every measured
  figure.
- **[evaluations.md](evaluations.md)** -- three scenarios for testing this skill, with the baseline
  failures they were built from.

## Hard rules

- Establish what is graded before choosing a preconditioner.
- A fixed trip count, a graded count, or a binding iteration cap freezes the preconditioner.
- Locate where a cap binds across the whole input range, not at one size.
- Test singularity with one matvec rather than trusting the documentation.
- Level scheduling preserves the mathematics; reordering does not. Know which one you did.
- Measure a level structure; never infer it from size or nonzero count.
- Matching a convergence rate is not matching an iterate.
- Setup counts, always. Report two problem sizes.
- Report error as a multiple of the grading budget, never as "close".
- Never loosen a tolerance to obtain a pass.
- Never reshape correct mathematics to hide a toolchain failure.
- Blocks follow the graph, not the index range.
- A strength threshold belongs to its strength measure and does not transfer.
- Assert pivots; incomplete-factorization breakdown is silent.
- Where a precision is named, it is the algorithm, not a knob.
- Link a library kernel that computes the same mathematics; mine a framework, do not submit its
  answer.
- Triage a disagreement to a named cause before fixing it.
- Neither silently reproduce nor silently repair a defect in the reference. Say which you did.
- Do not benchmark preconditioners a frozen kernel cannot use.
- Flag an open question rather than guessing it.
