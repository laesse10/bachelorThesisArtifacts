# Making a frozen preconditioner fast

Read this when Step 1 established that the preconditioner cannot change. The job is to compute the
same thing faster.

## Contents

- The governing distinction
- Measure the level structure, never infer it
- Structured lexicographic orderings
- Unstructured matrices
- When level scheduling is worth nothing
- Transforms that are always available
- Verifying a transform preserves the mathematics

## The governing distinction

> **Level scheduling preserves the mathematics. Reordering does not.**

Level-set scheduling groups rows the analysis *proved* independent, so every row computes the
identical value. The only difference from a sequential sweep is reassociation inside the row
reduction. Measured at 2.5e-5 of the grading budget, four orders of margin.

Multicolouring changes which values a row reads. That is different mathematics. Measured at 8.7e+8,
nine orders outside.

Both are called "parallelizing Gauss-Seidel" in the literature. Only one is a parallelization of
*your* Gauss-Seidel.

The analysis phase belongs outside the timed region: one schedule amortizes over many solves, which
is how these methods are used.

## Measure the level structure, never infer it

It is a property of the *ordering*, not of the size or the nonzero count. Compute it with

```
level[i] = 1 + max(level[j] : j < i, A[i,j] != 0)
```

taken over the rows in order, then look at the level count and the distribution of rows per level.
Run `measurements/01_level_structure_stencil.py` for structured operators and
`measurements/04_level_structure_suitesparse.py` for unstructured ones.

## Structured lexicographic orderings

A 27-point operator on a `k³` grid, measured and confirmed on k = 16, 24, 32, 40, 48, 56, 64:

| grid | N | levels | avg rows/level | max rows/level |
|---|---|---|---|---|
| 16³ | 4,096 | 106 | 39 | 64 |
| 32³ | 32,768 | 218 | 150 | 256 |
| 48³ | 110,592 | 330 | 335 | 576 |
| 64³ | 262,144 | 442 | 593 | 1,024 |

Exactly `levels = 7k - 6`, `max rows/level = (k/2)²`, average about `k²/7`. At 192³ that is 1,338
levels holding ~5,300 rows each.

Those two numbers are what the schedule offers you, and they are the ones to reason from:

- **width**, the rows available to run concurrently, here about `k²/7` and growing only quadratically
  while the problem grows cubically;
- **depth**, the ordered levels, each one a synchronization point, here growing linearly in `k`.

Whether that is a good trade is a property of your target, not of the matrix. Compare the width
against the concurrency the device wants to keep busy, and the depth against the cost of a barrier
there. A width of a few thousand rows is generous for a handful of cores and thin for a device that
wants hundreds of thousands of threads in flight, but do not take that as settled: **measure it on
the hardware you are targeting.** The width and depth above transfer; a speed-up does not.

If the schedule turns out to be too thin, the remaining time in a preconditioned Krylov iteration
sits in the operator application, the dot products and the vector updates, which carry no
dependence at all.

## Unstructured matrices

Measured on four SPD operators drawn from applications, spanning two orders of magnitude in size:

| operator | N | nnz(L) | levels | avg | max | levels under 32 rows |
|---|---|---|---|---|---|---|
| thermal conduction, small | 82,654 | 328,556 | 971 | 85 | 14,203 | 471 |
| electromagnetics | 259,789 | 2,251,231 | 3,452 | 75 | 963 | 1,226 |
| thermal conduction, large | 1,228,045 | 4,904,179 | 1,239 | 991 | 207,967 | 114 |
| structural mechanics | 914,898 | 28,191,660 | 2,367 | 387 | 5,991 | 102 |

Savagely imbalanced: one level of 14,203 rows alongside 471 levels holding fewer than 32. A
per-level kernel launch spends its life on empty launches.

That is the canonical case for a **synchronization-free sparse triangular solve**: one thread or
warp per row, spinning on a per-row ready flag, no analysis phase and no global barriers.
It tolerates the imbalance instead of paying for it: a row runs as soon as its dependencies land,
so a level holding three rows costs three rows of work rather than a launch. CapelliniSpTRSV (Su et
al.) and AG-SpTRSV (Lu, Liu et al.) are the implementations to read. Both report large margins over
a level-scheduled solve, but those margins are theirs, on their hardware and their matrices; take
the algorithm from them and measure the margin yourself.

## When level scheduling is worth nothing

A level structure is only useful in a band.

- Many levels holding one row each is a serial chain wearing a schedule.
- Seven levels over half a million rows is embarrassingly parallel and needs no schedule at all.
- The interesting matrices sit between, with enough rows per level to fill the machine and enough
  levels that the ordering matters.

## Transforms that are always available

These preserve the mathematics regardless of what is frozen.

- **Reassociate reductions.** A parallel reduction necessarily reassociates, and the grading band
  covers it.
- **Batch an orthogonalization into BLAS-2/3.** A Gram-Schmidt sweep written as `2(j+1)` separate
  dot-and-axpy passes becomes one GEMV pair. This turns modified Gram-Schmidt into classical
  Gram-Schmidt, which is different arithmetic, but where the reorthogonalization is already done
  twice it holds: measured at 9.1e-5 of budget for m=50 and 3.0e-4 for m=100, drifting roughly
  linearly in the step count, so verify at your largest size. Usually the single biggest legal win
  in a Krylov or Lanczos kernel. Reproduce with `measurements/05_lanczos_cgs_and_lapack_qr.py`.
- **Call a library kernel that computes the same mathematics.** LAPACK's `dgeqrf` uses the same
  reflector sign convention as the textbook algorithm, so measured against a scalar reference at
  M=2000, N=64: R at 1.3e-4 of budget, Q matching to 5.9e-15. Watch the conventions: economy versus
  full Q, and whether the reference leaves zeros below the diagonal where LAPACK leaves the
  reflectors.
- **Tile, fuse, block, vectorize, change layout, thread a loop that carries no dependence.** All
  ordinary and all legal.

## Verifying a transform preserves the mathematics

State, in one sentence, why every row in your parallel region computes the same value it computed
sequentially.

- "The rows are independent because the analysis proved it" means you are scheduling. Legal.
- "The rows are independent because I coloured them" means you are reordering. The mathematics
  changed.

If the sentence cannot be written, the transform is a substitution and must be measured against the
grading band before it is trusted.
