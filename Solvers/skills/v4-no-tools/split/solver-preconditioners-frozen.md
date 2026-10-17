---
name: solver-preconditioners-frozen
description: "Makes a frozen preconditioner fast without changing what it computes: level scheduling versus reordering, measured level structures, synchronization-free triangular solves, always-legal transforms."
when: "ALWAYS read before you parallelize a solver kernel whose preconditioner, sweep order or iteration count must stay as in the reference (most solver kernels): it lists what made such kernels fast"
---

# Making a frozen preconditioner fast

A graded solver kernel is judged elementwise against one reference computation, not by iteration count. A preconditioner is **frozen** when the output is a fixed iterate (a fixed sweep, cycle or step count), when any graded output is a count, flag or history, or when a tolerance loop's iteration cap binds anywhere in the input range. The page `solver-preconditioners-verdict` decides which case you are in.

## If the preconditioner is frozen

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
substitution and must pass the check on the page `solver-preconditioners-verify` before it is trusted.

## Other pages of this skill

- `solver-preconditioners-verdict`: the first page to read for any solver kernel: whether the preconditioner may change
- `solver-preconditioners-ladder`: the preconditioner may change: which one to choose
- `solver-preconditioners-verify`: checking agreement and triaging a wrong answer
- `solver-preconditioners-samples`: NumPy code of the runs behind every table, with outcome and whether it was good or bad
