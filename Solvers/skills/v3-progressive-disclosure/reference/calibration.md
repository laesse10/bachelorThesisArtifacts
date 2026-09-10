# What the verdict returns in practice, and the failure catalogue

The freedom verdict is cheap to apply but its answer is counterintuitive, so this file gives the
distribution you should expect and the list of moves that look like optimizations and are not.

## Contents

- The verdict by kernel pattern
- Why almost everything comes back frozen
- Failure catalogue
- What is always legal

## The verdict by kernel pattern

Solver kernels recur in a small number of shapes. The verdict follows from the shape, so you can
usually predict it before reading a line of code and then confirm it.

| kernel pattern | what is typically graded | verdict |
|---|---|---|
| Krylov iteration with a fixed sweep count | the iterate | frozen: the preconditioner is in the recurrence |
| multigrid cycle with a fixed cycle count | the corrected solution | frozen: smoother, cycle type, damping weight |
| sparse triangular solve | the solution vector | frozen: the solve *is* the preconditioner apply |
| incomplete factorization | the factor, on its own pattern | frozen: the factorization *is* the setup |
| coloured or ordered relaxation sweep | the relaxed field | frozen: the ordering is the algorithm |
| hierarchy setup, aggregation or coarsening | level sizes, nonzero counts, operator complexity | frozen: graded on the hierarchy's shape |
| direct factorization with a supplied ordering | the solution | frozen: the ordering arrives as an input |
| Krylov basis construction | the basis and its projected coefficients | unpreconditioned by construction |
| dense factorization | the factors | no preconditioner involved |
| iterative refinement reporting a step count | the solution and the count | frozen by the count |
| implicit integrator reporting order or step history | the state and the histories | frozen by the histories |
| adaptive integrator reporting accept/reject tallies | the state and the counts | frozen by the counts |
| explicit ensemble integrator | the states | no preconditioner involved |
| Newton-Krylov run to a tolerance under an iteration cap | the converged state | open below the cap, frozen above it |

## Why almost everything comes back frozen

Count the rows: most freeze the preconditioner outright, a few have none to change, and the only
open case closes again as soon as its iteration cap starts binding.

That is not an accident of one corpus. It follows from what elementwise grading against a reference
means. A reference implementation picks one route to the answer, and a grader that compares
iterates rather than converged solutions is asking you to reproduce that route. Any benchmark
constructed this way produces the same distribution.

The practical consequence: **the work that scores is making the given preconditioner fast, not
choosing a better one.** Run the three questions before assuming otherwise, and expect the answer
to close the door more often than it opens it.

## Failure catalogue

1. Turning a Gauss-Seidel sweep into a Jacobi sweep to remove the dependence.
2. Reordering a sweep, by colouring, permutation or reversal, in a kernel graded on an iterate.
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
   comparing the resulting operator. Different aggregates give a different coarse operator, still a
   valid hierarchy but not this one.
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

## What is always legal

Reassociating a reduction, level scheduling on the reference's own ordering, batching an
orthogonalization into BLAS-2/3 where the measurement supports it, tiling, fusion, layout changes,
SIMD, threading a loop that carries no dependence, and linking a library whose kernel computes the
same mathematics.
